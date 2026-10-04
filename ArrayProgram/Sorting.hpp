#ifndef SORTING_HPP
#define SORTING_HPP

#include "PatientArray.hpp"
#include <string>
#include <cstdint>
#include <chrono>

/**
 * @enum SortCriterion
 * @brief Criteria supported for sorting experiments as specified in Assignment Task 6a.
 */
enum class SortCriterion {
    Age,            // Sort by patient age (ascending)
    LengthOfStay,   // Sort by visit duration in hours (ascending)
    TotalCost       // Sort by total medical billing in MYR (ascending)
};

/**
 * @struct SortMetrics
 * @brief Empirical and theoretical metrics recorded during a sorting experiment.
 */
struct SortMetrics {
    std::string dataStructure;        // "Dynamic Array" (for comparing with Singly Linked List)
    std::string algorithmName;        // "Bubble Sort" or "Merge Sort"
    std::string criterionName;        // "Patient Age", "Length of Stay", "Total Medical Cost"
    std::string timeComplexity;       // "O(n^2)" or "O(n log n)"
    long long executionTimeMicrosec;  // Measured execution time in microseconds (std::chrono)
    uint64_t comparisons;             // Total element-to-element comparisons performed
    uint64_t swapsMovements;          // Total swaps (Bubble Sort) or buffer movements (Merge Sort)
    size_t memoryFootprintBytes;      // Estimated allocated patient buffer: capacity * sizeof(Patient)
    size_t memoryUsageBytes;          // Extra temporary memory allocated during sorting (0 B vs O(n) scratchpad)
};

/**
 * @struct BenchmarkPair
 * @brief Bundles benchmark results for both algorithms under the same sorting criterion.
 */
struct BenchmarkPair {
    SortCriterion criterion;
    SortMetrics bubbleMetrics;
    SortMetrics mergeMetrics;
};

/**
 * @class SortingEngine
 * @brief Zero-STL sorting implementations and benchmark harness for Task 6.
 */
class SortingEngine {
public:
    /**
     * @brief Sorts a PatientArray in-place using Bubble Sort with early-exit optimization.
     * Time Complexity: O(n^2) worst/average, O(n) best.
     * Space Complexity: O(1) auxiliary.
     */
    static SortMetrics bubbleSort(PatientArray& arr, SortCriterion criterion);

    /**
     * @brief Sorts a PatientArray using divide-and-conquer Merge Sort.
     * Time Complexity: O(n log n) in all cases.
     * Space Complexity: O(n) auxiliary scratchpad buffer.
     */
    static SortMetrics mergeSort(PatientArray& arr, SortCriterion criterion);

    /**
     * @brief Runs both algorithms against identical clones of an unsorted source dataset.
     * @param source The unsorted source PatientArray
     * @param criterion The sorting criterion to benchmark
     * @param outSorted Optional pointer to store the resulting sorted PatientArray (for Task 7)
     * @return BenchmarkPair containing metrics for both Bubble Sort and Merge Sort
     */
    static BenchmarkPair runComparison(const PatientArray& source, SortCriterion criterion, PatientArray* outSorted = nullptr);

    /**
     * @brief Helper to retrieve the human-readable display name of a sorting criterion.
     */
    static std::string getCriterionName(SortCriterion criterion);
};

#endif // SORTING_HPP
