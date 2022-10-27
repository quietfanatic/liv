#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/src/external/make-pl'};
use MakePl;
use MakePl::C;
use File::Copy;

##### COMMAND FLAGS

my @includes = ();
 # -Wno-strict-aliasing is potentially dangerous but everything seems to work fine...for now.
my @compile_opts = (map("-I$_", @includes), qw(
    -msse2 -mfpmath=sse
    -Wall -Wextra -Wno-format-security -Wno-unused-function -Wno-strict-aliasing
    -fmax-errors=5 -fdiagnostics-color -fno-diagnostics-show-caret
));
my @link_opts = qw(
    -lSDL2 -lSDL2_image
);
#my @link_opts = (('-L' . rel2abs("$mingw_sdl2/lib")), qw(
#    -static-libgcc -static-libstdc++
#    -lmingw32 -lSDL2main -lSDL2
#));

my %configs = (
    deb => {
        compile_opts => [qw(-ggdb)],
        strip => 0,
    },
    opt => {
        compile_opts => [qw(-Os -DNDEBUG)],
        strip => 1,
    },
    rel => {
        compile_opts => [qw(-Os -DNDEBUG -DTAP_DISABLE_TESTS)],
        strip => 1,
    },
);

##### STANDARD RULE TYPES

sub ensure_path {
    if ($_[0] =~ /^(.*)\//) {
        -d $1 or do {
            require File::Path;
            File::Path::make_path($1);
        }
    }
}

sub c_rule {
    my ($to, $from, @opts) = @_;
    rule $to, $from, sub {
        ensure_path($_[0][0]);
        run qw(gcc -c),
            @{$_[1]}, @opts,
            '-o', $_[0][0];
    }, {fork => 1};
}

sub cpp_rule {
    my ($to, $from, @opts) = @_;
    rule $to, $from, sub {
        ensure_path($_[0][0]);
        run qw(g++ -std=c++17 -c),
            @{$_[1]}, @opts,
            '-o', $_[0][0];
    }, {fork => 1};
}

sub link_rule {
    my ($to, $from, @opts) = @_;
    rule $to, $from, sub {
        ensure_path($_[0][0]);
        run qw(g++),
            @{$_[1]}, @opts,
            '-o', $_[0][0];
    };
}

sub strip_rule {
    my ($to, $from, @opts) = @_;
    rule $to, $from, sub {
        ensure_path($_[0][0]);
        run qw(strip),
            @{$_[1]}, @opts,
            '-o', $_[0][0];
    };
}

sub copy_rule {
    my ($to, $from) = @_;
    rule $to, $from, sub {
        ensure_path($_[0][0]);
        copy($from, $to) or die "Copy failed: $!";
    }, {fork => 1};
}

##### SOURCES

my $program = 'iv';

my @modules = qw(
    base/geo/mat
    base/hacc/accessors
    base/hacc/common
    base/hacc/compat
    base/hacc/document
    base/hacc/dynamic
    base/hacc/haccable-standard
    base/hacc/parse
    base/hacc/path
    base/hacc/print
    base/hacc/reference
    base/hacc/registry
    base/hacc/resource
    base/hacc/serialize
    base/hacc/tree
    base/hacc/type
    base/tap/tap
    base/glow/common
    base/glow/gl
    base/glow/objects
    base/glow/program
    base/uni/common
    base/wind/loop
    base/wind/window
    image-texture
    main
);
my %opts = (
    'base/tap/tap' => [qw(-DTAP_SELF_TEST)],
);

my @resources = qw(
    base/hacc/test/*
    base/glow/test/*
    base/glow/texture-program.hacc
);
my %dlls = ();
#my %dlls = (
#    'SDL2.dll' => "$mingw_sdl2/bin/SDL2.dll"
#);


##### RULES

for my $cfg (keys %configs) {
    my @objects;
    for my $mod (@modules) {
        push @objects, "tmp/$cfg/$mod.o";
        if (-e "src/$mod.cpp") {
            cpp_rule(
                "tmp/$cfg/$mod.o",
                "src/$mod.cpp",
                @{$opts{$mod} // []},
                @compile_opts,
                @{$configs{$cfg}{compile_opts}}
            );
        }
        elsif (-e "src/$mod.c") {
            c_rule(
                "tmp/$cfg/$mod.o",
                "src/$mod.c",
                @{$opts{$mod} // []},
                @compile_opts,
                @{$configs{$cfg}{compile_opts}}
            );
        }
        else {
            die ("Can't find .c or .cpp file for program module $mod");
        }
    }

    my @out_resources;
    for (@resources) {
        my @files = glob "src/$_";
        @files or die "No resources matched $_\n";
        for my $from (glob "src/$_") {
            (my $to = $from) =~ s[^src/][out/$cfg/res/];
            copy_rule($to, $from);
            push @out_resources, $to;
        }
    }

    my $out_program = "out/$cfg/$program";
    if ($configs{$cfg}{strip}) {
        my $tmp_program = "tmp/$cfg/$program";
        link_rule($tmp_program, [@objects], @link_opts);
        strip_rule($out_program, $tmp_program);
    }
    else {
        link_rule($out_program, [@objects], @link_opts);
    }
    my @out_dlls;
    for (keys %dlls) {
        my $to = "out/$cfg/$_";
        copy_rule($to, $dlls{$_});
        push @out_dlls, $to;
    }
    phony "out/$cfg/build", [$out_program, @out_resources, @out_dlls];
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
