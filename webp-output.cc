#pragma implementation "webp-output.hh"
#include "webp-output.hh"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "utils.hh"
#include "webp/types.h"
#include "webp/encode.h"

WebpImageOutput::WebpImageOutput(const char *name, int max_width, int max_height, int quality)
: m_quality(quality)
, m_width(max_width)
, m_height(max_height)
, m_pageno(-1)
, m_ctx(NULL)
{
	assert( name != NULL );
	m_name = ::strdup(name);
}

WebpImageOutput::~WebpImageOutput() {
	assert( m_pageno == -1 );
	assert( m_ctx == NULL );
	::free(m_name);
}

char *
WebpImageOutput::filename( RenderContext *ctx, int pageno ) {
    assert( ctx != NULL );
    assert( pageno > 0 );
    return ctx->path("page_%04d_%s.webp", pageno, profileName());
}

static int
webp_file_writer(const uint8_t* data, size_t data_size,
				 const WebPPicture* const pic) {
  FILE* const out = (FILE*)pic->custom_ptr;
  return data_size ? (fwrite(data, data_size, 1, out) == 1) : 1;
}

void
WebpImageOutput::write( const Bitmap *bitmap ) {
	int dstW, dstH;
    scaleWidthHeight( bitmap->width(), bitmap->height(),
                      m_width, m_height,
                      &dstW, &dstH );
    char *out_filename = filename(m_ctx, m_pageno);
    FILE *out_file = ::fopen(out_filename, "wb");
    if( out_file == NULL ) {
    	m_ctx->log->error("Cannot open '%s' for writing", out_file);
    	free(out_filename);
    	return;
    }    

	WebPConfig config;
	memset(&config, 0, sizeof(WebPConfig));
	if (!WebPConfigPreset(&config, WEBP_PRESET_PHOTO, m_quality)) return; 	
	config.method = 4;	// quality/speed trade-off (0=fast, 6=slower-better)
	config.pass = 3;	// number of entropy-analysis passes (in [1..10]).
	config.autofilter = 1;	// Auto adjust filter's strength [0 = off, 1 = on]
	//config.preprocessing = 1;// preprocessing filter (0=none, 1=segment-smooth)
	if( ! WebPValidateConfig(&config) ) {
		m_ctx->log->error("Invalid WebP config!");
	}

	// Setup picture
	WebPPicture pic;
  	if (!WebPPictureInit(&pic)) {
  		m_ctx->log->warn("Cannot WebPPictureInit");
  		return;  // version error
  	}
  	pic.use_argb = 1;
  	pic.argb_stride = bitmap->width();
  	pic.width = bitmap->width();
  	pic.height = bitmap->height();
  	if (!WebPPictureAlloc(&pic)) {
  		m_ctx->log->warn("Cannot WebPPictureAllocate!");
  		return;   // memory error
  	}

  	// Copy bitmap into webp picture... Slooooowww! :(
  	for( int x = 0; x < bitmap->width(); x++ ) {
  		for( int y = 0; y < bitmap->height(); y++ ) {
  			BitmapPixel bpx = bitmap->pixel(x, y);
			pic.argb[y * pic.argb_stride + x] = bpx.toARGB();
  		}
  	}

  	// Re-scale to target size
  	if (!WebPPictureRescale(&pic, dstW, dstH)) {
      m_ctx->log->error("Cannot perform WebPPictureRescale");
      return;
    }

	pic.writer = webp_file_writer;
	pic.custom_ptr = out_file;

	m_ctx->log->debug("Page %d: Encoding with WebP quality %d (%dx%d)",
	                             m_pageno, m_quality, dstW, dstH);
	int ok = WebPEncode(&config, &pic);
	fclose(out_file);

	WebPPictureFree(&pic);
	if (!ok) {
		m_ctx->log->error("WebPEncode error: %d\n", pic.error_code);
		::remove(out_filename);
	}
	else {		
		m_ctx->log->info("Page %d: Written image '%s' (%dx%d)",
	                             m_pageno, out_filename, dstW, dstH);
	}
	free(out_filename);
}

int
WebpImageOutput::maxWidth() const {
    return m_width;
}

int
WebpImageOutput::maxHeight() const {
    return m_height;
}

const char *
WebpImageOutput::profileName() const {
    return  m_name;
}

bool
WebpImageOutput::startPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    m_pageno = pageno;
    m_ctx = ctx;
    return true;
}

bool
WebpImageOutput::finishPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == ctx );
    assert( m_pageno == pageno );
    m_pageno = -1;
    m_ctx = NULL;
    return true;
}