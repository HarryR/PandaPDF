```
  ____                 _       ____  ____  _____
 |  _ \ __ _ _ __   __| | __ _|  _ \|  _ \|  ___|
 | |_) / _` | '_ \ / _` |/ _` | |_) | | | | |_
 |  __/ (_| | | | | (_| | (_| |  __/| |_| |  _|
 |_|   \__,_|_| |_|\__,_|\__,_|_|   |____/|_|
```

PandaPDF was developed as one of the components used to fully automate a the 
workflow of a digital magazine publishing company, the aim was to convert PDF
files into images so they can be distributed  without requiring full PDF
reading software on the client, this was necessary because delivering high
DPI print-quality PDF files to portable devices wasn't feasible.

This software allows for a PDF file to be converted into individual image files
at different resolutions which can be streamed to clients on an as-needed basis,
when using WEBP or lower quality JPEG the aim is for the total download size to
be lower than if the PDF were to be retrieved in-full.

*In almost all situations PandaPDF, when used with the Poppler and Ghostscript
backends, will be visually identical to PhotoShop even with very complex PDFs.*

Super-sampling is used in an attempt to and mitigate aliasing and float-rounding
problems that occur with highly complex graphical artwork, when this type of PDF
is rendered by PhotoShop there are no visual artefacts, but when rendered with
Acrobat, Ghostscript and Poppler small white gaps between elements appear. The
super-sampling technique renderes the PDF at a higher than required resolution
then down-samples it to produce the final images.

For best bandwidth efficiency *we highly recommend using WEBP output*, on average
this is half the size of JPEG output at 80% quality and results in significant
bandwidth savings.

PandaPDF software provides the following features:

 * High-Quality PDF rasterization
 * Thumbnail Generation
 * Content extraction
 * Interactive Region extraction
 * Manifest creator
 * Output to PNG, JPEG and WEBP

[![Build Status](https://drone.io/github.com/HarryR/PandaPDF/status.png)](https://drone.io/github.com/HarryR/PandaPDF/latest)

v3.0 Goals
----------

 * Multiple PDF rasterizers, Photoshop and Ghostscript
 * Remove usage of boost dependencies.
 * Remove GPLv2 code from project, open-source it
 * Decouple output format from rendering component.
 * Document problems with coordinate transforms etc.
 * Improved build system, builds on Win32, OSX and Linux
 * Fully modular architecture with cleaner code
 * Include 'poppler-data' package in distribution. (FAIL)
 * Regression tests. (FAIL)
 * Unit tests. (FAIL)


Version History
---------------

 * 3.0 - Photoshop & Ghostscript support
 * 2.x - Refactoring, Modular code and backends
 * 2.0 - Improved Quality, Cairo backend
 * 1.x - Multi-threaded, Text Extraction
 * 1.0 - Production run, cloud enabled
 * 0.x - C++/Poppler Prototype
 * -1  - Java Prototype


Commandline
-----------
```
Usage: pandapdf <-options> [<name,type@quality,size> ...]

Options:
 -debug : Enable debug logging
 -quiet : Log only warnings & errors
 -pdf <str> : PDF File
 -out <str> : Output Directory
 -box <str> : Which PDF box to use [crop,trim,media]
 -no-images : Disable Image Output
 -json-regions  : Enable JSON regions
 -json-words  : Enable JSON words
 -text-words  : Enable text words
 -opw <str> : Owner/Modify Password
 -upw <str> : User/Open Password
 -first <int> : First Page
 -last <int>  : Last Page
 -image-backend <str> : Which backend to use
 -supersample <float> : Supersample at N times resolution [default 1.5]

Image Backends, set with '-image-backend':
 - poppler-cairo (default)
 - poppler-splash
 - ghostscript

Examples:
  $ pandapdf -pdf test.pdf -out dir -json-regions large,jpeg@80,1500 medium,webp@90,720

Versions:
 PandaPDF   : 3.0.1 (c) 2009-2015 PixelMags Inc., H Roberts
 Poppler    : 0.38.0
 Cairo      : 1.12.18
 Freetype   : 0e0fdc5dc89e5079898c5da67b56f994c439fee1
 FontConfig : 2.11.94
 Pixman     : 0.32.8
 libJBIG    : 2.1
 libPNG     : 1.2.54
 libJPEG    : 8d
 libTIFF    : 4.0.6
 libWEBP    : 0.4.4
```

Example Usage:

    pandapdf -pdf test.pdf -out ~/test/ large,jpeg@80,800 medium,webp@80,800

This would extract images of all pages from `test.pdf` into the `~/test/` directory. The output files are named according to the format:

    page_%04d_%s.%s - e.g. page_0023_large.jpg

The different profiles specify the maximum dimension and output quality for
each image. 


Building
--------

The software can be built using system dependencies, but also includes a build 
system that compiles all required libraries from scratch to produce a mostly
static executable.

The `pandapdf` target produces a dynamic executable, if `make dependencies` is
run it will build, compile and then statically link all libraries into a single executable leaving only the system libc dynamic. If `make dependencies` is not
run it will dynamically link all required dependencies from the system.

```
apt-get install autoconf automake make g++ gcc wget git libtool pkg-config xz-utils libexpat1-dev ghostscript upx cmake libbzip2-dev
make dependencies
make
```

On OSX, Windows and Linux the executable is about 9mb.

After compression with UPX the executable is a little over 2mb.


Licensing
---------

The software is inexplicably tied to the Poppler library and includes one
source file (parseargs) from the project. As Poppler is released under the GPLv2
license this program must comply with the licensing restrictions, this means
that PandaPDF binaries cannot be distributed without releasing the source code
as it is considered a derivative work. As such the source code has been released
under the terms of the GPLv2 (see the `COPYING` file) so that it can be further
developed, improved and distributed.
