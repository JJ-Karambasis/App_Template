static const u8 G_ClassUTF8[32] = 
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

u32 UTF8_Read(const char* Str, u32* OutLength) {
    u32 Result = 0xFFFFFFFF;
    
    u8 Byte = (u8)Str[0];
    u8 ByteClass = G_ClassUTF8[Byte >> 3];
    
    static const u32 BITMASK_3 = 0x00000007;
    static const u32 BITMASK_4 = 0x0000000f;
    static const u32 BITMASK_5 = 0x0000001f;
    static const u32 BITMASK_6 = 0x0000003f;
    
    switch(ByteClass)
    {
        case 1:
        {
            Result = Byte;
        } break;
        
        case 2:
        {
            u8 NextByte = (u8)Str[1];
            if(G_ClassUTF8[NextByte >> 3] == 0)
            {
                Result = (Byte & BITMASK_5) << 6;
                Result |= (NextByte & BITMASK_6);
            }
        } break;
        
        case 3:
        {
            u8 NextBytes[2] = {(u8)Str[1], (u8)Str[2]};
            if(G_ClassUTF8[NextBytes[0] >> 3] == 0 &&
                G_ClassUTF8[NextBytes[1] >> 3] == 0)
            {
                Result = (Byte & BITMASK_4) << 12;
                Result |= ((NextBytes[0] & BITMASK_6) << 6);
                Result |= (NextBytes[1] & BITMASK_6);
            }
        } break;
        
        case 4:
        {
            u8 NextBytes[3] = {(u8)Str[1], (u8)Str[2], (u8)Str[3]};
            if(G_ClassUTF8[NextBytes[0] >> 3] == 0 &&
                G_ClassUTF8[NextBytes[1] >> 3] == 0 &&
                G_ClassUTF8[NextBytes[2] >> 3] == 0)
            {
                Result = (Byte & BITMASK_3) << 18;
                Result |= ((NextBytes[0] & BITMASK_6) << 12);
                Result |= ((NextBytes[1] & BITMASK_6) << 6);
                Result |= (NextBytes[2] & BITMASK_6);
            }
        } break;
    }
    
    if(OutLength) *OutLength = ByteClass;
    return Result;
}

u32 UTF8_Write(char* Str, u32 Codepoint) {
    static const u32 BIT_8 = 0x00000080;
    static const u32 BITMASK_2 = 0x00000003;
    static const u32 BITMASK_3 = 0x00000007;
    static const u32 BITMASK_4 = 0x0000000f;
    static const u32 BITMASK_5 = 0x0000001f;
    static const u32 BITMASK_6 = 0x0000003f;
    
    u32 Result = 0;
    if (Codepoint <= 0x7F)
    {
        Str[0] = (char)Codepoint;
        Result = 1;
    }
    else if (Codepoint <= 0x7FF)
    {
        Str[0] = (char)((BITMASK_2 << 6) | ((Codepoint >> 6) & BITMASK_5));
        Str[1] = (char)(BIT_8 | (Codepoint & BITMASK_6));
        Result = 2;
    }
    else if (Codepoint <= 0xFFFF)
    {
        Str[0] = (char)((BITMASK_3 << 5) | ((Codepoint >> 12) & BITMASK_4));
        Str[1] = (char)(BIT_8 | ((Codepoint >> 6) & BITMASK_6));
        Str[2] = (char)(BIT_8 | ( Codepoint       & BITMASK_6));
        Result = 3;
    }
    else if (Codepoint <= 0x10FFFF)
    {
        Str[0] = (char)((BITMASK_4 << 3) | ((Codepoint >> 18) & BITMASK_3));
        Str[1] = (char)(BIT_8 | ((Codepoint >> 12) & BITMASK_6));
        Str[2] = (char)(BIT_8 | ((Codepoint >>  6) & BITMASK_6));
        Str[3] = (char)(BIT_8 | ( Codepoint        & BITMASK_6));
        Result = 4;
    }
    else
    {
        Str[0] = '?';
        Result = 1;
    }
    
    return Result;
}

u32 UTF16_Read(const wchar_t* Str, u32* OutLength) {
    u32 Offset = 1;
    u32 Result = *Str;
    if (0xD800 <= Str[0] && Str[0] < 0xDC00 && 0xDC00 <= Str[1] && Str[1] < 0xE000)
    {
        Result = ((Str[0] - 0xD800) << 10) | (Str[1] - 0xDC00);
        Offset = 2;
    }
    if(OutLength) *OutLength = Offset;
    return Result;
}

u32 UTF16_Write(wchar_t* Str, u32 Codepoint) {
    static const u32 BITMASK_10 = 0x000003ff;
    
    u32 Result = 0;
    if(Codepoint == 0xFFFFFFFF)
    {
        Str[0] = (wchar_t)'?';
        Result = 1;
    }
    else if(Codepoint < 0x10000)
    {
        Str[0] = (wchar_t)Codepoint;
        Result = 1;
    }
    else
    {
        Codepoint -= 0x10000;
        Str[0] = 0xD800 + (wchar_t)(Codepoint >> 10);
        Str[1] = 0xDC00 + (wchar_t)(Codepoint & BITMASK_10);
        Result = 2;
    }
    
    return Result;
}

inline string::string() { }

inline string::string(const char* _Str) : Str(_Str), Size(strlen(_Str)) { }

inline string::string(const char* _Str, uptr _Size) : Str(_Str), Size(_Size) { }

string::string(allocator* Allocator, const string& String) {
    char* Buffer = (char*)Allocator->Allocate(sizeof(char)*(String.Size+1));
    Buffer[String.Size] = 0;
    Memory_Copy(Buffer, String.Str, String.Size*sizeof(char));
    Str = Buffer;
    Size = String.Size;
}

string::string(allocator* Allocator, const wstring& Str) {
    scratch Scratch = Get_Scratch();

    const wchar_t* WStrAt = Str.Str;
    const wchar_t* WStrEnd = WStrAt+Str.Size;

    char* StrStart = (char*)Scratch.Allocate((Str.Size*4)+1);
    char* StrEnd = StrStart + Str.Size*4;
    char* StrAt = StrStart;

    for(;;) {
        Assert(StrAt <= StrEnd, "Overflow!");
        if(WStrAt >= WStrEnd) {
            Assert(WStrAt == WStrEnd, "Invalid str reading!");
            break;
        }

        u32 Length;
        u32 Codepoint = UTF16_Read(WStrAt, &Length);
        WStrAt += Length;
        StrAt += UTF8_Write(StrAt, Codepoint);
    }

    *StrAt = 0;
    *this = string(Allocator, string(StrStart, StrAt-StrStart));
}

string::string(allocator* Allocator, const char* Format, va_list Args) {
    static char TempBuffer[1];
    int LogLength = stbsp_vsnprintf(TempBuffer, 1, Format, Args);
    char* Buffer = (char*)Allocator->Allocate(LogLength+1);
    stbsp_vsnprintf(Buffer, LogLength+1, Format, Args);
    *this = string(Buffer, LogLength);
}

string::string(allocator* Allocator, const char* Format, ...) {
    va_list List;
    va_start(List, Format);
    *this = string(Allocator, Format, List);
    va_end(List);
}

const char& string::operator[](uptr Index) const {
    Assert(Index < Size, "Overflow!");
    return Str[Index];
}

string string::Concat(allocator* Allocator, const string& A, const string& B) {
    uptr TotalSize = A.Size+B.Size;
    char* Buffer = (char*)Allocator->Allocate((TotalSize+1)*sizeof(char));
    Memory_Copy(Buffer, A.Str, A.Size*sizeof(char));
    Memory_Copy(Buffer+A.Size*sizeof(char), B.Str, B.Size*sizeof(char));
    Buffer[TotalSize] = 0;
    return string(Buffer, TotalSize);
}

string string::Concat(allocator* Allocator, const span<string>& Strings) {
    uptr TotalSize = 0;
    for(const string& Str : Strings) {
        TotalSize += Str.Size;
    }
    char* Buffer = (char*)Allocator->Allocate((TotalSize+1)*sizeof(char));
    char* BufferAt = Buffer;

    for(const string& Str : Strings) {
        Memory_Copy(BufferAt, Str.Str, Str.Size*sizeof(char));
        BufferAt += Str.Size;
    }
    *BufferAt = 0;
    return string(Buffer, TotalSize);
}

bool Str_Equals(const string& StrA, const string& StrB, str_case Casing) {
    if(StrA.Size != StrB.Size) return false;

    if(Casing == str_case::Sensitive) {
        for(uptr Index = 0; Index < StrA.Size; Index++) {
            if(StrA[Index] != StrB[Index]) {
                return false;
            }
        }
    } else {
        for(uptr Index = 0; Index < StrA.Size; Index++) {
            if(To_Lower(StrA[Index]) != To_Lower(StrB[Index])) {
                return false;
            }
        }
    }

    return true;
}

inline bool operator==(const string& StrA, const string& StrB) {
    return Str_Equals(StrA, StrB, str_case::Sensitive);
}

inline string To_Date_Format(allocator* Allocator, u32 Value) {
    return Value < 10 ? string(Allocator, "0%d", Value) : string(Allocator, "%d", Value);
}

string To_Millisecond_Format(allocator* Allocator, u32 Value) {
    if(Value < 10) {
        return string(Allocator, "00%d", Value);
    } else if(Value < 100) {
        return string(Allocator, "%0%d", Value);
    } else {
        return string(Allocator, "%d", Value);
    }
}

inline wstring::wstring() { }

inline wstring::wstring(const wchar_t* _Str) : Str(_Str), Size(wcslen(_Str)) { } 

inline wstring::wstring(const wchar_t* _Str, uptr _Size) : Str(_Str), Size(_Size) { }

wstring::wstring(allocator* Allocator, const wstring& String) {
    wchar_t* Buffer = (wchar_t*)Allocator->Allocate(sizeof(wchar_t)*(String.Size+1));
    Buffer[String.Size] = 0;
    Memory_Copy(Buffer, String.Str, String.Size*sizeof(wchar_t));
    Str = Buffer;
    Size = String.Size;
}

wstring::wstring(allocator* Allocator, const string& String) {
    scratch Scratch = Get_Scratch();

    const char* StrAt = String.Str;
    const char* StrEnd = StrAt+String.Size;

    wchar_t* WStrStart = (wchar_t*)Scratch.Allocate((String.Size+1)*sizeof(wchar_t));
    wchar_t* WStrEnd = WStrStart + String.Size;
    wchar_t* WStrAt = WStrStart;

    for(;;) {
        Assert(WStrAt <= WStrEnd, "Overflow!");
        if(StrAt >= StrEnd) {
            Assert(StrAt == StrEnd, "Invalid str reading!");
            break;
        }

        u32 Length;
        u32 Codepoint = UTF8_Read(StrAt, &Length);
        StrAt += Length;
        WStrAt += UTF16_Write(WStrAt, Codepoint);
    }

    *WStrAt = 0;
    *this = wstring(Allocator, wstring(WStrStart, WStrAt-WStrStart));
}

wstring::wstring(allocator* Allocator, const wchar_t* Format, va_list Args) {
    static wchar_t TempBuffer[2];
    int LogLength = swprintf(TempBuffer, 1, Format, Args);

    wchar_t* Buffer = (wchar_t*)Allocator->Allocate(sizeof(wchar_t)*(LogLength+1));
    swprintf(Buffer, LogLength, Format, Args);
    *this = wstring(Buffer, LogLength);
}

wstring::wstring(allocator* Allocator, const wchar_t* Format, ...) {
    va_list List;
    va_start(List, Format);
    *this = wstring(Allocator, Format, List);
    va_end(List);
}

const wchar_t& wstring::operator[](uptr Index) const {
    Assert(Index < Size, "Overflow!");
    return Str[Index];
}

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"