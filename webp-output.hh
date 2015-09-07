#pragma once
#ifndef WEBP_OUTPUT_HH
#define WEBP_OUTPUT_HH

#pragma interface

#include "internal.hh"

class WebpImageOutput
: public ImageOutput {
public:
	WebpImageOutput(const char *name, int max_width, int max_height, int quality);
    virtual ~WebpImageOutput();
    virtual void write( const Bitmap *bitmap );
    virtual bool startPage( RenderContext *ctx, int pageno );
    virtual bool finishPage( RenderContext *ctx, int pageno );
    virtual int maxWidth() const;
    virtual int maxHeight() const;
    virtual const char *profileName() const;
    char *filename( RenderContext *ctx, int pageno );

private:
    int m_quality;
	int m_width;
	int m_height;
	int m_pageno;
	RenderContext *m_ctx;
	char *m_name;
};

#endif
