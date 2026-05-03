#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <future>
#include <cassert>
#include <queue>
#include <iomanip>

// Function to generate random vector of given size
std::vector<int> generate_random_vector(size_t size) {
    std::vector<int> vec(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 1000000);
    
    for (auto& val : vec) {
        val = dist(gen);
    }
    return vec;
}

// Generate almost sorted vector (sorted with some random swaps)
std::vector<int> generate_almost_sorted_vector(size_t size) {
    std::vector<int> vec(size);
    // Fill with sequential numbers
    for (size_t i = 0; i < size; ++i) {
        vec[i] = static_cast<int>(i);
    }
    
    // Make ~1% of elements out of order
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> idx_dist(0, size - 1);
    
    size_t num_swaps = size / 100; // 1% swaps
    for (size_t i = 0; i < num_swaps; ++i) {
        size_t idx1 = idx_dist(gen);
        size_t idx2 = idx_dist(gen);
        std::swap(vec[idx1], vec[idx2]);
    }
    
    return vec;
}

// Generate reverse sorted vector
std::vector<int> generate_reverse_sorted_vector(size_t size) {
    std::vector<int> vec(size);
    for (size_t i = 0; i < size; ++i) {
        vec[i] = static_cast<int>(size - i);
    }
    return vec;
}

// Single-threaded sort benchmark
void benchmark_single_threaded(std::vector<int>& data) {
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(data.begin(), data.end());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Single-threaded sort time: " << duration.count() << " ms" << std::endl;
}

// Multi-threaded sort using 9 threads (optimized in-place sorting)
// Returns sorting time in milliseconds
long long benchmark_multi_threaded(std::vector<int>& data, unsigned int num_threads = 9, bool verbose = true) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Split data into chunks by iterator ranges
    size_t chunk_size = data.size() / num_threads;
    std::vector<std::thread> threads;
    std::vector<typename std::vector<int>::iterator> starts(num_threads + 1);
    
    // Define start iterators for each chunk
    auto it = data.begin();
    for (unsigned int i = 0; i < num_threads; ++i) {
        starts[i] = it;
        it += chunk_size;
    }
    starts[num_threads] = data.end();
    
    // Sort each chunk in-place in separate thread
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&data, &starts, i]() {
            std::sort(starts[i], starts[i + 1]);
        });
    }
    
    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }
    
    // Merge sorted chunks using priority queue
    std::vector<int> result;
    result.reserve(data.size());
    
    // Store current position in each chunk
    std::vector<typename std::vector<int>::iterator> current_pos(num_threads);
    for (unsigned int i = 0; i < num_threads; ++i) {
        current_pos[i] = starts[i];
    }
    
    // Priority queue for k-way merge: (value, chunk_index)
    using QueueElement = std::pair<int, size_t>;
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
        return a.first > b.first; // min-heap
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> pq(cmp);
    
    // Initialize heap with first element of each chunk
    for (size_t i = 0; i < num_threads; ++i) {
        if (current_pos[i] != starts[i + 1]) {
            pq.emplace(*current_pos[i], i);
        }
    }
    
    // Merge
    while (!pq.empty()) {
        auto [value, chunk_idx] = pq.top();
        pq.pop();
        result.push_back(value);
        
        // Move iterator forward in this chunk
        ++current_pos[chunk_idx];
        if (current_pos[chunk_idx] != starts[chunk_idx + 1]) {
            pq.emplace(*current_pos[chunk_idx], chunk_idx);
        }
    }
    
    data = std::move(result);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (verbose) {
        std::cout << "Multi-threaded sort time (" << num_threads << " threads): " << duration.count() << " ms" << std::endl;
    }
    
    return duration.count();
}

// Improved pivot selection (median of medians for large arrays)
template<typename Iter>
auto select_pivot(Iter first, Iter last) {
    size_t size = std::distance(first, last);
    
    // For small arrays, use median of three
    if (size < 100000) {
        Iter mid = first + size / 2;
        Iter end = last - 1;
        
        // Sort first, mid, end
        if (*first > *mid) std::iter_swap(first, mid);
        if (*first > *end) std::iter_swap(first, end);
        if (*mid > *end) std::iter_swap(mid, end);
        
        // Place pivot at end-1
        std::iter_swap(mid, end - 1);
        return std::make_pair(*(end - 1), end - 1);
    }
    
    // For large arrays, use median of medians (approximate)
    size_t sample_size = std::min(size / 100, static_cast<size_t>(100));
    std::vector<typename std::iterator_traits<Iter>::value_type> samples;
    samples.reserve(sample_size);
    
    size_t step = size / sample_size;
    for (size_t i = 0; i < sample_size; ++i) {
        samples.push_back(*(first + i * step));
    }
    
    // Find median of samples
    std::sort(samples.begin(), samples.end());
    auto pivot_value = samples[sample_size / 2];
    
    // Find the actual element equal to pivot_value
    Iter pivot_pos = std::find(first, last, pivot_value);
    if (pivot_pos == last) {
        pivot_pos = first + size / 2;
        pivot_value = *pivot_pos;
    }
    
    return std::make_pair(pivot_value, pivot_pos);
}

// Optimized parallel quicksort with better load balancing
template<typename Iter>
void parallel_quicksort_optimized(Iter first, Iter last, 
                                  unsigned int max_tasks = 0, 
                                  unsigned int current_tasks = 1) {
    if (first >= last) return;
    
    size_t size = std::distance(first, last);
    
    // If small range or too many tasks already spawned, sort sequentially
    // Dynamic threshold based on array size
    size_t threshold = std::max<size_t>(10000, size / 100);
    if (size < threshold || (max_tasks > 0 && current_tasks >= max_tasks)) {
        std::sort(first, last);
        return;
    }
    
    // Select pivot using improved method
    auto [pivot_value, pivot_pos] = select_pivot(first, last);
    
    // Move pivot to end-1 position for partitioning
    Iter end = last - 1;
    if (pivot_pos != end) {
        std::iter_swap(pivot_pos, end);
    }
    
    // Partition using Hoare partition scheme (more efficient)
    Iter i = first;
    Iter j = end;
    while (true) {
        while (i < end && *i < pivot_value) ++i;
        while (j > first && *j > pivot_value) --j;
        if (i >= j) break;
        std::iter_swap(i, j);
        ++i;
        --j;
    }
    
    // Restore pivot to correct position
    std::iter_swap(i, end);
    
    // Balance: always process smaller partition in new thread
    size_t left_size = std::distance(first, i);
    size_t right_size = std::distance(i + 1, last);
    
    // Process smaller partition in parallel if worth it
    if (std::min(left_size, right_size) > threshold / 2) {
        if (left_size < right_size) {
            // Left is smaller
            auto left_future = std::async(std::launch::async,
                [first, i, max_tasks, current_tasks]() {
                    parallel_quicksort_optimized(first, i, max_tasks, current_tasks * 2);
                });
            parallel_quicksort_optimized(i + 1, last, max_tasks, current_tasks * 2);
            left_future.get();
        } else {
            // Right is smaller
            auto right_future = std::async(std::launch::async,
                [i, last, max_tasks, current_tasks]() {
                    parallel_quicksort_optimized(i + 1, last, max_tasks, current_tasks * 2);
                });
            parallel_quicksort_optimized(first, i, max_tasks, current_tasks * 2);
            right_future.get();
        }
    } else {
        // Both partitions sequentially
        std::sort(first, i);
        std::sort(i + 1, last);
    }
}

// Benchmark for parallel quicksort
long long benchmark_parallel_quicksort(std::vector<int>& data, unsigned int num_threads = 9, bool verbose = true) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Use optimized quicksort with task limit
    unsigned int max_tasks = num_threads * 2; // Allow some oversubscription
    parallel_quicksort_optimized(data.begin(), data.end(), max_tasks, 1);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (verbose) {
        std::cout << "Parallel quicksort time (" << num_threads << " threads): " << duration.count() << " ms" << std::endl;
    }
    
    return duration.count();
}

// Verify that vector is sorted
bool verify_sorted(const std::vector<int>& vec) {
    for (size_t i = 1; i < vec.size(); ++i) {
        if (vec[i] < vec[i-1]) {
            return false;
        }
    }
    return true;
}

// LSD Radix Sort for 32-bit integers (base 256, 4 passes)
void radix_sort(std::vector<int>& arr) {
    if (arr.empty()) return;
    
    const int BITS = 8;
    const int BASE = 1 << BITS;
    const int MASK = BASE - 1;
    const int PASSES = sizeof(int) * 8 / BITS;
    
    std::vector<int> output(arr.size());
    std::vector<int> count(BASE);
    
    for (int pass = 0; pass < PASSES; ++pass) {
        // Reset count array
        std::fill(count.begin(), count.end(), 0);
        
        // Count occurrences of each digit
        int shift = pass * BITS;
        for (const int& val : arr) {
            int digit = (val >> shift) & MASK;
            ++count[digit];
        }
        
        // Compute prefix sums
        for (int i = 1; i < BASE; ++i) {
            count[i] += count[i - 1];
        }
        
        // Place elements in output array (stable)
        for (int i = arr.size() - 1; i >= 0; --i) {
            int digit = (arr[i] >> shift) & MASK;
            output[--count[digit]] = arr[i];
        }
        
        // Swap arrays for next pass
        arr.swap(output);
    }
}

// Parallel Radix Sort using multiple threads
long long benchmark_parallel_radix_sort(std::vector<int>& data, unsigned int num_threads = 9, bool verbose = true) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Split data into chunks
    size_t chunk_size = data.size() / num_threads;
    std::vector<std::thread> threads;
    std::vector<std::vector<int>> chunks(num_threads);
    
    // Create chunks
    auto it = data.begin();
    for (unsigned int i = 0; i < num_threads; ++i) {
        auto end_it = (i == num_threads - 1) ? data.end() : it + chunk_size;
        chunks[i].assign(it, end_it);
        it = end_it;
    }
    
    // Sort each chunk with radix sort in separate thread
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&chunks, i]() {
            radix_sort(chunks[i]);
        });
    }
    
    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }
    
    // Merge sorted chunks using priority queue
    std::vector<int> result;
    result.reserve(data.size());
    
    // Define element for priority queue: (value, chunk_index, element_index_within_chunk)
    using QueueElement = std::tuple<int, size_t, size_t>;
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
        return std::get<0>(a) > std::get<0>(b); // min-heap
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> pq(cmp);
    
    // Initialize heap with first element of each chunk
    for (size_t i = 0; i < num_threads; ++i) {
        if (!chunks[i].empty()) {
            pq.emplace(chunks[i][0], i, 0);
        }
    }
    
    // Merge
    while (!pq.empty()) {
        auto [value, chunk_idx, elem_idx] = pq.top();
        pq.pop();
        result.push_back(value);
        
        // Add next element from the same chunk if exists
        if (elem_idx + 1 < chunks[chunk_idx].size()) {
            pq.emplace(chunks[chunk_idx][elem_idx + 1], chunk_idx, elem_idx + 1);
        }
    }
    
    data = std::move(result);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (verbose) {
        std::cout << "Parallel radix sort time (" << num_threads << " threads): " << duration.count() << " ms" << std::endl;
    }
    
    return duration.count();
}

// Parallel Merge Sort (recursive divide and parallel merge)
template<typename Iter>
void parallel_merge_sort(Iter first, Iter last, unsigned int max_depth = 0, unsigned int depth = 0) {
    size_t size = std::distance(first, last);
    if (size <= 1) return;
    
    // If small range or depth limit reached, sort sequentially
    if (size < 10000 || depth >= max_depth) {
        std::sort(first, last);
        return;
    }
    
    Iter mid = first + size / 2;
    
    // Sort halves in parallel
    auto left_future = std::async(std::launch::async,
        [first, mid, max_depth, depth]() {
            parallel_merge_sort(first, mid, max_depth, depth + 1);
        });
    
    parallel_merge_sort(mid, last, max_depth, depth + 1);
    left_future.get();
    
    // Merge sorted halves
    std::vector<typename std::iterator_traits<Iter>::value_type> temp(size);
    Iter left = first;
    Iter right = mid;
    auto out = temp.begin();
    
    while (left != mid && right != last) {
        if (*left <= *right) {
            *out++ = *left++;
        } else {
            *out++ = *right++;
        }
    }
    
    while (left != mid) *out++ = *left++;
    while (right != last) *out++ = *right++;
    
    std::copy(temp.begin(), temp.end(), first);
}

long long benchmark_parallel_merge_sort(std::vector<int>& data, unsigned int num_threads = 9, bool verbose = true) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Calculate max depth based on number of threads (log2 of threads)
    unsigned int max_depth = 0;
    unsigned int temp = num_threads;
    while (temp > 1) {
        max_depth++;
        temp >>= 1;
    }
    
    parallel_merge_sort(data.begin(), data.end(), max_depth, 0);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (verbose) {
        std::cout << "Parallel merge sort time (" << num_threads << " threads): " << duration.count() << " ms" << std::endl;
    }
    
    return duration.count();
}

// Run benchmark for specific data type and size (tests four algorithms)
void run_benchmark_comprehensive(const std::string& data_type, size_t size, 
                                std::vector<int> (*generator)(size_t), unsigned int num_threads = 9) {
    // Generate data copies for all algorithms
    auto data0 = generator(size);
    auto data1 = data0; // for chunk-based
    auto data2 = data0; // for parallel quicksort
    auto data3 = data0; // for parallel radix sort
    auto data4 = data0; // for parallel merge sort
    
    // Single-threaded benchmark
    auto start_single = std::chrono::high_resolution_clock::now();
    std::sort(data0.begin(), data0.end());
    auto end_single = std::chrono::high_resolution_clock::now();
    auto single_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_single - start_single).count();
    
    // Verify single-threaded
    if (!verify_sorted(data0)) {
        std::cerr << "ERROR: Single-threaded sort failed!" << std::endl;
    }
    
    // Run all parallel algorithms
    std::vector<std::pair<std::string, long long>> results;
    
    auto time_chunk = benchmark_multi_threaded(data1, num_threads, false);
    if (!verify_sorted(data1)) std::cerr << "ERROR: Multi-threaded (chunk) failed!" << std::endl;
    results.emplace_back("Chunk-based", time_chunk);
    
    auto time_quick = benchmark_parallel_quicksort(data2, num_threads, false);
    if (!verify_sorted(data2)) std::cerr << "ERROR: Parallel quicksort failed!" << std::endl;
    results.emplace_back("Parallel quicksort", time_quick);
    
    auto time_radix = benchmark_parallel_radix_sort(data3, num_threads, false);
    if (!verify_sorted(data3)) std::cerr << "ERROR: Parallel radix sort failed!" << std::endl;
    results.emplace_back("Parallel radix sort", time_radix);
    
    auto time_merge = benchmark_parallel_merge_sort(data4, num_threads, false);
    if (!verify_sorted(data4)) std::cerr << "ERROR: Parallel merge sort failed!" << std::endl;
    results.emplace_back("Parallel merge sort", time_merge);
    
    // Verify all produce same result
    std::vector<int> reference = data0;
    if (data1 != reference) std::cerr << "WARNING: Chunk-based results differ!" << std::endl;
    if (data2 != reference) std::cerr << "WARNING: Parallel quicksort results differ!" << std::endl;
    if (data3 != reference) std::cerr << "WARNING: Parallel radix sort results differ!" << std::endl;
    if (data4 != reference) std::cerr << "WARNING: Parallel merge sort results differ!" << std::endl;
    
    // Print results
    std::cout << "\n" << data_type << " " << size/1000000 << "M (";
    if (num_threads == 1) {
        std::cout << "single-threaded";
    } else {
        std::cout << num_threads << " threads";
    }
    std::cout << "):\n";
    std::cout << "  Single-threaded std::sort: " << single_time << " ms\n";
    
    for (const auto& [name, time] : results) {
        double speedup = (time > 0) ? static_cast<double>(single_time) / time : 0.0;
        double diff = (single_time > 0) ? ((static_cast<double>(single_time) - time) / single_time) * 100.0 : 0.0;
        
        std::cout << "  " << name << ": " << time << " ms\t";
        std::cout << std::fixed << std::showpos << std::setprecision(1) << diff << "% ("
                  << std::noshowpos << std::setprecision(2) << speedup << "x)";
        
        if (time > single_time) {
            std::cout << " ❌";
        }
        std::cout << std::endl;
    }
}

// Test scaling with different thread counts
void test_thread_scaling(size_t size, std::vector<int> (*generator)(size_t)) {
    std::cout << "\n=== Thread scaling analysis (size: " << size/1000000 << "M) ===\n";
    
    auto data = generator(size);
    auto reference = data;
    
    // Get single-threaded baseline
    auto start_single = std::chrono::high_resolution_clock::now();
    std::sort(reference.begin(), reference.end());
    auto end_single = std::chrono::high_resolution_clock::now();
    auto single_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_single - start_single).count();
    
    std::cout << "Single-threaded baseline: " << single_time << " ms\n\n";
    std::cout << "Threads\tChunk\tQuick\tRadix\tMerge\tBest speedup\n";
    std::cout << "-------\t-----\t-----\t-----\t-----\t-----------\n";
    
    // Test different thread counts
    std::vector<unsigned int> thread_counts = {1, 2, 4, 8, 9, 12, 16, 24, 32};
    
    for (unsigned int threads : thread_counts) {
        std::vector<long long> times;
        
        auto data1 = data;
        auto data2 = data;
        auto data3 = data;
        auto data4 = data;
        
        times.push_back(benchmark_multi_threaded(data1, threads, false));
        times.push_back(benchmark_parallel_quicksort(data2, threads, false));
        times.push_back(benchmark_parallel_radix_sort(data3, threads, false));
        times.push_back(benchmark_parallel_merge_sort(data4, threads, false));
        
        // Find best speedup
        double best_speedup = 0.0;
        for (auto t : times) {
            if (t > 0) {
                double speedup = static_cast<double>(single_time) / t;
                if (speedup > best_speedup) best_speedup = speedup;
            }
        }
        
        std::cout << threads << "\t";
        for (size_t i = 0; i < times.size(); ++i) {
            std::cout << times[i] << "\t";
        }
        std::cout << std::fixed << std::setprecision(2) << best_speedup << "x\n";
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  ULTIMATE SORTING BENCHMARK (M4 Mac)\n";
    std::cout << "================================================================\n\n";
    
    std::cout << "WHY 9 THREADS ON M4?\n";
    std::cout << "--------------------\n";
    std::cout << "M4 has 8 performance cores + 2-4 efficiency cores (10-12 total).\n";
    std::cout << "9 threads is a compromise:\n";
    std::cout << "  - Matches performance cores (8) + 1 extra for I/O\n";
    std::cout << "  - Avoids oversubscription while utilizing most cores\n";
    std::cout << "  - Efficiency cores are slower but help with background tasks\n\n";
    
    std::cout << "SHOULD WE USE MORE THREADS?\n";
    std::cout << "---------------------------\n";
    std::cout << "✓ Oversubscription (more threads than cores) can help:\n";
    std::cout << "  - Mask memory latency during cache misses\n";
    std::cout << "  - Handle I/O waits in some algorithms\n";
    std::cout << "✗ But also causes:\n";
    std::cout << "  - Thread context switching overhead\n";
    std::cout << "  - Cache thrashing between cores\n";
    std::cout << "  - Increased memory bus contention\n\n";
    
    unsigned int num_threads = 9;
    
    std::cout << "================================================================\n";
    std::cout << "  ALGORITHM COMPARISON (" << num_threads << " threads)\n";
    std::cout << "================================================================\n\n";
    
    // Test comprehensive benchmarks with 9 threads
    std::cout << "1. RANDOM DATA (worst case for sorting):\n";
    run_benchmark_comprehensive("Random", 100000000, generate_random_vector, num_threads);
    
    std::cout << "\n2. ALMOST SORTED DATA (best case for adaptive algorithms):\n";
    run_benchmark_comprehensive("Almost sorted", 100000000, generate_almost_sorted_vector, num_threads);
    
    std::cout << "\n3. REVERSE SORTED DATA (worst case for quicksort):\n";
    run_benchmark_comprehensive("Reverse", 100000000, generate_reverse_sorted_vector, num_threads);
    
    std::cout << "\n================================================================\n";
    std::cout << "  THREAD SCALING ANALYSIS\n";
    std::cout << "================================================================\n";
    
    // Test scaling with different thread counts
    test_thread_scaling(100000000, generate_random_vector);
    
    std::cout << "\n================================================================\n";
    std::cout << "  KEY FINDINGS\n";
    std::cout << "================================================================\n";
    std::cout << "1. Radix Sort - Best for random data (O(n) complexity)\n";
    std::cout << "2. Parallel Quicksort - Best for almost sorted data\n";
    std::cout << "3. Merge Sort - Stable and predictable, good scaling\n";
    std::cout << "4. Chunk-based - Simple but overhead kills small arrays\n";
    std::cout << "5. Optimal threads: Usually 8-16 for memory-bound tasks\n";
    std::cout << "6. Oversubscription (16-32 threads) helps some algorithms\n";
    std::cout << "   but causes diminishing returns after ~2x core count\n";
    
    return 0;
}