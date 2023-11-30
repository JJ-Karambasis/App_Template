#include "osx_app.h"
#include "osx_shared.mm"

static atom32 G_Running;

@implementation osx_window_delegate
- (BOOL)windowShouldClose:(NSWindow*)window {
    if(window == _mainWindow) {
        _isMainWindowClosed = TRUE;
    }
    return TRUE;
}

- (void)setMainWindow:(id)window {
    _mainWindow = window;
    _isMainWindowClosed = FALSE;
}
@end

int OSX_Run_Game() {
    osx_context OSXContext = {};
    OSXContext.Arena = arena(allocator::Get_Default());

    osx_window_delegate* WindowDelegate = [[osx_window_delegate alloc] init];
    NSWindow* Window = OSX_Create_Window(1920, 1080, @"App", WindowDelegate);
    [WindowDelegate setMainWindow:Window];

    shared_library* RendererLibrary = Create_Shared_Library(&OSXContext.Arena, String_Lit("libapp-renderer"));
    if(!RendererLibrary) {
        //TODO: Message box output for user. Inform them that libapp-renderer.so is not available in the application directory
        Log_Error(modules::OSX, "Failed to load the renderer library libapp-renderer.so");
        return 1;
    }
    
    app_renderer_create* App_Renderer_Create = (app_renderer_create*)RendererLibrary->Get_Function("App_Renderer_Create");
    Assert(App_Renderer_Create, "No valid export function for app-renderer. Must export App_Renderer_Create!");

    osx_window_data WindowData = {
        .Layer = (CAMetalLayer*)[[Window contentView] layer]
    };

    app_renderer* Renderer = App_Renderer_Create(core::Get(), &WindowData);
    if(!Renderer) {
        //TODO: Message box output for user. Inform them that the renderer could not be initialized. Perhaps get more 
        //info from the renderer, otherwise the logger will get more info
        return 1;
    }
    
    shared_library* AppLibrary = Create_Shared_Library(&OSXContext.Arena, String_Lit("libapp"));
    if(!AppLibrary) {
        //TODO: Message box output for user. Inform them that libapp.so is not available in the application directory
        Log_Error(modules::OSX, "Failed to load the app library libapp.so");
        return 1;
    }
    
    app_create* App_Create = (app_create*)AppLibrary->Get_Function("App_Create");
    Assert(App_Create, "No valid export function app. Must export App_Create!");

    app* App = App_Create(core::Get(), &OSXContext, Renderer);

    audio_toolbox* AudioToolbox = Audio_Toolbox_Create(&OSXContext.Arena, App);
    if(!AudioToolbox) {
        //TODO: Diagnostic and error logging
        return 1;
    }

    AudioToolbox->Start_Playing();

    for(;;) {
        NSEvent* Event;

        @autoreleasepool {
            do {
                Event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                untilDate: nil
                                inMode: NSDefaultRunLoopMode
                                dequeue: YES];

                switch([Event type]) {
                    default: {
                        [NSApp sendEvent: Event];
                    } break;
                }
            } while(Event != nil);
        }

        if([WindowDelegate isMainWindowClosed]) {
            AudioToolbox->Stop_Playing();
            thread_manager::Wait_All();            
            return 0;
        }
        
        App->Update(App);
        Renderer->Render(Renderer, App);
    }
}

int main(int ArgCount, char* Args[]) {
    core::Create();
    Atomic_Store(&G_Running, true);

    int Result = 0;
    Result = OSX_Run_Game();
    
    return Result;
}

#include <shared/audiotoolbox/audiotoolbox.cpp>
#include <app_shared.cpp>
#include <core.cpp>
