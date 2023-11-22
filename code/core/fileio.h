#ifndef FILEIO_H
#define FILEIO_H

struct file {
    static const u64 No_Offset = (u64)-1;
    static const u32 Read_Flag  = (1 << 0);
    static const u32 Write_Flag = (1 << 1);

    bool Read(void* Data, u32 ReadSize, u64 Offset = No_Offset);
    u64  Get_File_Size() const;
    void Release();
};

file* Create_File(allocator* Allocator, const string& Path, u32 Flags);
const_buffer Read_Entire_File(allocator* Allocator, const string& Path);

#endif