#include <cstdio>
#include <unistd.h>
#include <string.h>

#include "render-driver.hh"
#include "argparser.hh"
#include "logger.hh"
#include "version.h"

// Copyright (c) 2013 PixelMags Inc. All Rights Reserved

static void
show_help(Logger *log, ArgParser *args) {
    log->println("Usage: pandapdf <-options> [<name,type,size> ...]");
    log->println("");
    log->println("Options:");
    args->help();
    log->println("Image Backends, set with '-image-backend':");
    log->println(" - poppler-cairo (default)");
    log->println(" - poppler-splash");
    log->println(" - ghostscript");
    #ifdef _WIN32
    log->println(" - photoshop");
    #endif
    log->println("");
    log->println("Examples:");
    log->println("  $ pandapdf -pdf test.pdf -out dir -json-regions large,jpeg@80,1500 medium,webp@90,720");
    log->println("");
    log->println("Versions:");
    log->println(" PandaPDF   : %s (c) 2009-2013 PixelMags Inc.", PANDAPDF_VERSION);
    log->println(" Poppler    : %s", LIB_VER_POPPLER);
    log->println(" Cairo      : %s", LIB_VER_CAIRO);
    log->println(" Freetype   : %s", LIB_VER_FREETYPE);
    log->println(" FontConfig : %s", LIB_VER_FONTCONFIG);
    log->println(" Pixman     : %s", LIB_VER_PIXMAN);
    log->println(" libJBIG    : %s", LIB_VER_JBIG);
    log->println(" libPNG     : %s", LIB_VER_PNG);
    log->println(" libJPEG    : %s", LIB_VER_JPEG);
    log->println(" libTIFF    : %s", LIB_VER_TIFF);
    log->println(" libWEBP    : %s", LIB_VER_WEBP);
}

static void
setup_args( ArgParser *args ) {
    args->add("supersample", ARG_FLOAT, "Supersample at N times resolution [default 1.5]");
    //args->add("content-backend", ARG_STR, "Which backend to use");
    args->add("image-backend", ARG_STR, "Which backend to use");
    args->add("last", ARG_INT, "Last Page");
    args->add("first", ARG_INT, "First Page");    
    args->add("upw", ARG_STR, "User/Open Password");
    args->add("opw", ARG_STR, "Owner/Modify Password");
    args->add("text-words", ARG_BOOL, "Enable text words");
    args->add("json-words", ARG_BOOL, "Enable JSON words");
    args->add("json-regions", ARG_BOOL, "Enable JSON regions");
    args->add("no-images", ARG_BOOL, "Disable Image Output");
    args->add("box", ARG_STR, "Which PDF box to use [crop,trim,media]");
    args->add("out", ARG_STR, "Output Directory");
    args->add("pdf", ARG_STR, "PDF File");
    args->add("quiet", ARG_BOOL, "Log only warnings & errors");
    args->add("debug", ARG_BOOL, "Enable debug logging");
}

static void
apply_args( Logger *logger, ArgParser *args, RenderDriver *driver ) {
    driver->setImageBackend("poppler-cairo");
    driver->setContentBackend("poppler-text");
    driver->enableImages(true);
    driver->setBoundingBox("trim");

    if( args->has("quiet") ) logger->set_level(LOG_WARN);
    if( args->has("debug") ) logger->set_level(LOG_DEBUG);

    if( args->has("first") ) driver->setFirstPage(args->getInt("first"));
    if( args->has("last") ) driver->setLastPage(args->getInt("last"));

    if( args->has("out") ) driver->setOutputDir(args->getString("out"));
    if( args->has("pdf") ) driver->setPdfFile(args->getString("pdf"));
    if( args->has("box") ) driver->setBoundingBox(args->getString("box"));    

    if( args->has("image-backend") ) driver->setImageBackend(args->getString("image-backend"));
    if( args->has("upw") ) driver->setUserPassword(args->getString("upw"));
    if( args->has("opw") ) driver->setOwnerPassword(args->getString("opw"));
    if( args->has("text-words") ) driver->enableTextWords(true);
    if( args->has("json-words") ) driver->enableJsonWords(true);
    if( args->has("json-regions") ) driver->enableJsonRegions(true);
    if( args->has("no-images") ) driver->enableImages(false);        
}

int
main( int argc, char **argv ) {
    Logger logger(stdout, stderr, LOG_INFO);
    RenderDriver render(&logger);
    ArgParser args(&logger);
    int profiles_offset;

    // Setup & parse args
    setup_args(&args);
    profiles_offset = args.parse(argc, argv);
    apply_args(&logger, &args, &render);
    
    // Setup image profiles
    while( profiles_offset < argc ) {
        if( ! render.addImageProfile(argv[profiles_offset]) ) {
            logger.warn("Cannot parse profile '%s'", argv[profiles_offset]);
        }
        profiles_offset++;
    }
    
    if( ! render.validate() ) {
        show_help(&logger, &args);
        return 1;
    }

    return render.run() ? 0 : 1;
}
