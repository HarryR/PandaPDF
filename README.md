```
  ____                 _       ____  ____  _____
 |  _ \ __ _ _ __   __| | __ _|  _ \|  _ \|  ___|
 | |_) / _` | '_ \ / _` |/ _` | |_) | | | | |_
 |  __/ (_| | | | | (_| | (_| |  __/| |_| |  _|
 |_|   \__,_|_| |_|\__,_|\__,_|_|   |____/|_|
```

The PandaPDF software provides the following features:

 * High-Quality PDF rasterization
 * Thumbnail Generation
 * Content extraction
 * Interactive Region extraction
 * Manifest creator


v3.0 Goals
----------

 * Remove usage of boost dependencies.
 * Remove all GPLv2 licensed code from project.
 * Decouple output format from rendering component.
 * Include 'poppler-data' package in distribution.
 * Document problems with coordinate transforms etc.
 * Regression tests.
 * Unit tests.


Version History
---------------

 * 3.x - Third Iteration
 * 2.1 - Bug Fixes
 * 2.0 - Minor Improvements
 * 1.9 - Feature Stabalization
 * 1.x - Second Iteration


Commandline
-----------
```
Usage: pandapdf [options] <profile=max_dimension ...>
  -f <int>             : Start extraction from this page
  -l <int>             : Finish extraction at this page
  -driver <string>     : Rendering driver (splash or cairo)
  -pdf <string>        : Input PDF File
  -out <string>        : Output Directory
  -imgFormat <string>  : Image output format (jpeg or png) [default=jpeg]
  -txtFormat <string>  : Text output format (plain,json or all) [default=plain]
  -page <int>          : Only extract a specific page
  -opw <string>        : owner password (for encrypted files)
  -upw <string>        : user password (for encrypted files)
  -quality <int>       : JPEG output quality (1-99) [0 = default]
  -Xoversample <fp>    : Master rendering oversample (see manual)
  -noimg               : Disable image output
  -text                : Enable wordlist output
  -json                : Enable JSON wordlist output
  -regions             : Enable region extraction
  -quiet               : Disable progress messages
  -bleed               : Use bleedbox instead of trimbox/cropbox
  -manifest            : Output JSON manifest file
  -debug <int>         : Amount of debug info, 0 = everything, 4 = nothing

Example Usage:
  pandapdf -pdf test.pdf -out ~/test/ large=1000 medium=500 small=100

Example Output Files:
  ~/test/page_0001.jpg
  ~/test/page_0002.jpg
  ~/test/page_0003.jpg
  ~/test/page_...

```

Example Usage:

    pandapdf -pdf test.pdf -out ~/test/ large=1000 medium=500 small=100

This would extract images of all pages from `test.pdf` into the `~/test/` directory. The output files are named according to the format:

    page_%04d_%s.%s - e.g. page_0023_large.jpg

The different profiles specify the maximum dimension for each image.



Licensing
---------

The software is inexplicably tied to the Poppler library and includes one
source file (parseargs) from the project. As Poppler is released under the GPLv2
license this program must comply with the licensing restrictions, this means
that PandaPDF binaries cannot be distributed outside of PixelMags without
releasing the source code as it is considered a derivative work.


## Building

```
apt-get install autoconf automake make g++ gcc wget git libtool pkg-config xz-utils libexpat1-dev ghostscript upx
```
