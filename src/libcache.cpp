#include "libcache.hpp"

#include "ipc_cache_shmem.hpp"

m_private IPCCacheSHM* cache = nullptr;

m_export int cache_init(const char* shmName) {
    try {
        if (!cache) {
            cache = new IPCCacheSHM(shmName);
        }
        return 0;
    }
    catch(...) {
        return -1;
    }
}

m_export void cache_put(const char* key, const char* value) {
    if (cache) {
        cache->put(key, value);
    }
}

m_export const char* cache_get(const char* key) {
    if (!cache) return nullptr;
    return cache->get(key);
}

m_export bool cache_contains(const char* key) {
    if (!cache) return false;
    return cache->contains(key);
}

m_export void cache_destroy(void) {
    if (cache) {
        delete cache;
        cache = nullptr;
    }
}