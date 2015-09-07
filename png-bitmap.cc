#pragma implementation "png-bitmap.hh"
#include "png-bitmap.hh"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

#include <cstdlib>

PngFileBitmap *
PngFileBitmap::Open(const char *filename) {
    PngFileBitmap *bitmap = new PngFileBitmap(filename);
    if( bitmap->loadPngFile() ) {
        return bitmap;
    }
    delete bitmap;
    return NULL;
}

PngFileBitmap::PngFileBitmap( const char *filename )
: m_rows(NULL)
{
    m_filename = ::strdup(filename);
}

PngFileBitmap::~PngFileBitmap() {
    if( m_rows ) {
        png_uint_32 y;
        for( y = 0; y < m_height; y++ ) {
            ::free(m_rows[y]);
        }
        ::free(m_rows);
    }
    ::free(m_filename);
}

int
PngFileBitmap::width() const {
    return m_width;
}

int
PngFileBitmap::height() const {
    return m_height;
}

BitmapPixel
PngFileBitmap::pixel(int x, int y) const {
    BitmapPixel px;
    png_bytep row = m_rows[y];
    png_bytep pixel = row + (x * 3);
    px.red = pixel[0];
    px.green = pixel[1];
    px.blue = pixel[2];        
    return px;
}    

bool
PngFileBitmap::loadPngFile()
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height, y;
    int depth, coltype;
    png_bytep *pp;
    size_t rowbytes, passes, n;
    png_byte header[8];
    FILE *fp;

    if(NULL == (fp = fopen(m_filename, "rb")))
    {
        perror(m_filename);
        return false;
    }
    fread(header, 1, sizeof(header), fp);
    if(0 != png_sig_cmp(header, 0, sizeof(header)))
    {
        fprintf(stderr, "%s: does not appear to be a PNG file\n", m_filename);
        fclose(fp);
        return false;
    }
    if(NULL == (png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
    {
        fprintf(stderr, "failed to initialise libpng (png_create_read_struct)\n");
        fclose(fp);
        return false;
    }
    if(NULL == (info_ptr = png_create_info_struct(png_ptr)))
    {
        fprintf(stderr, "failed to initialise libpng (png_create_info_struct)\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return false;
    }

#ifdef PNG_SETJMP_SUPPORTED
    if((setjmp(png_jmpbuf(png_ptr))))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return false;
    }
#endif

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, sizeof(header));
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &depth, &coltype, NULL, NULL, NULL);

    if(NULL == (pp = (png_bytep *)::realloc(m_rows, sizeof(png_bytep) * height)))
    {
        fprintf(stderr, "failed to realloc to %lu bytes\n", sizeof(png_bytep) * height);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return false;
    }
    m_rows = pp;
    m_height = height;
    m_width = width;
    memset(m_rows, 0, sizeof(png_bytep) * height);

    // XXX: most of these are not used because we know we should get
    //      images 8bit RGB format.
    if(coltype == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
    }
    if(coltype == PNG_COLOR_TYPE_GRAY || coltype == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        if(depth < 8)
        {
            png_set_gray_1_2_4_to_8(png_ptr);
        }
        png_set_gray_to_rgb(png_ptr);
    }
    if(depth == 16)
    {
        png_set_strip_16(png_ptr);
    }    
    if(depth < 8)
    {
        png_set_packing(png_ptr);
    }

    // PNG should be overlaid onto a white background
	png_color_16 my_background;
    png_color_16p image_background;
    my_background.red = 255;
    my_background.green = 255;
    my_background.blue = 255;
    image_background = &my_background;
    if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
        png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    }
    else {
        png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
    }

    passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    
    for (y = 0; y < height; y++)
    {
        m_rows[y] = (png_bytep)::realloc(m_rows[y], rowbytes);
    }

    for(n = 0; n < passes; n++)
    {
        png_read_rows(png_ptr, m_rows, NULL, height);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return true;
}