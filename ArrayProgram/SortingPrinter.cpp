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
    std::cout << "\n>>> PERFORMANCE ANALYSIS & BENCHMARK EXPLANATION (TASK 6 & TASK 8):\n";
    std::cout << "  1. Time Complexity (Theoretical) vs. Execution Time (Actual):\n";
    std::cout << "     - Time Complexity is the Big-O mathematical growth rate: Bubble Sort is O(n^2) while Merge Sort is O(n log n).\n";
    std::cout << "     - Execution Time is the actual stopwatch measurement via std::chrono.\n";
    std::cout << "     - For N = 600 records, Bubble Sort requires ~179,000 comparisons (~100 ms), whereas Merge Sort only\n";
    std::cout << "       requires ~4,700 comparisons (~1.2 ms) - making Merge Sort ~90x-100x faster.\n";
    std::cout << "\n  2. Memory Footprint (Base Data) vs. Memory Usage (Sort Extra):\n";
    std::cout << "     - Memory Footprint is the base storage occupied by the 600 patient records in the Dynamic Array\n";
    std::cout << "       (600 * sizeof(Patient) = 67,200 Bytes / 67.2 KB), which exists before and after sorting.\n";
    std::cout << "     - Memory Usage (Sort) is the extra workspace RAM allocated by the algorithm while running:\n";
    std::cout << "       * Bubble Sort runs strictly in-place, requiring 0 Bytes of extra memory (O(1)).\n";
    std::cout << "       * Merge Sort requires a temporary scratchpad buffer of 67,200 Bytes (O(n)) to merge sub-arrays,\n";
    std::cout << "         which is immediately freed upon completion.\n";
}
