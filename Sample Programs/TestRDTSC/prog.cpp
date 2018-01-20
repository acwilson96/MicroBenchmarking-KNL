#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <cstring>

#ifdef __linux__    // Linux only
#include <sched.h>  // sched_setaffinity
#endif

/*
    Sourced from Intel white paper by Gabriele Paoloni.
    Code used detailed on page 16.
    https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf
*/

inline __attribute__((always_inline)) volatile void timestamp(uint32_t *time_hi, uint32_t *time_lo) {
    asm volatile (                  
        "MFENCE\n\t"
        "RDTSCP\n\t"
        "MFENCE\n\t"
        "mov %%edx, %0\n\t"
        "MFENCE\n\t"
        "mov %%eax, %1\n\t"
        "MFENCE\n\t":
            "=r" (*time_hi),
            "=r" (*time_lo):
            :"%rax", "%rbx", "%rcx", "%rdx"
    );
    asm volatile("MFENCE");
}

int main(int argc, char *argv[]) {
    #ifdef __linux__
    int cpuAffinity = argc > 1 ? atoi(argv[1]) : -1;

    if (cpuAffinity > -1)
    {
        cpu_set_t mask;
        int status;

        CPU_ZERO(&mask);
        CPU_SET(cpuAffinity, &mask);
        status = sched_setaffinity(0, sizeof(mask), &mask);
        if (status != 0)
        {
            perror("sched_setaffinity");
        }
    }
    #endif

    int ohead_lat[500];                                     // i'th element of array indicates how many times a NOP took i cycles.
    memset(ohead_lat, 0, sizeof(ohead_lat));                // Initialise count of overhead latencies to 0.

    int l1_lat[500];                                        // i'th element of array indicates how many times an L1 Load took i cycles.
    memset(l1_lat, 0, sizeof(l1_lat));                      // Initialise count of overhead latencies to 0.
    
    uint32_t start_hi, start_lo, end_hi, end_lo;            // 32bit integers to hold the high/low 32 bits of start/end timestamp counter values.
    uint64_t start, end;                                    // 64bit integers to hold the start/end timestamp counter values.

    uint64_t latency;

    // Do 1000 test runs of gathering the overhead.
    latency = 0;
    for (int i=0; i < 1000; i++) {
        start_hi = 0; start_lo = 0;                         // Initialise values of start_hi/start_lo so the values are already in L1 Cache.
        end_hi   = 0; end_lo   = 0;                         // Initialise values of end_hi/end_lo so the values are already in L1 Cache.
        
        // Take a starting measurement of the TSC.
        timestamp(&start_hi, &start_lo);
            // asm volatile (                  
            //     "MFENCE\n\t"
            //     "RDTSCP\n\t"
            //     "MFENCE\n\t"
            //     "mov %%edx, %0\n\t"
            //     "MFENCE\n\t"
            //     "mov %%eax, %1\n\t"
            //     "MFENCE\n\t":
            //         "=r" (start_hi),
            //         "=r" (start_lo):
            //         :"%rax", "%rbx", "%rcx", "%rdx"
            // );
            // asm volatile("MFENCE");

        // Calculating overhead, so no instruction to be timed here.
        asm volatile("MFENCE");
        asm volatile("MFENCE");

        // Take an ending measurement of the TSC.
        timestamp(&end_hi, &end_lo);
            // asm volatile(
            //     "MFENCE\n\t"        
            //     "RDTSCP\n\t"
            //     "MFENCE\n\t"
            //     "mov %%edx, %0\n\t"
            //     "MFENCE\n\t"
            //     "mov %%eax, %1\n\t"
            //     "MFENCE\n\t":
            //         "=r" (end_hi), 
            //         "=r" (end_lo):
            //         :"%rax", "%rbx", "%rcx", "%rdx"
            // );
            // asm volatile("MFENCE");

        // Convert the 4 x 32bit values into 2 x 64bit values.
        start   = ( ((uint64_t)start_hi << 32) | start_lo );
        end     = ( ((uint64_t)end_hi << 32) | end_lo );
        latency = end - start;

        // Increment the appropriate indexes of our latency tracking arrays.
        if (latency < 500) ohead_lat[latency]++;            // Only increment the latency if its within an acceptable range, otherwise this latency was most likely a random error.
    }

    // Do 1000 test runs of timing an L1 Load.
    latency = 0;
    volatile int data;
    for (int i=0; i < 1000; i++) {
        data     = 100;                                     // Initialise a data variable, such that we know it will be in the L1 Cache.

        start_hi = 0; start_lo = 0;                         // Initialise values of start_hi/start_lo so the values are already in L1 Cache.
        end_hi   = 0; end_lo   = 0;                         // Initialise values of end_hi/end_lo so the values are already in L1 Cache.
        
        // Take a starting measurement of the TSC.
        timestamp(&start_hi, &start_lo);
            // asm volatile (                  
            //     "MFENCE\n\t"
            //     "RDTSCP\n\t"
            //     "MFENCE\n\t"
            //     "mov %%edx, %0\n\t"
            //     "MFENCE\n\t"
            //     "mov %%eax, %1\n\t"
            //     "MFENCE\n\t":
            //         "=r" (start_hi),
            //         "=r" (start_lo):
            //         :"%rax", "%rbx", "%rcx", "%rdx"
            // );
            // asm volatile("MFENCE");

        // Calculating overhead, so no instruction to be timed here.
        asm volatile("MFENCE");
        asm volatile("mov %%eax, %0": "=r"(data)::);
        // asm volatile("mov %%eax, %0": "=m"(data):: "eax");  // Load the data variable, which will exist in the L1 Cache.
        asm volatile("MFENCE");

        // Take an ending measurement of the TSC.
        timestamp(&end_hi, &end_lo);
            // asm volatile(
            //     "MFENCE\n\t"        
            //     "RDTSCP\n\t"
            //     "MFENCE\n\t"
            //     "mov %%edx, %0\n\t"
            //     "MFENCE\n\t"
            //     "mov %%eax, %1\n\t"
            //     "MFENCE\n\t":
            //         "=r" (end_hi), 
            //         "=r" (end_lo):
            //         :"%rax", "%rbx", "%rcx", "%rdx"
            // );
            // asm volatile("MFENCE");

        // Convert the 4 x 32bit values into 2 x 64bit values.
        start   = ( ((uint64_t)start_hi << 32) | start_lo );
        end     = ( ((uint64_t)end_hi << 32) | end_lo );
        latency = end - start;

        // Increment the appropriate indexes of our latency tracking arrays.
        if (latency < 500) l1_lat[latency]++;        // Only increment the latency if its within an acceptable range, otherwise this latency was most likely a random error.
    }

    // Output results
    printf("\n\tLAT\t|\tO/Head\t\t|\tL1 Load");
    printf("\n\t--------+-----------------------+------------------");
    for (int i=0; i < 500; i++) {
        double oh_perc = (double)ohead_lat[i] / (double)10;
        double l1_perc = (double)l1_lat[i] / (double)10;
        if (oh_perc > 1 || l1_perc > 1) {
            printf("\n\t%d\t|\t%d(%.2f%%)\t|\t%d(%.2f%%)", i, ohead_lat[i], oh_perc, l1_lat[i], l1_perc);
        }
    }
    printf("\n");
}