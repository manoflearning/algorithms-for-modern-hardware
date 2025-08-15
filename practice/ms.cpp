#include <iostream>
#include <chrono>
#include <algorithm>
#include <cassert>

const size_t N = (1ll << 32) / 4; // size of data set (stored in external memory)
const size_t M = (1 << 28) / 4; // available internal memory
const size_t B = (1 << 20) / 4; // block size

static inline int seek64(FILE* f, uint64_t byte_off) {
#if defined(_WIN32)
    return _fseeki64(f, static_cast<__int64>(byte_off), SEEK_SET);
#else
    return fseeko(f, static_cast<off_t>(byte_off), SEEK_SET);
#endif
}

inline void read_slice(FILE* a, size_t i, int32_t* b, size_t j, size_t n) {
    assert(a && b);
    uint64_t off = static_cast<uint64_t>(i) * sizeof(int32_t);
    if (seek64(a, off) != 0) { perror("read_slice: seek"); std::abort(); }

    size_t done = 0;
    while (done < n) {
        size_t got = std::fread(b + j + done, sizeof(int32_t), n - done, a);
        if (got == 0) {
            if (std::feof(a)) { std::fprintf(stderr, "read_slice: EOF\n"); std::abort(); }
            if (std::ferror(a)) { perror("read_slice: fread"); std::abort(); }
        }
        done += got;
    }
}

inline void write_slice(FILE* a, size_t i, const int32_t* b, size_t j, size_t n) {
    assert(a && b);
    uint64_t off = static_cast<uint64_t>(i) * sizeof(int32_t);
    if (seek64(a, off) != 0) { perror("write_slice: seek"); std::abort(); }

    size_t done = 0;
    while (done < n) {
        size_t put = std::fwrite(b + j + done, sizeof(int32_t), n - done, a);
        if (put == 0) {
            if (std::ferror(a)) { perror("write_slice: fwrite"); std::abort(); }
        }
        done += put;
    }
}

void merge_sort(size_t l, size_t r, FILE *in, FILE *out, int32_t *buf) {
    if (l + 1 == r) return;

    size_t mid = (l + r - 1) >> 1;

    merge_sort(l, mid + 1, in, out, buf);
    merge_sort(mid + 1, r, in, out, buf);

    size_t i = l, j = mid + 1;
    size_t i_prv = i, j_prv = j, k_prv = l;

    read_slice(in, i, buf, 0, B);
    read_slice(in, j, buf, B, B);

    for (size_t k = l; k < r; k++) {
        if (i - i_prv >= B) {
            size_t n = (B <= mid - i + 1 ? B : mid - i + 1);
            read_slice(in, i, buf, 0, n);
            i_prv = i;
        }
        if (j - j_prv >= B) {
            size_t n = (B <= r - j ? B : r - j);
            read_slice(in, j, buf, B, n);
            j_prv = j;
        }

        if (k - k_prv >= B) {
            write_slice(out, k_prv, buf, 2 * B, B);
            k_prv = k;
        }

        if (i <= mid && (j == r || buf[i - i_prv] < buf[j - j_prv + B])) {
            buf[k - k_prv + 2 * B] = buf[i++ - i_prv];
        } else {
            buf[k - k_prv + 2 * B] = buf[j++ - j_prv + B];
        }
    }
    write_slice(out, k_prv, buf, 2 * B, r - k_prv);

    for (size_t k = l; k < r; k++) {
        if ((k - l) % B == 0) {
            size_t n = (B <= r - k ? B : r - k);
            read_slice(out, k, buf, 0, n);
            write_slice(in, k, buf, 0, n);
        }
    }
}

bool is_sorted(size_t n, FILE *a, int32_t *buf) {
    read_slice(a, 0, buf, 0, B);
    size_t i_prv = 0;

    for (size_t i = 0; i + 1 < n; i++) {
        if (i + 1 - i_prv >= B) {
            read_slice(a, i + 1, buf, 0, B);
            i_prv = i;
        }
        if (buf[i - i_prv] > buf[i - i_prv + 1]) return 0;
    }
    return 1;
}

int main() {
    FILE *a = fopen("dataset/ms-data.bin", "rb");
    FILE *b = fopen("dataset/ms-data-sorted.bin", "w+b");
    FILE *c = fopen("dataset/ms-data-tmp.bin", "w+b");

    if (!a || !b || !c) return 1;

    int32_t *buf = new int32_t[M];

    for (size_t i = 0; i < N; i++) {
        if (i % B == 0) {
            size_t n = B <= N - i ? B : N - i;
            read_slice(a, i, buf, 0, n);
            write_slice(b, i, buf, 0, n);
        }
    }

    auto t0 = std::chrono::steady_clock::now();
    merge_sort(0, N, b, c, buf);
    auto t1 = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::clog << "took " << diff << " ms\n";

    assert(is_sorted(N, b, buf));

    delete[] buf;

    return 0;
}
