#include "string_allocator.hpp"

#include <cstring>
#include <stdio.h>

void StringAlloc::setStorageAddres(char* storage_address) {
    arena = storage_address;
}

char *StringAlloc::create(const char* str) {
    char *ptr = (char *) alloc(strlen(str));
    set(ptr, str);
    return ptr;
}

void StringAlloc::remove(char *ptr) {
    ::memset(ptr, 0, strlen(ptr));
}

void *StringAlloc::alloc(int size) {
    void *ptr = (void*) & arena[bytes];
    bytes += size + 1;
    return ptr;
}

void StringAlloc::set(char *dest, const char* source) {
    ::memcpy((void *) dest, (const void *) source, (int) strlen(source) + 1);
}