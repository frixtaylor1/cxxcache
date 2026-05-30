#include "posix_mutex.hpp"

#include "common.hpp"

PosixMutex::PosixMutex() = default;

PosixMutex::~PosixMutex() = default;

void PosixMutex::initialize(void) {
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&mutex, &attr);

    pthread_mutexattr_destroy(&attr);
}

void PosixMutex::shutdown(void) {
    pthread_mutex_destroy(&mutex);
}

void PosixMutex::lock(void) {
    int rc = pthread_mutex_lock(&mutex);

    #define EOWNERDEAD 130
    if (rc == EOWNERDEAD) {
        pthread_mutex_consistent(&mutex);
    }
}

void PosixMutex::unlock(void) {
    pthread_mutex_unlock(&mutex);
}
