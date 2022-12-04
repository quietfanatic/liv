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
    -fstrict-aliasing -finline-small-functions
    -Wall -Wextra
    -fmax-errors=5 -fdiagnostics-color -fno-diagnostics-show-caret
));
my @link_opts = (qw(-lSDL2 -lSDL2_image));
#my @link_opts = (('-L' . rel2abs("$mingw_sdl2/lib")), qw(
#    -static-libgcc -static-libstdc++
#    -lmingw32 -lSDL2main -lSDL2
#));

$ENV{ASAN_OPTIONS} = 'new_delete_type_mismatch=0';
my %configs = (
    deb => {
        opts => [qw(-ggdb)],
    },
    opt => {
        opts => [qw(-Os -DNDEBUG -ggdb -flto)],
    },
    val => {
        opts => [qw(-Os -DNDEBUG -ggdb)],
    },
    san => {
        opts => [qw(-ggdb), '-fsanitize=address,undefined', '-fno-sanitize=enum'],
    },
    ana => {
        opts => [qw(-ggdb -fanalyzer)],
        fork => 0,  # Absolutely thrashes RAM
    },
    pro => {
        opts => [qw(-Og -DNDEBUG -flto -pg)],
    },
    rel => {
        opts => [qw(-Os -DNDEBUG -DTAP_DISABLE_TESTS -flto)],
        strip => 1,
    },
);

##### SOURCES

my $program = 'liv';

my @sources = (qw(
    app/app.cpp
    app/app-commands.cpp
    app/book.cpp
    app/main.cpp
    app/page.cpp
    app/settings.cpp
    base/ayu/describe-standard.cpp
    base/ayu/src/accessors.cpp
    base/ayu/src/common.cpp
    base/ayu/src/compat.cpp
    base/ayu/src/document.cpp
    base/ayu/src/dynamic.cpp
    base/ayu/src/location.cpp
    base/ayu/src/parse.cpp
    base/ayu/src/print.cpp
    base/ayu/src/reference.cpp
    base/ayu/src/resource.cpp
    base/ayu/src/resource-scheme.cpp
    base/ayu/src/scan.cpp
    base/ayu/src/serialize.cpp
    base/ayu/src/traversal.cpp
    base/ayu/src/tree.cpp
    base/ayu/src/type.cpp
    base/control/command.cpp
    base/control/command-builtins.cpp
    base/control/input.cpp
    base/geo/mat.cpp
    base/glow/common.cpp
    base/glow/file-texture.cpp
    base/glow/gl.cpp
    base/glow/image.cpp
    base/glow/objects.cpp
    base/glow/program.cpp
    base/glow/test-environment.cpp
    base/glow/texture-program.cpp
    base/iri/iri.cpp
    base/uni/common.cpp
    base/uni/text.cpp
    base/wind/passive_loop.cpp
    base/wind/window.cpp
),
    [qw(base/tap/tap.cpp -DTAP_SELF_TEST)],
);

my @resources = (qw(
    app/page.ayu
    app/settings-default.ayu
    app/settings-template.ayu
    base/ayu/src/test/*
    base/glow/test/*
    base/glow/texture-program.ayu
), ["app/settings-template.ayu", "../settings-template.ayu"]
);


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
            run @$compiler, '-S', @{$_[1]},
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
