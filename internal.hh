#pragma once
#ifndef INTERNAL_HH_
#define INTERNAL_HH_

#pragma interface

#include <vector>
#include "logger.hh"
#include <stdint.h>

// -----------------------------------------------------------------------------

class PageRenderer;
class RenderContext;

typedef PageRenderer *(*PageRendererFactory)(RenderContext *ctx);

/**
 * Contains all parameters that a PDF rendering backend would need to run.
 */
class RenderContext {
public:
    Logger *log;

    /** Output directory */
    char *out_dir;

    /** Full path to PDF file */
    char *pdf_file;

    /** Owner / change permissions password */
    char *owner_password;

    /** User / document open password */
    char *user_password;

    /* Output should be scaled to fit exactly within these pixel sizes */
    int max_width;
    int max_height;

    /* Which crop boxes to use when cropping the page */
    bool use_trimbox;
    bool use_cropbox;
    bool use_mediabox;
    bool use_bleedbox;

    int first_page;
    int last_page;

public:
    RenderContext(Logger *logger);
    ~RenderContext();
    bool validate();
    void display();
    char *path( const char *filename, ... );
};

struct Word {
    double box[4]; /* x1, y1, x2, y2 */
    char *text;
};

struct Region {
    double box[4]; /* x1, y1, x2, y2 */
    char *type;
    char *title;
    char *url;
};

struct BitmapPixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    uint32_t toARGB();
};

/**
 * Supports RGB24, ARGB32 and RGBA32 formats.
 * RGB24 {bpp:3,roff:0}
 * RGBA32 {bpp:4,roff:0}
 * ARGB32 {bpp:4,roff:1}
 */
class Bitmap {
public:
    virtual ~Bitmap();
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual BitmapPixel pixel(int x, int y) const = 0;
};

// -----------------------------------------------------------------------------

class Output {
protected:
    Output();

public:
    virtual ~Output();
    virtual bool startPage( RenderContext *ctx, int pageno ) = 0;
    virtual bool finishPage( RenderContext *ctx, int pageno ) = 0;
    virtual char *filename( RenderContext *ctx, int pageno ) = 0;
};

class WordOutput
: public Output {
protected:
    WordOutput();
public:
    virtual ~WordOutput(){}
    virtual void write( const Word *word ) = 0;
};

class RegionOutput
: public Output {
protected:
    RegionOutput();
public:
    virtual ~RegionOutput(){}
    virtual void write( const Region *region ) = 0;
};

class ImageOutput
: public Output {
protected:
    ImageOutput();
public:
    virtual ~ImageOutput(){}
    virtual void write( const Bitmap *bitmap ) = 0;
    virtual int maxWidth() const = 0;
    virtual int maxHeight() const = 0;
    virtual const char *profileName() const = 0;
};

// -----------------------------------------------------------------------------

class PageRenderer {
protected:
    PageRenderer(RenderContext *c);

public:
    virtual ~PageRenderer();

    virtual bool doesRegions() = 0;
    virtual bool doesImages() = 0;
    virtual bool doesWords() = 0;

    /** Add output channel for Regions */
    void addOutput( RegionOutput *out );

    /** Add output channel for image bitmaps */
    void addOutput( ImageOutput *out );

    /** Add output channel for Words */
    void addOutput( WordOutput *out );

    /** Did the PDF file load successfully */
    virtual bool isOk() = 0;

    virtual bool run() = 0;

    void startPage( int pageno );
    void finishPage( int pageno );

protected:
    RenderContext *ctx();
    virtual void write( const Bitmap *bitmap );
    virtual void write( const Region *region );
    virtual void write( const Word *word );

private:
    int m_pageno;
    RenderContext *m_ctx;
    std::vector<WordOutput*> m_out_word;
    std::vector<RegionOutput*> m_out_region;
    std::vector<ImageOutput*> m_out_image;
};

#endif
