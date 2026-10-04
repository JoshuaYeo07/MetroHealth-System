#include "SortingPrinter.hpp"
#include <iostream>
#include <iomanip>

namespace {
    void printLine(int width = 152, char border = '-', char cross = '+') {
        std::cout << cross;
        for (int i = 0; i < width - 2; ++i) {
            std::cout << border;
        }
        std::cout << cross << "\n";
    }

    void printMetricRow(const SortMetrics& m) {
        double ms = static_cast<double>(m.executionTimeMicrosec) / 1000.0;
        std::cout << "| " << std::left  << std::setw(16) << m.dataStructure
                  << "| " << std::left  << std::setw(13) << m.algorithmName
                  << "| " << std::left  << std::setw(26) << m.criterionName
                  << "| " << std::left  << std::setw(17) << m.timeComplexity
                  << "| " << std::right << std::setw(12) << std::fixed << std::setprecision(2) << ms << " ms "
                  << "| " << std::right << std::setw(12) << m.comparisons << " "
                  << "| " << std::right << std::setw(13) << m.memoryFootprintBytes << " B "
                  << "| " << std::right << std::setw(16) << m.memoryUsageBytes << " B |\n";
    }
}

void SortingPrinter::printBenchmarkReport(const BenchmarkPair pairs[], size_t pairCount,
                                         const std::string& datasetName, size_t recordCount) {
    std::cout << "\n========================================================================================================================================================\n";
    std::cout << "  TASK 6: SORTING EXPERIMENTS & PERFORMANCE BENCHMARK (" << datasetName 
              << " - " << recordCount << " Records)\n";
    std::cout << "========================================================================================================================================================\n";

    printLine(152);
    std::cout << "| " << std::left  << std::setw(16) << "Data Structure"
              << "| " << std::left  << std::setw(13) << "Algorithm"
              << "| " << std::left  << std::setw(26) << "Sort Criterion"
              << "| " << std::left  << std::setw(17) << "Time Complexity"
              << "| " << std::right << std::setw(15) << "Execution Time"
              << "| " << std::right << std::setw(13) << "Comparisons"
              << "| " << std::right << std::setw(15) << "Mem Footprint"
              << "| " << std::right << std::setw(18) << "Mem Usage (Sort)"
              << " |\n";
    printLine(152);

    for (size_t i = 0; i < pairCount; ++i) {
        printMetricRow(pairs[i].bubbleMetrics);
        printMetricRow(pairs[i].mergeMetrics);
        if (i + 1 < pairCount) {
            printLine(152, '-');
        }
    }
    printLine(152);
}

void SortingPrinter::printSortedPreview(const PatientArray& arr, const std::string& title,
                                       SortCriterion criterion, size_t sampleCount) {
    std::cout << "\n----------------------------------------------------------------------------------------------------------------------\n";
    std::cout << "  SAMPLE PREVIEW: " << title << " (Sorted by " 
              << SortingEngine::getCriterionName(criterion) << " - Ascending)\n";
    std::cout << "----------------------------------------------------------------------------------------------------------------------\n";

    if (arr.isEmpty()) {
        std::cout << "  [Notice] Array is empty.\n";
        return;
    }

    size_t limit = (sampleCount > 0 && sampleCount < arr.size()) ? sampleCount : arr.size();

    std::cout << "| " << std::left  << std::setw(12) << "Patient ID"
              << "| " << std::right << std::setw(6)  << "Age"
              << " | " << std::left  << std::setw(16) << "Care Type"
              << "| " << std::right << std::setw(12) << "Stay (hrs)"
              << "| " << std::right << std::setw(14) << "Rate (MYR/hr)"
              << "| " << std::right << std::setw(11) << "Visits/Yr"
              << "| " << std::right << std::setw(18) << "Total Cost (MYR)"
              << " |\n";
    printLine(100);

    for (size_t i = 0; i < limit; ++i) {
        const Patient& p = arr[i];
        std::cout << "| " << std::left  << std::setw(12) << p.patientId
                  << "| " << std::right << std::setw(6)  << p.age
                  << " | " << std::left  << std::setw(16) << p.careType
                  << "| " << std::right << std::setw(12) << p.lengthOfStay
                  << "| " << std::right << std::setw(14) << std::fixed << std::setprecision(2) << p.baseCostPerHour
                  << "| " << std::right << std::setw(11) << p.daysVisitsPerYear
                  << "| " << std::right << std::setw(18) << std::fixed << std::setprecision(2) << p.calculateTotalCost()
                  << " |\n";
    }
    printLine(100);
    std::cout << "  * Displaying first " << limit << " records confirming ascending order.\n";
}

void SortingPrinter::printAlgorithmicObservations() {
    std::cout << R"(
>>> PERFORMANCE ANALYSIS & BENCHMARK EXPLANATION (TASK 6 & TASK 8):

1. Time Complexity
   Describes how sorting work grows as the number of patients increases.
   - Bubble Sort: O(n^2) on average and in the worst case.
     Its early-exit feature gives O(n) when the data is already sorted.
   - Merge Sort: O(n log n), making it more efficient for larger datasets.

2. Execution Time
   Shows how long the sorting phase took in milliseconds.
   A lower value means sorting finished faster in this run.
   Results can vary depending on the build, computer, and background activity.
   Dataset copying and Merge Sort buffer allocation/cleanup are excluded.

3. Comparisons
   Counts how often two patients' ages, visit durations, or total costs
   are compared to decide their order.
   It does not count every operation, such as copying or swapping patients.
   Fewer comparisons can help explain faster execution.

4. Memory Footprint
   Estimates the allocated patient-array buffer, including unused slots.
   Calculation: array capacity x size of one Patient object.
   Unused slots still occupy memory.

5. Memory Usage (Sort)
   Estimates the separate temporary buffer allocated for sorting.
   - Bubble Sort: no separate buffer, so this column shows 0 B.
     It still uses a temporary patient when swapping.
   - Merge Sort: creates a temporary array with one slot per patient.
     This buffer is freed after sorting.

Memory estimates exclude container metadata, additional string allocations,
stack variables, recursion, and allocation overhead.
)";
}
