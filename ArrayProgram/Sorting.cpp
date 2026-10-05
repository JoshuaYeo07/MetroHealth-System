#include "Sorting.hpp"
#include <chrono>

namespace {
    // Auxiliary-memory constants.
    // Rule: count working pointers, indices, flags, and temporary objects.
    //       Include return address + saved frame pointer for recursive frames.
    //       Exclude pass-through parameters (criterion, comparisons, movements),
    //       instrumentation, and dynamic string storage.

    // bubbleSort workspace: data pointer; n, i, j counters; swapped flag; Patient temp for swap
    static constexpr size_t BUBBLE_AUX_BYTES =
        sizeof(Patient*) + 3 * sizeof(size_t) + sizeof(bool) + sizeof(Patient);

    // One mergeSortRecursive frame: arr, temp pointers; left, right, mid + return address + frame pointer
    static constexpr size_t MERGE_FRAME_BYTES =
        2 * sizeof(Patient*) + 3 * sizeof(size_t) + 2 * sizeof(void*);

    // merge() helper at deepest level: arr, temp pointers; left, mid, right, i, j, k + return address
    static constexpr size_t MERGE_STEP_BYTES =
        2 * sizeof(Patient*) + 6 * sizeof(size_t) + sizeof(void*);

    // Recursion depth for n elements: number of levels in the merge-sort call tree
    static size_t mergeDepth(size_t n) {
        size_t depth = 1;
        for (size_t length = n; length > 1; length = length / 2 + length % 2) {
            ++depth;
        }
        return depth;
    }

    // Helper to evaluate if patient 'a' should follow patient 'b' in ascending order
    inline bool isGreater(const Patient& a, const Patient& b, SortCriterion criterion, uint64_t& comparisons) {
        comparisons++;
        switch (criterion) {
            case SortCriterion::Age:
                return a.age > b.age;
            case SortCriterion::LengthOfStay:
                return a.lengthOfStay > b.lengthOfStay;
            case SortCriterion::TotalCost:
                return a.calculateTotalCost() > b.calculateTotalCost();
            case SortCriterion::CareType:
                return a.careType > b.careType;
        }
        return false;
    }

    // Merge helper for Merge Sort: merges [left..mid] and [mid+1..right] into temp buffer, then writes back to arr
    void merge(Patient* arr, Patient* temp, size_t left, size_t mid, size_t right,
               SortCriterion criterion, uint64_t& comparisons, uint64_t& movements) {
        size_t i = left;
        size_t j = mid + 1;
        size_t k = left;

        while (i <= mid && j <= right) {
            // In ascending sort, if left <= right, keep left first
            if (!isGreater(arr[i], arr[j], criterion, comparisons)) {
                temp[k++] = arr[i++];
            } else {
                temp[k++] = arr[j++];
            }
            movements++;
        }

        while (i <= mid) {
            temp[k++] = arr[i++];
            movements++;
        }

        while (j <= right) {
            temp[k++] = arr[j++];
            movements++;
        }

        // Copy merged elements back into the original array segment
        for (size_t idx = left; idx <= right; ++idx) {
            arr[idx] = temp[idx];
            movements++;
        }
    }

    // Recursive helper for Merge Sort
    void mergeSortRecursive(Patient* arr, Patient* temp, size_t left, size_t right,
                            SortCriterion criterion, uint64_t& comparisons, uint64_t& movements) {
        if (left < right) {
            size_t mid = left + (right - left) / 2;
            mergeSortRecursive(arr, temp, left, mid, criterion, comparisons, movements);
            mergeSortRecursive(arr, temp, mid + 1, right, criterion, comparisons, movements);
            merge(arr, temp, left, mid, right, criterion, comparisons, movements);
        }
    }
}

std::string SortingEngine::getCriterionName(SortCriterion criterion) {
    switch (criterion) {
        case SortCriterion::Age:          return "Patient Age";
        case SortCriterion::LengthOfStay: return "Length of Stay (hrs)";
        case SortCriterion::TotalCost:    return "Total Medical Cost (MYR)";
        case SortCriterion::CareType:     return "Care Type (A-Z)";
    }
    return "Unknown";
}

SortMetrics SortingEngine::bubbleSort(PatientArray& arr, SortCriterion criterion) {
    SortMetrics m;
    m.dataStructure = "Dynamic Array";
    m.algorithmName = "Bubble Sort";
    m.criterionName = getCriterionName(criterion);
    m.timeComplexity = "O(n^2)";
    m.comparisons = 0;
    m.swapsMovements = 0;

    size_t n = arr.size();
    m.memoryFootprintBytes = arr.getMemoryUsageBytes(); // Container object plus allocated patient slots
    m.auxiliaryMemoryBytes = 0; // Early return uses no sorting workspace in the model.

    if (n <= 1) {
        m.executionTimeMicrosec = 0;
        return m;
    }

    Patient* data = arr.rawData();

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (size_t j = 0; j < n - i - 1; ++j) {
            if (isGreater(data[j], data[j + 1], criterion, m.comparisons)) {
                Patient temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
                m.swapsMovements++;
                swapped = true;
            }
        }
        // Early-exit optimization: If no swaps occurred, array is already sorted
        if (!swapped) {
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    m.executionTimeMicrosec = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    m.auxiliaryMemoryBytes = BUBBLE_AUX_BYTES;

    return m;
}

SortMetrics SortingEngine::mergeSort(PatientArray& arr, SortCriterion criterion) {
    SortMetrics m;
    m.dataStructure = "Dynamic Array";
    m.algorithmName = "Merge Sort";
    m.criterionName = getCriterionName(criterion);
    m.timeComplexity = "O(n log n)";
    m.comparisons = 0;
    m.swapsMovements = 0;

    size_t n = arr.size();
    m.memoryFootprintBytes = arr.getMemoryUsageBytes(); // Container object plus allocated patient slots

    if (n <= 1) {
        m.auxiliaryMemoryBytes = 0;
        m.executionTimeMicrosec = 0;
        return m;
    }

    size_t depth = mergeDepth(n);
    m.auxiliaryMemoryBytes = n * sizeof(Patient) + depth * MERGE_FRAME_BYTES + MERGE_STEP_BYTES;

    Patient* data = arr.rawData();

    // Allocate scratchpad buffer once at root to avoid repeated heap allocations
    Patient* temp = new Patient[n];

    auto start = std::chrono::high_resolution_clock::now();

    mergeSortRecursive(data, temp, 0, n - 1, criterion, m.comparisons, m.swapsMovements);

    auto end = std::chrono::high_resolution_clock::now();
    m.executionTimeMicrosec = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Free scratchpad memory to prevent memory leaks
    delete[] temp;

    return m;
}

BenchmarkPair SortingEngine::runComparison(const PatientArray& source, SortCriterion criterion, PatientArray* outSorted) {
    BenchmarkPair result;
    result.criterion = criterion;

    // Clone source for Bubble Sort benchmark
    PatientArray bubbleCopy = source;
    result.bubbleMetrics = bubbleSort(bubbleCopy, criterion);

    // Clone source for Merge Sort benchmark
    PatientArray mergeCopy = source;
    result.mergeMetrics = mergeSort(mergeCopy, criterion);

    // Save the verified sorted array if requested (for Task 7 searching)
    if (outSorted != nullptr) {
        *outSorted = mergeCopy;
    }

    return result;
}
