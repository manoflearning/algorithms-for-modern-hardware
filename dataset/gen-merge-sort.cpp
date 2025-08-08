#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>

const int64_t N = (1ll << 32); // size of data set (stored in external memory)
const int32_t M = (1 << 28) / 4; // available internal memory

const int XMIN = INT_MIN;
const int XMAX = INT_MAX;

int main() {
    std::ofstream fout("data-merge-sort.bin", std::ios::binary | std::ios::out);

    if (!fout) {
        std::cerr << "failed to open data-merge-sort.bin\n";
        return 1;
    }

    std::mt19937 rng(123'456);
    std::uniform_int_distribution<int> dist(XMIN, XMAX);

    std::vector<int> buf;

    int64_t rem = N;
    int64_t written = 0;
    int last_progress = -1;

    while (rem) {
        int chunk = std::min<int64_t>(M, rem);
        buf.clear();
        buf.resize(chunk);

        for (int i = 0; i < chunk; i++) {
            buf[i] = dist(rng);
        }

        fout.write(reinterpret_cast<const char*>(buf.data()), chunk * sizeof(int));
        if (!fout) {
            std::cerr << "failed to write file\n";
            return 2;
        }
        rem -= chunk;
        written += chunk;

        int progress = static_cast<int>((written * 100) / N);
        if (progress != last_progress) {
            std::clog << std::fixed << std::setprecision(2)
                << "progress: " << std::setw(6)
                << (written * 100.0 / N) << " %\n";
            std::clog.flush();
            last_progress = progress;
        }
    }

    fout.close();

    std::clog << "done\n";
}
