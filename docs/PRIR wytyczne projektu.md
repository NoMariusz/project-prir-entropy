# **Programowanie równoległe i rozproszone**

## **Tematy projektów 2026/2027**

Projekty są przeznaczone do realizacji indywidualnej lub w zespołach maksymalnie dwuosobowych. Katalog obejmuje 41 tematów: 11 ogólnych, 12 dla Cyberbezpieczeństwa, 12 dla Data Science oraz 6 dla specjalności Systemy Inteligentne i Rozszerzona Rzeczywistość. Nie jest koniecznym wybieranie tematu pod swoją specjalność.

### **Wymagania wspólne**

Każdy projekt powinien zawierać implementację referencyjną oraz trzy wersje wykorzystujące OpenMP, MPI i CUDA lub OpenCL. Dopuszczalne są odpowiedniki tych technologii w wybranym języku po uzgodnieniu z prowadzącym. Wszystkie wersje muszą przetwarzać te same dane i realizować ten sam cel obliczeniowy. Różnice algorytmów i dokładności należy jawnie opisać.

Własna implementacja powinna obejmować wskazany w temacie rdzeń obliczeń. Biblioteki mogą służyć do odczytu danych, wizualizacji, pomocniczych operacji oraz jako zoptymalizowane punkty odniesienia. Gdy dopuszczono bibliotekę dla części obliczeń, w raporcie trzeba wskazać jej udział i wyodrębnić wkład własny. Rozbudowany interfejs graficzny nie jest wymagany; wystarczy poprawnie udokumentowany interfejs wiersza poleceń.

### **Zakres dla jednej i dwóch osób**

Każda karta określa zakres podstawowy i konkretny dodatek dla zespołu dwuosobowego. W obu przypadkach obowiązują trzy technologie. Zespół jednoosobowy ogranicza liczbę wariantów i scenariuszy zgodnie z kartą. Zespół dwuosobowy realizuje również wskazane porównanie lub usprawnienie. Podział pracy należy przedstawić w dokumentacji; obie osoby powinny rozumieć całość rozwiązania.

### **Dokumentacja projektu**

Do projektu należy dołączyć kod, instrukcję budowania i uruchomienia, opis danych, testy poprawności, konfigurację komputerów i oprogramowania, surowe wyniki pomiarów oraz skrypty tworzące wykresy. Sprawozdanie powinno wyjaśniać podział obliczeń, synchronizację, komunikację i najważniejsze decyzje implementacyjne. Wnioski muszą wynikać z pomiarów, także wtedy, gdy GPU lub MPI nie daje przyspieszenia.

### **Rozwinięcie naukowe**

Ostatnia część każdej karty proponuje pytanie badawcze do rozwinięcia w artykuł lub pracę magisterską. Nie jest to obowiązkowy zakres zaliczenia. Dalsza praca wymaga przeglądu literatury, mocnych metod odniesienia, weryfikacji nowości i poszerzonej walidacji. Samo wykonanie trzech wersji znanego algorytmu nie przesądza o wkładzie naukowym.

## **Pomiary i organizacja pracy**

### **Poprawność i porównywalność**

Przed pomiarami należy porównać wyniki z referencją sekwencyjną, sprawdzić przypadki brzegowe i ustalić tolerancję dla liczb zmiennoprzecinkowych. W eksperymentach losowych zapisywać ziarna i zapewnić rozłączne strumienie generatorów. W uczeniu maszynowym oddzielić dane treningowe, walidacyjne i testowe; przygotowanie cech i dobór progów nie mogą korzystać z danych testowych.

### **Czas i skalowanie**

Osobno mierzyć czas rdzenia obliczeń oraz pełny czas przetwarzania, obejmujący istotne transfery, komunikację i wejście/wyjście. Wykonać rozgrzewkę oraz co najmniej pięć niezależnych pomiarów konfiguracji. Pokazać medianę i rozrzut, a dla metod losowych także wyniki wielu ziaren. Operacje GPU zsynchronizować przed końcem pomiaru.

Dla CPU i MPI wykonać wykresy przyspieszenia S(p)=T(1)/T(p) i efektywności E(p)=S(p)/p przy stałym problemie i tej samej metodzie. Podać liczbę wątków, procesów, rdzeni i węzłów. Wynik MPI na jednym komputerze należy oznaczyć jako lokalny; nie stanowi on pomiaru skalowalności klastra. Gdy MPI dzieli niezależne zadania, raportować przepustowość wsadu, odróżniając ją od czasu pojedynczego zadania.

Dla GPU pokazać czas, przepustowość oraz wpływ rozmiaru danych i konfiguracji bloków. Liczby wątków GPU nie używać jako odpowiednika liczby rdzeni CPU w formule efektywności. Przyspieszenie względem CPU wymaga jawnego wskazania referencji. Zestawienie najlepszych wyników musi zachować tę samą jakość i zakres mierzonych operacji. P95 oznacza 95. percentyl opóźnienia i wymaga wielu obserwacji zadań, nie tylko pięciu pomiarów całego programu.

### **Etapy realizacji i ocena**

Proponowany podział semestru: tygodnie 1–2 – wybór tematu, zakres i dostęp do sprzętu; 3–4 – referencja oraz testy; 5–7 – OpenMP i MPI; 8–10 – GPU; 11–12 – eksperymenty; 13–14 – raport i obrona. Konkretne terminy konsultacji i oddania ustala prowadzący. Proponowane wagi oceny: poprawność 25%, implementacje 30%, metodologia i analiza 25%, odtwarzalność 10%, dokumentacja i obrona 10%.

## **16 Entropia i segmentacja plików binarnych**

Identyfikator C05 | Zespół 1–2 osoby

### **Opis**

Projekt oblicza entropię rozkładu bajtów w kolejnych fragmentach pliku. W każdym oknie powstaje histogram 256 wartości, z którego wyznaczana jest entropia Shannona. Zmiany entropii mogą wskazywać granice obszarów o innej strukturze, np. tekstu, danych skompresowanych lub losowych. Sama wysoka entropia nie jest dowodem złośliwości ani szyfrowania.

Program przyjmuje plik, długość i krok okna. Zwraca wykres entropii względem pozycji oraz listę zmian przekraczających ustalony próg. Należy określić obsługę końcowego niepełnego okna i pominąć składniki histogramu o zerowym prawdopodobieństwie.

### **Zakres implementacji**

**OpenMP –** Przydzielić okna do wątków i tworzyć osobne histogramy, aby uniknąć współdzielonych liczników przy każdym bajcie.

**MPI –** Rozdzielić fragmenty pliku z zakładką odpowiadającą oknom. Zachować globalne pozycje i wyeliminować podwójne wyniki na granicach.

**CUDA lub OpenCL –** Budować histogramy lokalne dla bloków lub okien, następnie obliczać entropię. Zbadać wpływ koncentracji bajtów na konkurencję operacji atomowych.

### **Dodatkowe wyniki do raportu**

Użyć pliku stałych bajtów, kontrolowanego rozkładu i plików z doklejonymi różnymi segmentami. Raportować błąd entropii, GB/s, koszt transferów i dokładność lokalizacji znanych granic.

### **Zakres zespołu**

1 osoba: stałe okna i próg zmiany. 2 osoby: wariant przyrostowej aktualizacji histogramu albo adaptacyjny krok okna.

### **Rozwinięcie do artykułu lub pracy magisterskiej**

Czy adaptacyjny krok okna zmniejsza koszt bez utraty lokalizacji zmian? Sama entropia nie rozstrzyga o złośliwości pliku.
