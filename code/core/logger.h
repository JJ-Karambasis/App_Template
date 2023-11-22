enum class log_level {
    Debug,
    Info,
    Warning,
    Error,
    Count
};

#ifdef OS_ANDROID
static const int G_AndroidPriority[] = {
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR
};

static_assert(Array_Count(G_AndroidPriority) == (u32)log_level::Count);
#endif

static const string LogLevelStr[] = {
    String_Lit("Debug"),
    String_Lit("Info"),
    String_Lit("Warning"),
    String_Lit("Error")
};

static_assert(Array_Count(LogLevelStr) == (u32)log_level::Count);

struct log_event {
    date_time DateTime;
    string    Module;
    log_level Level;
    string    Message;

    string To_String(allocator* Allocator);
};

struct logger {
    arena             Arena;
    vector<log_event> Logs;

    void LogV(log_level Level, const string& Module, const char* MessageFormat, va_list Args);
    void Log(log_level Level, const string& Module, const char* MessageFormat, ...);

    static logger* Get();
private:
    static thread_local logger* s_Logger;
};

struct log_manager {
    friend struct logger;

    mutex                 Mutex;
    allocator*            Allocator;
    hashmap<u64, logger*> LoggerMap;

    static inline log_manager* Get() {
        return s_LogManager;
    }

    static inline void Set(log_manager* LogManager) {
        s_LogManager = LogManager;
    }

    static log_manager* Create(allocator* Allocator);
    static void Flush();
    static void Delete();
    static void Flush_And_Delete();

private:
    static log_manager* s_LogManager;
    logger* Get_Logger();
};

#if defined(COMPILER_GCC)
#define Log_Debug(module, str, ...) logger::Get()->Log(log_level::Debug, module, str, ##__VA_ARGS__)
#define Log_Info(module, str, ...) logger::Get()->Log(log_level::Info, module, str, ##__VA_ARGS__)
#define Log_Warning(module, str, ...) logger::Get()->Log(log_level::Warning, module, str, ##__VA_ARGS__)
#define Log_Error(module, str, ...) logger::Get()->Log(log_level::Error, module, str, ##__VA_ARGS__) 
#else
#define Log_Debug(module, str, ...) logger::Get()->Log(log_level::Debug, module, str, __VA_ARGS__)
#define Log_Info(module, str, ...) logger::Get()->Log(log_level::Info, module, str, __VA_ARGS__)
#define Log_Warning(module, str, ...) logger::Get()->Log(log_level::Warning, module, str, __VA_ARGS__)
#define Log_Error(module, str, ...) logger::Get()->Log(log_level::Error, module, str, __VA_ARGS__) 
#endif