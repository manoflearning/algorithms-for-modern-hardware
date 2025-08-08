#include <iostream>

const int64_t N = (1ll << 32); // size of data set (stored in external memory)
const int32_t M = (1 << 28) / 4; // available internal memory
const int32_t B = (1 << 20) / 4; // 1 MB blocks of integers


int main() {
    FILE *input = fopen("data-merge-sort.bin", "rb");
    std::vector<FILE*> parts;

    

    while (1) {
        static int part[M]; // better delete it right after
        int n = fread(part, 4, M, input);

        if (n == 0) break;
    }
}