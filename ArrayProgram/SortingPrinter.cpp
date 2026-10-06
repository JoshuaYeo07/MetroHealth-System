#include "SortingPrinter.hpp"
#include <iostream>
#include <iomanip>

namespace {
    void printLine(int width = 164, char border = '-', char cross = '+') {
        std::cout << cross;
        for (int i = 0; i < width - 2; ++i) {
            std::cout << border;
        }
        std::cout << cross << "\n";
    }

    std::string getSectionHeader(size_t index, SortCriterion criterion) {
        std::string prefix = std::to_string(index + 1) + ". ";
        switch (criterion) {
            case SortCriterion::Age:
                return prefix + "PATIENT AGE - Ascending";
            case SortCriterion::LengthOfStay:
                return prefix + "LENGTH OF STAY - Ascending";
            case SortCriterion::TotalCost:
                return prefix + "TOTAL MEDICAL COST - Ascending";
            case SortCriterion::CareType:
                return prefix + "CARE TYPE - Alphabetical (A-Z)";
        }
        return prefix + SortingEngine::getCriterionName(criterion);
    }

    void printTableHeader() {
        printLine(164);
        std::cout << "| " << std::left  << std::setw(16) << "Data Structure"
                  << "| " << std::left  << std::setw(13) << "Algorithm"
                  << "| " << std::left  << std::setw(26) << "Sort Criterion"
                  << "| " << std::left  << std::setw(17) << "Time Complexity"
                  << "| " << std::right << std::setw(15) << "Execution Time"
                  << "| " << std::right << std::setw(13) << "Comparisons"
                  << "| " << std::right << std::setw(16) << "Mem Footprint"
                  << "| " << std::right << std::setw(14) << "Aux Memory"
                  << "| " << std::right << std::setw(14) << "Peak Total"
                  << " |\n";
        printLine(164);
    }

    void printMetricRow(const SortMetrics& m) {
        double ms = static_cast<double>(m.executionTimeMicrosec) / 1000.0;
        size_t peak = m.getPeakMemoryBytes();
        std::cout << "| " << std::left  << std::setw(16) << m.dataStructure
                  << "| " << std::left  << std::setw(13) << m.algorithmName
                  << "| " << std::left  << std::setw(26) << m.criterionName
                  << "| " << std::left  << std::setw(17) << m.timeComplexity
                  << "| " << std::right << std::setw(11) << std::fixed << std::setprecision(2) << ms << " ms "
                  << "| " << std::right << std::setw(12) << m.comparisons << " "
                  << "| " << std::right << std::setw(13) << m.memoryFootprintBytes << " B "
                  << "| " << std::right << std::setw(11) << m.auxiliaryMemoryBytes << " B "
                  << "| " << std::right << std::setw(12) << peak << " B |\n";
    }
}

void SortingPrinter::printBenchmarkReport(const BenchmarkPair pairs[], size_t pairCount,
                                         const std::string& datasetName, size_t recordCount) {
    std::cout << "\n";
    for (int i = 0; i < 164; ++i) std::cout << "=";
    std::cout << "\n  SORTING EXPERIMENTS & PERFORMANCE BENCHMARK (" << datasetName 
              << " - " << recordCount << " Records)\n";
    for (int i = 0; i < 164; ++i) std::cout << "=";
    std::cout << "\n";

    for (size_t i = 0; i < pairCount; ++i) {
        std::cout << "\n" << getSectionHeader(i, pairs[i].criterion) << "\n";
        printTableHeader();
        printMetricRow(pairs[i].bubbleMetrics);
        printMetricRow(pairs[i].mergeMetrics);
        printLine(164);
    }
}

void SortingPrinter::printSortedPreview(const PatientArray& arr, const std::string& title,
                                       SortCriterion criterion, size_t sampleCount) {
    std::string orderLabel = (criterion == SortCriterion::CareType) ? "Alphabetical (A-Z)" : "Ascending";
    const int PREVIEW_WIDTH = 98;

    std::cout << "\n";
    for (int i = 0; i < PREVIEW_WIDTH; ++i) std::cout << "-";
    std::cout << "\n  SAMPLE PREVIEW: " << title << " (Sorted by " 
              << SortingEngine::getCriterionName(criterion) << " - " << orderLabel << ")\n";
    for (int i = 0; i < PREVIEW_WIDTH; ++i) std::cout << "-";
    std::cout << "\n";

    if (arr.isEmpty()) {
        std::cout << "  [Notice] Array is empty.\n";
        return;
    }

    size_t limit = (sampleCount > 0 && sampleCount < arr.size()) ? sampleCount : arr.size();

    printLine(PREVIEW_WIDTH);
    std::cout << "| " << std::left  << std::setw(12) << "Patient ID"
              << "| " << std::right << std::setw(5)  << "Age"
              << " | " << std::left  << std::setw(17) << "Care Type"
              << "| " << std::right << std::setw(10) << "Stay (hrs)"
              << " | " << std::right << std::setw(13) << "Rate (MYR/hr)"
              << " | " << std::right << std::setw(9)  << "Visits/Yr"
              << " | " << std::right << std::setw(16) << "Total Cost (MYR)"
              << " |\n";
    printLine(PREVIEW_WIDTH);

    for (size_t i = 0; i < limit; ++i) {
        const Patient& p = arr[i];
        std::cout << "| " << std::left  << std::setw(12) << p.patientId
                  << "| " << std::right << std::setw(5)  << p.age
                  << " | " << std::left  << std::setw(17) << p.careType
                  << "| " << std::right << std::setw(10) << p.lengthOfStay
                  << " | " << std::right << std::setw(13) << std::fixed << std::setprecision(2) << p.baseCostPerHour
                  << " | " << std::right << std::setw(9)  << p.daysVisitsPerYear
                  << " | " << std::right << std::setw(16) << std::fixed << std::setprecision(2) << p.calculateTotalCost()
                  << " |\n";
    }
    printLine(PREVIEW_WIDTH);
    std::cout << "  * Displaying first " << limit << " records confirming ascending order.\n";
}

void SortingPrinter::printAlgorithmicObservations() {
    std::cout << R"(
>>> PERFORMANCE ANALYSIS & BENCHMARK EXPLANATION (TASK 6 & TASK 8):

1. Time Complexity
   Describes how sorting work grows as the number of patients increases.
   - Bubble Sort: O(n^2) on average and in the worst case.
     Its early-exit feature gives O(n) when the data is already sorted.
   - Merge Sort: O(n log n), making it far more efficient for larger datasets.

2. Execution Time
   Shows how long the sorting phase took in milliseconds via std::chrono.
   A lower value means sorting finished faster in this run.
   Results can vary depending on the build, computer, and background activity.
   Dataset copying and Merge Sort buffer allocation/cleanup are excluded.

3. Comparisons
   Counts how often two patients' ages, visit durations, or total costs
   are compared to decide their order.
   Fewer comparisons directly translate to faster execution time.

4. Mem Footprint (The Storage)
   Estimates the array container object and its allocated patient slots.
   Calculation: sizeof(PatientArray) + array capacity x sizeof(Patient).
   The table uses the current array capacity and this build's object sizes.
   Unused slots still occupy memory in a dynamic array.

5. Auxiliary Memory (The Extra Workspace)
   Estimates peak source-level workspace using sizeof for selected algorithm
   parameters and local variables; these are not measured stack bytes.
   - Bubble Sort: working variables plus one Patient if any swap occurs; O(1).
   - Merge Sort: N x sizeof(Patient) plus estimated active recursive and merge
     workspace. The deepest leaf and merge are considered separately; O(n) overall.
   - Zero or one record: 0 B of sorting workspace under this model.

6. Peak Total Memory (Mem Footprint + Aux Memory)
   Adds Mem Footprint and the estimated peak auxiliary workspace.
   This is a scoped estimate for one sort, not measured process peak RAM.

Estimates exclude additional string allocations, instrumentation, reference
parameters, comparator/library calls, stack padding and call overhead,
other dataset copies, and allocator overhead. sizeof values depend on the build.
)";
}
