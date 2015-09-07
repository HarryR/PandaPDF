#pragma implementation "render-driver.hh"
#include "render-driver.hh"

// Copyright (c) 2013 PixelMags Inc. All Rights Reserved

#include <cstring>
#include <cstdlib>
#include "image-output.hh"
#include "webp-output.hh"
#include "content-output.hh"
#include "poppler-backends.hh"
#include "ghostscript-backend.hh"
#ifdef _WIN32
#include "photoshop-backend.hh"
#endif

// ----------------------------------------------------------------------------


class AvailableRenderer {
public:
    char *name;
    PageRendererFactory factory;
    AvailableRenderer *next;

public:
    AvailableRenderer(const char *name, PageRendererFactory factory, AvailableRenderer *next) {
    	this->name = ::strdup(name);
	    this->factory = factory;
	    this->next = next;
    }

    PageRendererFactory
    find( const char *name ) {
    	if( ::strcmp(this->name, name) == 0 ) return this->factory;
	    if( this->next ) {
	        return this->next->find(name);
	    }
	    return NULL;
    }

    ~AvailableRenderer() {
    	::free(this->name);
	    if( this->next ) {
	        delete this->next;
	    }
    }
};


// ----------------------------------------------------------------------------


class ImageProfile {
public:
	~ImageProfile() {
		if( m_next ) {
			delete m_next;
		}

		if( m_output ) {
			delete m_output;
		}
	}

	ImageOutput *
	getOutput() {
		return m_output;
	}

	ImageProfile *
	getNext() {
		return m_next;
	}

	/**
	 * Parse an image output description.
	 * The string is in the format of 'name,format,resolution'
	 *
	 * Where name is an alphanumeric string, e.g. 'large' or 'medium'
	 * Format is the file extension and an optional quality, e.g. jpeg or jpeg@80
	 * Resolution is the maximum dimension, or bounding box in pixels, e.g. 2000 or 1500x2000
	 *
	 * Examples:
	 *   medium,jpeg@60,400
	 *   large,webp@90,2000
	 *   small,jpeg@75,45
	 */
	static ImageProfile *
	parse(Logger *logger, const char *profile, ImageProfile *next) {
		char *name = (char*)::malloc(::strlen(profile));
	    char *format = (char *)::malloc(::strlen(profile));
	    char *resolution = (char *)::malloc(::strlen(profile));
	    int format_quality = 90;
	    int max_width;
	    int max_height;
	    
	    // Example: large,jpeg@92,2000x2000
	    if( 3 != ::sscanf(profile, "%[a-z],%[jpegpngwebp@0-9],%[0-9x]", name, format, resolution) ) {
	    	logger->warn("Cannot parse profile '%s', must be 'name,format,resolution'", profile);
	        ::free(name);
	        ::free(format);
	        ::free(resolution);
	        return NULL;
	    }    

	    // Parse 'jpeg@70' etc.
	    if( strchr(format, '@') ) {
	        char *format_name = (char*)::malloc(::strlen(profile));
	        if( 2 == ::sscanf(format, "%[jpegpngwebp]@%d", format_name, &format_quality) ) {
	            ::free(format);
	            format = format_name;
	        }
	        else {
	        	logger->warn("Cannot parse format@quality '%s'", format);
	            ::free(name);
	            ::free(format);
	            ::free(format_name);
	            ::free(resolution);
	            return NULL;
	        }
	    }

	    if( ::strcmp(format, "jpg") && ::strcmp(format, "jpeg")
	     && ::strcmp(format, "png") && ::strcmp(format, "webp") )
	    {
	    	logger->warn("Unknown image '%s', must be 'webp', 'jpg', 'jpeg' or 'png'");
	        ::free(name);
	        ::free(format);
	        ::free(resolution);
	        return NULL;
	    }
	    
	    // Must be either 'MAXRES' or 'MAXWIDTHxMAXHEIGHT'
	    // e.g. '2000' or '2000x1500'
	    if( 2 == ::sscanf(resolution, "%dx%d", &max_width, &max_height) ) {
	        // Woo we parsed both width & height
	    }
	    else if( 1 == ::sscanf(resolution, "%d", &max_width) ) {
	        max_height = max_width;
	    }
	    else {
	        ::free(name);
	        ::free(format);
	        ::free(resolution);
	        return NULL;
	    }

	    ImageProfile *ipro = new ImageProfile(name, format, max_width, max_height, format_quality, next);

	    logger->debug("Profile '%s': %s@%d %dx%d", name, format, format_quality, max_width, max_height);

	    ::free(name);
	    ::free(format);
	    ::free(resolution);	    
	    return ipro;
	}



protected:
	ImageProfile(const char *name, const char *format, int max_width, int max_height, int quality, ImageProfile *next)
	: m_output(NULL)
	, m_next(next)
	{
		if( 0 == ::strcmp(format, "jpeg") || 0 == ::strcmp(format, "jpg") ) {
	        m_output = new ResizeToJpeg(name, max_width, max_height, quality);
	    }
	    else if( 0 == ::strcmp(format, "png") ) {
	        m_output = new ResizeToPng(name, max_width, max_height);
	    }
	    else if( 0 == ::strcmp(format, "webp") ) {
	    	m_output = new WebpImageOutput(name, max_width, max_height, quality);	
	    }
	}

private:
	ImageOutput *m_output;
	ImageProfile *m_next;
};


// ----------------------------------------------------------------------------


RenderDriver::RenderDriver(Logger *log)
: m_supersample(1.5f)
, m_text_words(false)
, m_json_words(false)
, m_json_regions(false)
, m_images(false)
, m_ctx(log)
, m_log(log)
, m_validated(false)
, m_profiles(NULL)
, m_content_backends(NULL)
, m_content_backend(NULL)
, m_image_backends(NULL)
, m_image_backend(NULL)
{
	// Populate available rendering backends
	m_content_backends = new AvailableRenderer("poppler-text", backend_poppler_text, m_content_backends);
	m_image_backends = new AvailableRenderer("poppler-cairo", backend_poppler_cairo, m_image_backends);
    m_image_backends = new AvailableRenderer("poppler-splash", backend_poppler_splash, m_image_backends);
    m_image_backends = new AvailableRenderer("ghostscript", backend_ghostscript, m_image_backends);
    #ifdef _WIN32
    m_image_backends = new AvailableRenderer("photoshop", backend_photoshop_images, m_image_backends);
    #endif
}

RenderDriver::~RenderDriver() {
	if( m_profiles ) delete m_profiles;
	if( m_image_backends ) delete m_image_backends;
	if( m_content_backends ) delete m_content_backends;
}

bool
RenderDriver::validate() {
	m_validated = true;

	if( m_supersample < 1.0 ) {
		m_log->warn("Supersampling must be 1.0 or above");
		m_validated = false;
	}

	// Validate image output
	if( m_images ) {
		if( m_image_backend == NULL ) {
			m_log->warn("Image output required, but no image backend specified");
			m_validated = false;
		}

		if( m_profiles == NULL ) {
			m_log->warn("Image output required, but no profiles specified");
			m_validated = false;
		}

		// Dimensions of the master bitmap is the same as the largest output profile.
		ImageProfile *profile = m_profiles;
		int profile_count = 0;
		while( profile ) {
			ImageOutput *output = profile->getOutput();
			if( output->maxWidth() > m_ctx.max_width
			 && output->maxHeight() > m_ctx.max_height ) {
				m_ctx.max_width = output->maxWidth();
				m_ctx.max_height = output->maxHeight();
			}
			profile_count++;
			profile = profile->getNext();
		}

		// Master bitmap is increased by the supersampling factor
		// e.g. 1000x1000 @ 1.5x = 1500x1500
		// XXX: when using Photoshop backend, there's no need to supersample
		m_ctx.max_width = (int)(m_supersample * (double)m_ctx.max_width);
		m_ctx.max_height = (int)(m_supersample * (double)m_ctx.max_height);
		m_log->debug("Image output enabled, %d profiles, max width: %d, max height: %d (%.2fx supersampling)",
			profile_count, m_ctx.max_width, m_ctx.max_height, m_supersample);
	}
	else {
		// With no image output we still need to have a target size for the text output renderer, the dimensions don't matter much as it's all converted to a 0.0-1.0 representation of page width/height.
		m_ctx.max_width = 1000;
		m_ctx.max_height = 1000;
	}

	// Validate content output
	if( m_text_words || m_json_words || m_json_regions ) {
		if( m_content_backend == NULL ) {
			m_log->warn("Content output required, but no content backend specified");
			m_validated = false;
		}
		else {
			if( m_text_words ) m_log->debug("Words to .txt enabled");		
			if( m_json_words ) m_log->debug("Words to .json enabled");
			if( m_json_regions ) m_log->debug("Regions to .json enabled");
		}

	}

	// Must have either image or content output
	if( ! m_images && ! m_text_words && ! m_json_words && ! m_json_regions ) {
		m_log->warn("No image or content outputs defined, cannot do anything");
		m_validated = false;
	}	

	if( m_validated ) {
		m_validated = m_ctx.validate();
	}

	return m_validated;
}

bool
RenderDriver::setImageBackend( const char *name ) {
	PageRendererFactory backend = m_image_backends->find(name);
	if( NULL == backend ) {
		m_log->error("Unknown image backend '%s'", name);
		return false;
	}
	m_image_backend = backend;
	m_validated = false;
	return true;
}

bool
RenderDriver::setContentBackend( const char *name ) {
	PageRendererFactory backend = m_content_backends->find(name);
	if( NULL == backend ) {
		m_log->error("Unknown content backend '%s'", name);
		return false;
	}
	m_content_backend = backend;
	m_validated = false;
	return true;
}

bool
RenderDriver::addImageProfile( const char *profile_description ) {
	ImageProfile *profile = ImageProfile::parse(m_log, profile_description, m_profiles);
	if( profile == NULL ) {
		return false;
	}
	m_profiles = profile;
	return true;
}

bool
RenderDriver::run() {
	bool success = true;

	if( ! m_validated && ! validate() ) {
		m_log->error("Settings must be valid before running");
		return false;
	}

	// Setup various different types of content outputs
	if( m_text_words || m_json_words || m_json_regions  ||m_content_backend ) {
		PageRenderer *content_renderer = m_content_backend(&m_ctx);
		WordOutput *words_text = NULL;
		WordOutput *words_json = NULL;
		RegionOutput *regions_json = NULL;
		if( m_text_words ) {
			words_text = new WordsToText();
			content_renderer->addOutput(words_text);
		}
		if( m_json_words ) {
			words_json = new WordsToJson();
			content_renderer->addOutput(words_json);
		}
		if( m_json_regions ) {
			regions_json = new RegionsToJson();
			content_renderer->addOutput(regions_json);
		}
		success = content_renderer->run();
		if( words_text ) delete words_text;
		if( words_json ) delete words_json;
		if( regions_json ) delete regions_json;

		if( ! success ) {
			m_log->warn("Content rendering failed");
			return false;
		}
	}

	// Image output enabled, hookup image profiles to image rendering backend
	if( m_images && m_image_backend ) {		
		PageRenderer *image_renderer = m_image_backend(&m_ctx);
		ImageProfile *profile = m_profiles;
		while( profile ) {
			image_renderer->addOutput(profile->getOutput());
			profile = profile->getNext();
		}
		success = image_renderer->run();
		if( ! success ) {
			m_log->warn("Image rendering failed");
			return false;
		}
	}

	return true;
}

void
RenderDriver::setSupersample(float amount) {
	m_supersample = amount;
	m_validated = false;
}

void
RenderDriver::setFirstPage(int pageno) {
	m_ctx.first_page = pageno;
	m_validated = false;
}

void
RenderDriver::setLastPage(int pageno) {
	m_ctx.last_page = pageno;
	m_validated = false;
}

void
RenderDriver::setOutputDir(const char *out_dir) {
	if( m_ctx.out_dir ) ::free(m_ctx.out_dir);
	m_ctx.out_dir = ::strdup(out_dir);
	m_validated = false;
}

void
RenderDriver::setPdfFile(const char *pdf_file) {
	if( m_ctx.pdf_file ) ::free(m_ctx.pdf_file);
	m_ctx.pdf_file = ::strdup(pdf_file);
	m_validated = false;
}

void
RenderDriver::setBoundingBox(const char *box) {
	m_ctx.use_trimbox = false;
	m_ctx.use_cropbox = false;
	m_ctx.use_mediabox = false;
	m_ctx.use_bleedbox = false;
	if( ::strcmp("trim", box) == 0 ) {
		m_ctx.use_trimbox = true;
	}
	else if( ::strcmp("crop", box) == 0 ) {
		m_ctx.use_cropbox = true;
	}
	else if( ::strcmp("media", box) == 0 ) {
		m_ctx.use_mediabox = true;
	}
	else if( ::strcmp("bleed", box) == 0 ) {
		m_ctx.use_bleedbox = true;
	}
	m_validated = false;
}

void
RenderDriver::setUserPassword(const char *user_password) {
	if( m_ctx.user_password ) ::free(m_ctx.user_password);
	m_ctx.user_password = ::strdup(user_password);
	m_validated = false;
}

void
RenderDriver::setOwnerPassword(const char *owner_password) {
	if( m_ctx.owner_password ) ::free(m_ctx.owner_password);
	m_ctx.owner_password = ::strdup(owner_password);
	m_validated = false;
}

void
RenderDriver::enableTextWords(bool enabled) {
	m_text_words = enabled;
	m_validated = false;
}

void
RenderDriver::enableJsonWords(bool enabled) {
	m_json_words = enabled;
	m_validated = false;
}

void
RenderDriver::enableJsonRegions(bool enabled) {
	m_json_regions = enabled;
	m_validated = false;
}

void
RenderDriver::enableImages(bool enabled) {
	m_images = enabled;
	m_validated = false;
}