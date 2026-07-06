#pragma once

#include <cstdint>
#include <cstring>

#include "../common/common.hpp"

namespace cxxcache {
template <class ValueType, uint32 MAX_ROWS>
class HashMap {
public:
    struct Entry {
        uint32 keyOffset;
        uint32 valueOffset;
    };

public:
    HashMap();
    
    void      setStorageAddress(void* storageAddress, void* baseAddr);
    void      set(const char* key, ValueType value);
    ValueType get(const char* key) const;
    bool      exists(const char* key) const;
    void      remove(const char* key);

private:
    uint32    getIndexOfKey(const char* key) const;
    uint64_t  hashKey(const char* key) const;

private:
    enum {
        FNV_OFFSET = 14695981039346656037UL,
        FNV_PRIME  = 1099511628211UL
    };

    Entry* rows = nullptr;
    char*  baseAddress = nullptr;
};

template <class ValueType, uint32 MAX_ROWS>
HashMap<ValueType, MAX_ROWS>::HashMap() = default;

template <class ValueType, uint32 MAX_ROWS>
void HashMap<ValueType, MAX_ROWS>::set(const char* key, ValueType value) {
    uint32 index = getIndexOfKey(key);
    rows[index].keyOffset = (uint32)((char*)key - baseAddress);
    rows[index].valueOffset = (uint32)((char*)value - baseAddress);
}

template <class ValueType, uint32 MAX_ROWS>
ValueType HashMap<ValueType, MAX_ROWS>::get(const char* key) const {
    uint32 index = getIndexOfKey(key);
    if (rows[index].keyOffset == 0) return nullptr;
    return (ValueType)(baseAddress + rows[index].valueOffset);
}

template <class ValueType, uint32 MAX_ROWS>
bool HashMap<ValueType, MAX_ROWS>::exists(const char* key) const {
    uint32 index = getIndexOfKey(key);
    return rows[index].keyOffset != 0 && rows[index].valueOffset != 0;
}

template <class ValueType, uint32 MAX_ROWS>
void HashMap<ValueType, MAX_ROWS>::remove(const char* key) {
    if (!exists(key)) return;
    uint32 index = getIndexOfKey(key);
    if (rows[index].keyOffset != 0) {
        char* key_ = baseAddress + rows[index].keyOffset;
        ::memset(key_, 0, ::strlen(key_));
    }
    if (rows[index].valueOffset != 0) {
        char* value_ = baseAddress + rows[index].valueOffset;
        ::memset(value_, 0, ::strlen(value_));
    }
}

template <class ValueType, uint32 MAX_ROWS>
uint32 HashMap<ValueType, MAX_ROWS>::getIndexOfKey(const char* key) const {
    uint64_t hash = hashKey(key);
    uint32 index = (uint32)(hash & (uint64_t)(MAX_ROWS - 1));

    uint32 start_index = index;

    while (rows[index].keyOffset != 0) {
        const char* storedKey = baseAddress + rows[index].keyOffset;
        if (::strcmp(storedKey, key) == 0) {
            return index;
        }
        index = (index + 1) & (MAX_ROWS - 1);
        
        if (index == start_index) throw;
    }
    return index;
}

template <class ValueType, uint32 MAX_ROWS>
uint64_t HashMap<ValueType, MAX_ROWS>::hashKey(const char* key) const {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

template <class ValueType, uint32 MAX_ROWS>
void HashMap<ValueType, MAX_ROWS>::setStorageAddress(void* storageAddress, void* baseAddr) {
    rows = (Entry* ) storageAddress;
    baseAddress = (char*) baseAddr;
}
}
