# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-05-03

### Added
- Initial release of Ultimate Sorting Benchmark
- Four parallel sorting algorithms:
  - Chunk-based parallel sort
  - Parallel Quicksort with improved pivot selection
  - Parallel Radix Sort (LSD, base 256)
  - Parallel Merge Sort with depth-limited parallelism
- Three data generation patterns:
  - Random data (uniform distribution)
  - Almost sorted data (99% sorted, 1% random swaps)
  - Reverse sorted data
- Comprehensive benchmarking framework:
  - Single-threaded vs multi-threaded comparison
  - Thread scaling analysis (1-32 threads)
  - Performance metrics with speedup percentages
- Detailed README with English and Russian documentation
- MIT License

### Technical
- Optimized for Apple M4 architecture (8 performance + 4 efficiency cores)
- C++17 implementation using std::async, std::thread, and std::future
- Memory-efficient algorithms with minimal copying
- Cache-aware optimizations for better performance

### Documentation
- Complete README with benchmark results
- Algorithm recommendations for different use cases
- Thread scaling insights for M4 architecture
- Build and usage instructions

[1.0.0]: https://github.com/fedosikser/sort_race_m4/releases/tag/v1.0.0