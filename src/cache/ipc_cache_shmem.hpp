#pragma once

#include "../allocator/string_allocator.hpp"
#include "../hashmap/hash_map.hpp"
#include "../mutex/posix_mutex.hpp"
namespace cxxcache {
class IPCCacheSHM {
private:
  m_private constexpr const int STRING_ARENA_SIZE = 1024 * 1024 * 1024;
  m_private constexpr const int HASH_MAP_MAX_ROWS = 1024 * 1024;

  typedef HashMap<char *, HASH_MAP_MAX_ROWS> IPCHash;

public:
  IPCCacheSHM(const char *name);
  ~IPCCacheSHM();

  void put(const char *key, const char *value);
  void update(const char *key, const char *value);
  void remove(const char *key);
  const char *get(const char *key);
  bool contains(const char *key);

  void put_nolock(const char* key, const char* value);
  void update_nolock(const char* key, const char* value);
  void remove_nolock(const char* key);
  const char* get_nolock(const char* key);
  bool contains_nolock(const char* key);

  void lock(void);
  void unlock(void);

private:
  struct SharedLayout {
    PosixMutex mutex;
    uint32 maxRows;
    uint32 stringArenaSize;
    uint32 stringArenaBytes;
  };

private:
  void openOrCreateShmFd(const char *shm_name);
  void mapSharedMemory(void);
  void initializeAsClient(char *stringStorage, char *hashMapStorage);
  void initializeAsOwner(char *hashMapStorage, uint32 hashMapSize,
                         char *stringStorage);

  uint32 allignToEightBytes(uint32 size) const;
  void cleanup(void);

private:
  const char *shmName = nullptr;
  int shmFd = -1;
  bool mapOwner = false;
  uint32 totalSize = 0;
  char *baseAddress = nullptr;

  SharedLayout *layout = nullptr;
  StringAlloc stringAlloc;
  HashMap<char *, HASH_MAP_MAX_ROWS> hashMap;
};

} // namespace cxxcache