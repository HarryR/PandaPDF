#pragma once
#ifndef IMAGE_OUTPUT_HH
#define IMAGE_OUTPUT_HH

#pragma interface

#include "internal.hh"

//#include "goo/ImgWriter.h"
class ImgWriter;

class ImageResizerOutput
: public ImageOutput {
protected:
    ImageResizerOutput(const char *name, int max_width, int max_height);
    virtual ~ImageResizerOutput();

public:
    virtual void write( const Bitmap *bitmap );
    virtual bool startPage( RenderContext *ctx, int pageno );
    virtual bool finishPage( RenderContext *ctx, int pageno );
    virtual int maxWidth() const;
    virtual int maxHeight() const;
    virtual const char *profileName() const;

private:
    void resampleBitmapToWriter( const Bitmap *bitmap );

protected:
    ImgWriter *m_writer;

private:
    int m_width;
    int m_height;
    int m_pageno;
    RenderContext *m_ctx;
    char *m_name;
};

class ResizeToJpeg
: public ImageResizerOutput {
public:
    ResizeToJpeg(const char *name, int max_x, int max_y, int quality);
    virtual ~ResizeToJpeg();
    char *filename( RenderContext *ctx, int pageno );

private:
    int m_quality;
};

class ResizeToPng
: public ImageResizerOutput {
public:
    ResizeToPng(const char *name, int max_x, int max_y);
    virtual ~ResizeToPng();
    char *filename(RenderContext *ctx, int pageno);
};

#endif
