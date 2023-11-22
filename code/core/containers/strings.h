#ifndef STRINGS_H
#define STRINGS_H

#include "stb_sprintf.h"

u32 UTF8_Read(const char* Str, u32* OutLength);
u32 UTF8_Write(char* Str, u32 Codepoint);
u32 UTF16_Read(const wchar_t* Str, u32* OutLength);
u32 UTF16_Write(wchar_t* Str, u32 Codepoint);

enum class str_case {
    Sensitive,
    Insensitive
};

#define String_Lit(str) string(str, sizeof(str)-1)

struct wstring;
struct string {
    const char* Str  = nullptr;
    uptr        Size = 0;

    string();
    string(const char* Str);
    string(const char* Str, uptr Size);
    string(allocator* Allocator, const string& Str);
    string(allocator* Allocator, const wstring& Str);
    string(allocator* Allocator, const char* Format, va_list Args);
    string(allocator* Allocator, const char* Format, ...);

    const char& operator[](uptr Index) const;

    static string Concat(allocator* Allocator, const string& A, const string& B);
};

bool Str_Equals(const string& StrA, const string& StrB, str_case Casing);
bool operator==(const string& StrA, const string& StrB);

string To_Date_Format(allocator* Allocator, u32 Value);
string To_Millisecond_Format(allocator* Allocator, u32 Value);

#define WString_Lit(str) wstring(L ## str, (sizeof(L##str)-1)/sizeof(wchar_t))

struct wstring {
    const wchar_t* Str  = nullptr;
    uptr           Size = 0;

    wstring();
    wstring(const wchar_t* Str);
    wstring(const wchar_t* Str, uptr Size);
    wstring(allocator* Allocator, const wstring& Str);
    wstring(allocator* Allocator, const string& Str);
    wstring(allocator* Allocator, const wchar_t* Format, va_list Args);
    wstring(allocator* Allocator, const wchar_t* Format, ...);

    const wchar_t& operator[](uptr Index) const;
};

#endif