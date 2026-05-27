#pragma once

#include <pthread.h>

#include "imutex.hpp"

struct PosixMutex : implements IMutex {
    PosixMutex();
    ~PosixMutex();

    void initialize(void);
    void shutdown(void);
    void lock(void);
    void unlock(void);

private:
    pthread_mutex_t mutex;
};