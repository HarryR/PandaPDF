#pragma implementation "image-output.hh"
#include "image-output.hh"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "utils.hh"
#include "goo/PNGWriter.h"
#include "goo/JpegWriter.h"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

// -----------------------------------------------------------------------------

int
ImageResizerOutput::maxWidth() const {
    return m_width;
}

int
ImageResizerOutput::maxHeight() const {
    return m_height;
}

const char *
ImageResizerOutput::profileName() const {
    return  m_name;
}

ImageResizerOutput::ImageResizerOutput(const char *name, int max_width, int max_height)
: ImageOutput()
, m_writer(NULL)
, m_width(max_width)
, m_height(max_width)
, m_pageno(-1)
, m_ctx(NULL)
{
    m_name = ::strdup(name);
}

ImageResizerOutput::~ImageResizerOutput() {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    ::free(m_name);
}

#define gdAlphaMax 127
#define floor_cast(exp) ((long) exp)

/**
 * Uses bicubic interpolation to resample image to required size.
 *
 * This produces smooth but accurate images from the 'master bitmap', allowing 
 * for supersampling based antialiasing.
 */
void
ImageResizerOutput::resampleBitmapToWriter( const Bitmap *bitmap ) {
    int dstX, dstY, srcX, srcY;
    dstX = dstY = srcX = srcY = 0;

    int srcW = bitmap->width(), srcH = bitmap->height();
    int dstW, dstH;
    scaleWidthHeight( srcW, srcH,
                      m_width, m_height,
                      &dstW, &dstH );

    int x, y;
    double sy1, sy2, sx1, sx2;

    unsigned char *out_row = (unsigned char *)::malloc(3 * dstW);
    for (y = dstY; (y < dstY + dstH); y++) {
        sy1 = ((double) y - (double) dstY) * (double) srcH / (double) dstH;
        sy2 = ((double) (y + 1) - (double) dstY) * (double) srcH / (double) dstH;
        for (x = dstX; (x < dstX + dstW); x++) {
            double sx, sy;
            double spixels = 0;
            double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;
            double alpha_factor, alpha_sum = 0.0, contrib_sum = 0.0;
            sx1 = ((double) x - (double) dstX) * (double) srcW / dstW;
            sx2 = ((double) (x + 1) - (double) dstX) * (double) srcW / dstW;
            sy = sy1;
            do {
                double yportion;
                if (floor_cast(sy) == floor_cast(sy1)) {
                    yportion = 1.0f - (sy - floor_cast(sy));
                    if (yportion > sy2 - sy1) {
                        yportion = sy2 - sy1;
                    }
                    sy = floor_cast(sy);
                } else if (sy == floorf(sy2)) {
                    yportion = sy2 - floor_cast(sy2);
                } else {
                    yportion = 1.0f;
                }
                sx = sx1;
                do {
                    double xportion;
                    double pcontribution;
                    if (floorf(sx) == floor_cast(sx1)) {
                        xportion = 1.0f - (sx - floor_cast(sx));
                        if (xportion > sx2 - sx1) {
                            xportion = sx2 - sx1;
                        }
                        sx = floor_cast(sx);
                    } else if (sx == floorf(sx2)) {
                        xportion = sx2 - floor_cast(sx2);
                    } else {
                        xportion = 1.0f;
                    }
                    pcontribution = xportion * yportion;

                    BitmapPixel pixel = bitmap->pixel(sx + srcX, sy + srcY);
                    alpha_factor = gdAlphaMax * pcontribution;
                    red += pixel.red * alpha_factor;
                    green += pixel.green * alpha_factor;
                    blue += pixel.blue * alpha_factor;
                    alpha += gdAlphaMax * pcontribution;
                    alpha_sum += alpha_factor;
                    contrib_sum += pcontribution;
                    spixels += xportion * yportion;
                    sx += 1.0f;
                }
                while (sx < sx2);

                sy += 1.0f;
            }

            while (sy < sy2);

            if (spixels != 0.0f) {
                red /= spixels;
                green /= spixels;
                blue /= spixels;
                alpha /= spixels;
            }
            if ( alpha_sum != 0.0f) {
                if( contrib_sum != 0.0f) {
                    alpha_sum /= contrib_sum;
                }
                red /= alpha_sum;
                green /= alpha_sum;
                blue /= alpha_sum;
            }
            /* Clamping to allow for rounding errors above */
            if (red > 255.0f) {
                red = 255.0f;
            }
            if (green > 255.0f) {
                green = 255.0f;
            }
            if (blue > 255.0f) {
                blue = 255.0f;
            }
            if (alpha > gdAlphaMax) {
                alpha = gdAlphaMax;
            }
            out_row[3*x] = (int)red;
            out_row[3*x+1] = (int)green;
            out_row[3*x+2] = (int)blue;
        }
        m_writer->writeRow(&out_row);
    }
    ::free(out_row);
}

void
ImageResizerOutput::write( const Bitmap *bitmap ) {
    assert( m_ctx != NULL );
    assert( m_writer != NULL );
    char *path = filename(m_ctx, m_pageno);
    FILE *file = ::fopen(path, "wb");

    if( NULL == file ) {
        m_ctx->log->warn("Cannot open '%s' for writing", path);
    }
    else {
        int dstW, dstH;
        scaleWidthHeight( bitmap->width(), bitmap->height(),
                          m_width, m_height,
                          &dstW, &dstH );
        bool writer_ok = m_writer->init(file, dstW, dstH, 72, 72);
        if( ! writer_ok ) {
            m_ctx->log->warn("Cannot initialize writer for '%s'", path);
        }
        else {
            resampleBitmapToWriter( bitmap );
            m_writer->close();
            m_ctx->log->info("Page %d: Written image '%s' (%dx%d)",
                             m_pageno, path, dstW, dstH);
        }
        ::fclose(file);
    }
    ::free(path);
}

bool
ImageResizerOutput::startPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    m_pageno = pageno;
    m_ctx = ctx;
    return true;
}

bool
ImageResizerOutput::finishPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == ctx );
    assert( m_pageno == pageno );
    m_pageno = -1;
    m_ctx = NULL;
    return true;
}

// -----------------------------------------------------------------------------

ResizeToJpeg::ResizeToJpeg(const char *name, int max_x, int max_y, int quality)
: ImageResizerOutput(name, max_x, max_y)
, m_quality(quality)
{
    m_writer = new JpegWriter(quality, false);
}

ResizeToJpeg::~ResizeToJpeg() {
    delete m_writer;
}

char *
ResizeToJpeg::filename( RenderContext *ctx, int pageno ) {
    assert( ctx != NULL );
    assert( pageno > 0 );
    return ctx->path("page_%04d_%s.jpg", pageno, profileName());
}

// -----------------------------------------------------------------------------

ResizeToPng::ResizeToPng(const char *name, int max_x, int max_y)
: ImageResizerOutput(name, max_x, max_y)
{
    m_writer = new PNGWriter();
}

ResizeToPng::~ResizeToPng() {
    delete m_writer;
}

char *
ResizeToPng::filename( RenderContext *ctx, int pageno ) {
    assert( ctx != NULL );
    assert( pageno > 0 );
    return ctx->path("page_%04d_%s.png", pageno, profileName());
}
