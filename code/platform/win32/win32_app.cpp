#include "win32_app.h"
#include "win32_shared.cpp"

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

int WINAPI WinMain(HINSTANCE HInstance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow) {
    core::Create();

    win32_context Win32Context = {};
    Win32Context.Arena = arena(allocator::Get_Default());

    WNDCLASSEXW WindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_OWNDC,
        .lpfnWndProc = Win32_Event_Proc,
        .hInstance = HInstance,
        .lpszClassName = L"App"
    };

    RegisterClassExW(&WindowClass);

    //TODO: Provide accurate game name for the window here
    HWND MainWindow = Win32_Create_Window(WindowClass, 1920, 1080, L"App");

    shared_library* RendererLibrary = Create_Shared_Library(&Win32Context.Arena, String_Lit("app-renderer"));
    app_renderer_create* App_Renderer_Create = (app_renderer_create*)RendererLibrary->Get_Function("App_Renderer_Create");

    win32_window_data WindowData = {
        .Instance = HInstance,
        .Window = MainWindow
    };

    app_renderer* Renderer = App_Renderer_Create(core::Get(), &WindowData);

    shared_library* AppLibrary = Create_Shared_Library(&Win32Context.Arena, String_Lit("app"));
    app_create* App_Create = (app_create*)AppLibrary->Get_Function("App_Create");

    app* App = App_Create(core::Get(), &Win32Context, Renderer);

    for(;;) {
        MSG Message;
        while(PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE)) {
            switch(Message.message) {
                case WM_QUIT: {
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

#include <core.cpp>