# Entropia i segmentacja plików binarnych

Projekt nr 16 (C05) z przedmiotu Programowanie równoległe i rozproszone, rok 2026/2027. Zespół: MG, KŚ.

Program dzieli plik na okna i w każdym oknie liczy entropię Shannona rozkładu bajtów. Następnie wypisuje miejsca, w których entropia sąsiednich okien różni się bardziej niż zadany próg. Takie zmiany mogą wskazywać granice części pliku o innej budowie, np. tekstu, zer lub danych losowych.

Program nie ocenia rodzaju danych. Wysoka entropia nie oznacza, że plik jest złośliwy lub zaszyfrowany.

Program ma cztery wersje, które przetwarzają te same pliki i dają ten sam wynik. Porównujemy je pod względem poprawności i czasu działania.

## Wersje i autorzy

| Program          | Katalog  | Technologia                     | Autor |
| ---------------- | -------- | ------------------------------- | ----- |
| `entropy_seq`    | `seq/`   | C++17, gcc 13                   | MG    |
| `entropy_omp`    | `omp/`   | C++17, gcc 13, OpenMP           | MG    |
| `entropy_mpi`    | `mpi/`   | MPI, język: [do ustalenia]      | KŚ    |
| `entropy_gpu`    | `gpu/`   | CUDA lub OpenCL: [do ustalenia] | KŚ    |
| generator plików | `tools/` | Python 3                        | MG    |

## Struktura repozytorium

```
README.md
common/     kod wspólny dla seq i omp
seq/        wersja referencyjna (sekwencyjna)
omp/        wersja OpenMP
mpi/        wersja MPI
gpu/        wersja CUDA lub OpenCL
tools/      generator plików testowych
data/       pliki testowe; wygenerowane nie trafiają do gita, w repozytorium
            są tylko dwa statyczne: lorem_ipsum.txt i only_text.pdf
results/    surowe wyniki pomiarów (CSV) i pliki, z których powstają wykresy
docs/       sprawozdanie i prezentacja
```

## Wspólna specyfikacja

Wszystkie wersje stosują dokładnie te same reguły. Tylko wtedy wyniki można porównać okno po oknie.

### Parametry

```
entropy_seq --input PLIK [--window 4096] [--step 4096] [--threshold 1.0] [--mode full] [--out .]
```

| Parametr      | Znaczenie                                               | Domyślnie |
| ------------- | ------------------------------------------------------- | --------- |
| `--input`     | ścieżka do pliku wejściowego                            | wymagany  |
| `--window`    | długość okna W w bajtach                                | 4096      |
| `--step`      | krok okna S w bajtach                                   | 4096      |
| `--threshold` | próg zmiany entropii (bit/bajt)                         | 1.0       |
| `--mode`      | `full` – histogram od zera, `incremental` – przyrostowo | `full`    |
| `--out`       | katalog na pliki wynikowe; tworzony, jeśli nie istnieje | `.`       |

Parametry dodatkowe:

- `entropy_omp`: `--threads N` (bez parametru liczbę wątków ustala OpenMP).
- `entropy_gpu`: `--block N`, rozmiar bloku wątków (domyślnie 256).
- `entropy_mpi`: liczbę procesów podaje się w `mpirun -np P`.

Zasady: W ≥ 1, 1 ≤ S ≤ W, próg ≥ 0. Kody wyjścia: 0 – sukces, 1 – błędne parametry (komunikat na stderr), 2 – błąd odczytu lub zapisu.

### Podział na okna

Plik ma N bajtów.

- **Okno pełne** nr k obejmuje bajty [k·S, k·S + W). Pełnych okien jest F = ⌊(N − W) / S⌋ + 1, gdy N ≥ W; w przeciwnym razie F = 0.
- **Okno niepełne** (najwyżej jedno) powstaje, gdy ostatnie pełne okno nie sięga końca pliku. Zaczyna się w bajcie F·S i obejmuje resztę pliku (długość N − F·S). Gdy N < W, jest to jedyne okno: [0, N).
- **Pusty plik** nie ma okien; pliki wynikowe zawierają tylko nagłówki.

Przykład: N = 10 000, W = S = 4096 daje okna pełne 0 i 1 oraz okno niepełne nr 2 od bajtu 8192 o długości 1808.

### Entropia

Dla okna o długości n bajtów liczymy histogram c₀…c₂₅₅ (liczby całkowite). Wtedy pᵢ = cᵢ / n oraz:

```
H = − Σ pᵢ · log2(pᵢ)     (sumujemy tylko składniki z cᵢ > 0)
```

Obliczenia wykonujemy w typie `double`, a sumujemy po i rosnąco od 0 do 255. Wynik mieści się w zakresie od 0 do 8 bitów na bajt.

### Wykrywanie zmian

Dla k = 0 … F − 2: jeśli |H(k+1) − H(k)| > próg, to zapisujemy zmianę w pozycji (k+1)·S.

Porównujemy tylko okna pełne; okno niepełne nie bierze udziału. Ten krok jest tani, więc każda wersja wykonuje go sekwencyjnie na gotowej tablicy entropii. Zrównoleglamy tylko histogramy i entropię.

### Tryb przyrostowy (`--mode incremental`)

Histogram okna k+1 powstaje z histogramu okna k. Odejmujemy bajty [k·S, k·S + S), które wypadły z okna, i dodajemy bajty [k·S + W, k·S + W + S), które weszły. Entropię liczymy dalej z całego histogramu, tak samo jak w trybie `full`.

- Okno niepełne zawsze liczymy od zera.
- W wersjach równoległych każdy wykonawca dostaje ciągły zakres okien. Pierwsze okno zakresu liczy od zera, kolejne przyrostowo.
- Histogram zawiera liczby całkowite, dlatego wynik musi być identyczny (bit w bit) z trybem `full`.
- Zysk czasu może pojawić się tylko przy S wyraźnie mniejszym od W.

### Pliki wynikowe

Pliki zapisujemy w katalogu `--out`. Format CSV: wiersz nagłówka, separator przecinek, kropka dziesiętna, entropia z 12 miejscami po przecinku. W wersji MPI pliki zapisuje tylko proces 0.

`entropia.csv` – jeden wiersz na okno:

```csv
okno,poczatek,dlugosc,entropia,niepelne
0,0,4096,4.512345678901,0
1,4096,4096,7.981234567890,0
2,8192,1808,7.954321098765,1
```

`zmiany.csv` – lista zmian przekraczających próg:

```csv
pozycja,entropia_przed,entropia_po,roznica
4096,4.512345678901,7.981234567890,3.468888888989
```

`roznica` to wartość bezwzględna |H(k+1) − H(k)|.

`czasy.csv` – czasy etapów jednego uruchomienia, w sekundach, z 6 miejscami po przecinku:

```csv
etap,czas_s
odczyt,0.120000
obliczenia,0.850000
wykrywanie_zmian,0.001000
zapis,0.030000
calosc,1.010000
```

- Etapy wspólne: `odczyt`, `obliczenia` (histogramy i entropia, czyli rdzeń), `wykrywanie_zmian`, `zapis`, `calosc`.
- Dodatkowo w MPI: `rozeslanie`, `zebranie`.
- Dodatkowo w GPU: `transfer_na_gpu`, `transfer_z_gpu`. Etap `obliczenia` mierzy wtedy tylko pracę na GPU, a przed końcem pomiaru trzeba zsynchronizować urządzenie.
- Zegar: `std::chrono::steady_clock`, `MPI_Wtime` albo zdarzenia GPU.

## Wymagania dla poszczególnych wersji

### `entropy_seq` – wersja referencyjna (MG)

- Najprostszy możliwy kod; to wzorzec do testów wszystkich wersji.
- Wczytuje cały plik do pamięci.
- Obsługuje tryby `full` i `incremental`.

### `entropy_omp` – OpenMP (MG)

- Okna rozdzielone między wątki przez `#pragma omp parallel for schedule(static)`.
- Każdy wątek ma własny histogram, a każde okno zapisuje wynik do własnej komórki tablicy. Nie ma wspólnych liczników ani operacji atomowych.
- W trybie `incremental` program sam dzieli okna na ciągłe zakresy, po jednym na wątek.

### `entropy_mpi` – MPI (KŚ)

- Okna dzielone na ciągłe, mniej więcej równe zakresy, po jednym na proces.
- Proces potrzebuje bajtów od początku swojego pierwszego okna do końca ostatniego, czyli zakładki W − S bajtów wspólnej z sąsiadem.
- Dane do procesów: każdy czyta swój fragment sam albo proces 0 rozsyła je (np. `MPI_Scatterv`). Wybór należy opisać i zmierzyć.
- Wyniki trafiają do procesu 0 (np. `MPI_Gatherv`), który wykrywa zmiany i zapisuje pliki.
- Każde okno jest liczone dokładnie raz, z pozycją globalną w pliku.
- Pomiary na jednym komputerze oznaczamy jako lokalne.

### `entropy_gpu` – CUDA lub OpenCL (KŚ)

- Plik trafia na urządzenie raz, a z powrotem wraca tylko tablica entropii.
- Proponowany układ: jeden blok wątków na okno, histogram w pamięci współdzielonej zliczany operacjami atomowymi, entropia w `double` liczona redukcją w bloku.
- Do zbadania: wpływ zawartości pliku na operacje atomowe. Najgorszy przypadek to `stale.bin`, gdzie wszystkie wątki trafiają w ten sam licznik.

## Generator plików testowych (MG)

Generator przyjmuje ziarno losowania i zapisuje je, aby pliki dało się odtworzyć. Pliki trafiają do `data/`.

| Plik            | Zawartość                                    | Oczekiwany wynik                      |
| --------------- | -------------------------------------------- | ------------------------------------- |
| `stale.bin`     | same zera                                    | H = 0 w każdym oknie                  |
| `wzorzec16.bin` | bajty 0–15 powtarzane po kolei               | H = 4 (gdy W jest wielokrotnością 16) |
| `losowy.bin`    | losowe bajty                                 | H blisko 8                            |
| `segmenty.bin`  | sklejone segmenty: tekst, zera, losowe bajty | zmiany w znanych miejscach            |
| `pusty.bin`     | 0 bajtów                                     | brak okien                            |
| `nierowny.bin`  | rozmiar niepodzielny przez S                 | ostatnie okno niepełne                |

- Granice segmentów celowo nie wypadają na wielokrotnościach 4096.
- Do pomiarów generator może tworzyć też duże pliki o zadanym rozmiarze, np. 100 MB, 500 MB i 1 GB.

Implementacja: `tools/gen_data.py` (Python 3, tylko biblioteka standardowa).

```
python3 tools/gen_data.py [--out data] [--seed 1]            # zestaw testowy
python3 tools/gen_data.py [--out data] [--seed 1] --size N   # losowy_N.bin, N bajtów losowych
```

- Pliki podstawowe mają 64 KiB (65 536 B); `nierowny.bin` ma 66 536 B.
- `segmenty.bin`: tekst (10 000 B), zera (15 000 B), losowe (20 000 B), tekst (20 536 B). Tekst to losowe małe litery i spacje.
- `segmenty_granice.csv` (`pozycja,segment_przed,segment_po`) zawiera granice 10000, 25000 i 45000.
- Ziarno trafia do `ziarno.txt`. To samo ziarno daje identyczne pliki.

## Testy poprawności

- `entropia.csv` każdej wersji porównujemy z `entropy_seq`: te same okna, pozycje i znaczniki, a |ΔH| ≤ 1e-9.
- Pozycje w `zmiany.csv` muszą być identyczne.
- Tryb `incremental` musi dać ten sam wynik co `full`.
- Wartości oczekiwane z tabeli generatora sprawdzamy dla każdej wersji. Największa różnica to błąd entropii.
- Przypadki brzegowe: pliki brzegowe, więcej wątków lub procesów niż okien, okna na granicach fragmentów MPI.

## Pomiary (zasady z wytycznych)

- Najpierw jedno uruchomienie na rozgrzewkę, potem co najmniej 5 pomiarów. Podajemy medianę i rozrzut.
- Czas obliczeń (rdzeń) i czas całości raportujemy osobno.
- Dla OpenMP i MPI liczymy przyspieszenie S(p) = T(1) / T(p) i efektywność E(p) = S(p) / p, przy tym samym pliku i tej samej metodzie. Podajemy liczbę wątków, procesów, rdzeni i węzłów.
- Dla GPU raportujemy czas, GB/s oraz wpływ rozmiaru pliku i rozmiaru bloku. Liczby wątków GPU nie używamy we wzorze na efektywność. Przy przyspieszeniu zawsze podajemy wersję odniesienia.
- Raportujemy też błąd entropii oraz dokładność wskazania granic z `segmenty_granice.csv`: odległość w bajtach, granice pominięte i fałszywe zmiany.
- Surowe wyniki trafiają do `results/`. Wykresy przygotowuje KŚ; pliki, z których powstały, też trafiają do repozytorium.

## Budowanie i uruchomienie

Wersje MG:

```
python3 tools/gen_data.py                     # pliki testowe do data/
make -C seq                                   # buduje seq/entropy_seq
seq/entropy_seq --input data/segmenty.bin --out results/seq
seq/entropy_seq --input data/segmenty.bin --step 1024 --mode incremental --out results/seq_inc
make -C omp                                   # buduje omp/entropy_omp
omp/entropy_omp --input data/segmenty.bin --threads 4 --out results/omp
```

Pliki źródłowe:

- `common/entropy_common.hpp` – kod wspólny dla `entropy_seq` i `entropy_omp`: parametry, podział na okna, entropia, `compute_range` (entropia zakresu okien, tryb `full` lub `incremental`), wykrywanie zmian, zapis plików CSV.
- `seq/main.cpp` – wersja sekwencyjna: odczyt → obliczenia → wykrywanie zmian → zapis, z pomiarem czasu etapów.
- `seq/Makefile` – `make -C seq`, `make -C seq clean`. Flagi: `-std=c++17 -O2 -Wall -Wextra`, bez `-ffast-math`, bo mogłaby zmienić kolejność sumowania entropii.
- `omp/main.cpp` – wersja OpenMP; różni się od `seq/main.cpp` tylko etapem obliczeń. Tryb `full`: `parallel for schedule(static)` po oknach. Tryb `incremental`: każdy wątek liczy `compute_range` dla swojego ciągłego zakresu okien.
- `omp/Makefile` – jak `seq/Makefile`, z dodatkową flagą `-fopenmp`.

Wyniki `entropy_omp` są identyczne (bit w bit) z `entropy_seq` dla każdej liczby wątków, bo każde okno liczy ta sama funkcja.

Wersje MPI i GPU: [do uzupełnienia przez KŚ].
