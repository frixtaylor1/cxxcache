#pragma once

#include "../mutex/posix_mutex.hpp"
#include "../allocator/string_allocator.hpp"
#include "../hashmap/hash_map.hpp"

class IPCCacheSHM {
private:
    m_private constexpr const int STRING_ARENA_SIZE = 1024 * 1024 * 1024;
    m_private constexpr const int HASH_MAP_MAX_ROWS = 1024 * 1024;

    typedef HashMap<char *, HASH_MAP_MAX_ROWS> IPCHash;
public:
    struct SharedLayout {
        PosixMutex mutex;
        uint32     maxRows;
        uint32     stringArenaSize;
    };

public:
    IPCCacheSHM(const char* shm_name);
    ~IPCCacheSHM();

    void          put(const char* key, const char* value);
    const char*   get(const char* key);
    bool          contains(const char* key);
    void          remove(const char* key);
    void          update(const char* key, const char* value);
    void          lock(void);
    void          unlock(void);

private:
    void          initializeAsClient(char *stringStorage, char *hashMapStorage);
    void          initializeAsOwner(char *hashMapStorage, uint32 hashMapSize, char *stringStorage);
    void          mapMemToProcessMemoryAddresses();
    void          openOrCreateShmFd(const char *shm_name);
    uint32        allignToEightBytes(uint32 input) const;
    void          cleanup(void);

private:
    const char*   shmName;
    int           shmFd;
    uint32        totalSize;
    void*         baseAddress;
    bool          mapOwner;
    SharedLayout* layout;
    IPCHash       hashMap;
    StringAlloc   stringAlloc;
};