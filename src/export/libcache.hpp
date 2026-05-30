#pragma once

#include "../common/common.hpp"

m_export int         cache_init(const char* shmName);
m_export void        cache_put(const char* key, const char* value);
m_export const char* cache_get(const char* key);
m_export bool        cache_contains(const char* key);
m_export void        cache_destroy(void);