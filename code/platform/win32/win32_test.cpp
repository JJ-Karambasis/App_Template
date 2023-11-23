#include "win32_test.h"
#include "win32_shared.cpp"

int main(int ArgCount, char* Args[]) {
    core::Create();

    
    win32_test_context TestContext = {};
    WNDCLASSEXW WindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_OWNDC,
        .lpfnWndProc = DefWindowProcW,
        .hInstance = GetModuleHandleW(nullptr),
        .lpszClassName = L"App_Test"
    };

    RegisterClassExW(&WindowClass);

    TestContext.Window = Win32_Create_Window(WindowClass, 1920, 1080, L"App_Test");

    testing::InitGoogleTest(&ArgCount, Args);
    return RUN_ALL_TESTS();
}

#include <test.cpp>