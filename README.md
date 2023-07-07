LIV - Little Image Viewer
=================

[[[UNDER CONSTRUCTION]]]

A lightning-fast image viewer.

### BUILDING

Builds on Linux Mint (Ubuntu 22.04 base).  Will probably build on Windows and
Mac if you tweak it a bit.

Requires libsdl2-dev and libsdl2-image-dev (or equivalent).

    perl make.pl --jobs=7 release

The program and all runtime files will be put into `out/rel`.

### USAGE

Requires a video driver that supports OpenGL 3.1 or higher.  Anything made since
2011 should work.

You can configure the app by editing the file `settings.ayu` in the program
directory (it may not exist until you run the program once).

### ROADMAP

- [x] Dragging and zooming
- [x] Nice filtering
- [x] View two pages side-by-side
- [x] Folder support
- [x] Better settings management
- [ ] Customizable mouse controls
- [ ] Support avif images (maybe switch to SAIL)
- [ ] Official Windows (mingw) support
- [ ] Off-thread image loading
- [ ] Simple animation

