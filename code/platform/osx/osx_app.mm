#include "osx_app.h"
#include "osx_shared.mm"

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
    app_renderer_create* App_Renderer_Create = (app_renderer_create*)RendererLibrary->Get_Function("App_Renderer_Create");

    osx_window_data WindowData = {
        .Layer = (CAMetalLayer*)[[Window contentView] layer]
    };

    app_renderer* Renderer = App_Renderer_Create(core::Get(), &WindowData);
    
    shared_library* AppLibrary = Create_Shared_Library(&OSXContext.Arena, String_Lit("libapp"));
    app_create* App_Create = (app_create*)AppLibrary->Get_Function("App_Create");

    app* App = App_Create(core::Get(), &OSXContext, Renderer);

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
            return 0;
        }
        
        App->Update(App);
        Renderer->Render(Renderer, App);
    }
}

int main(int ArgCount, char* Args[]) {
    core::Create();

    int Result = 0;
    Result = OSX_Run_Game();
    
    return Result;
}

#include <core.cpp>
