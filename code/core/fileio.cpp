//Shared posix functions
#if defined(OS_POSIX)
static int Posix_Open_File_Descriptor(const string& Path, u32 Flags) {
    int IOFlags = 0;

    mode_t Mode = 0;

    bool BothReadAndWrite = (Flags & (file::Read_Flag|file::Write_Flag)) == (file::Read_Flag|file::Write_Flag);
    if(BothReadAndWrite) {
        IOFlags |= (O_RDWR|O_CREAT);
        Mode = 0666;
    } else if(Flags & file::Read_Flag) {
        IOFlags |= O_RDONLY;
    } else if(Flags & file::Write_Flag) {
        IOFlags |= O_WRONLY|O_CREAT;
        Mode = 0666;
    } else {
        Invalid_Code();
    }

    return open(Path.Str, IOFlags, Mode);
}

static inline u64 Posix_Get_File_Size(int FileDescriptor) {
    u64 Result = lseek(FileDescriptor, 0, SEEK_END);
    lseek(FileDescriptor, 0, SEEK_SET);
    return Result;
}
#endif

#if defined(OS_WIN32)

struct win32_file : public file {
    allocator* Allocator;
    HANDLE     Handle;
};

bool file::Read(void* Data, u32 ReadSize, u64 Offset) {
    win32_file* File = (win32_file*)this;

    OVERLAPPED _Overlapped;
    OVERLAPPED* Overlapped = nullptr;

    if(Offset != file::No_Offset) {
        Overlapped = &_Overlapped;
        Overlapped->Offset = (u32)Offset;
        Overlapped->OffsetHigh = (u32)(Offset >> 32);
    }

    DWORD BytesRead;
    return ReadFile(File->Handle, Data, ReadSize, &BytesRead, Overlapped) && BytesRead == ReadSize;
}

u64 file::Get_File_Size() const {
    win32_file* File = (win32_file*)this;

    LARGE_INTEGER Result;
    GetFileSizeEx(File->Handle, &Result);
    return Result.QuadPart;
}

void file::Release() {
    win32_file* File = (win32_file*)this;
    allocator* Allocator = File->Allocator;
    CloseHandle(File->Handle);
    operator delete(File, Allocator);
}

file* Create_File(allocator* Allocator, const string& Path, u32 Flags) {
    DWORD CreationDisposition = 0;
    DWORD DesiredAccess = 0;

    bool BothReadAndWrite = (Flags & (file::Read_Flag|file::Write_Flag)) == (file::Read_Flag|file::Write_Flag);
    if(BothReadAndWrite) {
        CreationDisposition = OPEN_ALWAYS;
        DesiredAccess = GENERIC_READ|GENERIC_WRITE;
    } else if(Flags & file::Read_Flag) {
        CreationDisposition = OPEN_EXISTING;
        DesiredAccess = GENERIC_READ;
    } else if(Flags & file::Write_Flag) {
        CreationDisposition = CREATE_ALWAYS;
        DesiredAccess = GENERIC_WRITE;
    } else {
        Invalid_Code();
    }

    scratch Scratch = Get_Scratch();
    wstring PathW(&Scratch, Path);

    HANDLE Handle = CreateFileW(PathW.Str, DesiredAccess, 0, nullptr, CreationDisposition, DesiredAccess, nullptr);
    if(Handle == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    win32_file* Result = new(Allocator) win32_file;
    Result->Allocator = Allocator;
    Result->Handle = Handle;
    return Result;
}

#elif defined(OS_ANDROID)

enum class android_file_type {
    Asset,
    File_Descriptor
};

struct android_file : public file {
    allocator* Allocator;

    android_file_type Type;
    union {
        AAsset* Asset;
        int     FileDescriptor;
    };
};

bool file::Read(void* Data, u32 ReadSize, u64 Offset) {
    android_file* File = (android_file*)this;

    if(File->Type == android_file_type::Asset) {
        if(Offset != file::No_Offset) {
            AAsset_seek64(File->Asset, Offset, SEEK_SET);
        }
        return AAsset_read(File->Asset, Data, ReadSize) == ReadSize;
    } else {
        Not_Implemented();
        return false;
    }
}

u64 file::Get_File_Size() const {
    android_file* File = (android_file*)this;
    if(File->Type == android_file_type::Asset) {
        return AAsset_getLength64(File->Asset);
    } else {
        return Posix_Get_File_Size(File->FileDescriptor);
    }
}

void file::Release() {
    android_file* File = (android_file*)this;
    allocator* Allocator = File->Allocator;
    if(File->Type == android_file_type::Asset) {
        AAsset_close(File->Asset);
    } else {
        close(File->FileDescriptor);
    }
    operator delete(File, Allocator);
}

file* Create_File(allocator* Allocator, const string& Path, u32 Flags) {
    bool UseInternalStorage = false;
    android_thread_manager* ThreadManager = (android_thread_manager*)thread_manager::Get();
    ANativeActivity* Activity = ThreadManager->Activity;

    if(Flags & file::Write_Flag) {
        UseInternalStorage = true;
    } else {
        AAsset* Asset = AAssetManager_open(Activity->assetManager, Path.Str, AASSET_MODE_UNKNOWN);   
        if(!Asset) {
            UseInternalStorage = true;
        } else {
            android_file* AndroidFile = new(Allocator) android_file;
            AndroidFile->Allocator = Allocator;
            AndroidFile->Type  = android_file_type::Asset;
            AndroidFile->Asset = Asset;
            return AndroidFile;
        }
    }

    if(UseInternalStorage) {
        scratch Scratch = Get_Scratch();
        
        string FullPath = string::Concat(&Scratch, Activity->internalDataPath, Path);

        int FileDescriptor = Posix_Open_File_Descriptor(Path, Flags);
        if(FileDescriptor == -1) {
            return nullptr;
        }

        android_file* AndroidFile = new(Allocator) android_file;
        AndroidFile->Allocator      = Allocator;
        AndroidFile->Type           = android_file_type::File_Descriptor;
        AndroidFile->FileDescriptor = FileDescriptor;
    }

    return nullptr;
}

#elif defined(OS_POSIX)

struct posix_file : public file {
    allocator* Allocator;
    int        FileDescriptor;
};

bool file::Read(void* Data, u32 ReadSize, u64 Offset) {
    posix_file* File = (posix_file*)this;
    if(Offset != file::No_Offset) {
        lseek(File->FileDescriptor, Offset, SEEK_SET);
    }
    return read(File->FileDescriptor, Data, ReadSize) == ReadSize;
}

u64 file::Get_File_Size() const {
    posix_file* File = (posix_file*)this;
    return Posix_Get_File_Size(File->FileDescriptor);
}

void file::Release() {
    posix_file* File = (posix_file*)this;
    allocator* Allocator = File->Allocator;
    close(File->FileDescriptor);
    operator delete(File, Allocator);
}

file* Create_File(allocator* Allocator, const string& Path, u32 Flags) {
    string ActualPath = Path;
#if defined(OS_IOS)
    scratch Scratch = Get_Scratch();
    ActualPath = string::Concat(&Scratch, thread_manager::Get_Bundle_Path(), Path);
#elif defined(OS_OSX)
    scratch Scratch = Get_Scratch();
    ActualPath = string::Concat(&Scratch, {thread_manager::Get_Bundle_Path(), "Contents/Resources/", Path});
#endif

    int FileDescriptor = Posix_Open_File_Descriptor(ActualPath, Flags);

    if(FileDescriptor == -1) {
        return nullptr;
    }

    posix_file* Result = new(Allocator) posix_file;
    Result->Allocator = Allocator;
    Result->FileDescriptor = FileDescriptor;
    
    return Result;
}

#else
#error Not Implemented
#endif

const_buffer Read_Entire_File(allocator* Allocator, const string& Path) {
    scratch Scratch = Get_Scratch();
    file* File = Create_File(&Scratch, Path, file::Read_Flag);
    if(!File) {
        return {};
    }

    u32 FileSize = Safe_U32(File->Get_File_Size());
    void* Memory = Allocator->Allocate(FileSize);
    
    if(!Memory) {
        File->Release();
        return {};
    }

    File->Read(Memory, FileSize);
    File->Release();

    return const_buffer(Memory, FileSize);
}
