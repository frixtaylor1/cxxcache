#include "hash_map.hpp"

#include "string_allocator.hpp"

template <>
void HashMap<const char*>::set(const char* key, const char* value) {
    uint32 indexOfKey = getIndexOfKey(key);
    rows[indexOfKey].value = value;
}