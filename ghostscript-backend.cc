#pragma implementation "ghostscript-backend.hh"
#include "ghostscript-backend.hh"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

#include "asprintf.h"
#include "utils.hh"
#include "png-bitmap.hh"

#include <png.h>
#include <cstdlib>
#include <cstring>
#include <cassert>


class GhostscriptPageRenderer
: public PageRenderer {
public:
    GhostscriptPageRenderer( RenderContext *ctx )
    : PageRenderer(ctx)
    { }

    virtual bool doesRegions() { return false; }
    virtual bool doesImages() { return true; }
    virtual bool doesWords() { return false; }

    virtual bool isOk() {
        RenderContext *x = this->ctx();
        if( ! x->use_trimbox && ! x->use_cropbox
         && ! x->use_mediabox && ! x->use_bleedbox ) {
            ctx()->log->warn("Don't know which box to use");
            return false;
        }

        if( x->use_mediabox || x->use_bleedbox ) {
            ctx()->log->warn("Cannot use mediabox or bleedbox with ghostscript output!");
            return false;
        }

        // TODO: validate if PDF file is valid
        // how can we do this with ghostscript
        return true;
    }

    virtual ~GhostscriptPageRenderer() {
        // Nothing to free-up with ghostscript
    }

    virtual bool
    run() {
        RenderContext *x = this->ctx();
        const char usetrimbox[] = "Trim";
        const char usecropbox[] = "Crop";
        const char *whichbox = NULL;
        if( x->use_trimbox ) {
            whichbox = usetrimbox;
        }
        else if( x->use_cropbox ) {
            whichbox = usecropbox;
        }

        // We force to 200dpi, I figure the 'master' images will be big enough
        int resolution = 200;

        // Construct commandline and run ghostscript
        // TextAlphaBits + GraphicsAlphaBits = antialiasing
        // Printed = render onto 'background color'
        // DOINTERPOLATE = smoother scaled images 
        char *cmdline = NULL;
        char *escaped_filename = escapeshellarg(x->pdf_file);
        ::asprintf(&cmdline, "gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=png16m "
                            "-sOutputFile=%s/page_%%04d_master.png -r%d "
                            "-dTextAlphaBits=4 -dGraphicsAlphaBits=4 "
                            "-dFirstPage=%d -dLastPage=%d -dPrinted=false "
                            "-dShowAnnots=false -dUse%sBox -dDOINTERPOLATE %s",
                            x->out_dir, resolution,
                            x->first_page, x->last_page,
                            whichbox, escaped_filename);
        assert( cmdline != NULL );
        ctx()->log->debug("Running GhostScript: '%s'", cmdline);
        ::system(cmdline);
        ::free(escaped_filename);
        ::free(cmdline);

        // Now load each master PNG again and output it
        int gs_pageno;
        int pageno;
        for( pageno = x->first_page; pageno <= x->last_page; pageno++ ) {
            gs_pageno = (pageno - x->first_page) + 1;
            char *filename = NULL;
            ::asprintf(&filename, "%s/page_%04d_master.png", x->out_dir, gs_pageno);
            assert( filename != NULL );
            PngFileBitmap *bitmap = PngFileBitmap::Open(filename);
            if( bitmap == NULL ) {
                ctx()->log->warn("Failed to open master PNG file '%s'", filename);
                ::free(filename);
                return false;
            }
            startPage(pageno);
            this->write(bitmap);
            finishPage(pageno);
            delete bitmap;
            // Then delete master file, don't need it anymore
            ::remove(filename);
            ::free(filename);
        }

        return true;
    }
};

PageRenderer *
backend_ghostscript(RenderContext *ctx) {
    return new GhostscriptPageRenderer(ctx);
}