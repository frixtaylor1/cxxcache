#include "ipc_cache_shmem.hpp"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <new>
#include <cstdint>
#include <stdexcept>

#include "posix_mutex.hpp"
#include "string_allocator.hpp"
#include "hash_map.hpp"

m_private constexpr const int STRING_ARENA_SIZE = 1024 * 1024 * 1024;
m_private constexpr const int HASH_MAP_MAX_ROWS = 1024 * 1024;

IPCCacheSHM::IPCCacheSHM(const char* shm_name) 
    : shmName(shm_name), shmFd(-1), totalSize(0), baseAddress(nullptr), mapOwner(false),
        layout(nullptr), hashMap(nullptr), stringAlloc(nullptr) 
{
    uint32 layoutSize        = sizeof(SharedLayout);
    uint32 hashMapOffset     = allignToEigthBytes(layoutSize);
    uint32 hashMapSize       = (HASH_MAP_MAX_ROWS) * sizeof(typename HashMap<char*>::Entry);
    uint32 stringArenaOffset = allignToEigthBytes(hashMapOffset + hashMapSize);
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
    delete hashMap;
    delete stringAlloc;
    
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
        stringAlloc->create(value);
        stringAlloc->create(key);
        hashMap->set(key, (char*) value);
    } catch (...) {
        layout->mutex.unlock();
        throw;
    }
    layout->mutex.unlock();
}

const char* IPCCacheSHM::get(const char* key) {
    layout->mutex.lock();
    const char* val = hashMap->get(key);
    layout->mutex.unlock();
    return val;
}

bool IPCCacheSHM::contains(const char* key) {
    layout->mutex.lock();
    bool exists = hashMap->exists(key);
    layout->mutex.unlock();
    return exists;
}

void IPCCacheSHM::initializeAsClient(char *stringStorage, char *hashMapStorage) {
    stringAlloc = new StringAlloc(stringStorage);
    hashMap     = new HashMap<char *>(hashMapStorage, layout->maxRows);
}

void IPCCacheSHM::initializeAsOwner(char *hashMapStorage, uint32 hashMapSize, char *stringStorage) {
    new (&layout->mutex) PosixMutex();
    layout->maxRows = HASH_MAP_MAX_ROWS;
    layout->stringArenaSize = STRING_ARENA_SIZE;

    ::memset(hashMapStorage, 0, hashMapSize);

    stringAlloc = new StringAlloc(stringStorage);
    hashMap = new HashMap<char *>(hashMapStorage, HASH_MAP_MAX_ROWS);
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

uint32 IPCCacheSHM::allignToEigthBytes(uint32 input) {
    return (input + 7) & ~7;
}

void IPCCacheSHM::cleanup() {
    if (shmFd >= 0) {
        close(shmFd);
    }
    if (mapOwner && shmName) {
        shm_unlink(shmName);
    }
}