#ifndef LIBRARY_H
#define LIBRARY_H

struct shared_library {
    void* Get_Function(const char* Name);
    void  Delete();
};

shared_library* Create_Shared_Library(allocator* Allocator, const string& Library);

#endif