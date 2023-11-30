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

                Context->AudioStream->requestStart();
            }
        } break;
    }
}

struct android_audio_callback : public AudioStreamDataCallback {
    android_context* Android;

    DataCallbackResult onAudioReady(AudioStream *AudioStream, void *AudioData, int32_t NumFrames) {
        
        scratch Scratch = Get_Scratch();
        sound_samples_f32 Samples = Sound_Allocate_Samples(&Scratch, NumFrames);

        app* App = Android->App;
        if(App) {
            App->Mix_Samples(App, &Samples);
        }

        f32* LeftChannelAt = Samples.LeftChannel;
        f32* RightChannelAt = Samples.RightChannel;
        s16* OutputAudio = (s16*)AudioData;
        for(u32 FrameIndex = 0; FrameIndex < NumFrames; FrameIndex++) {
            *OutputAudio++ = SNorm_S16(*LeftChannelAt++);
            *OutputAudio++ = SNorm_S16(*RightChannelAt++);
        }
    
        return DataCallbackResult::Continue;
    }
};

void android_main(struct android_app* App) {
    core::Create(App->activity);
    
    android_context AndroidContext = {};

    App->userData = &AndroidContext;
    App->onAppCmd = Android_On_App_Cmd;
    App->onInputEvent = Android_Event_Proc;

    android_audio_callback AudioCallback;
    AudioCallback.Android = &AndroidContext;

    AudioStreamBuilder AudioStreamBuilder;

    AudioStreamBuilder.setDataCallback(&AudioCallback)
        ->setPerformanceMode(PerformanceMode::PowerSaving)
        ->setFormat(AudioFormat::I16)
        ->setSharingMode(SharingMode::Shared)
        ->setSampleRate(SOUND_SAMPLES_PER_SEC)
        ->setChannelCount(SOUND_NUM_CHANNELS)
        ->setBufferCapacityInFrames(SOUND_SAMPLES_PER_SEC); //One second sound buffer

    Result AudioResult = AudioStreamBuilder.openStream(&AndroidContext.AudioStream);
    if(AudioResult != Result::OK) {
        Log_Error(modules::Android, "Failed to create the android audio stream. Error: %s", convertToText(AudioResult));
        return;
    }

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

#include <app_shared.cpp>
#include <core.cpp>