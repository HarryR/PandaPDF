#pragma once
#ifndef GHOSTSCRIPT_BACKEND_HH
#define GHOSTSCRIPT_BACKEND_HH

#pragma interface

#include "internal.hh"

/** Open backend which renders using Poppler+CairoOutputDev */
PageRenderer *backend_ghostscript(RenderContext *ctx);

#endif
