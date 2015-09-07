#pragma once
#ifndef POPPLER_BACKENDS_HH
#define POPPLER_BACKENDS_HH

#pragma interface

#include "internal.hh"

/** Open backend which renders using Poppler+CairoOutputDev */
PageRenderer *backend_poppler_cairo(RenderContext *ctx);

/** Open backend which renders using Poppler+SplashOutputDev */
PageRenderer *backend_poppler_splash(RenderContext *ctx);

/** Open backend which renders using Poppler+TextOutputDev */
PageRenderer *backend_poppler_text(RenderContext *ctx);

void backend_poppler_cleanup();

#endif
