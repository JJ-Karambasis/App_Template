static string OSX_Get_Bundle_Path(allocator* Allocator) {
    // Get a reference to the main bundle
    CFBundleRef MainBundle = CFBundleGetMainBundle();
    if(MainBundle != nil) {
        // Get a reference to the file's URL
        CFURLRef ImageURL = CFBundleCopyBundleURL(MainBundle);

        // Convert the URL reference into a string reference
        CFStringRef ImagePath = CFURLCopyFileSystemPath(ImageURL, kCFURLPOSIXPathStyle);

        // Convert the string reference into a C string
        const char* Path = CFStringGetCStringPtr(ImagePath, kCFStringEncodingUTF8);
        
        //End with trailing slash since result is a path
        string Result = string::Concat(Allocator, Path, "/");

        CFRelease(ImagePath);
        CFRelease(ImageURL);

        return Result;
    }

    return {};
}

thread_manager* thread_manager::Create(allocator* Allocator) {
    osx_thread_manager* ThreadManager = new(Allocator) osx_thread_manager;
    ThreadManager->Allocator = Allocator;
    ThreadManager->ThreadMap = hashmap<u64, thread_context*>(Allocator);
    ThreadManager->Mutex.Init();

    ThreadManager->BundlePath = OSX_Get_Bundle_Path(Allocator);

    if(NSApp != nil) {
        @autoreleasepool {
            [NSApplication sharedApplication];
            Assert(NSApp != nil, "Invalid ns app!");
        }
    }

    Set(ThreadManager);
    ThreadManager->MainThreadContext = Create_Thread_Context_Raw(ThreadManager, Get_Current_Thread_ID());
    return ThreadManager;
}
