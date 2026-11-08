Entropia i segmentacja plików binarnych

Sprawozdanie PRIR - Projekt 16 (C05)

Mariusz Gosławski, Krzysztof Śliwiński, Informatyka, studia stacjonarne, specjalność Cyberbezpieczeństwo

# Informacje o dokumencie

**TODO**: usunąć przed oddaniem

| Przedmiot      | Programowanie równoległe i rozproszone                |
| -------------- | ----------------------------------------------------- |
| Temat          | nr 16 – Entropia i segmentacja plików binarnych (C05) |
| Rok akademicki | 2026/2027                                             |
| Repozytorium   | <https://github.com/NoMariusz/project-prir-entropy>   |

Oznaczenia: **Co tu wpisać:** – wskazówka do usunięcia; **Przykładowa treść:** – tekst do poprawienia; **\[DO UZUPEŁNIENIA\]** – brakujące dane.

# Podział pracy

**TODO**: usunąć przed oddaniem

| Zadanie                                         | Osoba                   |
| ----------------------------------------------- | ----------------------- |
| Przygotowanie Repozytorium (README.md)          | MG                      |
| Generator plików testowych                      | MG                      |
| Wersja referencyjna (sekwencyjna)               | MG                      |
| Wersja OpenMP                                   | MG                      |
| Wersja MPI                                      | KŚ                      |
| Wersja CUDA lub OpenCL                          | KŚ                      |
| Wykresy                                         | KŚ                      |
| Sprawozdanie: rozdziały 1–3                     | MG                      |
| Sprawozdanie: rozdział 4                        | każdy dla swoich wersji |
| Sprawozdanie: rozdziały 5–8, w tym podsumowanie | KŚ                      |
| Prezentacja                                     | MG/KŚ                   |

SPIS TREŚCI

[Informacje o dokumencie 1](#informacje-o-dokumencie)

[Podział pracy 1](#podział-pracy)

[1\. Wstęp 2](#1-wstęp)

[1.1 Opis problemu 3](#11-opis-problemu)

[1.2 Entropia Shannona 3](#12-entropia-shannona)

[1.3 Zastosowania analizy entropii 3](#13-zastosowania-analizy-entropii)

[2\. Cel i zakres projektu 4](#2-cel-i-zakres-projektu)

[2.1 Cel projektu 5](#21-cel-projektu)

[2.2 Zakres 5](#22-zakres)

[3\. Założenia 6](#3-założenia)

[3.1 Dane wejściowe i parametry 7](#31-dane-wejściowe-i-parametry)

[3.2 Reguły liczenia okien 7](#32-reguły-liczenia-okien)

[3.3 Wykrywanie zmian 8](#33-wykrywanie-zmian)

[3.4 Przyrostowa aktualizacja histogramu (rozszerzenie) 8](#34-przyrostowa-aktualizacja-histogramu-rozszerzenie)

[3.5 Format wyniku 9](#35-format-wyniku)

[3.6 Dobór parametrów okna i progu 10](#36-dobór-parametrów-okna-i-progu)

[4\. Implementacje 10](#4-implementacje)

[4.1 Wersja referencyjna 11](#41-wersja-referencyjna)

[4.2 Wersja OpenMP 16](#42-wersja-openmp)

[4.3 Wersja MPI - TODO 16](#43-wersja-mpi---todo)

[4.4 Wersja CUDA lub OpenCL - TODO 16](#44-wersja-cuda-lub-opencl---todo)

[5\. Testy poprawności - TODO 17](#5-testy-poprawności---todo)

[5.1 Pliki testowe 17](#51-pliki-testowe)

[5.2 Sposób porównania 17](#52-sposób-porównania)

[5.3 Wyniki testów i różnice między wersjami 17](#53-wyniki-testów-i-różnice-między-wersjami)

[6\. Metodyka pomiarów - TODO 18](#6-metodyka-pomiarów---todo)

[6.1 Sprzęt i oprogramowanie 18](#61-sprzęt-i-oprogramowanie)

[6.2 Sposób pomiaru 18](#62-sposób-pomiaru)

[6.3 Plan eksperymentów 18](#63-plan-eksperymentów)

[6.4 Miary 19](#64-miary)

[7\. Wyniki - TODO 19](#7-wyniki---todo)

[7.1 Przykładowy wynik programu 19](#71-przykładowy-wynik-programu)

[7.2 OpenMP i MPI 19](#72-openmp-i-mpi)

[7.3 CUDA lub OpenCL 19](#73-cuda-lub-opencl)

[7.4 Tryb przyrostowy 19](#74-tryb-przyrostowy)

[7.5 Zestawienie wersji 19](#75-zestawienie-wersji)

[8\. Wnioski I podsumowanie - TODO 20](#8-wnioski-i-podsumowanie---todo)

# 1. Wstęp

## 1.1 Opis problemu

Każdy plik to ciąg bajtów. Plik może mieć w środku części o różnej budowie, na przykład nagłówek, zwykły tekst, obszar wypełniony zerami albo dane skompresowane. Te części zwykle nie są nigdzie opisane i nie widać ich gołym okiem.

Prosty sposób, żeby je znaleźć, to podzielić plik na kolejne okna o stałej długości i w każdym oknie policzyć entropię rozkładu bajtów. Entropia mówi, jak bardzo zróżnicowane są bajty w oknie. Gdy entropia sąsiednich okien wyraźnie się różni, w tym miejscu najpewniej kończy się jedna część pliku i zaczyna druga.

Program z tego projektu robi dokładnie to. Wynikiem jest wykres entropii względem pozycji w pliku oraz lista miejsc, w których entropia zmienia się bardziej niż zadany próg. Program wskazuje tylko, gdzie są granice. Nie rozpoznaje, czym są obszary między nimi.

Sama wysoka entropia nie dowodzi, że plik jest złośliwy lub zaszyfrowany. Oznacza tylko, że bajty w oknie są rozłożone równomiernie. Tak samo wyglądają zwykłe pliki ZIP, JPEG czy MP4.

Liczenie histogramów dla dużego pliku to dużo prostej pracy na każdym bajcie. Okna można jednak liczyć niezależnie od siebie, więc to zadanie dobrze nadaje się do zrównoleglenia. Dlatego program powstał w czterech wersjach: sekwencyjnej, OpenMP, MPI oraz CUDA lub OpenCL.

## 1.2 Entropia Shannona

Dla okna o długości n bajtów liczymy histogram c₀ … c₂₅₅, czyli ile razy wystąpił każdy z 256 możliwych bajtów. Udział bajtu i to pᵢ = cᵢ / n. Entropia okna:  
H(X) = -\sum\_{i=1}^{255} pᵢ \log_2 pᵢ
Składniki z pᵢ = 0 pomijamy, a wynik jest w bitach na bajt, od 0 do 8.

| Zawartość okna                         | Entropia (bit/bajt) |
| -------------------------------------- | ------------------- |
| jeden powtarzany bajt, np. same zera   | 0                   |
| dwa różne bajty, każdy w połowie okna  | 1                   |
| 16 różnych bajtów, każdy równie często | 4                   |
| zwykły tekst                           | ok. 4–5             |
| dane skompresowane lub zaszyfrowane    | blisko 8            |

## 1.3 Zastosowania analizy entropii

Wykres entropii pliku to popularne narzędzie w analizie plików binarnych, zwłaszcza w cyberbezpieczeństwie. Kilka typowych zastosowań:

- Analiza oprogramowania układowego (firmware). Obraz pamięci urządzenia często składa się z kilku sklejonych części: programu rozruchowego, skompresowanego systemu plików i pustych obszarów. Wykres entropii szybko pokazuje, gdzie te części się zaczynają. Taką funkcję ma na przykład narzędzie binwalk.
- Wstępna analiza złośliwego oprogramowania. Programy spakowane lub zaszyfrowane mają długie obszary o entropii bliskiej 8. To sygnał, że kod trzeba najpierw rozpakować, ale jeszcze nie dowód, że program jest groźny.
- Informatyka śledcza. Na dysku lub w pliku można szukać obszarów zaszyfrowanych albo ukrytych danych doklejonych na końcu pliku.
- Wybór metody kompresji. Obszary o wysokiej entropii są już skompresowane i nie opłaca się ich kompresować ponownie.

We wszystkich tych przypadkach entropia jest tylko pierwszym krokiem. Wskazuje miejsca warte uwagi, a ostateczną ocenę trzeba wykonać innymi metodami.

# 2. Cel i zakres projektu

## 2.1 Cel projektu

Celem projektu jest przygotowanie programu, który dzieli plik na okna, w każdym oknie liczy entropię rozkładu bajtów i wskazuje miejsca, w których entropia wyraźnie się zmienia. Dodatkowo zapewnia on obsługę końcowego niepełnego okna i pomija składniki histogramu o zerowym prawdopodobieństwie.

Program powstał w czterech wersjach:

- sekwencyjnej, która jest wersją referencyjną i wzorcem do testów,
- OpenMP, z obliczeniami na wielu wątkach jednego procesora,
- MPI, z obliczeniami w wielu procesach, także na kilku komputerach,
- CUDA lub OpenCL, z obliczeniami na karcie graficznej.

Wszystkie wersje mają te same parametry, przetwarzają te same pliki i zapisują wynik w tym samym formacie. Dzięki temu można porównać wyniki okno po oknie i zmierzyć, ile zyskujemy na równoległości.

Ponadto celem projektu jest analiza uzyskanych wyników i porównanie przygotowanych wersji w ramach niniejszego sprawozdania. Uzyskane wyniki w ramach projektu powinny dostarczyć odpowiedzi na poniższe pytania (odpowiedzi w rozdziale 8):

1. Czy wszystkie wersje dają ten sam wynik co wersja referencyjna?
2. Ile przyspieszają wersje równoległe, gdy doliczymy odczyt pliku, przesyłanie danych i komunikację?
3. Czy zmiany entropii pokazują granice segmentów doklejonych do pliku i jak dokładnie?

## 2.2 Zakres

W zakresie projektu są:

- program wiersza poleceń w czterech wersjach: referencyjnej, OpenMP, MPI oraz CUDA lub OpenCL,
- okna o stałej długości i stałym kroku oraz stały próg zmiany,
- obsługa przypadków brzegowych: pusty plik, plik krótszy od okna, ostatnie niepełne okno,
- rozszerzenie dla zespołu dwuosobowego: przyrostowa aktualizacja histogramu (opis w 3.4),
- generator plików testowych o znanej zawartości i znanych granicach segmentów,
- testy poprawności każdej wersji względem wersji referencyjnej,
- pomiary czasu, przyspieszenia, efektywności i przepustowości oraz wykresy.

Poza zakresem projektu są:

- rozpoznawanie rodzaju danych w oknie (tekst, kod, obraz itp.),
- ocena, czy plik jest złośliwy lub zaszyfrowany,
- adaptacyjny krok okna (drugie możliwe rozszerzenie z karty tematu),
- interfejs graficzny (program obsługuje się z wiersza poleceń).

# 3. Założenia

## 3.1 Dane wejściowe i parametry

Wejściem jest dowolny plik, w dowolnym formacie, który program czyta jako ciąg bajtów. Do testów i pomiarów używamy plików z generatora (opis w 5.1) oraz dwóch prawdziwych plików: lorem_ipsum.txt (tekst) i only_text.pdf (dokument PDF).

Każda wersja to osobny program z tymi samymi parametrami, np.:

entropy_seq --input plik.bin --window 4096 --step 4096 --threshold 1.0

| Parametr     | Znaczenie                                                 | Domyślnie |
| ------------ | --------------------------------------------------------- | --------- |
| \--input     | ścieżka do pliku                                          | wymagany  |
| \--window    | długość okna **W** w bajtach                              | 4096      |
| \--step      | krok okna **S** w bajtach                                 | 4096      |
| \--threshold | próg zmiany entropii (bit/bajt)                           | 1.0       |
| \--mode      | full – histogram od zera, incremental – przyrostowo (3.4) | full      |
| \--out       | katalog na wyniki                                         | ./results |

Program sprawdza parametry przed rozpoczęciem pracy:

- W ≥ 1,
- 1 ≤ S ≤ W, więc okna nie mają przerw między sobą i każdy bajt pliku trafia do co najmniej jednego okna,
- próg ≥ 0,
- nieznany parametr albo parametr bez wartości to błąd.

Kody wyjścia:

- 0 – sukces
- 1 – błędne parametry (komunikat na stderr)
- 2 – błąd odczytu pliku lub zapisu wyników.

Parametry dodatkowe poszczególnych wersji (liczba wątków, liczba procesów, rozmiar bloku GPU) opisuje rozdział 4.

## 3.2 Reguły liczenia okien

Plik ma N bajtów, okno ma długość W, a krok wynosi S.

- Okno pełne nr k obejmuje bajty od k·S do k·S + W − 1. Pełnych okien jest F = ((N − W) / S) + 1, gdy N ≥ W, a w przeciwnym razie F = 0.
- Okno niepełne (najwyżej jedno) powstaje, gdy ostatnie pełne okno nie sięga końca pliku. Zaczyna się w bajcie F·S i obejmuje resztę pliku. Gdy plik jest krótszy od okna, jest to jedyne okno. Entropię okna niepełnego liczymy z dostępnych bajtów, a okno oznaczamy w wyniku jako niepełne.
- Pusty plik nie ma okien. Pliki wynikowe zawierają wtedy tylko nagłówki.
- Gdy S < W, okna na siebie nachodzą i ten sam bajt należy do kilku okien. Gdy S = W, okna leżą jedno za drugim.

Przykład: N = 10 000, W = S = 4096 daje pełne okna 0 (bajty 0–4095) i 1 (4096–8191) oraz okno niepełne nr 2 od bajtu 8192 o długości 1808.

Entropię każdego okna liczymy tak samo we wszystkich wersjach:

- histogram zawiera liczby całkowite, więc we wszystkich wersjach jest identyczny,
- obliczenia wykonujemy w typie double, a składniki z cᵢ = 0 pomijamy,
- sumujemy po i rosnąco od 0 do 255, bo inna kolejność dodawania liczb zmiennoprzecinkowych może dać minimalnie inny wynik,
- wersje sekwencyjna i OpenMP są kompilowane bez opcji -ffast-math, ponieważ ta opcja pozwala kompilatorowi zmienić kolejność sumowania.

## 3.3 Wykrywanie zmian

- Zmiana występuje między oknem k a k + 1, gdy |H(k+1) − H(k)| > próg. Pozycją zmiany jest początek okna k + 1.
- Różnica równa progowi nie jest zmianą.
- Porównujemy tylko okna pełne. Okno niepełne pomijamy, bo jest krótsze i jego entropia nie jest porównywalna z sąsiadem.
- Program wskazuje granicę z dokładnością do kroku S. Prawdziwa granica leży zwykle wewnątrz któregoś okna, więc wykryta pozycja może się od niej różnić najwyżej o mniej więcej długość okna. Mniejszy krok pozwala dokładniej wskazać granicę, ale wymaga liczenia większej liczby okien.
- Przy nachodzących oknach (S < W) entropia zmienia się łagodniej, bo okno przechodzi przez granicę stopniowo. Jedna granica może wtedy dać kilka zmian w sąsiednich pozycjach albo żadnej, jeśli każdy pojedynczy skok jest mniejszy od progu. Wpływ tego ograniczamy doborem progu.
- Wykrywanie zmian to jedno przejście po tablicy F liczb, więc jest bardzo tanie. Wszystkie wersje wykonują je sekwencyjnie tą samą funkcją na gotowej tablicy entropii. Zrównoleglamy tylko histogramy i entropię.

Przykład: dla kolejnych okien o entropii 4,5; 4,6; 7,9; 7,9 i progu 1,0 jedyna zmiana jest między oknami nr 1 i 2 (różnica 3,3), w pozycji 2·S.

## 3.4 Przyrostowa aktualizacja histogramu (rozszerzenie)

Gdy okno przesuwa się o krok mniejszy od swojej długości, część bajtów zostaje w oknie. Zamiast liczyć histogram od zera, odejmujemy bajty, które wypadły z okna, i dodajemy te, które weszły. Entropię liczymy dalej z całego histogramu, więc wynik musi być identyczny jak przy liczeniu od zera. Tryb włącza parametr --mode incremental. Jednak przy użyciu tego trybu zysk czasu może pojawić się tylko przy S wyraźnie mniejszym od W.

## 3.5 Format wyniku

entropia.csv – jeden wiersz na okno.

okno,poczatek,dlugosc,entropia,niepelne

0,0,4096,7.735312384543,0

1,1024,4096,7.961076792262,0

2,2048,4096,7.809322583805,0

3,3072,4096,7.365390706299,0

4,4096,4096,6.821844878775,0

zmiany.csv – (lista zmian przekraczających ustalony próg) pozycja zmiany w bajtach, entropia przed i po.

pozycja,entropia_przed,entropia_po,roznica

183296,6.347664566236,5.165865850681,1.181798715555

366592,7.145766749499,5.862761187348,1.283005562151

367616,5.862761187348,3.717886939667,2.144874247681

czasy.csv – czasy trwania poszczególnych etapów:

etap,czas_s

odczyt,0.000834

obliczenia,0.017739

wykrywanie_zmian,0.000001

zapis,0.000940

calosc,0.019514

Poniżej opis poszczególnych etapów:

| **Etap**         | **Co obejmuje**                                              |
| ---------------- | ------------------------------------------------------------ |
| odczyt           | wczytanie całego pliku do pamięci                            |
| obliczenia       | histogramy i entropia wszystkich okien, czyli rdzeń obliczeń |
| wykrywanie_zmian | porównanie sąsiednich okien                                  |
| zapis            | zapis plików CSV                                             |
| calosc           | cały program od odczytu do zapisu                            |

W przypadku tego pliku z wynikami, dokładnej analizie poddany zostanie jedynie czas "obliczenia", lub "całość", jako że to na jego wartość będzie wpływać zrównoleglanie, które w tym projekcie ma dotyczyć obliczeń entropi (w tym budowania histogramów i dzielenia). "wykrywanie_zmian", lub operacje odczytu zapisu nie są celem zrównoleglania.  
Powód dla którego umieszczono czasy innych etapów jest spełnienie wymogów z sekcji "Czas i skalowanie" wytycznych projektu.

## 3.6 Dobór parametrów okna i progu

Wynik programu zależy od trzech parametrów, a każdy z nich coś daje i coś kosztuje:

- Długość okna W - Krótkie okno pokazuje drobne zmiany, ale entropia jest mniej dokładna, bo okno ma mało bajtów. Okno krótsze niż 256 B nigdy nie osiągnie entropii 8, nawet dla danych losowych. Długie okno daje stabilny wynik, ale rozmywa małe obszary i gorzej wskazuje granice.
- Krok S - Mniejszy krok dokładniej wskazuje granice, ale liczba okien rośnie mniej więcej W/S razy, a z nią czas obliczeń. Tu właśnie może pomóc tryb przyrostowy (3.4).
- Próg (treshold) - Niski próg wykrywa też drobne wahania, czyli daje fałszywe zmiany. Wysoki próg może pominąć granice między podobnymi obszarami, np. tekstem i kodem.

Jako wartości domyślne przyjęliśmy W = S = 4096 B i próg 1,0 bit/bajt. 4096 B to typowy rozmiar strony pamięci i sektora dysku, a jednocześnie to wystarczająco dużo bajtów, żeby entropia danych losowych była bliska 8.

# 4. Implementacje

## 4.1 Wersja referencyjna

Język: C++17, kompilator g++ 11.4, flagi "-std=c++17 -O2 -Wall –Wextra".

Wersja referencyjna to najprostsza wersja programu, a jej wyniki są wzorcem dla pozostałych. Kod jest podzielony na dwa pliki:

- seq/main.cpp – funkcja main, która wywołuje kolejne etapy i mierzy ich czas,
- common/entropy_common.hpp – kod wspólny z wersją OpenMP: odczyt parametrów i pliku, podział na okna, entropia, wykrywanie zmian i zapis plików CSV.

Najważniejsze decyzje:

| **Decyzja**                                             | **Powód**                                                                                                                                                                |
| ------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Cały plik w pamięci zamiast czytania porcjami           | Najprostszy kod. Okna mogą na siebie nachodzić, a przy czytaniu porcjami trzeba by pilnować bajtów na granicach porcji. Aczkolwiek plik musi się zmieścić w pamięci RAM. |
| Okno niepełne zawsze liczone od zera                    | Ma inną długość, więc przyrostowe poprawianie histogramu nie ma sensu. To tylko jedno okno.                                                                              |
| Kompilacja bez -ffast-math                              | Ta opcja pozwala zmienić kolejność sumowania, a wynik ma być powtarzalny bit w bit. Aczkolwiek obliczenia mogą być nieco wolniejsze.                                     |
| Wynik okna zapisywany pod numerem okna (entropies\[k\]) | Kolejność liczenia okien nie wpływa na wynik. W wersjach równoległych każdy wątek pisze do innych komórek.                                                               |

Działanie programu:
`flowchart TD
    A["parse_args<br/>odczyt i sprawdzenie parametrów"] -->|błąd| X1["kod wyjścia 1"]
    A --> B["read_file<br/>cały plik do pamięci (N bajtów)"]
    B -->|błąd| X2["kod wyjścia 2"]
    B --> C["full_count<br/>liczba pełnych okien F"]
    C --> D{"--mode"}
    D -->|full| E1["compute_range<br/>każde okno: histogram od zera"]
    D -->|incremental| E2["compute_range<br/>pierwsze okno od zera,<br/>kolejne przyrostowo"]
    E1 --> F{"has_partial?<br/>czy jest okno niepełne"}
    E2 --> F
    F -->|tak| G["entropy_of<br/>okno niepełne od zera"]
    F -->|nie| H
    G --> H["detect_changes<br/>porównanie sąsiednich pełnych okien"]
    H --> I["write_results<br/>entropia.csv, zmiany.csv"]
    I --> J["write_times<br/>czasy.csv"]`

Funkcja _main_ wykonuje te etapy po kolei. Przed i po każdym etapie odczytuje zegar _steady_clock_:

Przy odczycie pliku program wczytuje go do pamięci w trybie binarnym, więc żaden bajt nie jest zmieniany, np. przy końcach linii w systemie Windows:  
Liczenie okien i wyznaczanie czy istnieje okno niepełne jest zgodne z zasadami z punktu 3.2:

Natomiast jeśli chodzi o liczenie entropi to opiera się ona o histogram w formie "_const uint64t hist\[256\]", więc jest zmienną lokalną i zawiera 256 liczb, czyli wartośc bajtu jest od razu numerem licznika. Zgodnie z wytycznymi puste liczniki są pomijane (continue), bo log₂(0) nie istnieje:_  
, gdzie pierwsze okno zakresu ma liczony histogram od zera, a kolejne przez poprawienie histogramu. Precyzyjniej przy przejściu z okna k − 1 do okna k:

- z początku okna wypada S bajtów \[prev, prev + S),
- na końcu dochodzi S bajtów \[prev + W, prev + W + S).

Przykład dla W = 4096 i S = 1024: okno 0 to bajty 0–4095, a okno 1 to bajty 1024–5119. Program odejmuje bajty 0–1023 i dodaje bajty 4096–5119. Pozostałe 3072 bajty nie są czytane drugi raz.

Ostatnią z ważniejszych rzeczy jest wykrywanie zmian przekraczających próg. Pętla porównuje tylko pełne okna i w przypadku przekraczania progu dodawany jest wpis w "_std::vector&lt;Change&gt;_".

## 4.2 Wersja OpenMP

Język: C++17, kompilator g++ 11.4, flagi "-std=c++17 -O2 -Wall -Wextra -fopenmp".

Wersja OpenMP (omp\\main.cpp) różni się od wersji referencyjnej tylko etapem obliczenia. Odczyt pliku, liczba okien, okno niepełne, wykrywanie zmian, zapis CSV i pomiar czasu działają tak samo. Używają tych samych funkcji z common/entropy_common.hpp.

Najważniejsze decyzje:

| **Decyzja**                                                   | **Powód**                                                                                                                                                                               |
| ------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| W trybie przyrostowym ręczny podział zamiast "_parallel for_" | Wątek musi przechodzić przez swoje okna kolejno i przenosić histogram między nimi. Ręczny podział daje gwarancję ciągłego zakresu i pozwala użyć bez zmian funkcji "_compute_range_".   |
| Użycie "_schedule(static)_"                                   | Wszystkie pełne okna mają tę samą długość W, więc wymagają tyle samo pracy. Równy podział z góry wystarcza, a "_dynamic"_ pewnie dodałby tylko koszt rozdzielania zadań w czasie pracy. |
| Wynik zapisywany pod numerem okna "_(entropies\[k\]_)"        | Brak zapisu do wspólnych miejsc i niezależność wyniku od kolejności pracy wątków.                                                                                                       |
| Kod wspólny z wersją referencyjną                             | Te same funkcje dają identyczne wyniki, a różnica między wersjami to tylko kilka linii, łatwych do porównania.                                                                          |

Działanie programu:
`flowchart TD
    A["parse_args + --threads<br/>omp_set_num_threads"] --> B["read_file<br/>(sekwencyjnie)"]
    B --> C["full_count → F"]
    C --> D{"--mode"}
    D -->|full| E1["#pragma omp parallel for schedule(static)<br/>okna rozdzielone między wątki"]
    D -->|incremental| E2["#pragma omp parallel<br/>każdy wątek: ciągły zakres okien<br/>compute_range(first, end)"]
    E1 --> F["koniec regionu równoległego<br/>(niejawna bariera)"]
    E2 --> F
    F --> G["okno niepełne<br/>(sekwencyjnie)"]
    G --> H["detect_changes, write_results, write_times<br/>(sekwencyjnie)"]`

W tej wersji występuje dodatkowy paramter "_\--threads_". Gdy go nie podamy, liczbę wątków ustala OpenMP, zwykle według zmiennej "_OMP_NUM_THREADS_" albo liczby rdzeni.

W przypadku trybu pełnego (bez przyrostowej aktualizacji histogramu) pętla po oknach jest taka sama jak wewnątrz compute*range. OpenMP dzieli zakres 0 … F − 1 na mniej więcej równe, ciągłe części, po jednej na wątek. Przy 4 wątkach i F = 1000 oknach wątek 0 liczy okna 0–249, wątek 1 okna 250–499 itd. Wątki nie potrzebuję sunchronizacji podczas liczenia, bo każdy wątek zapisuje wynik do osobnej komórki z "\_entropies\[k\]*", każdy wątek ma swój histogram, a dane wejściowe z pliku są tylko czytane.

Natomiast przy przyrostowej aktualizacji histogramu, histogram okna k powstaje z histogramu okna k − 1. Wątek musi więc liczyć okna po kolei w ciągłym zakresie i przez cały czas trzymać jeden histogram. Dlatego program sam dzieli okna i wywołuje dla każdego wątku tę samą funkcję "c_ompute_rang_e", której używa wersja referencyjna:  
, więc każde okno jest liczone dokładnie raz. Przykład dla F = 10 i 3 wątków:

| **Wątek** | **first** | **end** | **Okna**   |
| --------- | --------- | ------- | ---------- |
| 0         | 0         | 3       | 0, 1, 2    |
| 1         | 3         | 6       | 3, 4, 5    |
| 2         | 6         | 10      | 6, 7, 8, 9 |

W "_compute_range_" pierwsze okno zakresu jest liczone od zera, a kolejne przyrostowo (opis w 4.1). Każdy wątek ma własny histogram jako zmienną lokalną funkcji.

## 4.3 Wersja MPI - TODO

Język i biblioteka MPI: \[DO UZUPEŁNIENIA\].

**Co tu wpisać:**

- Podział pliku: każdy proces dostaje zakres okien i zakładkę (okno − krok bajtów) potrzebną do ostatnich okien.
- Każde okno liczy tylko proces, w którego zakresie leży początek okna; pozycja globalna = przesunięcie fragmentu + pozycja lokalna.
- Rozesłanie danych i zebranie wyników (np. MPI_Scatterv, MPI_Gatherv); ich koszt mierzony osobno.

## 4.4 Wersja CUDA lub OpenCL - TODO

Technologia i język: \[DO UZUPEŁNIENIA\].

**Co tu wpisać:**

- Przypisanie okien do bloków wątków; histogram w pamięci współdzielonej, zliczany operacjami atomowymi.
- Wpływ zawartości pliku: dla samych zer wszystkie wątki zwiększają ten sam licznik (najgorszy przypadek).
- Dane przesyłane na urządzenie raz, z powrotem tylko tablica entropii; transfery mierzone osobno.
- Wybrany rozmiar bloku.

# 5. Testy poprawności - TODO

Przed pomiarami czasu każda wersja musi dać ten sam wynik co wersja referencyjna.

## 5.1 Pliki testowe

Generator zapisuje użyte ziarno losowania, aby pliki dało się odtworzyć.

| Plik            | Zawartość                            | Oczekiwany wynik                      |
| --------------- | ------------------------------------ | ------------------------------------- |
| stale.bin       | same zera                            | H = 0                                 |
| wzorzec16.bin   | bajty 0–15 powtarzane po kolei       | H = 4                                 |
| losowy.bin      | losowe bajty                         | H blisko 8                            |
| segmenty.bin    | sklejone: tekst, zera, losowe bajty  | zmiany w znanych miejscach            |
| lorem_ipsum.txt | tekst lotem ipsum zakodowany w UTF-8 | taki sam dla wszystkich implementacji |
| only_text.pdf   | realny dokument pdf                  | taki sam dla wszystkich implementacji |

## 5.2 Sposób porównania

- Entropia porównywana okno po oknie, z tolerancją \[np. 1e-9\].
- Listy zmian muszą być takie same.
- Tryb przyrostowy musi dać ten sam wynik co liczenie od zera.
- Okna na granicach fragmentów MPI liczone dokładnie raz.

## 5.3 Wyniki testów i różnice między wersjami

Wymagania mówią, że różnice algorytmów i dokładności trzeba jawnie opisać.

| Wersja                          | Wynik testów             | Różnice względem referencji               |
| ------------------------------- | ------------------------ | ----------------------------------------- |
| Referencyjna – tryb przyrostowy | \[OK / największy błąd\] | \[np. brak\]                              |
| OpenMP                          |                          |                                           |
| OpenMP – tryb przyrostowy       |                          |                                           |
| MPI                             |                          |                                           |
| CUDA lub OpenCL                 |                          | \[np. funkcja log₂, kolejność sumowania\] |

# 6. Metodyka pomiarów - TODO

## 6.1 Sprzęt i oprogramowanie

| Element                                  | Wartość                           |
| ---------------------------------------- | --------------------------------- |
| Procesor (model, rdzenie, wątki)         | \[DO UZUPEŁNIENIA\]               |
| Pamięć RAM                               |                                   |
| Karta graficzna                          |                                   |
| System operacyjny                        |                                   |
| Kompilatory i wersje                     | gcc 13 (wersje MG), \[pozostałe\] |
| Biblioteka MPI, CUDA lub OpenCL – wersje |                                   |

Przy MPI na kilku komputerach podać też liczbę węzłów.

## 6.2 Sposób pomiaru

- Osobno czas obliczeń (histogramy i entropia) i czas całości (z odczytem pliku, transferami, komunikacją i zapisem).
- Jedno uruchomienie na rozgrzewkę, potem co najmniej 5 pomiarów; podajemy medianę i rozrzut (min–max).
- Na GPU synchronizacja przed końcem pomiaru.
- Surowe wyniki zapisujemy w plikach CSV w repozytorium.

## 6.3 Plan eksperymentów

| Co zmieniamy                              | Wartości (propozycja)          |
| ----------------------------------------- | ------------------------------ |
| liczba wątków OpenMP lub procesów MPI     | 1, 2, 4, 8, …                  |
| rozmiar pliku                             | np. 100 MB, 500 MB, 1 GB       |
| rozmiar bloku GPU                         | np. 128, 256, 512              |
| zawartość pliku (GPU)                     | same zera, 16 wartości, losowe |
| krok przy oknie 4096 B (tryb przyrostowy) | 4096, 1024, 256                |

## 6.4 Miary

Przyspieszenie i efektywność dla OpenMP i MPI, przy tym samym pliku i tej samej metodzie:

S(p) = \\frac{T(1)}{T(p)}, \\qquad E(p) = \\frac{S(p)}{p})}{p}

- Przepustowość w GB/s.
- Udział transferów i komunikacji w czasie całości.
- Dokładność wskazania znanych granic w segmenty.bin (odległość w bajtach).

Uwagi z wymagań: liczby wątków GPU nie używamy we wzorze na efektywność; przyspieszenie GPU podajemy z nazwą wersji odniesienia; wynik MPI z jednego komputera oznaczamy jako lokalny.

# 7. Wyniki - TODO

Każdy podrozdział to wykres lub tabela i 2–3 zdania o tym, co z nich wynika. Sposób tworzenia wykresów wybiera KŚ. Wymagania proszą o dołączenie surowych wyników i skryptów lub plików, z których powstały wykresy.

## 7.1 Przykładowy wynik programu

**Co tu wpisać:** wykres entropii dla segmenty.bin z zaznaczonymi wykrytymi zmianami i prawdziwymi granicami. Do tego krótko: błąd entropii (największa różnica względem wartości oczekiwanej, np. 0 dla stale.bin i 4 dla wzorzec16.bin) oraz dokładność lokalizacji znanych granic (odległość w bajtach, liczba pominiętych i fałszywych zmian).

## 7.2 OpenMP i MPI

**Co tu wpisać:** czas, przyspieszenie i efektywność w funkcji liczby wątków lub procesów, dla kilku rozmiarów pliku; udział komunikacji MPI.

## 7.3 CUDA lub OpenCL

**Co tu wpisać:** czas i GB/s dla różnych rozmiarów pliku i bloku, podział czasu na transfery i obliczenia oraz wpływ zawartości pliku.

## 7.4 Tryb przyrostowy

**Co tu wpisać:** czas trybu przyrostowego i liczenia od zera dla kilku kroków przy oknie 4096 B.

## 7.5 Zestawienie wersji

Najlepszy wynik każdej wersji przy tym samym pliku i tym samym zakresie mierzonych operacji.

| Wersja          | Konfiguracja       | Czas obliczeń | Czas całości | GB/s | Przyspieszenie względem \[wersja\] |
| --------------- | ------------------ | ------------- | ------------ | ---- | ---------------------------------- |
| Referencyjna    | 1 wątek            |               |              |      | 1,0                                |
| OpenMP          | \[wątki\]          |               |              |      |                                    |
| MPI             | \[procesy, węzły\] |               |              |      |                                    |
| CUDA lub OpenCL | \[rozmiar bloku\]  |               |              |      |                                    |

# 8. Wnioski I podsumowanie - TODO

**Co tu wpisać:**

- Odpowiedź na każde z 3 pytań z punktu 2.1, z odwołaniem do wyników z rozdziału 7.
- Kiedy opłaca się tryb przyrostowy. Oczekiwanie do sprawdzenia: zysk pojawia się tylko przy kroku wyraźnie mniejszym od okna; przy domyślnym kroku równym oknu zysku nie ma.
- Wyjaśnienie, gdy wersja równoległa nie przyspiesza (np. odczyt pliku, transfery). Wnioski muszą wynikać z pomiarów także w takim przypadku.
- Ograniczenia projektu, np. jeden komputer, ograniczony rozmiar plików.
