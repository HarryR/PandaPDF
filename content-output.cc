#pragma implementation "content-output.hh"
#include "content-output.hh"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cerrno>

#include "json-overload.hh"
#include "asprintf.h"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

// -----------------------------------------------------------------------------

WordsToText::WordsToText()
: WordOutput()
, m_wordcount(-1)
, m_pageno(-1)
, m_page_fd(NULL)
, m_ctx(NULL)
{ }

char *
WordsToText::filename( RenderContext *ctx, int pageno ) {
    assert( ctx != NULL );
    assert( pageno > 0 );
    return ctx->path("page_%04d_words.txt", pageno);
}

void
WordsToText::write( const Word *word ) {
    char *buf = NULL;
    int buf_sz = ::asprintf(&buf, "%s\n", word->text);
    if( buf_sz != -1 ) {
        ::fwrite(buf, buf_sz, 1, m_page_fd);
        ::free(buf);
        m_wordcount++;
    }
}

WordsToText::~WordsToText() {
    assert( m_pageno == -1 );
    assert( m_page_fd == NULL );
    assert( m_ctx == NULL );
}

bool
WordsToText::startPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    assert( m_page_fd == NULL );
    char *path = filename(ctx, pageno);
    assert( path != NULL );
    m_page_fd = ::fopen(path, "wb");
    if( m_page_fd == NULL ) {
        ctx->log->warn("Cannot open file '%s' for writing: %s", path, ::strerror(errno));
        return false;
    }
    m_ctx = ctx;
    m_pageno = pageno;
    m_wordcount = 0;
    return true;
}

bool
WordsToText::finishPage( RenderContext *ctx, int pageno ) {
    assert( ctx == m_ctx );
    assert( m_pageno == pageno );
    assert( m_page_fd != NULL );
    ctx->log->info("Page %d: Words to .txt - %d words", pageno, m_wordcount);
    ::fclose(m_page_fd);
    m_ctx = NULL;
    m_page_fd = NULL;
    m_pageno = -1;
    m_wordcount = -1;
    return true;
}

// -----------------------------------------------------------------------------

static int
jsonprint_file_callback( void *userdata, const char *s, uint32_t length ) {
    FILE *fh = (FILE*)userdata;
    ::fwrite(s, length, 1, fh);
    return 0;
}


WordsToJson::WordsToJson()
: WordOutput()
, m_wordcount(-1)
, m_pageno(-1)
, m_page_fd(NULL)
, m_ctx(NULL)
{ }

void
WordsToJson::write( const Word *word ) {
    m_wordcount++;
    json_print_raw(&m_json, JSON_OBJECT_BEGIN, NULL, 0);
        json_print_key(&m_json, "position");
            json_print_raw(&m_json, JSON_ARRAY_BEGIN, NULL, 0);
                json_print(&m_json, word->box[0]);
                json_print(&m_json, word->box[1]);
                json_print(&m_json, word->box[2]);
                json_print(&m_json, word->box[3]);
            json_print_raw(&m_json, JSON_ARRAY_END, NULL, 0);
        json_print_key(&m_json, "text", word->text);
    json_print_raw(&m_json, JSON_OBJECT_END, NULL, 0);
}

char *
WordsToJson::filename(  RenderContext *ctx, int pageno  ) {
    assert( ctx != NULL );
    assert( pageno > 0 );
    return ctx->path("page_%04d_words.json", pageno);
}

bool
WordsToJson::startPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    assert( m_page_fd == NULL );
    char *path = filename(ctx, pageno);
    assert( path != NULL );

    m_page_fd = ::fopen(path, "wb");
    if( m_page_fd == NULL ) {
        ctx->log->warn("Cannot open file '%s' for writing: %s", path, ::strerror(errno));
        return false;
    }
    m_ctx = ctx;
    m_pageno = pageno;
    m_wordcount = 0;

    json_print_init(&m_json, jsonprint_file_callback, m_page_fd);
    json_print_raw(&m_json, JSON_ARRAY_BEGIN, NULL, 0);
    return true;
}

bool
WordsToJson::finishPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == ctx );
    assert( m_pageno == pageno );
    assert( m_page_fd != NULL );

    json_print_raw(&m_json, JSON_ARRAY_END, NULL, 0);
    json_print_free(&m_json);

    ctx->log->info("Page %d: Words to .json - %d words", pageno, m_wordcount);

    ::fclose(m_page_fd);
    m_page_fd = NULL;
    m_pageno = -1;
    m_ctx = NULL;
    m_wordcount = -1;
    return true;
}

WordsToJson::~WordsToJson() {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    assert( m_page_fd == NULL );
}

// -----------------------------------------------------------------------------


RegionsToJson::RegionsToJson()
: RegionOutput()
, m_regioncount(-1)
, m_pageno(-1)
, m_page_fd(NULL)
, m_ctx(NULL)
{ }

void
RegionsToJson::write( const Region *region ) {
    m_regioncount++;
    json_print_raw(&m_json, JSON_OBJECT_BEGIN, NULL, 0);
        json_print_key(&m_json, "x1", region->box[0]);
        json_print_key(&m_json, "y1", region->box[1]);
        json_print_key(&m_json, "x2", region->box[2]);
        json_print_key(&m_json, "y2", region->box[3]);

        json_print_key(&m_json, "region_type", region->type);
        if( region->title != NULL ) {
            json_print_key(&m_json, "title", region->title);
        }
        json_print_key(&m_json, "url", region->url);
    json_print_raw(&m_json, JSON_OBJECT_END, NULL, 0);
}

char *
RegionsToJson::filename( RenderContext *ctx, int pageno ) {
    assert( ctx != NULL );
    assert( pageno > 0 );
    return ctx->path("page_%04d_regions.json", pageno);
}

bool
RegionsToJson::startPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    assert( m_page_fd == NULL );
    char *path = filename(ctx, pageno);
    assert( path != NULL );

    m_page_fd = ::fopen(path, "wb");
    if( m_page_fd == NULL ) {
        ctx->log->warn("Cannot open file '%s' for writing: %s", path, ::strerror(errno));
        return false;
    }
    m_pageno = pageno;
    m_ctx = ctx;
    m_regioncount = 0;

    json_print_init(&m_json, jsonprint_file_callback, m_page_fd);
    json_print_raw(&m_json, JSON_ARRAY_BEGIN, NULL, 0);
    return true;
}

bool
RegionsToJson::finishPage( RenderContext *ctx, int pageno ) {
    assert( m_ctx == ctx );
    assert( m_pageno == pageno );
    assert( m_page_fd != NULL );

    ctx->log->info("Page %d: Regions to .json - %d regions", pageno, m_regioncount);

    json_print_raw(&m_json, JSON_ARRAY_END, NULL, 0);
    json_print_free(&m_json);

    ::fclose(m_page_fd);
    m_page_fd = NULL;
    m_pageno = -1;
    m_ctx = NULL;
    m_regioncount = -1;
    return true;
}

RegionsToJson::~RegionsToJson() {
    assert( m_ctx == NULL );
    assert( m_pageno == -1 );
    assert( m_page_fd == NULL );
}
