// entropy_omp: wersja OpenMP. Różni się od entropy_seq tylko etapem obliczeń.
// przykład uruchomienia:
// ./entropy_omp --input ../data/segmenty.bin --threads 4
// ./entropy_omp --input ../data/only_text.pdf --threads 4
// ./entropy_omp --input ../data/only_text.pdf --step 1024 --mode incremental
// ./entropy_omp --input ../data/segmenty.bin --step 1024 --mode incremental

#include <chrono>

#include <omp.h>

#include "entropy_common.hpp"

using Clock = std::chrono::steady_clock;

static double seconds(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double>(b - a).count();
}

int main(int argc, char** argv) {
    Params p = parse_args(argc, argv, {"--threads"});
    if (p.extra.count("--threads")) {
        uint64_t threads = parse_uint(p.extra["--threads"], "--threads");
        if (threads < 1) fail(1, "--threads musi być >= 1");
        omp_set_num_threads((int)threads);
    }

    auto t0 = Clock::now();
    std::vector<uint8_t> data = read_file(p.input);
    auto t1 = Clock::now();

    uint64_t file_size = data.size();                                  // N
    uint64_t full_windows = full_count(file_size, p.window, p.step);  // F
    // Entropia każdego okna: najpierw F pełnych, potem ewentualnie okno niepełne.
    // Każdy wątek pisze tylko do swoich komórek, a histogramy są lokalne.
    std::vector<double> entropies(full_windows);
    if (!p.incremental) {
        #pragma omp parallel for schedule(static)
        for (uint64_t k = 0; k < full_windows; k++)
            entropies[k] = entropy_of(data.data(), k * p.step, p.window);
    } else {
        // Tryb przyrostowy: każdy wątek dostaje ciągły zakres okien [first, end).
        #pragma omp parallel
        {
            uint64_t t = omp_get_thread_num(), threads = omp_get_num_threads();
            uint64_t first = full_windows * t / threads;
            uint64_t end = full_windows * (t + 1) / threads;
            compute_range(data.data(), first, end, p.window, p.step, true, entropies.data());
        }
    }
    if (has_partial(file_size, p.window, p.step, full_windows)) {
        uint64_t begin = full_windows * p.step;
        entropies.push_back(entropy_of(data.data(), begin, file_size - begin));
    }
    auto t2 = Clock::now();

    std::vector<Change> changes = detect_changes(entropies, full_windows, p.step, p.threshold);
    auto t3 = Clock::now();

    write_results(p, file_size, full_windows, entropies, changes);
    auto t4 = Clock::now();

    write_times(p, {{"odczyt", seconds(t0, t1)},
                    {"obliczenia", seconds(t1, t2)},
                    {"wykrywanie_zmian", seconds(t2, t3)},
                    {"zapis", seconds(t3, t4)},
                    {"calosc", seconds(t0, t4)}});
    return 0;
}
