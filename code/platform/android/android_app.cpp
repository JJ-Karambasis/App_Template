#include "android_app.h"
#include "android_shared.cpp"

static s32 Android_Event_Proc(android_app* App, AInputEvent* Event) {
    return 0;
}

static void Android_On_App_Cmd(android_app* App, s32 Cmd) {
    android_context* Context = (android_context*)App->userData;

    switch(Cmd) {
        case APP_CMD_INIT_WINDOW: {
            if(App->window != nullptr && !Context->Window) {
                Context->Window = App->window;
                Context->Arena = arena(allocator::Get_Default());

                Context->RendererLibrary = Create_Shared_Library(&Context->Arena, String_Lit("libapp-renderer"));
                app_renderer_create* App_Renderer_Create = (app_renderer_create*)Context->RendererLibrary->Get_Function("App_Renderer_Create");

                android_window_data WindowData = {
                    .Window = Context->Window
                };

                Context->Renderer = App_Renderer_Create(core::Get(), &WindowData);

                Context->AppLibrary = Create_Shared_Library(&Context->Arena, String_Lit("libapp"));
                app_create* App_Create = (app_create*)Context->AppLibrary->Get_Function("App_Create");
                
                Context->App = App_Create(core::Get(), Context, Context->Renderer);
            }
        } break;
    }
}

void android_main(struct android_app* App) {
    core::Create(App->activity->vm, App->activity->env, App->activity->assetManager, App->activity->internalDataPath);
    
    android_context AndroidContext = {};

    App->userData = &AndroidContext;
    App->onAppCmd = Android_On_App_Cmd;
    App->onInputEvent = Android_Event_Proc;

    for(;;) {
        if(!Android_Poll_App(App)) {
            //End app
            return;
        }

        app* App = AndroidContext.App;
        if(App) {
            App->Update(App);
        }

        app_renderer* Renderer = AndroidContext.Renderer;
        if(Renderer) {
            Renderer->Render(Renderer, App);
        }
    }
}

#include <core.cpp>