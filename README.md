# Ultimate Sorting Benchmark (M4 Mac) 🇺🇸 🇷🇺

<div align="center">
  
**English** | [Русский](#русский)

</div>

---

## 🌐 Navigation / Навигация

- [English Version](#english)
  - [📊 Key Highlights](#-key-highlights)
  - [🏗️ Architecture](#️-architecture)
  - [🚀 Quick Start](#-quick-start)
  - [📈 Benchmark Results](#-benchmark-results)
  - [🧵 Thread Scaling Analysis](#-thread-scaling-analysis)
  - [🏆 Algorithm Recommendations](#-algorithm-recommendations)
  - [🔧 Technical Details](#-technical-details)
  - [📝 Code Structure](#-code-structure)
  - [🎯 Performance Insights](#-performance-insights)
  - [🔮 Future Improvements](#-future-improvements)
  - [🤝 Contributing](#-contributing)
  - [📄 License](#-license)

- [Русская версия](#русский)
  - [📊 Ключевые особенности](#-ключевые-особенности)
  - [🏗️ Архитектура](#️-архитектура)
  - [🚀 Быстрый старт](#-быстрый-старт)
  - [📈 Результаты бенчмарка](#-результаты-бенчмарка)
  - [🧵 Анализ масштабирования потоков](#-анализ-масштабирования-потоков)
  - [🏆 Рекомендации по алгоритмам](#-рекомендации-по-алгоритмам)
  - [🔧 Технические детали](#-технические-детали-1)
  - [📝 Структура кода](#-структура-кода)
  - [🎯 Инсайты производительности](#-инсайты-производительности)
  - [🔮 Будущие улучшения](#-будущие-улучшения)
  - [🤝 Участие в разработке](#-участие-в-разработке)
  - [📄 Лицензия](#-лицензия-1)

---

# English 🇺🇸

A comprehensive C++ benchmark comparing single-threaded vs. multi-threaded sorting algorithms on Apple M4 architecture. This project implements and analyzes four parallel sorting algorithms with detailed performance metrics across different data patterns and thread counts.

## 📊 Key Highlights

- **4 Parallel Algorithms**: Chunk-based, Parallel Quicksort, Parallel Radix Sort, Parallel Merge Sort
- **3 Data Patterns**: Random, Almost Sorted, Reverse Sorted
- **9 Thread Configurations**: 1, 2, 4, 8, 9, 12, 16, 24, 32 threads
- **Performance Analysis**: Speedup percentages, thread scaling, algorithm recommendations

## 🏗️ Architecture

### Algorithms Implemented

| Algorithm | Complexity | Best For | Parallel Strategy |
|-----------|------------|----------|-------------------|
| **Chunk-based** | O(n log n) | Large random data | Split array → sort chunks → merge |
| **Parallel Quicksort** | O(n log n) | Almost sorted data | Recursive async with load balancing |
| **Parallel Radix Sort** | O(n) | Random data (O(n) advantage) | LSD radix with parallel chunk processing |
| **Parallel Merge Sort** | O(n log n) | All data types (stable) | Recursive divide & parallel merge |

### Data Generation

- **Random**: Uniform distribution 1-1,000,000
- **Almost Sorted**: 99% sorted with 1% random swaps  
- **Reverse Sorted**: Perfectly reversed order (worst case for quicksort)

## 🚀 Quick Start

### Prerequisites
- C++17 compatible compiler (clang++ recommended)
- Apple M1/M2/M3/M4 Mac (or any modern CPU)
- 8GB+ RAM for large array tests

### Build & Run

```bash
# Clone and navigate
git clone <your-repo-url>
cd threads

# Compile with optimizations
clang++ -std=c++17 -O3 -pthread -o benchmark_sort main.cpp

# Run comprehensive benchmark
./benchmark_sort
```

## 📈 Benchmark Results (M4, 9 Threads)

### Random Data (100M elements)

| Algorithm | Time | Speedup | Status |
|-----------|------|---------|--------|
| Single-threaded `std::sort` | 1138 ms | 1.00x | Baseline |
| **Parallel Quicksort** | **642 ms** | **1.77x** | ✅ **Best** |
| Parallel Merge Sort | 699 ms | 1.63x | ✅ |
| Parallel Radix Sort | 1094 ms | 1.04x | ✅ |
| Chunk-based | 1106 ms | 1.03x | ✅ |

### Almost Sorted Data (100M elements)

| Algorithm | Time | Speedup | Status |
|-----------|------|---------|--------|
| Single-threaded `std::sort` | 736 ms | 1.00x | Baseline |
| **Parallel Quicksort** | **234 ms** | **3.15x** | ✅ **Best** |
| Parallel Merge Sort | 640 ms | 1.15x | ✅ |
| Chunk-based | 1040 ms | 0.71x | ❌ Slower |
| Parallel Radix Sort | 997 ms | 0.74x | ❌ Slower |

### Reverse Sorted Data (100M elements)

| Algorithm | Time | Speedup | Status |
|-----------|------|---------|--------|
| Single-threaded `std::sort` | 119 ms | 1.00x | Baseline |
| **Parallel Merge Sort** | **112 ms** | **1.06x** | ✅ **Best** |
| Parallel Quicksort | 130 ms | 0.92x | ❌ Slower |
| Chunk-based | 608 ms | 0.20x | ❌ Slower |
| Parallel Radix Sort | 704 ms | 0.17x | ❌ Slower |

## 🧵 Thread Scaling Analysis

### Optimal Thread Counts for 100M Random Data

```
Threads  Chunk    Quick   Radix   Merge   Best Speedup
-------  -----    -----   -----   -----   ------------
1        1582 ms  840 ms  849 ms  1199 ms  1.41x
2        1069 ms  693 ms  635 ms  849 ms   1.87x  ← Best scaling
4        874 ms   648 ms  772 ms  697 ms   1.83x
8        1092 ms  643 ms  1120 ms 660 ms   1.85x
9        1093 ms  669 ms  1003 ms  648 ms  1.83x
12       1099 ms  665 ms  1030 ms 646 ms   1.84x
16       1392 ms  678 ms  1244 ms 663 ms   1.79x
24       1325 ms  695 ms  1374 ms 667 ms   1.78x
32       1725 ms  689 ms  1516 ms 706 ms   1.72x
```

### Key Threading Insights

**Why 9 threads on M4?**
- M4 has 8 performance cores + 2-4 efficiency cores (10-12 total)
- 9 threads = 8 performance cores + 1 extra for I/O/background
- Efficiency cores help with background tasks but slower for compute

**Oversubscription Trade-offs**
- ✓ **Benefits**: Masks memory latency, handles I/O waits
- ✗ **Costs**: Context switching overhead, cache thrashing, memory bus contention

**Recommendation**: 8-12 threads optimal for M4, 2-4 threads often best for memory-bound tasks.

## 🏆 Algorithm Recommendations

| Use Case | Recommended Algorithm | Why |
|----------|---------------------|-----|
| **Random data** | **Parallel Quicksort** or **Parallel Merge Sort** | Best speedup (1.6-1.8x) |
| **Almost sorted** | **Parallel Quicksort** | Exceptional speedup (3.15x) |
| **Reverse sorted** | **Parallel Merge Sort** | Only algorithm with speedup |
| **Stable sort required** | **Parallel Merge Sort** | Preserves order of equal elements |
| **Known data range** | **Parallel Radix Sort** | O(n) complexity advantage |
| **Simple implementation** | **Chunk-based** | Easy to understand and debug |

## 🔧 Technical Details

### Parallel Quicksort Optimizations
- **Median of medians** pivot selection for large arrays
- **Hoare partition scheme** for efficiency
- **Load balancing** - smaller partition processed in separate thread
- **Dynamic threshold** for switching to sequential sort

### Parallel Radix Sort (LSD)
- **Base 256** (8-bit digits, 4 passes for 32-bit integers)
- **Stable counting sort** per digit
- **Parallel chunk processing** with priority queue merge

### Parallel Merge Sort
- **Recursive divide** with `std::async`
- **Depth-limited parallelism** based on thread count
- **Sequential merge** for small subarrays

### Memory Considerations
- 100M ints ≈ 400MB RAM
- 400M ints ≈ 1.6GB RAM
- Algorithms designed for cache efficiency and minimal copying

## 📝 Code Structure

```cpp
main.cpp
├── Data Generation
│   ├── generate_random_vector()
│   ├── generate_almost_sorted_vector()
│   └── generate_reverse_sorted_vector()
├── Sorting Algorithms
│   ├── benchmark_multi_threaded()        // Chunk-based
│   ├── benchmark_parallel_quicksort()    // Parallel quicksort
│   ├── benchmark_parallel_radix_sort()   // Parallel radix sort
│   └── benchmark_parallel_merge_sort()   // Parallel merge sort
├── Benchmark Framework
│   ├── run_benchmark_comprehensive()     // Compare all algorithms
│   └── test_thread_scaling()             // Thread count analysis
└── Utilities
    ├── verify_sorted()
    ├── radix_sort()
    └── parallel_quicksort_optimized()
```

## 🎯 Performance Insights

1. **Memory-bound nature**: Sorting large arrays is limited by memory bandwidth, not CPU
2. **Algorithm selection matters more than thread count**: 3.15x vs 0.2x speedup depending on data
3. **Parallel overhead significant**: Small arrays (<10k elements) better sorted sequentially
4. **std::async vs thread pool**: `std::async` convenient but creates thread per task overhead
5. **Cache efficiency**: Algorithms with good locality (quicksort) outperform others

## 🔮 Future Improvements

- [ ] Thread pool implementation to reduce `std::async` overhead
- [ ] SIMD vectorization for radix sort counting phase
- [ ] GPU offloading for radix sort (Metal/CUDA)
- [ ] Adaptive algorithm selection based on data characteristics
- [ ] Cache-aware tiling for better memory locality
- [ ] Support for custom comparators and data types

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- Additional sorting algorithms (timsort, introsort)
- Better thread pooling implementation
- More comprehensive benchmarking suite
- Cross-platform support (Linux, Windows)

## 📄 License

MIT License - see LICENSE file for details.

## 🙏 Acknowledgments

- Apple M4 architecture for providing 8+4 core configuration
- C++17 standard for `std::async` and parallel algorithms support
- Computer science researchers in parallel sorting algorithms

---

**Created by**: [Your Name]  
**Platform**: Apple M4 Mac (8 performance + 4 efficiency cores)  
**Language**: C++17  
**Focus**: Parallel algorithms, performance optimization, benchmarking

---

<div align="center">
  
---

</div>

# Русский 🇷🇺

Комплексный бенчмарк на C++, сравнивающий однопоточные и многопоточные алгоритмы сортировки на архитектуре Apple M4. В проекте реализованы и проанализированы четыре параллельных алгоритма сортировки с подробными метриками производительности для разных типов данных и количеств потоков.

## 📊 Ключевые особенности

- **4 параллельных алгоритма**: Chunk-based, Parallel Quicksort, Parallel Radix Sort, Parallel Merge Sort
- **3 типа данных**: Случайные, Почти отсортированные, Обратно отсортированные
- **9 конфигураций потоков**: 1, 2, 4, 8, 9, 12, 16, 24, 32 потока
- **Анализ производительности**: Проценты ускорения, масштабирование потоков, рекомендации по алгоритмам

## 🏗️ Архитектура

### Реализованные алгоритмы

| Алгоритм | Сложность | Лучше всего для | Стратегия параллелизации |
|----------|-----------|----------------|--------------------------|
| **Chunk-based** | O(n log n) | Большие случайные данные | Разделение массива → сортировка чанков → слияние |
| **Parallel Quicksort** | O(n log n) | Почти отсортированные данные | Рекурсивный async с балансировкой нагрузки |
| **Parallel Radix Sort** | O(n) | Случайные данные (преимущество O(n)) | LSD radix с параллельной обработкой чанков |
| **Parallel Merge Sort** | O(n log n) | Все типы данных (стабильный) | Рекурсивное разделение и параллельное слияние |

### Генерация данных

- **Случайные**: Равномерное распределение 1-1,000,000
- **Почти отсортированные**: 99% отсортированы, 1% случайных перестановок
- **Обратно отсортированные**: Идеально обратный порядок (худший случай для quicksort)

## 🚀 Быстрый старт

### Требования
- Компилятор с поддержкой C++17 (рекомендуется clang++)
- Apple M1/M2/M3/M4 Mac (или любой современный CPU)
- 8GB+ RAM для тестов с большими массивами

### Сборка и запуск

```bash
# Клонирование и переход
git clone <your-repo-url>
cd threads

# Компиляция с оптимизациями
clang++ -std=c++17 -O3 -pthread -o benchmark_sort main.cpp

# Запуск комплексного бенчмарка
./benchmark_sort
```

## 📈 Результаты бенчмарка (M4, 9 потоков)

### Случайные данные (100M элементов)

| Алгоритм | Время | Ускорение | Статус |
|----------|-------|-----------|--------|
| Однопоточный `std::sort` | 1138 ms | 1.00x | Базовый уровень |
| **Parallel Quicksort** | **642 ms** | **1.77x** | ✅ **Лучший** |
| Parallel Merge Sort | 699 ms | 1.63x | ✅ |
| Parallel Radix Sort | 1094 ms | 1.04x | ✅ |
| Chunk-based | 1106 ms | 1.03x | ✅ |

### Почти отсортированные данные (100M элементов)

| Алгоритм | Время | Ускорение | Статус |
|----------|-------|-----------|--------|
| Однопоточный `std::sort` | 736 ms | 1.00x | Базовый уровень |
| **Parallel Quicksort** | **234 ms** | **3.15x** | ✅ **Лучший** |
| Parallel Merge Sort | 640 ms | 1.15x | ✅ |
| Chunk-based | 1040 ms | 0.71x | ❌ Медленнее |
| Parallel Radix Sort | 997 ms | 0.74x | ❌ Медленнее |

### Обратно отсортированные данные (100M элементов)

| Алгоритм | Время | Ускорение | Статус |
|----------|-------|-----------|--------|
| Однопоточный `std::sort` | 119 ms | 1.00x | Базовый уровень |
| **Parallel Merge Sort** | **112 ms** | **1.06x** | ✅ **Лучший** |
| Parallel Quicksort | 130 ms | 0.92x | ❌ Медленнее |
| Chunk-based | 608 ms | 0.20x | ❌ Медленнее |
| Parallel Radix Sort | 704 ms | 0.17x | ❌ Медленнее |

## 🧵 Анализ масштабирования потоков

### Оптимальное количество потоков для 100M случайных данных

```
Потоки   Chunk    Quick   Radix   Merge   Лучшее ускорение
-------  -----    -----   -----   -----   ----------------
1        1582 ms  840 ms  849 ms  1199 ms  1.41x
2        1069 ms  693 ms  635 ms  849 ms   1.87x  ← Лучшее масштабирование
4        874 ms   648 ms  772 ms  697 ms   1.83x
8        1092 ms  643 ms  1120 ms 660 ms   1.85x
9        1093 ms  669 ms  1003 ms 648 ms   1.83x
12       1099 ms  665 ms  1030 ms 646 ms   1.84x
16       1392 ms  678 ms  1244 ms 663 ms   1.79x
24       1325 ms  695 ms  1374 ms 667 ms   1.78x
32       1725 ms  689 ms  1516 ms 706 ms   1.72x
```

### Ключевые инсайты по потокам

**Почему 9 потоков на M4?**
- M4 имеет 8 производительных ядер + 2-4 энергоэффективных (всего 10-12 ядер)
- 9 потоков = 8 производительных ядер + 1 дополнительный для I/O/фоновых задач
- Энергоэффективные ядра помогают с фоновыми задачами, но медленнее для вычислений

**Компромиссы oversubscription (избыточной подписки)**
- ✓ **Преимущества**: Маскирует латентность памяти, обрабатывает ожидания I/O
- ✗ **Издержки**: Накладные расходы на переключение контекста, вытеснение кэша, конкуренция за шину памяти

**Рекомендация**: 8-12 потоков оптимально для M4, 2-4 потока часто лучше для memory-bound задач.

## 🏆 Рекомендации по алгоритмам

| Сценарий использования | Рекомендуемый алгоритм | Почему |
|------------------------|-----------------------|--------|
| **Случайные данные** | **Parallel Quicksort** или **Parallel Merge Sort** | Лучшее ускорение (1.6-1.8x) |
| **Почти отсортированные данные** | **Parallel Quicksort** | Исключительное ускорение (3.15x) |
| **Обратно отсортированные данные** | **Parallel Merge Sort** | Единственный алгоритм с ускорением |
| **Требуется стабильная сортировка** | **Parallel Merge Sort** | Сохраняет порядок равных элементов |
| **Известный диапазон данных** | **Parallel Radix Sort** | Преимущество сложности O(n) |
| **Простая реализация** | **Chunk-based** | Легко понять и отлаживать |

## 🔧 Технические детали

### Оптимизации Parallel Quicksort
- **Медиана медиан** для выбора опорного элемента в больших массивах
- **Схема разделения Хоара** для эффективности
- **Балансировка нагрузки** - меньшая часть обрабатывается в отдельном потоке
- **Динамический порог** для переключения на последовательную сортировку

### Parallel Radix Sort (LSD)
- **Основание 256** (8-битные цифры, 4 прохода для 32-битных целых)
- **Стабильная сортировка подсчетом** на каждой цифре
- **Параллельная обработка чанков** с объединением через priority queue

### Parallel Merge Sort
- **Рекурсивное разделение** с `std::async`
- **Ограниченная глубина параллелизма** на основе количества потоков
- **Последовательное слияние** для маленьких подмассивов

### Соображения по памяти
- 100M int ≈ 400MB RAM
- 400M int ≈ 1.6GB RAM
- Алгоритмы разработаны для эффективности кэша и минимального копирования

## 📝 Структура кода

```cpp
main.cpp
├── Генерация данных
│   ├── generate_random_vector()
│   ├── generate_almost_sorted_vector()
│   └── generate_reverse_sorted_vector()
├── Алгоритмы сортировки
│   ├── benchmark_multi_threaded()        // Chunk-based
│   ├── benchmark_parallel_quicksort()    // Parallel quicksort
│   ├── benchmark_parallel_radix_sort()   // Parallel radix sort
│   └── benchmark_parallel_merge_sort()   // Parallel merge sort
├── Фреймворк бенчмаркинга
│   ├── run_benchmark_comprehensive()     // Сравнение всех алгоритмов
│   └── test_thread_scaling()             // Анализ количества потоков
└── Утилиты
    ├── verify_sorted()
    ├── radix_sort()
    └── parallel_quicksort_optimized()
```

## 🎯 Инсайты производительности

1. **Memory-bound природа**: Сортировка больших массивов ограничена bandwidth памяти, а не CPU
2. **Выбор алгоритма важнее количества потоков**: 3.15x vs 0.2x ускорение в зависимости от данных
3. **Значительные накладные расходы параллелизации**: Маленькие массивы (<10k элементов) лучше сортировать последовательно
4. **std::async vs пул потоков**: `std::async` удобен, но создает накладные расходы на создание потоков
5. **Эффективность кэша**: Алгоритмы с хорошей локальностью (quicksort) превосходят другие

## 🔮 Будущие улучшения

- [ ] Реализация пула потоков для уменьшения накладных расходов `std::async`
- [ ] Векторизация SIMD для фазы подсчета в radix sort
- [ ] Выгрузка на GPU для radix sort (Metal/CUDA)
- [ ] Адаптивный выбор алгоритма на основе характеристик данных
- [ ] Кэш-осознанное тайлирование для лучшей локальности памяти
- [ ] Поддержка пользовательских компараторов и типов данных

## 🤝 Участие в разработке

Приветствуются contributions! Области для улучшения:
- Дополнительные алгоритмы сортировки (timsort, introsort)
- Лучшая реализация пула потоков
- Более комплексный набор бенчмарков
- Кросс-платформенная поддержка (Linux, Windows)

## 📄 Лицензия

MIT License - смотрите файл LICENSE для деталей.

## 🙏 Благодарности

- Архитектура Apple M4 за предоставление конфигурации 8+4 ядер
- Стандарт C++17 за поддержку `std::async` и параллельных алгоритмов
- Исследователям computer science в области параллельных алгоритмов сортировки

---

**Создано**: [Ваше Имя]  
**Платформа**: Apple M4 Mac (8 производительных + 4 энергоэффективных ядра)  
**Язык**: C++17  
**Фокус**: Параллельные алгоритмы, оптимизация производительности, бенчмаркинг

---

<div align="center">
  
[⬆ Наверх / Back to top](#ultimate-sorting-benchmark-m4-mac-)

</div>