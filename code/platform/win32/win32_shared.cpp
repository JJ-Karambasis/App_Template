static HWND Win32_Create_Window(const WNDCLASSEXW& WindowClass, int32_t Width, int32_t Height, const wstring& Label) {
    DWORD Style = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    RECT WindowRect = {0, 0, Width, Height};

    AdjustWindowRectEx(&WindowRect, Style, FALSE, 0);

    HWND MainWindow = CreateWindowExW(0, WindowClass.lpszClassName, Label.Str, Style, CW_USEDEFAULT, CW_USEDEFAULT, 
                                      WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, nullptr, nullptr, 
                                      WindowClass.hInstance, nullptr);
    return MainWindow;
}