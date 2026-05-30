#pragma once

#include "../mutex/posix_mutex.hpp"

struct StringAlloc {
public:
    void   setStorageAddres(char *storage_address);
    char*  create(const char* str);
    void   remove(char *ptr);

    uint32 getBytesAllocated(void) const;
    void*  getAllocatorAddress(void) const;

private:
    void* alloc(uint32 size);
    void  set(char *dest, const char* source);

private:
    enum { 
        GB_1_ALLOC_SIZE = 1024 * 1024 * 1024
    };

    char *arena;
    uint32  bytes = 0;
} ;