#ifndef APP_RENDERER_H
#define APP_RENDERER_H

struct app_renderer;

#define APP_RENDERER_RENDER(name) void name(app_renderer* _Renderer, app* App)
typedef APP_RENDERER_RENDER(app_renderer_render);

struct app_renderer {
    app_renderer_render* Render;
};

#if defined(OS_WIN32)
struct win32_window_data {
    HINSTANCE Instance;
    HWND Window;
};
#elif defined(OS_ANDROID)
struct android_window_data {
    ANativeWindow* Window;
};
#elif defined(OS_OSX)

#ifdef __OBJC__
@class CAMetalLayer;
#else
typedef void CAMetalLayer;
#endif

struct osx_window_data {
    CAMetalLayer* Layer;
};
#else
#error Not Implemented
#endif

#define APP_RENDERER_RELOAD(name) void name(app_renderer* _Renderer, core* Core)
#define APP_RENDERER_CREATE(name) app_renderer* name(core* Core, void* WindowData)

typedef APP_RENDERER_RELOAD(app_renderer_reload);
typedef APP_RENDERER_CREATE(app_renderer_create);

#ifndef APP_NO_PROTOTYPES
extern "C" shared_function APP_RENDERER_RELOAD(App_Renderer_Reload);
extern "C" shared_function APP_RENDERER_CREATE(App_Renderer_Create);
#endif

#endif
