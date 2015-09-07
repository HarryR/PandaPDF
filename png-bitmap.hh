#pragma once
#ifndef PNG_BITMAP_HH
#define PNG_BITMAP_HH

#pragma interface

#include <png.h>
#include "internal.hh"

class PngFileBitmap
: public Bitmap {
public:
    static PngFileBitmap* Open(const char *filename);

protected:
    PngFileBitmap( const char *filename );    

public:
    virtual ~PngFileBitmap();
    virtual int width() const;
    virtual int height() const;
    inline virtual BitmapPixel pixel(int x, int y) const;

private:
    bool loadPngFile();

    char *m_filename;
    png_bytep *m_rows;
    png_uint_32 m_width;
    png_uint_32 m_height;
};

#endif
