// entropy_seq: wersja referencyjna (sekwencyjna).
// przykład uruchomienia:
// ./entropy_seq --input ../data/stale.bin --out ./results
// ./entropy_seq --input ../data/losowy.bin --out ./results
// ./entropy_seq --input ../data/segmenty.bin --out ./results
// ./entropy_seq --input ../data/lorem_ipsum.txt --out ./results
// ./entropy_seq --input ../data/only_text.pdf --out ./results

#include <chrono>

#include "entropy_common.hpp"

using Clock = std::chrono::steady_clock;

static double seconds(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double>(b - a).count();
}

int main(int argc, char** argv) {
    Params p = parse_args(argc, argv);

    auto t0 = Clock::now();
    std::vector<uint8_t> data = read_file(p.input);
    auto t1 = Clock::now();

    uint64_t file_size = data.size();                                  // N
    uint64_t full_windows = full_count(file_size, p.window, p.step);  // F
    // Entropia każdego okna: najpierw F pełnych, potem ewentualnie okno niepełne.
    std::vector<double> entropies(full_windows);
    compute_range(data.data(), 0, full_windows, p.window, p.step, p.incremental, entropies.data());
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
