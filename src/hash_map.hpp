#pragma once

#include <cstdint>
#include <cstring>

#include "common.hpp"

template <class ValueType, uint32 MAX_ROWS>
class HashMap {
public:
    struct Entry {
        const char* key;
        ValueType   value;
    };

public:
    HashMap();
    
    void            setStorageAddress(void* storageAddress);
    void            set(const char* key, ValueType value);
    ValueType       get(const char* key) const;
    bool            exists(const char* key) const;

private:
    uint32          getIndexOfKey(const char* key) const;
    static uint64_t hashKey(const char* key);

private:
    enum {
        FNV_OFFSET = 14695981039346656037UL,
        FNV_PRIME  = 1099511628211UL
    };

    Entry* rows;
};

template <class ValueType, uint32 MAX_ROWS>
HashMap<ValueType, MAX_ROWS>::HashMap() = default;

template <class ValueType, uint32 MAX_ROWS>
void HashMap<ValueType, MAX_ROWS>::set(const char* key, ValueType value) {
    uint32 index = getIndexOfKey(key);
    rows[index].value = value;
    rows[index].key = key;
}

template <class ValueType, uint32 MAX_ROWS>
ValueType HashMap<ValueType, MAX_ROWS>::get(const char* key) const {
    uint32 index = getIndexOfKey(key);
    return rows[index].value;
}

template <class ValueType, uint32 MAX_ROWS>
bool HashMap<ValueType, MAX_ROWS>::exists(const char* key) const {
    uint32 index = getIndexOfKey(key);
    return rows[index].key != nullptr && rows[index].value != ValueType{};
}

template <class ValueType, uint32 MAX_ROWS>
uint32 HashMap<ValueType, MAX_ROWS>::getIndexOfKey(const char* key) const {
    uint64_t hash = hashKey(key);
    uint32 index = (uint32)(hash & (uint64_t)(MAX_ROWS - 1));

    uint32 start_index = index;

    while (rows[index].key != nullptr) {
        if (::strcmp(rows[index].key, key) == 0) {
            return index;
        }
        index = (index + 1) & (MAX_ROWS - 1);
        
        if (index == start_index) throw;
    }
    return index;
}

template <class ValueType, uint32 MAX_ROWS>
uint64_t HashMap<ValueType, MAX_ROWS>::hashKey(const char* key) {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

template <class ValueType, uint32 MAX_ROWS>
void HashMap<ValueType, MAX_ROWS>::setStorageAddress(void* storageAddress) {
    rows = (Entry* ) storageAddress;
}
