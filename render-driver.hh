#pragma once
#ifndef RENDER_DRIVER_HH
#define RENDER_DRIVER_HH

#pragma interface

#include "internal.hh"
#include "logger.hh"

class AvailableRenderer;
class ImageProfile;

class RenderDriver {
public:
	RenderDriver(Logger *log);
	~RenderDriver();

	bool validate();
	bool setImageBackend( const char *name );
	bool setContentBackend( const char *name );
	bool addImageProfile( const char *profile );

	void setSupersample(float amount);
	void setFirstPage(int pageno);
	void setLastPage(int pageno);
	void setOutputDir(const char *out_dir);
	void setPdfFile(const char *pdf_file);
	void setBoundingBox(const char *box);
	void setUserPassword(const char *user_password);
	void setOwnerPassword(const char *owner_password);
	void enableTextWords(bool enabled = true);
	void enableJsonWords(bool enabled = true);
	void enableJsonRegions(bool enabled = true);
	void enableImages(bool enabled = true);

	bool run();

private:
	float m_supersample;
	bool m_text_words;
	bool m_json_words;
	bool m_json_regions;
	bool m_images;
	RenderContext m_ctx;
	Logger *m_log;
	bool m_validated;
	ImageProfile *m_profiles;
	AvailableRenderer *m_content_backends;
	PageRendererFactory m_content_backend;
	AvailableRenderer *m_image_backends;
	PageRendererFactory m_image_backend;
};

#endif
