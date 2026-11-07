// Wspólny kod wersji entropy_seq i entropy_omp
#pragma once

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct Params {
    std::string input;
    uint64_t window = 4096;
    uint64_t step = 4096;
    double threshold = 1.0;
    bool incremental = false;
    std::string out = ".";
    std::map<std::string, std::string> extra;  // dodatkowe opcje danej wersji, np. --threads
};

struct Change {
    uint64_t position;
    double before, after, diff;
};

using Stages = std::vector<std::pair<std::string, double>>;  // etap, czas w sekundach

[[noreturn]] inline void fail(int code, const std::string& msg) {
    std::fprintf(stderr, "%s\n", msg.c_str());
    std::exit(code);
}

inline uint64_t parse_uint(const std::string& s, const std::string& name) {
    size_t used = 0;
    uint64_t v = 0;
    try { v = std::stoull(s, &used); } catch (...) {}
    if (used == 0 || used != s.size() || s[0] == '-') fail(1, "Niepoprawna wartość " + name + ": " + s);
    return v;
}

inline double parse_double(const std::string& s, const std::string& name) {
    size_t used = 0;
    double v = 0;
    try { v = std::stod(s, &used); } catch (...) {}
    if (used == 0 || used != s.size() || !std::isfinite(v)) fail(1, "Niepoprawna wartość " + name + ": " + s);
    return v;
}

// extra_keys: opcje specyficzne dla wersji; ich wartości trafiają do p.extra.
inline Params parse_args(int argc, char** argv, const std::vector<std::string>& extra_keys = {}) {
    Params p;
    for (int i = 1; i < argc; i += 2) {
        std::string key = argv[i];
        if (i + 1 >= argc) fail(1, "Brak wartości dla " + key);
        std::string val = argv[i + 1];

        if (key == "--input") p.input = val;
        else if (key == "--window") p.window = parse_uint(val, key);
        else if (key == "--step") p.step = parse_uint(val, key);
        else if (key == "--threshold") p.threshold = parse_double(val, key);
        else if (key == "--out") p.out = val;
        else if (key == "--mode") {
            if (val != "full" && val != "incremental") fail(1, "--mode musi być full albo incremental");
            p.incremental = (val == "incremental");
        } else {
            bool known = false;
            for (const auto& k : extra_keys) known = known || k == key;
            if (!known) fail(1, "Nieznany parametr: " + key);
            p.extra[key] = val;
        }
    }
    if (p.input.empty()) fail(1, "Brak wymaganego parametru --input");
    if (p.window < 1) fail(1, "--window musi być >= 1");
    if (p.step < 1 || p.step > p.window) fail(1, "--step musi spełniać 1 <= S <= W");
    if (p.threshold < 0) fail(1, "--threshold musi być >= 0");
    return p;
}

inline std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) fail(2, "Nie można otworzyć pliku: " + path);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (f.bad()) fail(2, "Błąd odczytu pliku: " + path);
    return data;
}

// Oznaczenia z README: N = file_size, W = window, S = step, F = full_windows.

// Liczba pełnych okien F.
inline uint64_t full_count(uint64_t file_size, uint64_t window, uint64_t step) {
    return file_size >= window ? (file_size - window) / step + 1 : 0;
}

// Czy istnieje okno niepełne [F·S, N).
inline bool has_partial(uint64_t file_size, uint64_t window, uint64_t step, uint64_t full_windows) {
    return file_size > 0 && (full_windows == 0 || (full_windows - 1) * step + window < file_size);
}

// Entropia Shannona (bit/bajt) z histogramu `hist` okna o długości `len` bajtów.
inline double entropy(const uint64_t hist[256], uint64_t len) {
    double h = 0.0;
    for (int i = 0; i < 256; i++) {
        if (hist[i] == 0) continue;
        double p = (double)hist[i] / len;
        h -= p * std::log2(p);
    }
    return h;
}

// Entropia bajtów [begin, begin + len), histogram od zera.
inline double entropy_of(const uint8_t* data, uint64_t begin, uint64_t len) {
    uint64_t hist[256] = {0};
    for (uint64_t i = begin; i < begin + len; i++) hist[data[i]]++;
    return entropy(hist, len);
}

// Liczy entropię pełnych okien o numerach first_window … end_window − 1
// (zakres półotwarty) i zapisuje ją do entropies[k], gdzie k to numer okna.
//
//   data          – cały plik wejściowy
//   first_window  – numer pierwszego okna do policzenia
//   end_window    – numer pierwszego okna, którego już nie liczymy (≤ F)
//   window        – długość okna W w bajtach
//   step          – krok okna S w bajtach; okno k zaczyna się w bajcie k·S
//   incremental   – false: każde okno od zera (tryb full);
//                   true: okno first_window od zera, kolejne przez aktualizację
//                   histogramu (usuwamy S bajtów z początku, dodajemy S na końcu)
//   entropies     – tablica wyników indeksowana globalnym numerem okna
//
// Zakres pozwala wersji OpenMP dać każdemu wątkowi ciągły fragment okien.
inline void compute_range(const uint8_t* data, uint64_t first_window, uint64_t end_window,
                          uint64_t window, uint64_t step, bool incremental, double* entropies) {
    if (!incremental) {
        for (uint64_t k = first_window; k < end_window; k++)
            entropies[k] = entropy_of(data, k * step, window);
        return;
    }

    if (first_window >= end_window) return;
    uint64_t hist[256] = {0};
    uint64_t begin = first_window * step;
    for (uint64_t i = begin; i < begin + window; i++) hist[data[i]]++;
    entropies[first_window] = entropy(hist, window);
    for (uint64_t k = first_window + 1; k < end_window; k++) {
        uint64_t prev = (k - 1) * step;  // początek poprzedniego okna
        for (uint64_t i = prev; i < prev + step; i++) hist[data[i]]--;                  // wypadły
        for (uint64_t i = prev + window; i < k * step + window; i++) hist[data[i]]++;  // weszły
        entropies[k] = entropy(hist, window);
    }
}

// Porównuje sąsiednie pełne okna 0 … full_windows − 1 (okno niepełne pomijamy).
inline std::vector<Change> detect_changes(const std::vector<double>& entropies, uint64_t full_windows,
                                          uint64_t step, double threshold) {
    std::vector<Change> changes;
    for (uint64_t k = 0; k + 1 < full_windows; k++) {
        double diff = std::fabs(entropies[k + 1] - entropies[k]);
        if (diff > threshold) changes.push_back({(k + 1) * step, entropies[k], entropies[k + 1], diff});
    }
    return changes;
}

inline FILE* open_out(const std::string& dir, const std::string& name) {
    std::string path = (std::filesystem::path(dir) / name).string();
    FILE* f = std::fopen(path.c_str(), "w");
    if (!f) fail(2, "Nie można zapisać pliku: " + path);
    return f;
}

inline void close_out(FILE* f) {
    if (std::ferror(f) | std::fclose(f)) fail(2, "Błąd zapisu pliku wynikowego");
}

// entropies: najpierw full_windows pełnych okien, potem ewentualnie okno niepełne.
inline void write_results(const Params& p, uint64_t file_size, uint64_t full_windows,
                          const std::vector<double>& entropies,
                          const std::vector<Change>& changes) {
    std::error_code ec;
    std::filesystem::create_directories(p.out, ec);
    if (ec) fail(2, "Nie można utworzyć katalogu: " + p.out);

    FILE* e = open_out(p.out, "entropia.csv");
    std::fprintf(e, "okno,poczatek,dlugosc,entropia,niepelne\n");
    for (uint64_t k = 0; k < entropies.size(); k++) {
        bool partial = k >= full_windows;
        uint64_t start = k * p.step;
        uint64_t len = partial ? file_size - start : p.window;
        std::fprintf(e, "%llu,%llu,%llu,%.12f,%d\n", (unsigned long long)k,
                     (unsigned long long)start, (unsigned long long)len, entropies[k], partial);
    }
    close_out(e);

    FILE* z = open_out(p.out, "zmiany.csv");
    std::fprintf(z, "pozycja,entropia_przed,entropia_po,roznica\n");
    for (const auto& c : changes)
        std::fprintf(z, "%llu,%.12f,%.12f,%.12f\n", (unsigned long long)c.position,
                     c.before, c.after, c.diff);
    close_out(z);
}

inline void write_times(const Params& p, const Stages& stages) {
    FILE* t = open_out(p.out, "czasy.csv");
    std::fprintf(t, "etap,czas_s\n");
    for (const auto& [name, sec] : stages) std::fprintf(t, "%s,%.6f\n", name.c_str(), sec);
    close_out(t);
}
