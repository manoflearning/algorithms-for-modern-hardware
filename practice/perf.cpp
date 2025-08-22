// compile: g++ -O3 -march=native perf.cpp -o perf
// run 1: perf stat ./perf
// run 2: perf record ./perf && perf report

#include <iostream>
#include <algorithm>

const int N = 5050505;

int n = 5'000'000;
int a[N];

void setup() {
    for (int i = 0; i < n; i++)
        a[i] = rand();
    std::sort(a, a + n);
}

int query() {
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        int idx = std::lower_bound(a, a + n, rand()) - a;
        checksum += idx;
    }
    return checksum;
}

int main() {
    setup();
    query();
}