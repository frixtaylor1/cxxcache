#include "string_allocator.hpp"

#include <cstring>
#include <stdio.h>

namespace cxxcache {
void StringAlloc::setStorageAddres(char* storage_address, uint32* bytes_counter) {
    arena = storage_address;
    bytes = bytes_counter;
}

char *StringAlloc::create(const char* str) {
    char *ptr = (char *) alloc(strlen(str));
    set(ptr, str);
    return ptr;
}

void StringAlloc::remove(char *ptr) {
    ::memset(ptr, 0, strlen(ptr));
}

uint32 StringAlloc::getBytesAllocated(void) const {
    return bytes ? *bytes : 0;
}

void *StringAlloc::getAllocatorAddress(void) const {
    return (void *) & arena[0];
}

void *StringAlloc::alloc(uint32 size) {
    void *ptr = (void*) & arena[*bytes];
    *bytes += size + 1;
    return ptr;
}

void StringAlloc::set(char *dest, const char* source) {
    ::memcpy((void *) dest, (const void *) source, (int) strlen(source) + 1);
}
}