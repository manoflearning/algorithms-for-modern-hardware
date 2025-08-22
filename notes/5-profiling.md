# Profiling

Three main profiling techiques, allowing for different levels of precision:
- instrumentation: lets you time the program as a whole or by parts and count specific events you are interested in.
- statistical profiling: lets you go down to the assembly level and track various hardware events such as branch mispredictions or cache misses, which are critical for performance.
- program simulation: lets you go down to the individual cycle level and look into what is happening inside the CPU on each cycle when it is executing a small assembly snippet.

## Instrumentation

Example:
```cpp
clock_t start = clock();
do_something();
float seconds = float(clock() - start) / CLOCKS_PER_SEC;
printf("do_something() took $.4f", seconds);
```

One nuance here is that you can't measure the execution time of particularly quick functions this way because the `clock` function returns the current timestamp in microseconds ($10^{-6}$) and also by itself takes up to a few hundred nanoseconds to complete. All other time-related utilities similarly have at least microsecond granularity, which is an eternity in the world of low-level optimization.

To achieve higher precision, you can invoke the function repeatedly in a loop, time the whole thing once, and then divide the total time by the number of iterations.

### Event Sampling

Instrumentation can also be used to collect other types of information that can give useful insights about the performance of a particular algorithm:
- for a hash function, we are interested in the average length of its input;
- for a binary tree, we care about its size and height;
- for a sorting algorithm, we want to know about how many comparisons it does

In a similar way, we can insert counters in the code that compute these algorithm-specific statistics.

## Statistical Profiling

To interrupt the execution of a program at random intervals and look where the instruction pointer is. The number of times the pointer stopped in each function's block would be roughly proportional to the total time spent executing these functions. You can also get some other useful information this way, like finding out which functions are called by which funtions by inspecting the call stack.

This could be done by just running a program with `gdb` and `ctrl+c`ing it at random intervals but modern CPUs and operating systems provide special utilities for this type of profiling.

### Hardware Events

**Hardware performance counters** are special registers built into microprocessors that can store the counts of certain hardware-related activities. They are cheap to add on a microchip, as they are basically just binary counters with an activation wire connected to them.

### Profiling with perf

SKIPPED

## Program Simulation

Not to gather the data by actually running the program but to analyze what should happen by simulating it with specialized tools.

### Profiling with Cachegrind

Cachegrind essentially inspects that binary for "interesting" instructions (e.g., perform memory reads / writes and conditional / indirect jumps) and replaces them with code that simulates corresponding hardware operations using software data structures.

Cachegrind only tracks memory accesses and branches. When the bottleneck is caused by something else, we need other simulation tools.

## Machine Code Analyzers

A **machine code analyzer** is a program that takes a small snippet of assembly code and simulates its execution on a particular microarchitecture using information available to compilers, and outputs the latency and throughput of the whole block, as well as cycle-perfect utilization of various resources within the CPU.

### Using `llvm-mca`

`llvm-mca`:
- (you can probably install via a package manager together with `clang`)
- runs a set number of iterations of a given assembly snippet and computes statistics about the resource usage of each instruction, which is useful for finding out where the bottleneck is.

## Benchmarking

### Benchmarking Inside C++

SKIPPED

### Splitting Up Implementations

SKIPPED

### Makefiles

SKIPPED

### Jupyter Notebooks

SKIPPED

## Getting Accurate Results

### Measuring the Right Thing

There are many things that can introduce bias into benchmarks.

**Differing datasets**: performance can somehow depends on the dataset distribution

**Multiple objectives**:
- memory usage,
- latency of add query,
- latency of positive membership query,
- latency of negative membership query.

**Latency vs Throughput**: SKIPPED

**Cold cache**: memory reads initially take longer time because the required data is not in cache yet.

**Over-optimization**: Sometimes the benchmark is outright erroneous because the compiler just optimized the benchmarked code away. To prevent the compiler from cutting corners, you need to add checksums and either print them somewhere or add the `volatile` qualifier, which also prevents any sort of interleaving of loop iterations.

### Reducing Noise

Problem: benchmarking that result in either unpredictable skews or just completely random noise, thus increasing variance.

Caused by side effects and some sort of external noise, mostly due to noisy neighbors and CPU frequency scaling:
- If you benchmark a compute-bound algorithm, measure its performance in cycles using `perf stat`: this way it will be independent of clock frequency, fluctuations of which is usually the main source of noise.