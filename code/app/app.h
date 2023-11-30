#ifndef APP_H
#define APP_H

struct app;

#include <core.h>
#include <modules.h>

#include "sound.h"

#include <app_platform.h>
#include <app_renderer.h>

#define APP_UPDATE(name) void name(app* App)
#define APP_MIX_SAMPLES(name) void name(app* App, sound_samples_f32* OutSamples)
typedef APP_UPDATE(app_update);
typedef APP_MIX_SAMPLES(app_mix_samples);

struct app {
    app_update*      Update;
    app_mix_samples* Mix_Samples;
    app_platform*    Platform;
    app_renderer*    Renderer;
    arena            Arena;
};

#define APP_RELOAD(name) void name(app* App, core* Core)
#define APP_CREATE(name) app* name(core* Core, app_platform* Platform, app_renderer* Renderer)

typedef APP_RELOAD(app_reload);
typedef APP_CREATE(app_create);

#ifndef APP_NO_PROTOTYPES
extern "C" shared_function APP_RELOAD(App_Reload);
extern "C" shared_function APP_CREATE(App_Create);
#endif

#endif