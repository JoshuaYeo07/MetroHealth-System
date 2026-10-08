#include "SearchingPrinter.hpp"
#include "TablePrinter.hpp"
#include <iomanip>
#include <iostream>

namespace {
    const int REPORT_WIDTH = 162;

    // Time differences smaller than this are treated as run-to-run variation, not a real difference
    const double TIME_NOISE_PERCENT = 10.0;

    void printMetricRow(const SearchMetrics& m) {
        std::cout << "| " << std::left  << std::setw(23) << m.algorithmName
                  << "| " << std::left  << std::setw(19) << m.dataOrder
                  << "| " << std::right << std::setw(7)  << m.matches
                  << " | " << std::setw(9)  << m.keyComparisons
                  << " | " << std::setw(12) << m.boundChecks
                  << " | " << std::setw(11) << m.totalComparisons()
                  << " | " << std::setw(13) << std::fixed << std::setprecision(2) << m.avgTimeMicrosec
                  << " | " << std::setw(13) << m.memoryFootprintBytes << " B"
                  << " | " << std::setw(10) << m.auxiliaryMemoryBytes << " B"
                  << " | " << std::setw(10) << m.getPeakMemoryBytes() << " B |\n";
    }

    // Percentage of total comparisons the Sentinel version saved compared with Linear Search
    double savedPercent(const SearchMetrics& linear, const SearchMetrics& sentinel) {
        uint64_t base = linear.totalComparisons();
        if (base == 0) {
            return 0.0;
        }
        return 100.0 * (static_cast<double>(base) - static_cast<double>(sentinel.totalComparisons()))
               / static_cast<double>(base);
    }

    // The work done when testing one record against the query, for the execution time explanation
    const char* conditionWork(SearchField field) {
        switch (field) {
            case SearchField::AgeRange:  return "checking both ends of the age range";
            case SearchField::CareType:  return "comparing care type text";
            case SearchField::StayAbove: return "comparing the visit duration";
            case SearchField::CostAbove: return "calculating stay x rate x visits";
        }
        return "testing the search condition";
    }

    // Percentage of time the Sentinel version saved compared with Linear Search (negative if slower)
    double timeSavedPercent(const SearchMetrics& linear, const SearchMetrics& sentinel) {
        if (linear.avgTimeMicrosec <= 0.0) {
            return 0.0;
        }
        return 100.0 * (linear.avgTimeMicrosec - sentinel.avgTimeMicrosec) / linear.avgTimeMicrosec;
    }
}

void SearchingPrinter::printComparisonReport(const SearchComparison& result, const std::string& datasetName,
                                             size_t recordCount) {
    std::cout << "\n" << std::string(REPORT_WIDTH, '=') << "\n";
    std::cout << "  SEARCHING EXPERIMENTS (" << result.linearUnsorted.dataStructure << ") - " << result.query.label
              << " (" << datasetName << " - " << recordCount << " Records)\n";
    std::cout << std::string(REPORT_WIDTH, '=') << "\n";

    TablePrinter::printHorizontalLine(REPORT_WIDTH);
    std::cout << "| " << std::left  << std::setw(23) << "Algorithm"
              << "| " << std::left  << std::setw(19) << "Data Order"
              << "| " << std::right << std::setw(7)  << "Matches"
              << " | " << std::setw(9)  << "Key Comps"
              << " | " << std::setw(12) << "Bound Checks"
              << " | " << std::setw(11) << "Total Comps"
              << " | " << std::setw(13) << "Avg Time (us)"
              << " | " << std::setw(15) << "Mem Footprint"
              << " | " << std::setw(12) << "Aux Memory"
              << " | " << std::setw(12) << "Peak Total" << " |\n";
    TablePrinter::printHorizontalLine(REPORT_WIDTH);
    printMetricRow(result.linearUnsorted);
    printMetricRow(result.linearSorted);
    TablePrinter::printHorizontalLine(REPORT_WIDTH);
    printMetricRow(result.sentinelUnsorted);
    printMetricRow(result.sentinelSorted);
    TablePrinter::printHorizontalLine(REPORT_WIDTH);

    std::cout << "  Key Comps    : records tested against the search condition (Sentinel includes the sentinel)\n";
    std::cout << "  Bound Checks : end-of-array checks (i < n for Linear, i == n after each match for Sentinel)\n";
    std::cout << "  Avg Time     : time per search in microseconds (us), median of " << SearchEngine::TIMING_BATCHES
              << " batches of " << SearchEngine::RUNS_PER_BATCH << " searches (after a "
              << SearchEngine::WARM_UP_MILLISECONDS << " ms warm-up)\n";
}

void SearchingPrinter::printMatches(const PatientArray& matches, const SearchQuery& query,
                                    const std::string& sortedLabel, size_t maxRows) {
    TablePrinter::displayPatientTable(matches, "Matching Records: " + query.label + " | Order: " + sortedLabel,
                                      maxRows);
}

void SearchingPrinter::printObservations(const SearchComparison& result, size_t recordCount) {
    const SearchMetrics& linear = result.linearUnsorted;
    const SearchMetrics& sentinel = result.sentinelUnsorted;

    double saved = savedPercent(linear, sentinel);
    // Averaged over the unsorted and sorted runs
    double timeSaved = (timeSavedPercent(result.linearUnsorted, result.sentinelUnsorted)
                        + timeSavedPercent(result.linearSorted, result.sentinelSorted)) / 2.0;
    bool sameForSortedAndUnsorted =
        result.linearUnsorted.totalComparisons() == result.linearSorted.totalComparisons() &&
        result.sentinelUnsorted.totalComparisons() == result.sentinelSorted.totalComparisons();
    bool sentinelGrewArray = result.sentinelUnsorted.arrayGrew || result.sentinelSorted.arrayGrew;
    size_t capacity = (linear.memoryFootprintBytes - sizeof(PatientArray)) / sizeof(Patient);
    size_t bufferSlots = recordCount > 0 ? recordCount : 1;

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "\n>>> PERFORMANCE ANALYSIS & SEARCH EXPLANATION (TASK 7 & TASK 8):\n";

    std::cout << "\n1. Time Complexity\n";
    std::cout << "   Describes how searching work grows as the number of patients increases.\n";
    std::cout << "   - " << linear.algorithmName << ": " << linear.timeComplexity
              << "; every record is tested once, with a bound check before each one.\n";
    std::cout << "   - " << sentinel.algorithmName << ": " << sentinel.timeComplexity
              << "; the same record tests, but the bound is only checked after each match.\n";

    std::cout << "\n2. Execution Time\n";
    std::cout << "   Shows how long one search took in microseconds (us) via std::chrono: the median of "
              << SearchEngine::TIMING_BATCHES << " batches\n";
    std::cout << "   of " << SearchEngine::RUNS_PER_BATCH << " searches, after a "
              << SearchEngine::WARM_UP_MILLISECONDS << " ms warm-up.\n";
    std::cout << "   - On average, Sentinel Linear Search was " << (timeSaved >= 0.0 ? timeSaved : -timeSaved)
              << "% " << (timeSaved >= 0.0 ? "faster" : "slower") << " than Linear Search.\n";
    if (timeSaved > -TIME_NOISE_PERCENT && timeSaved < TIME_NOISE_PERCENT) {
        std::cout << "   - That is within normal run-to-run variation, so both run at about the same speed.\n";
        std::cout << "     Most of the time per record goes into reading the record and testing the condition\n";
        std::cout << "     (" << conditionWork(result.query.field) << "), so removing the bound check saves little time.\n";
    } else if (timeSaved >= TIME_NOISE_PERCENT) {
        std::cout << "   - It skips the bound check for most records, so each step of the scan does less work.\n";
    } else {
        std::cout << "   - Copying the sentinel record into the array on every search can outweigh the\n";
        std::cout << "     bound checks it removes.\n";
    }
    std::cout << "   Results can vary depending on the build, computer, and background activity.\n";
    std::cout << "   Result buffer allocation and copying the matching records for display are excluded.\n";

    std::cout << "\n3. Comparisons\n";
    std::cout << "   Key Comps count records tested against the search condition; Bound Checks count\n";
    std::cout << "   end-of-array checks. Total Comps = Key Comps + Bound Checks.\n";
    std::cout << "   - Linear Search: about 2n + 1 comparisons.\n";
    std::cout << "   - Sentinel Linear Search: about n + matches + 2 comparisons.\n";
    std::cout << "   - Result: Sentinel Linear Search made " << saved << "% fewer comparisons ("
              << sentinel.totalComparisons() << " vs " << linear.totalComparisons() << ").\n";
    std::cout << "     The saving shrinks as the number of matches grows, because each match adds a bound check.\n";

    std::cout << "\n4. Unsorted vs Sorted Data\n";
    if (sameForSortedAndUnsorted) {
        std::cout << "   - Comparison counts are identical for unsorted and sorted data. Both algorithms test every\n";
        std::cout << "     record, so they cannot use the sorted order to skip records or stop early.\n";
    } else {
        std::cout << "   - Comparison counts differ between unsorted and sorted data.\n";
    }
    std::cout << "   - Sorting only pays off with an order-aware algorithm, such as Binary Search (O(log n)).\n";

    std::cout << "\n5. Mem Footprint (The Storage)\n";
    std::cout << "   Estimates the array container object and its allocated patient slots.\n";
    std::cout << "   Calculation: sizeof(PatientArray) + array capacity x sizeof(Patient)\n";
    std::cout << "              = " << sizeof(PatientArray) << " + " << capacity << " x " << sizeof(Patient)
              << " = " << linear.memoryFootprintBytes << " B (" << recordCount << " records + "
              << (capacity - recordCount) << " unused slots).\n";
    std::cout << "   It is the same for both algorithms, because neither copies the dataset.\n";

    std::cout << "\n6. Auxiliary Memory (The Extra Workspace)\n";
    std::cout << "   Estimates source-level workspace using sizeof; these are not measured stack bytes.\n";
    std::cout << "   - Both algorithms: working variables (data and result pointers; n, i, found counters)\n";
    std::cout << "     plus a result index buffer of N x sizeof(size_t) = " << bufferSlots << " x " << sizeof(size_t)
              << " = " << bufferSlots * sizeof(size_t) << " B,\n";
    std::cout << "     sized for the worst case where every record matches. O(n) overall.\n";
    std::cout << "   - Sentinel Linear Search: also one temporary Patient for the sentinel record ("
              << sizeof(Patient) << " B).\n";
    if (sentinelGrewArray) {
        std::cout << "     The array was full, so it had to grow to 2x capacity; the added slots are counted here.\n";
    } else {
        std::cout << "     Its slot after the last record lies in spare capacity already counted in Mem Footprint,\n";
        std::cout << "     so no reallocation was needed. A full array would first have to grow to 2x capacity.\n";
    }

    std::cout << "\n7. Peak Total Memory (Mem Footprint + Aux Memory)\n";
    std::cout << "   - Linear Search: " << linear.getPeakMemoryBytes() << " B | Sentinel Linear Search: "
              << sentinel.getPeakMemoryBytes() << " B\n";
    std::cout << "   This is a scoped estimate for one search, not measured process peak RAM.\n";

    std::cout << "\nEstimates exclude additional string allocations, instrumentation, reference\n";
    std::cout << "parameters, stack padding and call overhead, other dataset copies, and allocator\n";
    std::cout << "overhead. sizeof values depend on the build.\n";
}
