thread_local logger* logger::s_Logger = nullptr;
log_manager* log_manager::s_LogManager = nullptr;

string log_event::To_String(allocator* Allocator) {
    scratch Scratch = Get_Scratch();
    string Date = DateTime.Date.To_String(Scratch.Get_Arena());
    string Time = DateTime.Time.To_String(Scratch.Get_Arena());

    return string(Allocator, "%.*s %.*s - %.*s - %.*s - %.*s", Date.Size, Date.Str, Time.Size, Time.Str, 
                  Module.Size, Module.Str, LogLevelStr[(u32)Level].Size, LogLevelStr[(u32)Level].Str, 
                  Message.Size, Message.Str);
}

void logger::LogV(log_level Level, const string& Module, const char* MessageFormat, va_list Args) {
    Logs.Push({
        .DateTime = Get_Date_Time(),
        .Module = string(&Arena, Module),
        .Level = Level,
        .Message = string(&Arena, MessageFormat, Args)
    });

#if defined(OS_ANDROID)
    int Priority = G_AndroidPriority[(u32)Level];
    __android_log_vprint(Priority, Module.Str, MessageFormat, Args);
#endif

#if defined(DEBUG_BUILD) && !defined(OS_ANDROID)
    scratch Scratch = Get_Scratch();
    string DebugLog = Logs.Last().To_String(&Scratch);
    wstring DebugLogW(&Scratch, DebugLog);

# ifdef OS_WIN32
    OutputDebugStringW(DebugLogW.Str);
    OutputDebugStringA("\n");
# else
    wprintf(DebugLogW.Str);
    printf("\n");
# endif

#endif 
}

void logger::Log(log_level Level, const string& Module, const char* MessageFormat, ...) {
    va_list List;
    va_start(List, MessageFormat);
    LogV(Level, Module, MessageFormat, List);
    va_end(List);
}

logger* logger::Get() {
    if(!s_Logger) {
        s_Logger = log_manager::Get()->Get_Logger();
    }
    return s_Logger;
}

log_manager* log_manager::Create(allocator* Allocator) {
    log_manager* LogManager = new(Allocator) log_manager;
    LogManager->Allocator = Allocator;
    LogManager->LoggerMap = hashmap<u64, logger*>(Allocator);
    LogManager->Mutex.Init();
    Set(LogManager);
    return LogManager;
}

void log_manager::Flush() {
    log_manager* LogManager = Get();

    //TODO: Implement this crap at some point
}

void log_manager::Delete() {
    log_manager* LogManager = Get();

    for(auto KVP : LogManager->LoggerMap) {
        logger*& Logger = KVP.Value;
        if(Logger) {
            Logger->Logs.Release();
            Logger->Arena.Release();
            operator delete(Logger, LogManager->Allocator);
        }
        Logger = nullptr;
    }

    allocator* Allocator = LogManager->Allocator;
    LogManager->LoggerMap.Release();
    LogManager->Mutex.Release();
    operator delete(LogManager, Allocator);
}

void log_manager::Flush_And_Delete() {
    Flush();
    Delete();
}

logger* log_manager::Get_Logger() {
    u64 ThreadID = Get_Current_Thread_ID();
        
    Mutex.Lock();
    logger** pLogger = LoggerMap.Get(ThreadID);
    Mutex.Unlock();
    
    if(pLogger) {
        return *pLogger;
    }

    logger* Logger = new(Allocator) logger;
    Logger->Arena = arena(Allocator);
    Logger->Logs = vector<log_event>(Allocator);
    
    Mutex.Lock();
    LoggerMap.Add(ThreadID, Logger);
    Mutex.Unlock();

    return Logger;
}