#include "ipc_cache_shmem.hpp"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <new>
#include <cstdint>
#include <stdexcept>

#include "../mutex/posix_mutex.hpp"
#include "../allocator/string_allocator.hpp"
#include "../hashmap/hash_map.hpp"


IPCCacheSHM::IPCCacheSHM(const char* shm_name) 
    : shmName(shm_name), shmFd(-1), totalSize(0), baseAddress(nullptr), mapOwner(false),
        layout(nullptr)
{
    uint32 layoutSize        = sizeof(SharedLayout);
    uint32 hashMapOffset     = allignToEightBytes(layoutSize);
    uint32 hashMapSize       = (HASH_MAP_MAX_ROWS) * sizeof(typename HashMap<char*, HASH_MAP_MAX_ROWS>::Entry);
    uint32 stringArenaOffset = allignToEightBytes(hashMapOffset + hashMapSize);
    totalSize                = stringArenaOffset + STRING_ARENA_SIZE;

    openOrCreateShmFd(shm_name);

    mapMemToProcessMemoryAddresses();

    layout = (SharedLayout*) baseAddress;
    char* hashMapStorage = ((char*) baseAddress) + hashMapOffset;
    char* stringStorage  = ((char*) baseAddress) + stringArenaOffset;

    if (mapOwner) {
        initializeAsOwner(hashMapStorage, hashMapSize, stringStorage);
    } else {
        initializeAsClient(stringStorage, hashMapStorage);
    }
}

IPCCacheSHM::~IPCCacheSHM() {
    if (baseAddress && baseAddress != MAP_FAILED) {
        munmap(baseAddress, totalSize);
    }
    if (shmFd >= 0) {
        close(shmFd);
    }
    if (mapOwner && shmName) {
        shm_unlink(shmName);
    }
}

void IPCCacheSHM::put(const char* key, const char* value) {
    layout->mutex.lock();
    try {
        char* key_ = stringAlloc.create(key);
        char* value_ = stringAlloc.create(value);
        hashMap.set(key_, (char*) value_);
    } catch (...) {
        layout->mutex.unlock();
        throw;
    }
    layout->mutex.unlock();
}

const char* IPCCacheSHM::get(const char* key) {
    layout->mutex.lock();
    const char* val = hashMap.get(key);
    layout->mutex.unlock();
    return val;
}

bool IPCCacheSHM::contains(const char* key) {
    layout->mutex.lock();
    bool exists = hashMap.exists(key);
    layout->mutex.unlock();
    return exists;
}

void IPCCacheSHM::initializeAsClient(char *stringStorage, char *hashMapStorage) {
    stringAlloc.setStorageAddres(stringStorage);
    hashMap.setStorageAddress(hashMapStorage);
}

void IPCCacheSHM::initializeAsOwner(char *hashMapStorage, uint32 hashMapSize, char *stringStorage) {
    new (&layout->mutex) PosixMutex();
    layout->maxRows = HASH_MAP_MAX_ROWS;
    layout->stringArenaSize = STRING_ARENA_SIZE;

    ::memset(hashMapStorage, 0, hashMapSize);

    stringAlloc.setStorageAddres(stringStorage);
    hashMap.setStorageAddress(hashMapStorage);
}

void IPCCacheSHM::mapMemToProcessMemoryAddresses() {
    baseAddress = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (baseAddress == MAP_FAILED) {
        cleanup();
        throw std::runtime_error("Error en mmap de la memoria compartida.");
    }
}

void IPCCacheSHM::openOrCreateShmFd(const char *shm_name) {
    shmFd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (shmFd >= 0) {
        mapOwner = true;
        if (ftruncate(shmFd, totalSize) == -1) {
            cleanup();
            throw std::runtime_error("Error al dimensionar la memoria compartida.");
        }
    } else {
        shmFd = shm_open(shm_name, O_RDWR, 0666);
        if (shmFd < 0) {
            throw std::runtime_error("Error al abrir el segmento SHM existente.");
        }
    }
}

uint32 IPCCacheSHM::allignToEightBytes(uint32 input) const {
    return (input + 7) & ~7;
}

void IPCCacheSHM::cleanup(void) {
    if (shmFd >= 0) {
        close(shmFd);
    }
    if (mapOwner && shmName) {
        shm_unlink(shmName);
    }
}