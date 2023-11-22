string date::To_String(allocator* Allocator) {
    scratch Scratch = Get_Scratch();
    string DayStr = To_Date_Format(&Scratch, Day);
    return string(Allocator, "%d-%.*s-%.*s", Year, MonthNumStr[(u32)Month].Size, MonthNumStr[(u32)Month].Str,
                  DayStr.Size, DayStr.Str);
}

string time::To_String(allocator* Allocator) {
    scratch Scratch = Get_Scratch();
    string HourStr = To_Date_Format(&Scratch, Hour);
    string MinuteStr = To_Date_Format(&Scratch, Minute);
    string SecondStr = To_Date_Format(&Scratch, Second);
    string MillisecondStr = To_Millisecond_Format(&Scratch, Millisecond);

    return string(Allocator, "%.*s:%.*s:%.*s.%.*s", HourStr.Size, HourStr.Str, MinuteStr.Size, MinuteStr.Str, 
                  SecondStr.Size, SecondStr.Str, MillisecondStr.Size, MillisecondStr.Str);
}

#if defined(OS_WIN32)
date_time Get_Date_Time() {
    FILETIME FileTime;
    GetSystemTimePreciseAsFileTime(&FileTime);

    SYSTEMTIME SystemTime;
    FileTimeToSystemTime(&FileTime, &SystemTime);

    SYSTEMTIME LocalTime;
    SystemTimeToTzSpecificLocalTime(nullptr, &SystemTime, &LocalTime);

    return {
        .Date = {
            .Year      = LocalTime.wYear,
            .Month     = (month)(LocalTime.wMonth-1),
            .DayOfWeek = (day_of_week)(LocalTime.wDayOfWeek),
            .Day       = LocalTime.wDay
        },
        .Time = {
            .Hour        = LocalTime.wHour,
            .Minute      = LocalTime.wMinute,
            .Second      = LocalTime.wSecond,
            .Millisecond = LocalTime.wMilliseconds
        }
    };
}
#elif defined(OS_POSIX)
date_time Get_Date_Time() {
    timespec Timespec;
    clock_gettime(CLOCK_REALTIME, &Timespec);

    u32 Millisecond;
    if(Timespec.tv_nsec >= 999500000) {
        Timespec.tv_sec++;
        Millisecond = 0;
    } else {
        Millisecond = (u32)((Timespec.tv_nsec + 500000) / 1000000);
    }

    struct tm* Time = localtime(&Timespec.tv_sec);
    return {
        .Date = {
            .Year      = 1900+(u32)Time->tm_year,
            .Month     = (month)(Time->tm_mon+1),
            .DayOfWeek = (day_of_week)(Time->tm_wday),
            .Day       = (u32)Time->tm_mday
        },
        .Time = {
            .Hour        = (u32)Time->tm_hour,
            .Minute      = (u32)Time->tm_min,
            .Second      = (u32)Time->tm_sec,
            .Millisecond = Millisecond
        }
    };
}
#else
#error Not Implemented
#endif