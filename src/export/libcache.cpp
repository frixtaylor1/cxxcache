#include "libcache.hpp"

#include "../cache/ipc_cache_shmem.hpp"

m_private cxxcache::IPCCacheSHM* cache = nullptr;

m_export int cache_init(const char* shmName) {
    try {
        if (!cache) {
            cache = new cxxcache::IPCCacheSHM(shmName);
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

m_export void cache_update(const char* key, const char* value) {
    if (cache) {
        cache->update(key, value);
    }
}

m_export void cache_remove(const char* key) {
    if (cache) {
        cache->remove(key);
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

m_export void cache_lock(void) {
    if (cache) {
        cache->lock();
    }
}

m_export void cache_unlock(void) {
    if (cache) {
        cache->unlock();
    }
}

m_export const char* cache_get_nolock(const char* key) {
    if (!cache) return nullptr;
    return cache->get_nolock(key);
}

m_export void cache_update_nolock(const char* key, const char* value) {
    if (cache) {
        cache->update_nolock(key, value);
    }
}