#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

cpu_set_t mask;

// int loneSize = 32768;
// int numElems = 4096;

// int data[4096];

// static inline uint64_t rdtsc() {
//     uint64_t out;
//     asm("rdtsc" : "=A"(out));
//     return out;
// }

static inline uint64_t test_rdtsc() {    
    uint64_t a;
    uint64_t b;
    uint64_t out;
    asm("rdtscp" : "=A"(a));
    asm("rdtscp" : "=A"(b));
    out = b - a;
    return out;
}

inline void assignToCore(int core_id) {
    CPU_ZERO(&mask);
    CPU_SET(core_id, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
}

// void init() {
//     for (int i=0; i < numElems; i++) {
//         data[i] = 1;
//     }
// }

// void run() {
//    uint64_t before;
//    uint64_t after;
//    int temp = 0;
//    for (int i=0; i < numElems; i++) {
//        before = rdtsc();
//        temp = data[i];
//        after = rdtsc();
//     //    printf("%" PRIu64 "\n", after - before);
//    }
// }

int main() {
    assignToCore(0);
    for (int i = 0; i < 100; i++) {
        printf("%" PRIu64 "\n", test_rdtsc());
    }
    // printf("test_rdtsc(): %" PRIu64 "\n", test_rdtsc());
    // init();
    // run();
}