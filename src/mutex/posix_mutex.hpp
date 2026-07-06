#pragma once

#include <pthread.h>

#include "../common/common.hpp"

namespace cxxcache {
struct PosixMutex {
    PosixMutex();
    ~PosixMutex();

    void initialize(void);
    void shutdown(void);
    void lock(void);
    void unlock(void);

private:
    pthread_mutex_t mutex;
};
}