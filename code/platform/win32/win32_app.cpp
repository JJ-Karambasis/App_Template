#include "win32_app.h"
#include "win32_shared.cpp"

static atom32 G_Running;

static s32 Win32_Audio_Thread(thread_context* ThreadContext) {

    win32_context* Win32 = (win32_context*)ThreadContext->UserData;
    win32_dsound* DSound = Win32->DSound;
    app* App = Win32->App;

    IDirectSoundBuffer* SoundBuffer = DSound->Buffer;

    u64 ClockFrequency = Get_Performance_Frequency();

    DSBCAPS Caps = {
        .dwSize = sizeof(DSBCAPS)
    };

    SoundBuffer->GetCaps(&Caps);

    SoundBuffer->Play(0, 0, DSBPLAY_LOOPING);

    u32 LatencyCount = 0;
    DWORD CurrentPosition = 0;

    DWORD LastWriteCursor;
    SoundBuffer->GetCurrentPosition(nullptr, &LastWriteCursor);

    CurrentPosition = LastWriteCursor;
    u32 SamplesPerFrame = Ceil_U32((f64)SOUND_SAMPLES_PER_SEC*WIN32_SOUND_HZ);

    while(Atomic_Load(&G_Running)) {
        u64 StartCounter = Get_Performance_Counter();

        DWORD WriteCursor;
        SoundBuffer->GetCurrentPosition(nullptr, &WriteCursor);

        //First determine if the new write cursor is passed the last playing position
        //There are two cases. One where the write cursor is behind the current position,
        //but the last write cursor is in front of the write cursor. This means the write cursor
        //has wrapped passed the buffer.

        bool AdjustLatency = false;
        if(WriteCursor > CurrentPosition && (CurrentPosition >= LastWriteCursor)) {
            AdjustLatency = true;
        }
        else {
            if((LastWriteCursor > WriteCursor) && (LastWriteCursor < CurrentPosition)) {
                AdjustLatency = true;
            }
        }

        //If we need to adjust the latency from the previous logic, adjust it here
        u32 BytesToWrite = 0;
        if(AdjustLatency) {
            //When we adjust the latency make sure the current position is the write cursor
            //and we need to write the latency amount of audio data into the sound buffer 

            if(LatencyCount < WIN32_SOUND_MAX_LATENCY) {
                LatencyCount++;
            }
            CurrentPosition = WriteCursor;
            BytesToWrite = (SamplesPerFrame*SOUND_NUM_CHANNELS*2)*LatencyCount;

        } else {
            //Otherwise the samples to write is just the difference between the write cursor and last write cursor
            if(LastWriteCursor > WriteCursor) {
                BytesToWrite = (Caps.dwBufferBytes-LastWriteCursor)+WriteCursor;
            } else {
                BytesToWrite = WriteCursor-LastWriteCursor;
            }
        }

        void* RegionBytes[2];
        DWORD RegionByteCount[2];

        SoundBuffer->Lock(CurrentPosition, BytesToWrite, &RegionBytes[0], &RegionByteCount[0], &RegionBytes[1], &RegionByteCount[1], 0);
        
        DWORD SamplesToWrite = BytesToWrite/(SOUND_NUM_CHANNELS*2);

        scratch Scratch = Get_Scratch();
        sound_samples_f32 Samples = Sound_Allocate_Samples(&Scratch, SamplesToWrite);

        App->Mix_Samples(App, &Samples);

        s16* RegionSamples[2] = {(s16*)RegionBytes[0], (s16*)RegionBytes[1]};
        u32  RegionSampleCount[2] = {RegionByteCount[0]/(SOUND_NUM_CHANNELS*2), RegionByteCount[1]/(SOUND_NUM_CHANNELS*2)};

        Assert(RegionSampleCount[0]+RegionSampleCount[1] == SamplesToWrite, "Invalid samples to write!");

        f32* LeftChannelAt = Samples.LeftChannel;
        f32* RightChannelAt = Samples.RightChannel;
        for(u32 RegionIndex = 0; RegionIndex < Array_Count(RegionSamples); RegionIndex++) {
            s16* RegionSample = RegionSamples[RegionIndex];
            for(u32 RegionSampleIndex = 0; RegionSampleIndex < RegionSampleCount[RegionIndex]; RegionSampleIndex++) {                
                *RegionSample++ = SNorm_S16(*LeftChannelAt++);
                *RegionSample++ = SNorm_S16(*RightChannelAt++);
            }  
        }

        SoundBuffer->Unlock(RegionBytes[0], RegionByteCount[0], RegionBytes[1], RegionByteCount[1]);
        CurrentPosition = (CurrentPosition+BytesToWrite) % Caps.dwBufferBytes;

        LastWriteCursor = WriteCursor;

        u64 EndCounter = Get_Performance_Counter();

        f64 Delta = (f64)(EndCounter-StartCounter)/(f64)ClockFrequency;
        if(Delta <= WIN32_SOUND_HZ) {
            f64 RemainingSeconds = WIN32_SOUND_HZ-Delta;
            u32 MillisecondsToSleep = Ceil_U32(RemainingSeconds*1000.0);
            Delay(MillisecondsToSleep);
        } 
    }

    return 0;
}

static bool Win32_Init_DSound(win32_dsound* DSound, HWND Window) {
    DSound->Library = LoadLibraryA("dsound.dll");
    if(!DSound->Library) {
        //TODO: Diagnostic and error logging
        return false;
    }

    direct_sound_create* DirectSoundCreate = (direct_sound_create*)GetProcAddress(DSound->Library, "DirectSoundCreate");

    if(FAILED(DirectSoundCreate(nullptr, &DSound->Context, nullptr))) {
        //TODO: Diagnostic and error logging
        return false;
    }

    if(FAILED(DSound->Context->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
        //TODO: Diagnostic and error logging
        return false;
    }

    WAVEFORMATEX WaveFormat = {
        .wFormatTag = WAVE_FORMAT_PCM,
        .nChannels = SOUND_NUM_CHANNELS,
        .nSamplesPerSec = SOUND_SAMPLES_PER_SEC,
        .nAvgBytesPerSec = SOUND_SAMPLES_PER_SEC*SOUND_NUM_CHANNELS*WIN32_SOUND_BYTES_PER_SAMPLE,
        .nBlockAlign = SOUND_NUM_CHANNELS*WIN32_SOUND_BYTES_PER_SAMPLE,
        .wBitsPerSample = WIN32_SOUND_BYTES_PER_SAMPLE*8,
        .cbSize = sizeof(WAVEFORMATEX)
    };

    DSBUFFERDESC BufferDesc = {
        .dwSize = sizeof(DSBUFFERDESC),
        .dwBufferBytes = WaveFormat.nAvgBytesPerSec, //One second sound buffer
        .lpwfxFormat = &WaveFormat
    };

    if(FAILED(DSound->Context->CreateSoundBuffer(&BufferDesc, &DSound->Buffer, nullptr))) {
        //TODO: Diagnostic and error logging
        return false;
    }

    return true;
}

static LRESULT CALLBACK Win32_Event_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    LRESULT Result = 0;
    
    switch(Message) {
        case WM_CLOSE: {
            PostQuitMessage(0);
        } break;

        default: {
            Result = DefWindowProcW(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

static void Win32_Finish() {
    Atomic_Store(&G_Running, false);
    thread_manager::Wait_All();
}

static int Win32_Run(HINSTANCE Instance) {
    win32_context Win32Context = {};
    Win32Context.Arena = arena(allocator::Get_Default());

    WNDCLASSEXW WindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_OWNDC,
        .lpfnWndProc = Win32_Event_Proc,
        .hInstance = Instance,
        .lpszClassName = L"App"
    };

    RegisterClassExW(&WindowClass);

    //TODO: Provide accurate game name for the window here
    HWND MainWindow = Win32_Create_Window(WindowClass, 1920, 1080, L"App");    

    win32_dsound DSound = {};
    if(!Win32_Init_DSound(&DSound, MainWindow)) {
        //TODO: Diagnostic and error logging
        return false;
    }

    Win32Context.DSound = &DSound;

    shared_library* RendererLibrary = Create_Shared_Library(&Win32Context.Arena, String_Lit("app-renderer"));
    if(!RendererLibrary) {
        //TODO: Message box output for user. Inform them that app-renderer.dll is not available in the application directory
        Log_Error(modules::Win32, "Failed to load the renderer library app-renderer.dll.");
        return 1;
    }

    app_renderer_create* App_Renderer_Create = (app_renderer_create*)RendererLibrary->Get_Function("App_Renderer_Create");
    Assert(App_Renderer_Create, "No valid export function for app-renderer. Must export App_Renderer_Create!");

    win32_window_data WindowData = {
        .Instance = Instance,
        .Window = MainWindow
    };

    app_renderer* Renderer = App_Renderer_Create(core::Get(), &WindowData);
    if(!Renderer) {
        //TODO: Message box output for user. Inform them that the renderer could not be initialized. Perhaps get more 
        //info from the renderer, otherwise the logger will get more info
        return 1;
    }

    shared_library* AppLibrary = Create_Shared_Library(&Win32Context.Arena, String_Lit("app"));
    if(!AppLibrary) {
        //TODO: Message box output for user. Inform them that app.dll is not available in the application directory
        Log_Error(modules::Win32, "Failed to load the app library app.dll.");
        return 1;
    }

    app_create* App_Create = (app_create*)AppLibrary->Get_Function("App_Create");
    Assert(App_Create, "No valid export function app. Must export App_Create!");

    app* App = App_Create(core::Get(), &Win32Context, Renderer);
    Win32Context.App = App;

    thread_manager::Create_Thread(Win32_Audio_Thread, &Win32Context);

    for(;;) {
        MSG Message;
        while(PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE)) {
            switch(Message.message) {
                case WM_QUIT: {
                    Win32_Finish();
                    return 0;
                } break;

                default: {
                    TranslateMessage(&Message);
                    DispatchMessageW(&Message);
                } break;
            }
        }

        App->Update(App);
        Renderer->Render(Renderer, App);
    }
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow) {
    core::Create();
    Atomic_Store(&G_Running, true);

    __try {
        return Win32_Run(Instance);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        //TODO: GetExceptionCode() and try and output a useful error message
        Win32_Finish();
        return 1;
    }
}

#include <app_shared.cpp>
#include <core.cpp>