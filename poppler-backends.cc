#pragma implementation "poppler-backends.hh"
#include "poppler-backends.hh"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cairo.h>
#include <cstdio>
#include <cassert>

#include "asprintf.h"
#include "poppler/GlobalParams.h"
#include "poppler/Link.h"
#include "poppler/Page.h"
#include "poppler/PDFDoc.h"
#include "poppler/CairoOutputDev.h"
#include "poppler/SplashOutputDev.h"
#include "splash/SplashBitmap.h"
#include "splash/Splash.h"

/**
 * Generic backend for Poppler based renderings
 * Opens PDF document, handles coordinate transformation etc.
 */
class PopplerPageRenderer
: public PageRenderer {
public:
    PopplerPageRenderer( RenderContext *ctx )
    : PageRenderer(ctx), m_doc(NULL)
    {
        assert( ctx->first_page >= 1 );
        assert( ctx->last_page >= ctx->first_page );

        // These become owned by the PDFDoc
        GooString *file_name = new GooString(ctx->pdf_file);
        GooString *ownerPW = NULL;
        GooString *userPW = NULL;

        if (ctx->owner_password) {
            ownerPW = new GooString(ctx->owner_password);
        }
        if (ctx->user_password) {
            userPW = new GooString(ctx->user_password);
        }

        ctx->log->debug("Loading PDF '%s'", ctx->pdf_file);
        m_doc = new PDFDoc(file_name, ownerPW, userPW);        

        initGlobalParams();
    }

    virtual bool isOk() {
        RenderContext *x = this->ctx();
        if( ! m_doc->isOk() ) return false;

        if( x->first_page < 1 ) {
            x->log->warn("Invalid first page: %d", x->first_page);
            return false;
        }

        if( x->last_page > m_doc->getNumPages() ) {
            x->log->warn("Invalid last page: %d (PDF has %d pages)",
                          x->last_page, m_doc->getNumPages());
            return false;
        }

        return true;
    }

    virtual ~PopplerPageRenderer() {
        delete m_doc;
    }

    virtual bool
    run() {
        int pageno;
        RenderContext *x = ctx();        
        for( pageno = x->first_page; pageno <= x->last_page; pageno++ ) {
            startPage(pageno);
            if( false == renderPage(pageno) ) {
                this->ctx()->log->warn("Failed to render page %d", pageno);
                return false;
            }
            finishPage(pageno);
        }
        return true;
    }

protected:
    virtual bool renderPage(int pageno) = 0;

private:
    void
    initGlobalParams() {
        if( NULL == globalParams ) {
            char yes[] = "yes";
            globalParams = new GlobalParams("/usr/share/poppler/");
            globalParams->setEnableFreeType(yes);
        }
    }

protected:
    Page *
    getPage(int pageno) {
        Catalog* catalog = m_doc->getCatalog();
        return catalog->getPage(pageno);
    }

    /**
     * Return whichever cropping rectangle we're going to use
     */
    PDFRectangle *
    getCorrectRectangle( Page *page ) {
        PDFRectangle *trim = page->getTrimBox();
        PDFRectangle *crop = page->getCropBox();

        if( ctx()->use_mediabox ) return page->getMediaBox();
        if( ctx()->use_bleedbox ) return page->getBleedBox();
        if( ctx()->use_cropbox ) return crop;
        if( ctx()->use_trimbox ) return trim;

        // Use whichever is smallest - trimbox or cropbox
        if( trim->x1 > crop->x1
         && trim->y1 > crop->y1
         && trim->y2 < crop->y2
         && trim->x2 < crop->x2 )
        {
            return trim;
        }
        return crop;
    }

    /**
     * Calculate page slice coordinates and DPI for rendering to exact pixel
     * sizes.
     */
    void
    calculateSlice( Page *pdfpage, double *hDPI, double *vDPI, int *sliceX, int *sliceY, int *sliceW, int *sliceH ) {
        // Calculate correct width/height for canvas
        PDFRectangle *crop_rect = getCorrectRectangle(pdfpage);
        PDFRectangle rect;
        fixRectangle(pdfpage->getMediaBox(), crop_rect, rect, pdfpage->getRotate());
        double pg_w = rect.x2 - rect.x1;
        double pg_h = rect.y2 - rect.y1;
        double scaleTo = (pg_w > pg_h ? ctx()->max_width : ctx()->max_height);
        double dpi = 72.0;
        double res = (dpi * scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
        double x_res, y_res;
        x_res = y_res = res;

        // Actual pixel sizes
        int x, y, w, h;
        x = (x_res/dpi) * rect.x1;
        y = (y_res/dpi) * rect.y1;
        w = (x_res/dpi) * (rect.x2 - rect.x1);
        h = (y_res/dpi) * (rect.y2 - rect.y1);

        *hDPI = x_res;
        *vDPI = y_res;
        *sliceX = x;
        *sliceY = y;
        *sliceW = w;
        *sliceH = h;
    }

    void
    fixRectangle( PDFRectangle *page, PDFRectangle *crop, PDFRectangle &out, int rotation ) {
        assert( rotation == 0 || rotation == 90 || rotation == 180 || rotation == 270 );

        double height = crop->y2 - crop->y1;
        double width = crop->x2 - crop->x1;
        double y1space = crop->y1 - page->y1;
        double x1space = crop->x1 - page->x1;
        double y2space = page->y2 - crop->y2;
        double x2space = page->x2 - crop->x2;

        if( rotation == 0 ) {
            out.x1 = x1space;
            out.y1 = y2space;
            out.x2 = out.x1 + width;
            out.y2 = out.y1 + height;
        }
        else if( rotation == 90 ) {
            out.x1 = y1space;
            out.y1 = x1space;
            out.x2 = out.x1 + height;
            out.y2 = out.y1 + width;
        }
        else if( rotation == 180 ) {
            out.x1 = x2space;
            out.y1 = y1space;
            out.x2 = out.x1 + width;
            out.y2 = out.y1 + height;
        }
        else if( rotation == 270 ) {
            out.x1 = y2space;
            out.y1 = x2space;
            out.x2 = out.x1 + height;
            out.y2 = out.y1 + width;
        }
    }

    void
    displaySlice( Page *pdfpage, OutputDev *out ) {
        double hDPI, vDPI;
        int sliceX, sliceY, sliceW, sliceH;
        calculateSlice(pdfpage, &hDPI, &vDPI, &sliceX, &sliceY, &sliceW, &sliceH);

        // Render page to output device
        pdfpage->displaySlice(
            out,
            hDPI,
            vDPI,
            0,          // rotate
            gTrue,      // useMediaBox
            gFalse,     // crop
            sliceX, sliceY, sliceW, sliceH,
            gFalse,     // printing
            NULL,       // abortCheckCbk
            NULL,       // abortCheckCbkData
            NULL,       // annotDisplayDecideCbk
            NULL        // annotDisplayDecideCbkData
        );
    }

protected:
    PDFDoc *m_doc;
};

class PopplerCairoBitmap
: public Bitmap {
public:
    PopplerCairoBitmap( cairo_surface_t *bitmap )
    : m_bitmap(bitmap)
    { }

    virtual int
    width() const {
        return cairo_image_surface_get_width(m_bitmap);
    }

    virtual int
    height() const {
        return cairo_image_surface_get_height(m_bitmap);
    }

    inline virtual BitmapPixel
    pixel(int x, int y) const {
        BitmapPixel px;

        int pixel_stride = cairo_image_surface_get_stride(m_bitmap) / width();
        unsigned char* imgdata = cairo_image_surface_get_data(m_bitmap);
        int pos_offset = (x * pixel_stride) + (y * cairo_image_surface_get_stride(m_bitmap));
        unsigned char *p = &imgdata[pos_offset];

        px.red = p[2];
        px.green = p[1];
        px.blue = p[0];

        return px;
    }

private:
    cairo_surface_t *m_bitmap;
};

/**
 * Emit bitmaps using Cairo backend
 */
class CairoPageRenderer
: public PopplerPageRenderer {
public:
    CairoPageRenderer( RenderContext *ctx )
    : PopplerPageRenderer(ctx)
    { }

    virtual bool doesRegions() { return false; }
    virtual bool doesImages() { return true; }
    virtual bool doesWords() { return false; }

protected:
    virtual bool
    renderPage(int pageno) {
        Page *pdfpage = getPage(pageno);
        if( pdfpage == NULL ) return false;

        // Setup output device
        CairoOutputDev cairo_out;
        cairo_out.setVectorAntialias(gFalse);
        cairo_out.startDoc(m_doc, NULL);
        cairo_out.setPrinting(gFalse);

        // Setup buffer
        cairo_surface_t *master_bitmap;
        {
            double hDPI, vDPI;
            int sliceX, sliceY, sliceW, sliceH;
            calculateSlice(pdfpage, &hDPI, &vDPI, &sliceX, &sliceY, &sliceW, &sliceH);
            master_bitmap = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sliceW, sliceH);
        }
        cairo_t *cr = cairo_create(master_bitmap);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);

        // Display Page
        cairo_save(cr);
            cairo_out.setCairo(cr);
            displaySlice(pdfpage, &cairo_out);
        cairo_restore(cr);

        // Blend onto white page
        cairo_save(cr);
            cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_paint(cr);
        cairo_restore(cr);

        // Destroy cairo context
        cairo_out.setCairo(NULL);
        cairo_surface_flush(master_bitmap);
        cairo_destroy(cr);

        // Write bitmap
        PopplerCairoBitmap bitmap(master_bitmap);
        write(&bitmap);
        cairo_surface_destroy(master_bitmap);

        return true;
    }
};

class PopplerSplashBitmap
: public Bitmap {
public:
    PopplerSplashBitmap( SplashBitmap *bitmap )
    : m_bitmap(bitmap)
    { }

    virtual ~PopplerSplashBitmap() {
        delete m_bitmap;
    }

    virtual int width() const {
        return m_bitmap->getWidth();
    }

    virtual int height() const {
        return m_bitmap->getHeight();
    }

    inline virtual BitmapPixel pixel(int x, int y) const {
        BitmapPixel px;
        SplashColor color;
        m_bitmap->getPixel(x, y, color);
        px.red = splashRGB8R(color);
        px.green = splashRGB8G(color);
        px.blue = splashRGB8B(color);
        return px;
    }

private:
    SplashBitmap *m_bitmap;
};

/**
 * Emit bitmaps using Splash backend
 */
class SplashPageRenderer 
: public PopplerPageRenderer {
public:
    SplashPageRenderer( RenderContext *ctx )
    : PopplerPageRenderer(ctx)
    {}

    virtual bool doesRegions() { return false; }
    virtual bool doesImages() { return true; }
    virtual bool doesWords() { return false; }

protected:
    virtual bool
    renderPage(int pageno) {
        Page *pdfpage = getPage(pageno);
        if( pdfpage == NULL ) return false;

        SplashColor paperColor = {0xff, 0xff, 0xff};
        SplashOutputDev splash_out(
            splashModeRGB8, // colorModeA
            4,          // bitmapRowPad
            gFalse,     // reverseVideoA
            paperColor, // paperColorA
            gTrue      // bitmapTopDownA

            // New versions of poppler have these params
            // thinLineMode
            // overprintPreviewA

            // Older versions of poppler have these params
            //gTrue       // allowAntiAlias
        );
        splash_out.setFontAntialias(gTrue);
        splash_out.setVectorAntialias(gFalse);
        splash_out.startDoc(m_doc);
        displaySlice(pdfpage, &splash_out);

        PopplerSplashBitmap bitmap(splash_out.takeBitmap());
        write(&bitmap);

        return true;
    }
};

/**
 * Emit Words and Regions using TextOutputDev
 */
class TextPageRenderer
: public PopplerPageRenderer {
public:
    TextPageRenderer( RenderContext *ctx )
    : PopplerPageRenderer(ctx)
    { }

    virtual bool doesRegions() { return true; }
    virtual bool doesImages() { return false; }
    virtual bool doesWords() { return true; }

protected:
    virtual bool
    renderPage(int pageno) {
        Page *pdfpage = getPage(pageno);
        if( pdfpage == NULL ) return false;

        TextOutputDev text_out(
            NULL,   // filename
            gFalse, // physlayout
            0.0,    // fixedpitch
            gFalse, // raworder
            gFalse  // append
        );
        double hDPI, vDPI;
        int sliceX, sliceY, sliceW, sliceH;
        calculateSlice(pdfpage, &hDPI, &vDPI, &sliceX, &sliceY, &sliceW, &sliceH);
        displaySlice(pdfpage, &text_out);

        // Iterate + output words
        TextWordList *page_words = text_out.makeWordList();
        for( int i = 0; i < page_words->getLength(); i++ ) {
            TextWord *word = page_words->get(i);
            Word out_word;
            word->getBBox( &out_word.box[0], &out_word.box[1],
                           &out_word.box[2], &out_word.box[3] );
            // Transform box coordinates into 0.0 - 1.0 relative to page
            out_word.text = word->getText()->getCString();
            out_word.box[0] = out_word.box[0] / sliceW;
            out_word.box[1] = out_word.box[1] / sliceH;
            out_word.box[2] = out_word.box[2] / sliceW;
            out_word.box[3] = out_word.box[3] / sliceH;

            // Clip to coordinates
            if( out_word.box[0] < 0.0f ) out_word.box[0] = 0.0f;
            if( out_word.box[1] < 0.0f ) out_word.box[1] = 0.0f;
            if( out_word.box[2] > 1.0f ) out_word.box[2] = 1.0f;
            if( out_word.box[3] > 1.0f ) out_word.box[3] = 1.0f;
            write(&out_word);
        }

        // Iterate + output links
        Links *page_links = pdfpage->getLinks();
        for( int y = 0; y < page_links->getNumLinks(); y++ ) {
            AnnotLink *link = page_links->getLink(y);
            Region out_region;
            out_region.type = NULL;
            out_region.title = NULL;
            out_region.url = NULL;

            if( makeRegion(link, &out_region) ) {
                fixRegionBox(&out_region, pdfpage, vDPI, hDPI);
                out_region.box[0] = out_region.box[0] / sliceW;
                out_region.box[1] = out_region.box[1] / sliceH;
                out_region.box[2] = out_region.box[2] / sliceW;
                out_region.box[3] = out_region.box[3] / sliceH;

                // clip to coordinates
                if( out_region.box[0] < 0.0f ) out_region.box[0] = 0.0f;
                if( out_region.box[1] < 0.0f ) out_region.box[1] = 0.0f;
                if( out_region.box[2] > 1.0f ) out_region.box[2] = 1.0f;
                if( out_region.box[3] > 1.0f ) out_region.box[3] = 1.0f;
                
                write(&out_region);
                if( out_region.title ) ::free(out_region.title);
                if( out_region.url ) ::free(out_region.url);
                if( out_region.type ) ::free(out_region.type);
            }
        }

        return true;
    }

private:
    /** Get page number of link destination */
    int
    linkDestToPageNo( LinkDest *dest, GooString *named_dest ) {
        if( dest == NULL && named_dest ) {
            Catalog *cat = m_doc->getCatalog();
            dest = cat->findDest(named_dest);
        }

        if( NULL == dest ) {
            return -1;
        }

        if( false == dest->isPageRef() ) {
            return dest->getPageNum();
        }

        XRef *xr = m_doc->getXRef();
        Ref r = dest->getPageRef();

        // Grab the page
        if( r.num < 0 || r.num >= xr->getNumObjects() ) {
            return -1;
        }

        return m_doc->findPage(r.num, r.gen);
    }

    /**
     * When processing regions we get the *raw* PDF coordinates.
     *
     * PDF coordinates start at the bottom left hand corner.
     *
     * We need to apply the cropbox, then translate them to starting at the
     * top left. Normally poppler would do all of this for us, but we're
     * accessing the PDF object structure directly.
     *
     * The slice is X, Y, Width & Height, starting at top left hand corner.
     */
    void
    fixRegionBox( Region *region, Page *pdfpage, double vDPI, double hDPI ) {
        PDFRectangle *media_box = pdfpage->getMediaBox();                
        PDFRectangle region_out_rect;
        PDFRectangle region_rect;
        region_rect.x1 = region->box[0];
        region_rect.y1 = region->box[1];
        region_rect.x2 = region->box[2];
        region_rect.y2 = region->box[3];

        // Subtract cropbox from region rectangle
        PDFRectangle *crop_rect = getCorrectRectangle(pdfpage);
        PDFRectangle crop_rect_fixed;
        fixRectangle(media_box, crop_rect, crop_rect_fixed, pdfpage->getRotate());

        // Translate regionbox against media box & rotation
        fixRectangle(media_box, &region_rect, region_out_rect, pdfpage->getRotate());

        // Apply DPI to get correct page relative coords
        // Adjusting to be relative to the cropbox
        region->box[0] = (region_out_rect.x1 - crop_rect_fixed.x1) * hDPI / 72.0;
        region->box[1] = (region_out_rect.y1 - crop_rect_fixed.y1) * vDPI / 72.0;
        region->box[2] = (region_out_rect.x2 - crop_rect_fixed.x1) * hDPI / 72.0;
        region->box[3] = (region_out_rect.y2 - crop_rect_fixed.y1)* vDPI / 72.0;
    }

    /** Make a Region from an AnnotLink */
    bool
    makeRegion( AnnotLink *annot_link, Region *region ) {
        PDFRectangle *link_rect;
        LinkAction *action;
        assert( annot_link != NULL );

        link_rect = annot_link->getRect();
        if( link_rect == NULL ) return false;
        
        region->box[0] = link_rect->x1;
        region->box[1] = link_rect->y1;
        region->box[2] = link_rect->x2;
        region->box[3] = link_rect->y2;

        action = annot_link->getAction();
        if( action == NULL || false == action->isOk() )  {
            // Weeell... crap
            return false;
        }

        // Goto = page or 'name' within current document
        if( action->getKind() == actionGoTo ) {
            LinkGoTo *link = dynamic_cast<LinkGoTo*>(action);
            LinkDest *dest = link->getDest();
            GooString *named_dest = link->getNamedDest();        
            if( dest || named_dest ) {
                int target_pageno = linkDestToPageNo(dest, named_dest);
                ::asprintf(&region->url, "%d", target_pageno);
            }
            else {
                return false;
            }
            region->type = ::strdup("goto");
        }
        // GotoR = go to page or 'name' in another document
        else if( action->getKind() == actionGoToR ) {
            LinkGoToR *link = dynamic_cast<LinkGoToR*>(action);
            LinkDest *dest = link->getDest();
            GooString *named_dest = link->getNamedDest();
            // When we get a filename... wtf can we do?
            // As with GoTo - `dest` and `named_dest` are mutually exclusive
            if( dest || named_dest ) {
                ::asprintf(&region->url, "%s#%d",
                    link->getFileName()->getCString(), linkDestToPageNo(dest, named_dest) );
            }            
            else {
                return false;
            }
            region->type = ::strdup("gotor");
        }
        // normal URI/URL
        else if( action->getKind() == actionURI ) {
            LinkURI *link = dynamic_cast<LinkURI*>(action);
            region->type = ::strdup("uri");
            ::asprintf(&region->url, "%s", link->getURI()->getCString());
        }
        // Name/bookmark
        else if( action->getKind() == actionNamed ) {
            LinkNamed *link = dynamic_cast<LinkNamed*>(action);
            region->type = ::strdup("named");
            ::asprintf(&region->url, "%s", link->getName()->getCString());
        }
        // TODO: handle 'rendition' for HTML embedding!
        else {
            // Don't know how to handle this!
            return false;
        }

        return true;
    }
};

PageRenderer *
backend_poppler_cairo(RenderContext *ctx) {
    return new CairoPageRenderer(ctx);
}

PageRenderer *
backend_poppler_splash(RenderContext *ctx) {
    return new SplashPageRenderer(ctx);
}

PageRenderer *
backend_poppler_text(RenderContext *ctx) {
    return new TextPageRenderer(ctx);
}

void
backend_poppler_cleanup() {
    if( globalParams ) {
        delete globalParams;        
    }
}
