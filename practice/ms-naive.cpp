#include <iostream>
#include <chrono>
#include <cassert>

const int32_t N = (1ll << 32) / 4; // size of data set (stored in external memory)

void merge_sort(int32_t l, int32_t r, int32_t *a, int32_t *b) {
    if (l + 1 == r) return;

    int32_t mid = (l + r - 1) >> 1;

    merge_sort(l, mid + 1, a, b);
    merge_sort(mid + 1, r, a, b);

    int32_t i = l, j = mid + 1;
    for (int32_t k = l; k < r; k++) {
        if (i <= mid && (j == r || a[i] < a[j])) {
            b[k] = a[i++];
        } else {
            b[k] = a[j++];
        }
    }

    for (int32_t k = l; k < r; k++) {
        a[k] = b[k];
    }
}

bool is_sorted(int32_t n, int32_t *a) {
    for (int32_t i = 0; i + 1 < n; i++) {
        if (a[i] > a[i + 1]) return 0;
    }
    return 1;
}

int main() {
    FILE *input = fopen("dataset/ms-data.bin", "rb");
    if (!input) return 1;

    int32_t *a = new int32_t[N];
    int32_t *b = new int32_t[N];

    int32_t read_cnt = std::fread(a, sizeof(int32_t), N, input);
    std::fclose(input);
    if (read_cnt != N) return 1;

    auto t0 = std::chrono::steady_clock::now();
    merge_sort(0, N, a, b);
    auto t1 = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::clog << "took " << diff << " ms\n";

    assert(is_sorted(N, a));

    FILE *output = std::fopen("dataset/ms-data-sorted.bin", "wb");
    if (!output) return 1;
    int32_t write_cnt = std::fwrite(a, sizeof(int32_t), N, output);
    if (write_cnt != N) return 1;

    delete[] a;
    delete[] b;

    return 0;
}
