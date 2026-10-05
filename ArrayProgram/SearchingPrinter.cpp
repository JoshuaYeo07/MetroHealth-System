#include "SearchingPrinter.hpp"
#include "TablePrinter.hpp"
#include <iomanip>
#include <iostream>

namespace {
    const int REPORT_WIDTH = 125;

    // Time differences smaller than this are treated as run-to-run variation, not a real difference
    const double TIME_NOISE_PERCENT = 10.0;

    void printMetricRow(const SearchMetrics& m) {
        std::cout << "| " << std::left  << std::setw(23) << m.algorithmName
                  << "| " << std::left  << std::setw(15) << m.dataOrder
                  << "| " << std::right << std::setw(7)  << m.matches
                  << " | " << std::setw(9)  << m.keyComparisons
                  << " | " << std::setw(12) << m.boundChecks
                  << " | " << std::setw(11) << m.totalComparisons()
                  << " | " << std::setw(13) << std::fixed << std::setprecision(1) << m.avgTimeNanosec
                  << " | " << std::setw(10) << m.extraMemoryBytes << " B |\n";
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

    // Percentage of time the Sentinel version saved compared with Linear Search (negative if slower)
    double timeSavedPercent(const SearchMetrics& linear, const SearchMetrics& sentinel) {
        if (linear.avgTimeNanosec <= 0.0) {
            return 0.0;
        }
        return 100.0 * (linear.avgTimeNanosec - sentinel.avgTimeNanosec) / linear.avgTimeNanosec;
    }
}

void SearchingPrinter::printComparisonReport(const SearchComparison& result, const std::string& datasetName,
                                             size_t recordCount) {
    std::cout << "\n" << std::string(REPORT_WIDTH, '=') << "\n";
    std::cout << "  TASK 7: SEARCHING EXPERIMENTS - " << result.query.label
              << " (" << datasetName << " - " << recordCount << " Records)\n";
    std::cout << std::string(REPORT_WIDTH, '=') << "\n";

    TablePrinter::printHorizontalLine(REPORT_WIDTH);
    std::cout << "| " << std::left  << std::setw(23) << "Algorithm"
              << "| " << std::left  << std::setw(15) << "Data Order"
              << "| " << std::right << std::setw(7)  << "Matches"
              << " | " << std::setw(9)  << "Key Comps"
              << " | " << std::setw(12) << "Bound Checks"
              << " | " << std::setw(11) << "Total Comps"
              << " | " << std::setw(13) << "Avg Time (ns)"
              << " | " << std::setw(12) << "Extra Memory" << " |\n";
    TablePrinter::printHorizontalLine(REPORT_WIDTH);
    printMetricRow(result.linearUnsorted);
    printMetricRow(result.linearSorted);
    TablePrinter::printHorizontalLine(REPORT_WIDTH);
    printMetricRow(result.sentinelUnsorted);
    printMetricRow(result.sentinelSorted);
    TablePrinter::printHorizontalLine(REPORT_WIDTH);

    size_t footprint = result.linearUnsorted.memoryFootprintBytes;
    std::cout << "  Array memory footprint : " << footprint / sizeof(Patient) << " slots x " << sizeof(Patient)
              << " B = " << footprint << " B (" << recordCount << " slots used)\n";
    std::cout << "  Key Comps              : records tested against the search condition (Sentinel includes the sentinel)\n";
    std::cout << "  Bound Checks           : end-of-array checks (i < n for Linear, i == n after each match for Sentinel)\n";
    std::cout << "  Avg Time               : time per search, median of " << SearchEngine::TIMING_BATCHES
              << " batches of " << SearchEngine::RUNS_PER_BATCH << " searches (after a "
              << SearchEngine::WARM_UP_MILLISECONDS << " ms warm-up)\n";
    std::cout << "  Extra Memory           : result index list (" << sizeof(size_t)
              << " B per match), plus one sentinel slot (" << sizeof(Patient) << " B) for Sentinel\n";
}

void SearchingPrinter::printMatches(const PatientArray& matches, const SearchQuery& query, size_t maxRows) {
    TablePrinter::displayPatientTable(matches, "Matching Records: " + query.label + " (listed in sorted order)",
                                      maxRows);
}

void SearchingPrinter::printObservations(const SearchComparison& result, size_t recordCount) {
    double saved = savedPercent(result.linearUnsorted, result.sentinelUnsorted);
    // Averaged over the unsorted and sorted runs
    double timeSaved = (timeSavedPercent(result.linearUnsorted, result.sentinelUnsorted)
                        + timeSavedPercent(result.linearSorted, result.sentinelSorted)) / 2.0;
    bool sameForSortedAndUnsorted =
        result.linearUnsorted.totalComparisons() == result.linearSorted.totalComparisons() &&
        result.sentinelUnsorted.totalComparisons() == result.sentinelSorted.totalComparisons();
    bool sentinelGrewArray = result.sentinelUnsorted.arrayGrew || result.sentinelSorted.arrayGrew;
    size_t footprint = result.linearUnsorted.memoryFootprintBytes;
    size_t capacity = footprint / sizeof(Patient);

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "\n>>> SEARCH PERFORMANCE ANALYSIS (TASK 7 & TASK 8):\n";

    std::cout << "\n1. Comparisons\n";
    std::cout << "   - Linear Search checks the array bound (i < n) before every record: about 2n + 1 comparisons.\n";
    std::cout << "   - Sentinel Linear Search copies a matching record into the slot after the last record, so the\n";
    std::cout << "     scan always stops there and the bound is only checked after each match:\n";
    std::cout << "     about n + matches + 2 comparisons.\n";
    std::cout << "   - Result: Sentinel Linear Search made " << saved << "% fewer comparisons ("
              << result.sentinelUnsorted.totalComparisons() << " vs " << result.linearUnsorted.totalComparisons()
              << ").\n";
    std::cout << "     The saving shrinks as the number of matches grows, because each match adds a bound check.\n";

    std::cout << "\n2. Execution Time (median of " << SearchEngine::TIMING_BATCHES << " batches x "
              << SearchEngine::RUNS_PER_BATCH << " searches)\n";
    std::cout << "   - On average, Sentinel Linear Search was " << (timeSaved >= 0.0 ? timeSaved : -timeSaved)
              << "% " << (timeSaved >= 0.0 ? "faster" : "slower") << " than Linear Search.\n";
    if (timeSaved > -TIME_NOISE_PERCENT && timeSaved < TIME_NOISE_PERCENT) {
        std::cout << "   - That is within normal run-to-run variation, so both run at about the same speed.\n";
        std::cout << "     Most of the time per record goes into testing the search condition (e.g. comparing\n";
        std::cout << "     care type strings), so removing the bound check saves little time.\n";
    } else if (timeSaved >= TIME_NOISE_PERCENT) {
        std::cout << "   - It skips the bound check for most records, so each step of the scan does less work.\n";
    } else {
        std::cout << "   - Copying the sentinel record into the array on every search can outweigh the\n";
        std::cout << "     bound checks it removes.\n";
    }
    std::cout << "   - Times vary between runs, builds (Debug/Release) and computers; comparison counts do not.\n";

    std::cout << "\n3. Unsorted vs Sorted Data\n";
    if (sameForSortedAndUnsorted) {
        std::cout << "   - Comparison counts are identical for unsorted and sorted data. Both algorithms test every\n";
        std::cout << "     record, so they cannot use the sorted order to skip records or stop early.\n";
    } else {
        std::cout << "   - Comparison counts differ between unsorted and sorted data.\n";
    }
    std::cout << "   - Sorting only pays off with an order-aware algorithm, such as Binary Search (O(log n)).\n";

    std::cout << "\n4. Memory Overhead\n";
    std::cout << "   - Array footprint: " << capacity << " slots x " << sizeof(Patient) << " B = " << footprint
              << " B (" << recordCount << " records + " << (capacity - recordCount)
              << " unused slots from capacity doubling).\n";
    std::cout << "   - Linear Search: O(1) auxiliary memory; only the result list (" << sizeof(size_t)
              << " B per matching index).\n";
    std::cout << "   - Sentinel Linear Search: one extra Patient slot (" << sizeof(Patient) << " B) for the sentinel.\n";
    if (sentinelGrewArray) {
        std::cout << "     The array was full, so it had to grow (2x capacity) before the sentinel could be placed.\n";
    } else {
        std::cout << "     The array had spare capacity, so the sentinel used a free slot without reallocation.\n";
        std::cout << "     A full array would first have to grow to 2x its capacity.\n";
    }
}
