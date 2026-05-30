#pragma once

#include "../mutex/posix_mutex.hpp"

struct StringAlloc {
public:
    void setStorageAddres(char *storage_address);
    char* create(const char* str);
    void  remove(char *ptr);

private:
    void* alloc(int size);
    void  set(char *dest, const char* source);

private:
    enum { 
        GB_1_ALLOC_SIZE = 1024 * 1024 * 1024
    };

    char *arena;
    int  bytes = 0;
} ;