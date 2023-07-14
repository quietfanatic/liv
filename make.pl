#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/src/external/make-pl'};
use MakePl;
use MakePl::C;
use File::Copy;

##### COMMAND LINE CONFIGURATION

my %compilers = (
    'cpp' => [qw(g++ -std=c++20)],
    'c' => ['gcc']
);
my @linker = 'g++';

my @includes = ();
my @compile_opts = (map("-I$_", @includes), qw(
    -msse2 -mfpmath=sse
    -fstrict-aliasing -finline-small-functions -fdce
    -Wall -Wextra -Wno-terminate
    -fmax-errors=10 -fdiagnostics-color -fno-diagnostics-show-caret
    -fconcepts-diagnostics-depth=4
));
my @link_opts = (qw(-lSDL2 -lSDL2_image));
#my @link_opts = (('-L' . rel2abs("$mingw_sdl2/lib")), qw(
#    -static-libgcc -static-libstdc++
#    -lmingw32 -lSDL2main -lSDL2
#));

my @optimize_opts = (qw(-O3 -fweb -frename-registers -flto));

$ENV{ASAN_OPTIONS} = 'new_delete_type_mismatch=0';
my %configs = (
    deb => {
        opts => [qw(-ggdb)],
    },
    opt => {
        opts => [qw(-DNDEBUG -ggdb), @optimize_opts],
    },
    dog => {
        opts => [qw(-DTAP_DISABLE_TESTS -ggdb), @optimize_opts],
    },
    rel => {
        opts => [qw(-DNDEBUG -DTAP_DISABLE_TESTS), @optimize_opts],
        strip => 1,
    },
    opt32 => {
        opts => [qw(-m32 -fno-pie -DNDEBUG -ggdb), @optimize_opts],
    },
    val => {
        opts => [qw(-DNDEBUG -ggdb), @optimize_opts, qw(-fno-inline-functions -fno-inline-small-functions)],
    },
    san => {
        opts => [qw(-ggdb), '-fsanitize=address,undefined', '-fno-sanitize=enum'],
    },
    pro => {
        opts => [qw(-Og -DNDEBUG -flto -pg)],
    },
    ana => {
        opts => [qw(-ggdb -fanalyzer)],
        fork => 0,  # Absolutely thrashes RAM
    },
);

##### SOURCES

my $program = 'liv';

my @sources = (qw(
    app/app.cpp
    app/app-commands.cpp
    app/book.cpp
    app/files.cpp
    app/layout.cpp
    app/main.cpp
    app/page-block.cpp
    app/page.cpp
    app/settings.cpp
    dirt/ayu/describe-standard.cpp
    dirt/ayu/src/accessors.cpp
    dirt/ayu/src/common.cpp
    dirt/ayu/src/document.cpp
    dirt/ayu/src/describe-builtin.cpp
    dirt/ayu/src/dynamic.cpp
    dirt/ayu/src/location.cpp
    dirt/ayu/src/parse.cpp
    dirt/ayu/src/pointer.cpp
    dirt/ayu/src/print.cpp
    dirt/ayu/src/reference.cpp
    dirt/ayu/src/resource.cpp
    dirt/ayu/src/resource-scheme.cpp
    dirt/ayu/src/scan.cpp
    dirt/ayu/src/serialize.cpp
    dirt/ayu/src/tree.cpp
    dirt/ayu/src/type.cpp
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
    dirt/glow/image.cpp
    dirt/glow/objects.cpp
    dirt/glow/program.cpp
    dirt/glow/test-environment.cpp
    dirt/glow/texture-program.cpp
    dirt/iri/iri.cpp
    dirt/uni/arrays.t.cpp
    dirt/uni/assertions.cpp
    dirt/uni/text.cpp
    dirt/uni/utf.cpp
    dirt/wind/passive_loop.cpp
    dirt/wind/window.cpp
),
    [qw(dirt/tap/tap.cpp -DTAP_SELF_TEST)],
);

my @resources = (qw(
    app/page.ayu
    app/settings-default.ayu
    app/settings-template.ayu
    dirt/ayu/src/test/*
    dirt/glow/test/*
    dirt/glow/texture-program.ayu
));

##### MISC

sub ensure_path {
    if ($_[0] =~ /^(.*)\//) {
        -d $1 or do {
            require File::Path;
            File::Path::make_path($1);
        }
    }
}

##### RULES

for my $cfg (keys %configs) {
     # Compile sources
    my @objects;
    for my $src (@sources) {
        my @opts = (@compile_opts, @{$configs{$cfg}{opts}});
        if (ref $src eq 'ARRAY') {
            my @src_opts = @$src;
            $src = shift @src_opts;
            push @opts, @src_opts;
        }

        $src =~ /(.*)\.([^\/.]+)$/ or die "No file extension in $src";
        my ($mod, $ext) = ($1, $2);
        my $compiler = $compilers{$ext} // die "Unrecognized source file extension in $src";

        rule "tmp/$cfg/$mod.o", "src/$src", sub {
            ensure_path($_[0][0]);
            run @$compiler, '-c', @{$_[1]},
                @opts,
                '-o', $_[0][0];
        }, {fork => $configs{$cfg}{fork} // 1};
        rule "tmp/$cfg/$mod.s", "src/$mod.cpp", sub {
            ensure_path($_[0][0]);
            run @$compiler, '-S', '-masm=intel', @{$_[1]},
                grep($_ ne '-ggdb' && $_ ne '-flto', @opts),
                '-o', $_[0][0];
        }, {fork => $configs{$cfg}{fork} // 1};

        push @objects, "tmp/$cfg/$mod.o";
    }

     # Link program
    my $tmp_program = "tmp/$cfg/$program";
    my $out_program = "out/$cfg/$program";
    my $link_target;
    if ($configs{$cfg}{strip} // 0) {
        $link_target = $tmp_program;
        rule $out_program, $tmp_program, sub {
            run 'strip', $tmp_program, '-o', $out_program;
        }, {fork => 1};
    }
    else {
        $link_target = $out_program;
    }
    rule $link_target, [@objects], sub {
        ensure_path $link_target;
        run @linker, @objects,
            @link_opts, @{$configs{$cfg}{opts} // []},
            '-o', $link_target;
    }, {fork => 1};

     # Copy resources
    my @out_resources;
    for (@resources) {
        my $name = ref $_ eq 'ARRAY' ? $_->[0] : $_;
        my @files = glob "src/$name";
        @files or die "No resources matched $name\n";
        for my $from (@files) {
            my $to = ref $_ eq 'ARRAY' ? $_->[1] : $from;
            $to =~ s[^src/][out/$cfg/res/];
            push @out_resources, $to;
            rule $to, $from, sub {
                ensure_path($to);
                copy($from, $to) or die "Copy failed: $!";
            }, {fork => 1};
        }
    }

     # Misc phonies
    phony "out/$cfg/build", [$out_program, @out_resources];
    phony "out/$cfg/test", "out/$cfg/build", sub {
        run "$out_program --test | perl -pe \"s/\\r//\" | prove -e \"$out_program --test\" -";
    };
}

phony 'debug', 'out/deb/build';
phony 'release', 'out/rel/build';
phony 'test', 'out/deb/test';
defaults 'test';

phony 'clean', [], sub {
    require File::Path;
    File::Path::remove_tree('tmp');
    File::Path::remove_tree('out');
};

make;
