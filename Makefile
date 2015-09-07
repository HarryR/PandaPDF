# For building natively on Windows we need pkg-config
# see:
# http://www.gaia-gis.it/spatialite-3.0.0-BETA/mingw_how_to.html#pkg-config

# Automatically detect windows builds under MSYS+MINGW
OS=$(shell uname -o)
ifeq ($(OS), Msys)
WINBUILD=1
endif

# Allow cross-compiling with `make CROSS=win32`
ifeq ($(CROSS),win32)
WINBUILD=1
# Dependencies for Win32 are available at: http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/
# However they are majorly out of date, so we must build them
CROSS_HOST=i686-w64-mingw32
CC      = $(CROSS_HOST)-gcc
CXX     = $(CROSS_HOST)-g++
EXE     = .exe
else
CROSS_HOST =
CC      = gcc
CXX     = g++
EXE     =
endif

TARGETS=pandapdf$(EXE) pandapdfstatic$(EXE)

all: $(TARGETS)

include Makefile.deps

PANDAPDF_VERSION=3.0.beta

define VERSION_HEADER_FILE
#ifndef VERSION_H_
#define VERSION_H_

#define PANDAPDF_VERSION "$(PANDAPDF_VERSION)"
#define LIB_VER_POPPLER "$(POPPLER_VERSION)"
#define LIB_VER_JBIG "$(JBIG_VER)"
#define LIB_VER_CAIRO "$(CAIRO_VERSION)"
#define LIB_VER_FREETYPE "$(FT_REV)"
#define LIB_VER_FONTCONFIG "$(FC_VERSION)"
#define LIB_VER_PIXMAN "$(PIXMAN_VERSION)"
#define LIB_VER_PNG "$(PNG_VER)"
#define LIB_VER_JPEG "$(JPEG_VER)"
#define LIB_VER_TIFF "$(TIFF_VER)"
#define LIB_VER_WEBP "$(WEBP_VER)"

#endif

endef
export VERSION_HEADER_FILE

STATIC_LIBS = $(POPPLER_LIB) $(POPPLER_LIBS) $(CAIRO_LIB) $(PIXMAN_LIB) \
			  $(LCMS_LIB) $(PNG_LIB) $(JPEG_LIB) $(TIFF_LIB) \
			  $(JBIG_LIB) $(ZLIB_LIB) $(WEBP_LIB) $(OJPG_LIB)

ifndef WINBUILD
STATIC_LIBS += $(FC_LIB)
endif
STATIC_LIBS += $(FT_LIB) 

CFLAGS    = -Wall $(CPPFLAGS)
CPPFLAGS  = -I$(POPPLER_DIR) -I$(POPPLER_DIR)/poppler/ $(CAIRO_CFLAGS) -I$(FT_DIR)/include -I$(DEP_ROOT)/include -I$(DEP_ROOT)/include/libpng12/
CXXFLAGS  = $(CFLAGS) $(CPPFLAGS)
LDLIBS    = 
LDFLAGS   = -static-libgcc

ifdef RELEASE
CFLAGS += -O3 -flto -ffunction-sections -fdata-sections -static-libgcc -fuse-linker-plugin
LDLIBS += -Wl,--gc-sections -Wl,--strip-all
LDFLAGS += -flto
else
CFLAGS += -O0 -ggdb
endif

obj_pandapdf=main.o internal.o png-bitmap.o argparser.o pdfinfo.o \
			 content-output.o image-output.o render-driver.o \
			 poppler-backends.o ghostscript-backend.o webp-output.o \
			 logger.o utils.o json.o json-overload.o

ifdef WINBUILD
# On Win32 we require disphelper for COM+ access to Photoshop
# And asprintf because the Windows libc sucks hard
obj_pandapdf += disphelper.o asprintf.o photoshop-backend.o
LDLIBS	+= -lgdi32 -lole32 -loleaut32 -luuid
else
# expat is required when fontconfig is used
LDLIBS  += -lexpat -lpthread
endif

# ------------------------------------------------------------------------------

ifdef RELEASE
release: clean $(TARGETS)
	strip -R .note -R .comment $(TARGETS)
	upx -9 $(TARGETS)
	rm -rf ../v$(PANDAPDF_VERSION)/
	mkdir ../v$(PANDAPDF_VERSION)/
	cp $(TARGETS) ../v$(PANDAPDF_VERSION)/
	cd ../ && tar --create --file=v$(PANDAPDF_VERSION).tar.gz -z v$(PANDAPDF_VERSION)
endif

clean:
	-rm -f $(obj_pandapdf)
	-rm -f $(TARGETS)
	-rm -f version.h

cleanall: clean clean-dependencies

%.o: %.cc
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

pandapdf$(EXE): version.h $(obj_pandapdf)
	@echo "[LNK] $@"
	@$(CXX) $(LDFLAGS) -o $@ $(obj_pandapdf) $(STATIC_LIBS) $(LDLIBS)

pandapdfstatic$(EXE): version.h $(obj_pandapdf)
	@echo "[LNK] $@"
	@$(CXX) $(LDFLAGS) -static -o $@ $(obj_pandapdf) $(STATIC_LIBS) $(LDLIBS)

version.h:
	@echo "$$VERSION_HEADER_FILE" > $@