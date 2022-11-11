LIV - Little Image Viewer
=================

[[[UNDER CONSTRUCTION]]]

A lightning-fast image viewer.

### BUILDING

Builds on Linux Mint (Ubuntu 22.04 base).  Will probably build on Windows and
Mac if you tweak it a bit.

Requires libsdl2-dev and libsdl2-image-dev (or equivalent).

    perl make.pl --jobs=7 release

Output will be in `out/rel`.

### USAGE

Requires a video driver that supports OpenGL 3.1 or higher.  Anything made since
2011 should work.

The main settings file is at `res/app/settings.ayu`.  It'll be moved to a more
convenient place eventually.

### ROADMAP

- [x] Dragging and zooming
- [x] Nice filtering
- [ ] View two pages side-by-side
- [ ] Folder support
- [ ] Better settings management
- [ ] Customizable mouse controls
- [ ] Support avif images (maybe switch to SAIL)
- [ ] Official Windows (mingw) support
- [ ] Off-thread image loading
- [ ] Simple animation

### KNOWN BUGS

- After entering fullscreen, the view will not be updated until it's redrawn
  one more time.
- Trying to load an unsupported image will cause an uncaught exception.

