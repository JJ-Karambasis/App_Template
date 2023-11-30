#include "app.h"

APP_MIX_SAMPLES(App_Mix_Samples) {
    static u64 PlayingSampleIndex;
    static sound_samples_f32 TestSound;

    if(TestSound.SampleCount == 0) {
        Sound_Load_Samples(&TestSound, allocator::Get_Default(), String_Lit("audio/TestMusic.wav"));
    }

    f32 Volume = 1.0f;
    for(u32 SampleIndex = 0; SampleIndex < OutSamples->SampleCount; SampleIndex++) {
        OutSamples->LeftChannel[SampleIndex] = TestSound.LeftChannel[PlayingSampleIndex]*Volume;
        OutSamples->RightChannel[SampleIndex] = TestSound.RightChannel[PlayingSampleIndex]*Volume;

        PlayingSampleIndex++;
        if(PlayingSampleIndex >= TestSound.SampleCount) {
            PlayingSampleIndex = 0;
        }
    }
}

APP_UPDATE(App_Update) {

}

extern "C" shared_function APP_RELOAD(App_Reload) {
    core::Set(Core);
    App->Update = App_Update;
    App->Mix_Samples = App_Mix_Samples;
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

#include "app_shared.cpp"
#include <core.cpp>