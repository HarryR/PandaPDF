#pragma implementation "internal.hh"
#include "internal.hh"
#include "asprintf.h"
#include "utils.hh"
#include "pdfinfo.hh"

#include <cstdlib>
#include <cassert>

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

// -----------------------------------------------------------------------------

Bitmap::~Bitmap() {}

uint32_t
BitmapPixel::toARGB() {
  return ((uint32_t)0xff << 24) |
         ((uint32_t)red << 16) |
         ((uint32_t)green <<  8) |
         ((uint32_t)blue);
}

Output::Output() {}

Output::~Output() {}

WordOutput::WordOutput() {}

RegionOutput::RegionOutput() {}

ImageOutput::ImageOutput() {}

// -----------------------------------------------------------------------------

RenderContext::RenderContext(Logger *logger)
: log(logger), out_dir(NULL), pdf_file(NULL),
  owner_password(NULL), user_password(NULL),
  max_width(-1), max_height(-1),
  use_trimbox(false), use_cropbox(false),
  use_mediabox(false), use_bleedbox(false),
  first_page(-1), last_page(-1)
{ }

RenderContext::~RenderContext() {
  if( out_dir ) ::free(out_dir);
  if( pdf_file ) ::free(pdf_file);
  if( owner_password ) ::free(owner_password);
  if( user_password ) ::free(user_password);
}

bool
RenderContext::validate() {
  if( out_dir == NULL ) {
    log->warn("No output directory specified");
    return false;
  }

  if( ! is_dir(out_dir) ) {
    log->warn("Specified output directory '%s' is not a directory", out_dir);
    return false;
  }

  if( pdf_file == NULL ) {
    log->warn("No PDF file specified");
    return false;
  }

  if( ! file_exists(pdf_file) ) {
    log->warn("Specified PDF file '%s' does not exist", pdf_file);
    return false;
  }

  int doc_pages;
  if( false == get_pdf_page_count(pdf_file, user_password, owner_password, &doc_pages) ) {
    log->error("Unable to open PDF file '%s'", pdf_file);
    return false;
  }

  if( first_page == -1 ) {
    first_page = 1;
  }

  if( last_page == -1 ) {
    last_page = doc_pages;
  }

  if( first_page > last_page ) {
    log->warn("First page (%d) after last page (%d)", first_page, last_page);
    return false;
  }
  if( last_page > doc_pages ) {
    log->error("Last page beyond PDF page count (%d)", doc_pages);
    return false;
  }

  return true;
}

char *
RenderContext::path( const char *filename, ... ) {
  va_list args;
  char *out = NULL;
  char *formatted_filename = NULL;
  assert( out_dir != NULL );

  ::va_start(args, filename);
  if( -1 == ::vasprintf(&formatted_filename, filename, args) ) {
    return NULL;
  }
  ::va_end(args);

  if( -1 == ::asprintf(&out, "%s/%s", out_dir, formatted_filename) ) {
    out = NULL;
  }
  ::free(formatted_filename);
  return out;
}

// -----------------------------------------------------------------------------

PageRenderer::PageRenderer(RenderContext *c)
: m_pageno(-1), m_ctx(c)
{ }

PageRenderer::~PageRenderer() {}

void
PageRenderer::addOutput( RegionOutput *out ) {
  if( this->doesRegions() )  
  m_out_region.push_back(out);
}

void
PageRenderer::addOutput( ImageOutput *out ) {
  if( this->doesImages() ) 
  m_out_image.push_back(out);
}

void
PageRenderer::addOutput( WordOutput *out ) {
  if( this->doesWords() )
  m_out_word.push_back(out);
}

/* Notify outputs that page is starting */
void
PageRenderer::startPage( int pageno ) {
  assert( m_pageno == -1 );
  m_pageno = pageno;

  std::vector<ImageOutput*>::iterator it_io = m_out_image.begin();
  while( it_io != m_out_image.end() )
    (*it_io++)->startPage(m_ctx, pageno);

  std::vector<RegionOutput*>::iterator it_ro = m_out_region.begin();
  while( it_ro != m_out_region.end() )
    (*it_ro++)->startPage(m_ctx, pageno);

  std::vector<WordOutput*>::iterator it_wo = m_out_word.begin();
  while( it_wo != m_out_word.end() )
    (*it_wo++)->startPage(m_ctx, pageno);
}

/* Notify outputs that page is finishing */
void
PageRenderer::finishPage( int pageno ) {
  assert( m_pageno != -1 );
  
  std::vector<ImageOutput*>::iterator it_io = m_out_image.begin();
  while( it_io != m_out_image.end() )
    (*it_io++)->finishPage(m_ctx, pageno);

  std::vector<RegionOutput*>::iterator it_ro = m_out_region.begin();
  while( it_ro != m_out_region.end() )
    (*it_ro++)->finishPage(m_ctx, pageno);

  std::vector<WordOutput*>::iterator it_wo = m_out_word.begin();
  while( it_wo != m_out_word.end() )
    (*it_wo++)->finishPage(m_ctx, pageno);

  m_pageno = -1;
}

void
PageRenderer::write( const Bitmap *bitmap ) {
    std::vector<ImageOutput*>::iterator it = m_out_image.begin();
    while( it != m_out_image.end() ) {
        (*it++)->write(bitmap);
    }
}

void
PageRenderer::write( const Region *region ) {
    std::vector<RegionOutput*>::iterator it = m_out_region.begin();
    while( it != m_out_region.end() )
        (*it++)->write(region);
}

void
PageRenderer::write( const Word *word ) {
    std::vector<WordOutput*>::iterator it = m_out_word.begin();
    while( it != m_out_word.end() )
        (*it++)->write(word);
}

RenderContext *
PageRenderer::ctx() {
    return m_ctx;
}
