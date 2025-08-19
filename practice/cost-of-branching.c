#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>

enum { N = 1000000 };

static inline uint64_t now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ull + ts.tv_nsec / 1000ull;
}

void f(int *arr) {
    volatile int s = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] < 50) s += arr[i];
    }
}

void test(int *arr) {
    for (int i = 0; i < 100; i++) f(arr);
    uint64_t t0 = now_us();
    f(arr);
    uint64_t t1 = now_us();
    printf("Took %" PRIu64 " µs\n", t1 - t0);
}

int cmp(const void* a, const void* b) {
    int x = *(const int*)a, y = *(const int*)b;
    return (x > y) - (x < y);
}

int a[N], b[N], c[N], d[N];

int main() {
    for (int i = 0; i < N; i++) a[i] = rand() % 100;
    for (int i = 0; i < N; i++) b[i] = 0;
    for (int i = 0; i < N; i++) c[i] = 99;
    for (int i = 0; i < N; i++) d[i] = a[i];
    qsort(d, N, sizeof(int), cmp);

    // CASE 1:
    // - pure 50/50 coin toss
    // - cannot predict which branch comes next
    test(a);

    // CASE 2:
    // - when the branch is always taken
    // - can predict which branch comes next
    test(b);

    // CASE 3:
    // - when the branch is never taken
    // - can predict which branch comes next
    test(c);

    // CASE 4:
    // - pure 50/50 coin toss, but with certain "pattern"
    test(d);
}
