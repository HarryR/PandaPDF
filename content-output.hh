#pragma once
#ifndef CONTENT_OUTPUT_HH
#define CONTENT_OUTPUT_HH

#pragma interface
#include "internal.hh"

#include "json.h"

// -----------------------------------------------------------------------------

/**
 * Writes all words to page_NNNN_words.txt
 */
class WordsToText
: public WordOutput {
public:
    WordsToText();
    virtual ~WordsToText();

    virtual char *filename( RenderContext *ctx, int pageno );
    virtual void write( const Word *word );
    virtual bool startPage( RenderContext *ctx, int pageno );
    virtual bool finishPage( RenderContext *ctx, int pageno );

private:
    int m_wordcount;
    int m_pageno;
    FILE *m_page_fd;
    RenderContext *m_ctx;
};

/**
 * Writes all words to page_NNNN_words.json
 */
class RegionsToJson
: public RegionOutput {
public:
    RegionsToJson();
    virtual ~RegionsToJson();

    virtual char *filename( RenderContext *ctx, int pageno );
    virtual void write( const Region *region );
    virtual bool startPage( RenderContext *ctx, int pageno ) ;
    virtual bool finishPage( RenderContext *ctx, int pageno );

private:
    int m_regioncount;
    int m_pageno;
    FILE *m_page_fd;
    json_printer m_json;
    RenderContext *m_ctx;
};

/**
 * Writes all words to page_NNNN_words.json
 */
class WordsToJson
: public WordOutput {
public:
    WordsToJson();
    virtual ~WordsToJson();

    virtual char *filename( RenderContext *ctx, int pageno );
    virtual void write( const Word *word );
    virtual bool startPage( RenderContext *ctx, int pageno );
    virtual bool finishPage( RenderContext *ctx, int pageno );

private:
    int m_wordcount;
    int m_pageno;
    FILE *m_page_fd;
    json_printer m_json;
    RenderContext *m_ctx;
};

#endif
