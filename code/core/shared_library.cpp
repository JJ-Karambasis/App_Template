#if defined(OS_WIN32)

struct win32_shared_library : shared_library {
    allocator* Allocator;
    HMODULE DLL;
};

void* shared_library::Get_Function(const char* Name) {
    win32_shared_library* Win32 = (win32_shared_library*)this;
    return (void*)GetProcAddress(Win32->DLL, Name);
}

void shared_library::Delete() {
    win32_shared_library* Win32 = (win32_shared_library*)this;
    FreeLibrary(Win32->DLL);
    allocator* Allocator = Win32->Allocator;
    operator delete(Win32, Allocator);
}

shared_library* Create_Shared_Library(allocator* Allocator, const string& Library) {
    scratch Scratch = Get_Scratch();
    string LibraryName = string::Concat(&Scratch, Library, String_Lit(".dll"));
    wstring LibraryNameW(&Scratch, LibraryName);

    HMODULE DLL = LoadLibraryW(LibraryNameW.Str);
    if(!DLL) {
        return nullptr;
    }

    win32_shared_library* Result = new(Allocator) win32_shared_library;
    Result->Allocator = Allocator;
    Result->DLL = DLL;

    return Result;
}

#elif defined(OS_POSIX)

struct posix_shared_library : shared_library {
    allocator* Allocator;
    void*      Library;
};

void* shared_library::Get_Function(const char* Name) {
    posix_shared_library* Posix = (posix_shared_library*)this;
    return dlsym(Posix->Library, Name);
}

void shared_library::Delete() {
    posix_shared_library* Posix = (posix_shared_library*)this;
    dlclose(Posix->Library);
    allocator* Allocator = Posix->Allocator;
    operator delete(Posix, Allocator);
}

shared_library* Create_Shared_Library(allocator* Allocator, const string& Library) {
    scratch Scratch = Get_Scratch();
#ifdef OS_OSX
    string LibraryName = string::Concat(&Scratch, Library, String_Lit(".dylib"));
#else
    string LibraryName = string::Concat(&Scratch, Library, String_Lit(".so"));
#endif

    void* Handle = dlopen(LibraryName.Str, RTLD_NOW|RTLD_LOCAL);
    if(!Handle) {
        printf("Debug error: %s\n", dlerror());
        return nullptr;
    }

    posix_shared_library* Result = new(Allocator) posix_shared_library;
    Result->Allocator = Allocator;
    Result->Library = Handle;

    return Result;
}

#else
#error Not Implemented
#endif