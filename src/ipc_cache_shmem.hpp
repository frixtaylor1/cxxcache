#pragma once

#include "posix_mutex.hpp"
#include "string_allocator.hpp"
#include "hash_map.hpp"

class IPCCacheSHM {
public:
    struct SharedLayout {
        PosixMutex   mutex;
        uint32       maxRows;
        uint32       stringArenaSize;
    };

public:
    IPCCacheSHM(const char* shm_name);
    ~IPCCacheSHM();

    void             put(const char* key, const char* value);
    const char*      get(const char* key);
    bool             contains(const char* key);

private:
    void             initializeAsClient(char *stringStorage, char *hashMapStorage);
    void             initializeAsOwner(char *hashMapStorage, uint32 hashMapSize, char *stringStorage);
    void             mapMemToProcessMemoryAddresses();
    void             openOrCreateShmFd(const char *shm_name);
    uint32           allignToEigthBytes(uint32 input);
    void             cleanup();

private:
    const char*      shmName;
    int              shmFd;
    uint32           totalSize;
    void*            baseAddress;
    bool             mapOwner;
    SharedLayout*    layout;
    HashMap<char *>* hashMap;
    StringAlloc*     stringAlloc;
};