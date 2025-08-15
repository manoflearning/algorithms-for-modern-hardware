#include <iostream>
#include <chrono>
#include <cassert>

const int32_t N = (1ll << 32) / 4; // size of data set (stored in external memory)
const int32_t M = (1 << 28) / 4; // available internal memory
const int32_t B = (1 << 20) / 4; // block size

inline bool read_slice(FILE* f, int32_t l, int32_t* dst, int32_t dst_off, size_t n) {
    if (fseeko(f, (off_t)(l * (int32_t)sizeof(int32_t)), SEEK_SET) != 0) return false;
    return std::fread(dst + (size_t)dst_off, sizeof(int32_t), n, f) == n;
}
inline bool write_slice(FILE* f, int32_t l, const int32_t* src, int32_t src_off, size_t n) {
    if (fseeko(f, (off_t)(l * (int32_t)sizeof(int32_t)), SEEK_SET) != 0) return false;
    return std::fwrite(src + (size_t)src_off, sizeof(int32_t), n, f) == n;
}

void merge_sort(int32_t l, int32_t r, FILE *a, FILE *b, int32_t *part) {
    if (l + 1 == r) return;

    int32_t mid = (l + r - 1) >> 1;

    merge_sort(l, mid + 1, a, b, part);
    merge_sort(mid + 1, r, a, b, part);

    int32_t i = l, j = mid + 1;
    int32_t i_prv = i, j_prv = j, k_prv = l;

    read_slice(a, i, part, 0, B);
    read_slice(a, j, part, B, B);

    for (int32_t k = l; k < r; k++) {
        if (i - i_prv >= B) {
            read_slice(a, i, part, 0, B);
            i_prv = i;
        }
        if (j - j_prv >= B) {
            read_slice(a, j, part, B, B);
            j_prv = j;
        }

        if (k - k_prv >= B) {
            write_slice(b, k_prv, part, 2 * B, B);
            k_prv = k;
        }

        if (i <= mid && (j == r || part[i - i_prv] < part[j - j_prv + B])) {
            part[k - k_prv + 2 * B] = part[i++ - i_prv];
        } else {
            part[k - k_prv + 2 * B] = part[j++ - j_prv + B];
        }
    }

    write_slice(b, k_prv, part, 2 * B, r - k_prv);
}

bool is_sorted(int32_t n, FILE *b, int32_t *part) {
    read_slice(b, 0, part, 0, B);
    int32_t i_prv = 0;

    for (int32_t i = 0; i + 1 < n; i++) {
        if (i + 1 - i_prv >= B) {
            read_slice(b, i + 1, part, 0, B);
            i_prv = i;
        }
        if (part[i - i_prv] > part[i - i_prv + 1]) return 0;
    }
    return 1;
}

int main() {
    FILE *a = fopen("dataset/ms-data.bin", "rb");
    FILE *b = fopen("dataset/ms-data-sorted.bin", "wb");
    if (!a || !b) return 1;

    int32_t *part = new int32_t[M];

    auto t0 = std::chrono::steady_clock::now();
    merge_sort(0, N, a, b, part);
    auto t1 = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::clog << "took " << diff << " ms\n";

    assert(is_sorted(N, a, part));

    delete[] part;

    return 0;
}
