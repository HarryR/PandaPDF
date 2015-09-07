#pragma implementation "photoshop-backend.hh"
#include "photoshop-backend.hh"

// Copyright (c) 2013 PixelMags Inc. All Rights Reserved

#include <cassert>
#include "asprintf.h"
#include "disphelper.h"
#include "png-bitmap.hh"

class PhotoshopPageRenderer
: public PageRenderer {
public:
    PhotoshopPageRenderer( RenderContext *ctx )
    : PageRenderer(ctx)
    {       
        startPhotoshop();
    }

    virtual ~PhotoshopPageRenderer() {
        endPhotoshop();
    }

    virtual bool doesRegions() { return false; }
    virtual bool doesImages() { return true; }
    virtual bool doesWords() { return false; }

    virtual bool isOk() {
        return true;
    }

    virtual bool
    run() {
        RenderContext *x = this->ctx();
        int pageno;
        for( pageno = x->first_page; pageno <= x->last_page; pageno++ ) {
            if( ! exportPage(pageno) ) {
                x->log->warn("Failed to export page %d", pageno);
                return false;
            }
        }
        return true;
    }

private:
    bool
    exportPage( int pageno ) {
        RenderContext *x = this->ctx();

        char *out_filename;
        ::asprintf(&out_filename, "%s/page_%04d_master.png", x->out_dir, pageno);
        assert(out_filename != NULL);

        char full_pdf_path[MAX_PATH];
        ::GetFullPathName(x->pdf_file, MAX_PATH, full_pdf_path, NULL);

        dhPutValue(ps_pdfopen, L".Page = %d", pageno);
        
        // Open(document, _as)
        IDispatch *image = NULL;
        x->log->debug("Loading PDF file '%s' with Photoshop", full_pdf_path);
        if( ! SUCCEEDED(dhGetValue(L"%o", &image, ps_app, L".Open(%s,%o)", full_pdf_path, ps_pdfopen)) ) {
            x->log->warn("Cannot open '%s'", full_pdf_path);
            free(out_filename);
            return false;
        }
        assert( image != NULL );

        // SaveAs(saveIn, options, asCopy)
        bool success;
        success = SUCCEEDED(dhCallMethod(image, L".SaveAs(%s, %o, %b)", out_filename, ps_pngopt, TRUE));

        // 2 = PsSaveOptions.psDoNotSaveChanges
        dhCallMethod(image, L".Close(%d)", 2);
        SAFE_RELEASE(image);     

        if( success ) {
            PngFileBitmap *bitmap = PngFileBitmap::Open(out_filename);
            if( bitmap == NULL ) {
                ctx()->log->warn("Failed to open master PNG file '%s'", out_filename);
                success = false;
            }            
            else {
                startPage(pageno);
                this->write(bitmap);
                finishPage(pageno);
                delete bitmap;
                ::remove(out_filename);
            }
        }

        ::free(out_filename);            
        return success;
    }

    void startPhotoshop() {
        dhInitialize(TRUE);
        dhToggleExceptions(TRUE);
        dhCreateObject(L"Photoshop.Application", NULL, &ps_app);
        
        char *photoshop_version;
        dhGetValue(L"%s", &photoshop_version, ps_app, L".Version");
        ctx()->log->info("Photoshop v%s", photoshop_version);
        free(photoshop_version);

        // Setup PDF Open Options
        dhCreateObject(L"Photoshop.PDFOpenOptions", NULL, &ps_pdfopen);
        dhPutValue(ps_pdfopen, L".ConstrainProportions = %b", TRUE);
        dhPutValue(ps_pdfopen, L".AntiAlias = %b", TRUE);
        dhPutValue(ps_pdfopen, L".Mode = %d", 2);
        dhPutValue(ps_pdfopen, L".Width = %d", ctx()->max_width);
        dhPutValue(ps_pdfopen, L".Height = %d", ctx()->max_height);     
        dhPutValue(ps_pdfopen, L".UsePageNumber = %b", TRUE);
        dhPutValue(ps_pdfopen, L".SuppressWarnings = %b", TRUE);

        /*
         * psBoundingBox = 0
         * psMediaBox = 1
         * psCropBox = 2
         * psBleedBox = 3
         * psTrimBox = 4
         * psArtBox = 5
         */
        int which_box;
        if( ctx()->use_trimbox ) which_box = 4;
        else if( ctx()->use_cropbox ) which_box = 2;
        else if( ctx()->use_mediabox ) which_box = 1;
        else if( ctx()->use_bleedbox ) which_box = 3;
        else {
            ctx()->log->warn("Unknown PDF cropping box, using Trim box");
            which_box = 4;
        }
        dhPutValue(ps_pdfopen, L".CropPage = %d", which_box);

        // PNG Save Options
        dhCreateObject(L"Photoshop.PNGSaveOptions", NULL, &ps_pngopt);
        dhPutValue(ps_pngopt, L".Interlaced = %b", FALSE);
    }

    void endPhotoshop() {
        SAFE_RELEASE(ps_pngopt);
        SAFE_RELEASE(ps_pdfopen);
        SAFE_RELEASE(ps_app);
        dhUninitialize(TRUE);
    }

    IDispatch *ps_app;
    IDispatch *ps_pngopt;
    IDispatch *ps_pdfopen;
};

PageRenderer *
backend_photoshop_images(RenderContext *ctx) {
    return new PhotoshopPageRenderer(ctx);
}