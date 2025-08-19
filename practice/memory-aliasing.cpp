#pragma GCC optimize("O3")
#include <iostream>
#include <chrono>

const int N = 1e8;

void add_1(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] += b[i];
    }
}

void add_2(int * __restrict__ a, const int * __restrict__ b) {
    for (int i = 0; i < N; i++) {
        a[i] += b[i];
    }
}

int a[N], b[N], c[N];

int main() {
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
    }

    {
        auto t0 = std::chrono::steady_clock::now();
        add_1(a, a + 1);
        auto t1 = std::chrono::steady_clock::now();

        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

        std::cout << "Took " << diff << " µs\n";
    }

    {
        auto t0 = std::chrono::steady_clock::now();
        add_1(a, c);
        auto t1 = std::chrono::steady_clock::now();

        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

        std::cout << "Took " << diff << " µs\n";
    }
    
    {
        auto t0 = std::chrono::steady_clock::now();
        add_2(b, c);
        auto t1 = std::chrono::steady_clock::now();

        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

        std::cout << "Took " << diff << " µs\n";
    }
}