#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <vector>
#include <string>
#include "./src/ipc_cache_shmem.hpp"

const char* SHM_NAME = "/mi_cache_stress_shm";

constexpr int WRITE_OPERATIONS = 1'000'000; 
constexpr int READ_OPERATIONS  = 5'000'000; 

void write_stress() {
    IPCCacheSHM cache(SHM_NAME);

    std::vector<std::string> keys(1000);
    for (int i = 0; i < 1000; ++i) {
        keys[i] = "key:" + std::to_string(i);
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < WRITE_OPERATIONS; ++i) {
        const char* key = keys[i % 1000].c_str(); 
        cache.put(key, "data_bloque_test_rendimiento_shm_forttia_2026");
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "\n[[Writing Results: (WRITER)]]\n";
    std::cout << "Total operations : " << WRITE_OPERATIONS << "\n";
    std::cout << "Time             : " << diff.count() << " sec\n";
    std::cout << "Performance      : " << (WRITE_OPERATIONS / diff.count()) << " ops/sec\n";
}

void read_stress() {
    /** 0.02 sec */
    usleep(200000);

    IPCCacheSHM cache(SHM_NAME);

    std::vector<std::string> keys(1000);
    for (int i = 0; i < 1000; ++i) {
        keys[i] = "key:" + std::to_string(i);
    }

    /** Avoiding compiler optimizations with volatile keywoard for testing porposes */
    volatile const char* dummy_sink; 

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < READ_OPERATIONS; ++i) {
        const char* key = keys[i % 1000].c_str();
        dummy_sink = cache.get(key);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "\n[[Readings Results: (READER)]]\n";
    std::cout << "Total operations : " << READ_OPERATIONS << "\n";
    std::cout << "Time             : " << diff.count() << " sec\n";
    std::cout << "Performance      : " << (READ_OPERATIONS / diff.count()) << " ops/sec\n";
}

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Error at fork().\n";
        return 1;
    }

    if (pid == 0) {
        read_stress();
    } else {
        write_stress();
        wait(nullptr);
        std::cout << "\n[Main] End of stress test.\n";
    }

    return 0;
}