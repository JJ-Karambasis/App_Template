#include "app.h"

APP_UPDATE(App_Update) {

}

extern "C" shared_function APP_RELOAD(App_Reload) {
    core::Set(Core);
    App->Update = App_Update;
}

extern "C" shared_function APP_CREATE(App_Create) {
    arena Arena(Core->Allocator);
    app* App = new(&Arena) app;
    App->Platform = Platform;
    App->Renderer = Renderer;
    App->Arena    = Arena;
    App_Reload(App, Core);
    return App;
}

#include <core.cpp>