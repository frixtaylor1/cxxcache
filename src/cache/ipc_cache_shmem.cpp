#include "ipc_cache_shmem.hpp"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <new>
#include <cstdint>
#include <stdexcept>
#include <cstring>

#include "../mutex/posix_mutex.hpp"
#include "../allocator/string_allocator.hpp"
#include "../hashmap/hash_map.hpp"


namespace cxxcache {

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

    mapSharedMemory();

    char* stringStorage  = baseAddress + stringArenaOffset;
    char* hashMapStorage = baseAddress + hashMapOffset;

    if (mapOwner) {
        initializeAsOwner(hashMapStorage, hashMapSize, stringStorage);
    } else {
        initializeAsClient(stringStorage, hashMapStorage);
    }
}

IPCCacheSHM::~IPCCacheSHM() {
    if (mapOwner && layout) {
        layout->mutex.shutdown();
    }
    if (baseAddress && baseAddress != MAP_FAILED) {
        munmap(baseAddress, totalSize);
    }
    cleanup();
}

void IPCCacheSHM::put_nolock(const char* key, const char* value) {
    char* key_ = stringAlloc.create(key);
    char* value_ = stringAlloc.create(value);
    hashMap.set(key_, (char*) value_);
}

void IPCCacheSHM::put(const char* key, const char* value) {
    layout->mutex.lock();
    try {
        put_nolock(key, value);
    } catch (...) {
        layout->mutex.unlock();
        throw;
    }
    layout->mutex.unlock();
}

const char* IPCCacheSHM::get_nolock(const char* key) {
    return hashMap.get(key);
}

const char* IPCCacheSHM::get(const char* key) {
    layout->mutex.lock();
    const char* val = get_nolock(key);
    layout->mutex.unlock();
    return val;
}

bool IPCCacheSHM::contains_nolock(const char* key) {
    return hashMap.exists(key);
}

bool IPCCacheSHM::contains(const char* key) {
    layout->mutex.lock();
    bool exists = contains_nolock(key);
    layout->mutex.unlock();
    return exists;
}

void IPCCacheSHM::remove_nolock(const char* key) {
    hashMap.remove(key);
}

void IPCCacheSHM::remove(const char* key) {
    layout->mutex.lock();
    remove_nolock(key);
    layout->mutex.unlock();
}

void IPCCacheSHM::update_nolock(const char* key, const char* value) {
    char* currentValue = (char*)get_nolock(key);
    if (!currentValue) {
        put_nolock(key, value);
        return;
    }
    uint32 lengthOfCurrentValue = ::strlen(currentValue);
    uint32 lengthOfNewValue     = ::strlen(value);

    if (lengthOfCurrentValue >= lengthOfNewValue) {
        ::memset(currentValue, 0, lengthOfCurrentValue);
        ::memcpy(currentValue, value, lengthOfNewValue);
    } else {
        remove_nolock(key);
        put_nolock(key, value);
    }
}

void IPCCacheSHM::update(const char* key, const char* value) {
    layout->mutex.lock();
    update_nolock(key, value);
    layout->mutex.unlock();
}

void IPCCacheSHM::lock(void) {
    layout->mutex.lock();
}

void IPCCacheSHM::unlock(void) {
    layout->mutex.unlock();
}

void IPCCacheSHM::mapSharedMemory(void) {
    baseAddress = (char*) mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (baseAddress == MAP_FAILED) {
        cleanup();
        throw std::runtime_error("Error al mapear la memoria compartida.");
    }
    layout = (SharedLayout*) baseAddress;
}

void IPCCacheSHM::initializeAsClient(char *stringStorage, char *hashMapStorage) {
    stringAlloc.setStorageAddres(stringStorage, &layout->stringArenaBytes);
    hashMap.setStorageAddress(hashMapStorage, baseAddress);
}

void IPCCacheSHM::initializeAsOwner(char *hashMapStorage, uint32 hashMapSize, char *stringStorage) {
    new (&layout->mutex) PosixMutex();
    layout->mutex.initialize();
    layout->maxRows = HASH_MAP_MAX_ROWS;
    layout->stringArenaSize = STRING_ARENA_SIZE;
    layout->stringArenaBytes = 0;

    ::memset(hashMapStorage, 0, hashMapSize);

    stringAlloc.setStorageAddres(stringStorage, &layout->stringArenaBytes);
    hashMap.setStorageAddress(hashMapStorage, baseAddress);
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

}