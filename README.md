LIV - Little Image Viewer
=================

[[[UNDER CONSTRUCTION]]]

A lightning-fast image viewer.

### BUILDING

Builds with GCC on Linux Mint (Ubuntu 22.04 base).  Will probably build on
Windows and Mac if you tweak it a bit.

Requires the following packages (or equivalent)

- libsdl2-dev
- libsail-dev
- libsail-common-dev
- libsail-manip-dev

Run this command to build.  The program and all runtime files will be put into
`out/rel`.

```
perl make.pl release
```

### USAGE

Requires a video driver that supports OpenGL ES 3.0 or higher.  Any personal
computer made since 2012 should work.

There are lots of settings you can configure by editing the file `settings.ayu`
in the program directory (it may not exist until you run the program once).

There are comparatively few command-line arguments.  You can get info about them
with `--help`.

Note that by default, left-clicking in the window will trap the pointer (make it
invisible and constrain it to the window).  Left-click again or press Escape to
bring it back.

### FEATURES AND ROADMAP

[x] Dragging and zooming
[x] Nice filtering
[x] View two pages side-by-side
[x] Folder support
[x] List support
[x] Sorting
[x] Switch to OpenGL ES
[ ] Zoom around cursor
[ ] Customizable mouse controls
[x] Support avif images (maybe switch to SAIL)
[ ] Archive file support
[ ] Official Windows (mingw) support
[ ] Off-thread image loading
[ ] Text rendering
[ ] Simple animation

