#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/src/external/make-pl'};
use MakePl;
use MakePl::C;
use File::Copy;

##### COMMAND LINE CONFIGURATION

my %compilers = (
    'cpp' => [qw(g++-14 -std=c++20 -fno-threadsafe-statics -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=4)],
    'c' => ['gcc-14']
);
my @linker = 'g++-14';

my @includes = ('/usr/include/sail');
my @compile_opts = (map("-I$_", @includes), qw(
    -msse2 -mfpmath=sse
    -fstrict-aliasing -fstack-protector
    -Wall -Wextra -Wno-unused-value
    -fmax-errors=10 -fdiagnostics-color -fno-diagnostics-show-caret
));
my @link_opts = (qw(-lSDL2 -lsail -lsail-common -lsail-manip));

 # Dead code elimination actually makes compilation slightly faster.
my @O0_opts = (qw(-fdce));

 # MFW I discovered parallel LTO
my @O3_opts = (qw(-O3 -flimit-function-alignment -flto=7));

my @no_tests = (qw(-DTAP_DISABLE_TESTS -DTAP_REMOVE_TESTS));

$ENV{ASAN_OPTIONS} = 'new_delete_type_mismatch=0';
my %configs = (
    deb => {
        opts => [qw(-ggdb), @O0_opts],
    },
    opt => {
        opts => [qw(-DNDEBUG -ggdb), @O3_opts],
    },
    dog => {
        opts => [qw(-fno-rtti -ggdb), @no_tests, @O3_opts],
    },
    rel => {
        opts => [qw(-DNDEBUG -fno-rtti -s), @no_tests, @O3_opts],
    },
    sss => {
        opts => [qw(-DNDEBUG -fno-rtti), @no_tests, @O3_opts],
    },
#    opt32 => {
#        opts => [qw(-m32 -fno-pie -DNDEBUG -ggdb), @O3_opts],
#    },
#    san => {
#        opts => [qw(-ggdb), @O0_opts, '-fsanitize=address,undefined', '-fno-sanitize=enum'],
#    },
#    pro => {
#        opts => [qw(-Og -DNDEBUG -flto -pg)],
#    },
#    ana => {
#        opts => [qw(-ggdb -fanalyzer)],
#        fork => 0,  # Absolutely thrashes RAM
#    },
);

##### SOURCES

my $program = 'liv';

my @sources = (qw(
    liv/app.cpp
    liv/commands.cpp
    liv/book-source.cpp
    liv/book-state.cpp
    liv/book-view.cpp
    liv/book.cpp
    liv/format.cpp
    liv/list.cpp
    liv/main.cpp
    liv/mark.cpp
    liv/page-block.cpp
    liv/page.cpp
    liv/settings.cpp
    liv/sort.cpp
    dirt/ayu/common.cpp
    dirt/ayu/data/parse.cpp
    dirt/ayu/data/print.cpp
    dirt/ayu/data/tree.cpp
    dirt/ayu/reflection/access.cpp
    dirt/ayu/reflection/anyptr.cpp
    dirt/ayu/reflection/anyref.cpp
    dirt/ayu/reflection/anyval.cpp
    dirt/ayu/reflection/describe-builtin.cpp
    dirt/ayu/reflection/describe-standard.cpp
    dirt/ayu/reflection/type.cpp
    dirt/ayu/resources/document.cpp
    dirt/ayu/resources/resource.cpp
    dirt/ayu/resources/scheme.cpp
    dirt/ayu/traversal/compound.cpp
    dirt/ayu/traversal/from-tree.cpp
    dirt/ayu/traversal/route.cpp
    dirt/ayu/traversal/scan.cpp
    dirt/ayu/traversal/test.cpp
    dirt/ayu/traversal/to-tree.cpp
    dirt/ayu/traversal/traversal.cpp
    dirt/control/command.cpp
    dirt/control/command-builtins.cpp
    dirt/control/input.cpp
    dirt/geo/floating.t.cpp
    dirt/geo/mat.t.cpp
    dirt/geo/vec.t.cpp
    dirt/glow/colors.cpp
    dirt/glow/common.cpp
    dirt/glow/file-texture.cpp
    dirt/glow/gl.cpp
    dirt/glow/image-texture.cpp
    dirt/glow/image-transform.cpp
    dirt/glow/image.cpp
    dirt/glow/load-image.cpp
    dirt/glow/program.cpp
    dirt/glow/resource-image.cpp
    dirt/glow/resource-texture.cpp
    dirt/glow/test-environment.cpp
    dirt/glow/texture-program.cpp
    dirt/glow/texture.cpp
    dirt/iri/iri.cpp
    dirt/iri/path.cpp
    dirt/uni/arrays.t.cpp
    dirt/uni/assertions.cpp
    dirt/uni/errors.cpp
    dirt/uni/io.cpp
    dirt/uni/lilac-global-override.cpp
    dirt/uni/lilac.cpp
    dirt/uni/shell.cpp
    dirt/uni/text.cpp
    dirt/uni/utf.cpp
    dirt/whereami/whereami.c
    dirt/wind/passive_loop.cpp
    dirt/wind/window.cpp
),
    [qw(dirt/tap/tap.cpp -DTAP_SELF_TEST)],
);

my @resources = (qw(
    liv/page.ayu
    liv/settings-default.ayu
    liv/settings-template.ayu
),
    ["liv/help/commands.md" => "help/commands.md"],
    ["liv/help/formats.md" => "help/formats.md"],
    ["../LICENSE" => "LICENSE"],
    ["../README.md" => "README.md"],
);
my @test_resources = (qw(
    liv/test/*
    dirt/ayu/test/*.ayu
    dirt/ayu/test/*.json
    dirt/glow/test/*
    dirt/glow/texture-program.ayu
));

##### RULES

for my $cfg (keys %configs) {
     # Compile sources
    my @objects;
    for (@sources) {
        my $src = $_; # Don't alias
        my @opts = (@compile_opts, @{$configs{$cfg}{opts}});
        if (ref $_ eq 'ARRAY') {
            my @src_opts = (@$src);
            $src = shift @src_opts;
            push @opts, @src_opts;
        }

        $src =~ /(.*)\.([^\/.]+)$/ or die "No file extension in $src";
        my ($mod, $ext) = ($1, $2);
        my $compiler = $compilers{$ext} // die "Unrecognized source file extension in $src";

        step "tmp/$cfg/$mod.o", "src/$src", sub {
            run @$compiler, '-c', @{$_[1]},
                @opts,
                '-o', $_[0][0];
        }, {fork => $configs{$cfg}{fork} // 1, mkdir => 1};
        step "tmp/$cfg/$mod.s", "src/$mod.cpp", sub {
            run @$compiler, '-S', '-masm=intel', @{$_[1]},
                grep($_ ne '-ggdb' && $_ !~ /^-flto/, @opts),
                '-o', $_[0][0];
        }, {fork => $configs{$cfg}{fork} // 1, mkdir => 1};

        push @objects, "tmp/$cfg/$mod.o";
    }

     # Link program
    my $out_program = "out/$cfg/$program";
    step $out_program, [@objects], sub {
        run @linker, @objects,
            @link_opts, @{$configs{$cfg}{opts} // []},
            '-o', $out_program;
        if ($cfg eq 'rel') {
            print "Final program size: ", -s $out_program, "\n";
        }
    }, {fork => 1, mkdir => 1};

     # Copy resources
    my $testing_disabled = grep $_ eq '-DTAP_DISABLE_TESTS', @{$configs{$cfg}{opts}};
    my @res = @resources;
    push @res, @test_resources unless $testing_disabled;
    my @out_resources;
    for (@res) {
        my $name = ref $_ eq 'ARRAY' ? $_->[0] : $_;
        my @files = glob "src/$name";
        @files or die "No resources matched $name\n";
        for my $from (@files) {
            my $to;
            if (ref $_ eq 'ARRAY') {
                $to = "out/$cfg/$_->[1]";
            }
            else {
                $to = $from;
                $to =~ s[^src/][out/$cfg/res/];
            }
            push @out_resources, $to;
            step $to, $from, sub {
                copy($from, $to) or die "Copy failed: $!";
            }, {fork => 1, mkdir => 1};
        }
    }

     # Misc phonies
    phony "out/$cfg/build", [$out_program, @out_resources];
    phony "out/$cfg/test", "out/$cfg/build", sub {
        run "$out_program --test | perl -pe \"s/\\r//\" | prove -f -e \"$out_program --test\" -";
    };
}

phony 'clean', [], sub {
    require File::Path;
    File::Path::remove_tree('tmp');
    File::Path::remove_tree('out');
};

suggest 'out/deb/test', 'Build in debug mode and run tests';
suggest 'out/dog/build', 'Build in dogfood mode (optimized but with debug assertions)';
suggest 'out/rel/build', 'Build in release mode';
suggest 'clean', 'Delete all temporary files and output products';
defaults 'out/deb/test';

make;
