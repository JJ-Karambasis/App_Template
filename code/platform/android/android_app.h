#ifndef ANDROID_APP_H
#define ANDROID_APP_H

#include <app.h>
#include <android_native_app_glue.h>

struct android_context : public app_platform {
    arena           Arena;
    shared_library* RendererLibrary;
    app_renderer*   Renderer;
    shared_library* AppLibrary;
    app*            App;
    ANativeWindow*  Window;
};

#endif