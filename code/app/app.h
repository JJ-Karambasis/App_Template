#ifndef APP_H
#define APP_H

struct app;

#include <core.h>
#include <modules.h>
#include <app_platform.h>
#include <app_renderer.h>

#define APP_UPDATE(name) void name(app* App)
typedef APP_UPDATE(app_update);

struct app {
    app_update*   Update;
    app_platform* Platform;
    app_renderer* Renderer;
    arena         Arena;
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