#include "memory.cpp"
#include "timer.cpp"
#include "allocators/allocator.cpp"
#include "containers/containers.cpp"
#include "datetime.cpp"
#include "thread_manager.cpp"
#include "logger.cpp"
#include "shared_library.cpp"
#include "fileio.cpp"

core* core::s_Core = nullptr;

#ifdef OS_ANDROID
core* core::Create(JavaVM* JavaVM, JNIEnv* JavaENV, AAssetManager* AssetManager, const char* InternalDataPath) {
    static core Core;
    if(!Core.Allocator)
        Core.Allocator = allocator::Get_Default();
    
    if(!Core.ThreadManager)
        Core.ThreadManager = thread_manager::Create(Core.Allocator, JavaVM, JavaENV, AssetManager, InternalDataPath);
    
    if(!Core.LogManager)
        Core.LogManager = log_manager::Create(Core.Allocator);

    Set(&Core);
    return &Core;
}
#else
core* core::Create() {
    static core Core;
    if(!Core.Allocator)
        Core.Allocator = allocator::Get_Default();
    
    if(!Core.ThreadManager)
        Core.ThreadManager = thread_manager::Create(Core.Allocator);
    
    if(!Core.LogManager)
        Core.LogManager = log_manager::Create(Core.Allocator);

    Set(&Core);
    return &Core;
}
#endif

void core::Set(core* Core) {
    s_Core = Core;
    allocator::Set_Default(s_Core->Allocator);
    thread_manager::Set(s_Core->ThreadManager);
    log_manager::Set(s_Core->LogManager);
}