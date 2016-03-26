#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <limits>

// Benchmark the operation 'op'. The number of iterations refers to
// how many times the operation is run for each time measurement, the
// result is the minimum over a number of samples runs. The result is the
// amount of time in seconds for one iteration.
#ifdef _WIN32

union _LARGE_INTEGER;
typedef union _LARGE_INTEGER LARGE_INTEGER;
extern "C" int __stdcall QueryPerformanceCounter(LARGE_INTEGER*);
extern "C" int __stdcall QueryPerformanceFrequency(LARGE_INTEGER*);

template <typename F>
double benchmark(int samples, int iterations, F op) {
    int64_t freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    double best = std::numeric_limits<double>::infinity();
    for (int i = 0; i < samples; i++) {
        int64_t t1;
        QueryPerformanceCounter((LARGE_INTEGER*)&t1);
        for (int j = 0; j < iterations; j++) {
            op();
        }
        int64_t t2;
        QueryPerformanceCounter((LARGE_INTEGER*)&t2);
        double dt = (t2 - t1) / static_cast<double>(freq);
        if (dt < best) best = dt;
    }
    return best / iterations;
}

#else

#ifndef NOCHRONO
#include <chrono>
#endif
#ifdef PCYCLES
#if defined(__hexagon__)
#include "io.h"
#endif
#endif

template <typename F>
double benchmark(int samples, int iterations, F op) {
    double best = std::numeric_limits<double>::infinity();
    for (int i = 0; i < samples; i++) {
#ifndef NOCHRONO
        auto t1 = std::chrono::high_resolution_clock::now();
#endif
#ifdef PCYCLES
#if defined(__hexagon__)
        RESET_PMU();
        long long start_cycles = READ_PCYCLES();
#endif
#endif
        for (int j = 0; j < iterations; j++) {
            op();
        }
#ifndef NOCHRONO
        auto t2 = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;
#else
        double dt = 0;
#endif
#ifdef PCYCLES
#if defined(__hexagon__)
        long long total_cycles = READ_PCYCLES() - start_cycles;
        DUMP_PMU();
        dt = total_cycles;
#endif
#endif
        if (dt < best) best = dt;
    }
    return best / iterations;
}

#endif

#endif
