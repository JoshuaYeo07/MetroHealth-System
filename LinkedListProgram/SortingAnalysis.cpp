#include "SortingAnalysis.hpp"
#include "TablePrinter.hpp"
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

namespace {

    // Each algorithm is timed on this many fresh copies of the list and the times are averaged,
    // because one run on a few hundred records is too short to time reliably
    const int REPEATS = 20;

    using SortFunction = SortStats (*)(PatientList&, PatientSorter::Comparator);

    // Result of running both algorithms on the same dataset with the same comparison rule
    struct Comparison {
        SortStats bubble;
        SortStats merge;
        bool bubbleSorted;
        bool mergeSorted;
        size_t recordCount;
        size_t listBytes;
        PatientList sorted;     // The merge sort result, used for the sample table

        Comparison() : bubbleSorted(false), mergeSorted(false), recordCount(0), listBytes(0) {}
    };

    // Sorts REPEATS copies of source and returns the stats of the last run with the averaged time.
    // Each copy is made before the sort function starts its own timer, so copying is not timed.
    SortStats measure(SortFunction sort, const PatientList& source,
                      PatientSorter::Comparator comesBefore, PatientList& sortedOut) {
        SortStats result;
        double totalMs = 0.0;
        for (int run = 0; run < REPEATS; ++run) {
            PatientList working = source;
            SortStats stats = sort(working, comesBefore);
            totalMs += stats.elapsedMs;
            result = stats;
            if (run == REPEATS - 1) {
                sortedOut = std::move(working);
            }
        }
        result.elapsedMs = totalMs / REPEATS;
        return result;
    }

    Comparison compare(const PatientList& patients, SortField field, bool ascending) {
        PatientSorter::Comparator rule = PatientSorter::comparatorFor(field, ascending);

        Comparison result;
        PatientList bubbleResult;
        result.bubble = measure(PatientSorter::bubbleSort, patients, rule, bubbleResult);
        result.merge = measure(PatientSorter::mergeSort, patients, rule, result.sorted);

        // A correct sort keeps every record and leaves them in order
        result.bubbleSorted = bubbleResult.size() == patients.size() && PatientSorter::isSorted(bubbleResult, rule);
        result.mergeSorted = result.sorted.size() == patients.size() && PatientSorter::isSorted(result.sorted, rule);
        result.recordCount = patients.size();
        result.listBytes = patients.getMemoryUsageBytes();
        return result;
    }

    std::string orderName(bool ascending) {
        return ascending ? "ascending" : "descending";
    }

    // ---- Table helpers: widths[i] is the text width of column i ----

    template <size_t N>
    int tableWidth(const int (&widths)[N]) {
        int total = 1;
        for (size_t i = 0; i < N; ++i) {
            total += widths[i] + 3;     // "| " + text + " "
        }
        return total;
    }

    // Pass as rightAlignFrom to left align every column
    const size_t LEFT_ALIGN_ALL = static_cast<size_t>(-1);

    // Columns from rightAlignFrom onwards are right aligned (numbers); earlier ones are left aligned
    template <size_t N>
    void printRow(const std::string (&cells)[N], const int (&widths)[N], size_t rightAlignFrom) {
        for (size_t i = 0; i < N; ++i) {
            std::cout << "| ";
            if (i >= rightAlignFrom) {
                std::cout << std::right;
            } else {
                std::cout << std::left;
            }
            std::cout << std::setw(widths[i]) << TablePrinter::fit(cells[i], widths[i]) << " ";
        }
        std::cout << "|\n";
    }

    void printPerformanceTable(const Comparison& c) {
        const int widths[] = { 14, 8, 15, 14, 12, 10 };
        const int width = tableWidth(widths);
        const std::string header[] = { "Algorithm", "Records", "Avg Time (ms)", "Comparisons", "Node Moves", "Sorted OK" };
        const std::string bubbleRow[] = {
            "Bubble Sort", std::to_string(c.recordCount), TablePrinter::formatDecimal(c.bubble.elapsedMs, 4),
            std::to_string(c.bubble.comparisons), std::to_string(c.bubble.moves), c.bubbleSorted ? "YES" : "NO" };
        const std::string mergeRow[] = {
            "Merge Sort", std::to_string(c.recordCount), TablePrinter::formatDecimal(c.merge.elapsedMs, 4),
            std::to_string(c.merge.comparisons), std::to_string(c.merge.moves), c.mergeSorted ? "YES" : "NO" };

        TablePrinter::printHorizontalLine(width);
        printRow(header, widths, 1);
        TablePrinter::printHorizontalLine(width);
        printRow(bubbleRow, widths, 1);
        printRow(mergeRow, widths, 1);
        TablePrinter::printHorizontalLine(width);

        std::cout << "  * Time is the average of " << REPEATS << " runs, each on a fresh copy of the list.\n"
                  << "  * Node Moves: one adjacent swap (bubble) or one node placed into the merged output (merge).\n";
        if (c.bubble.elapsedMs > 0.0 && c.merge.elapsedMs > 0.0) {
            if (c.bubble.elapsedMs >= c.merge.elapsedMs) {
                std::cout << "  * Merge sort was " << TablePrinter::formatDecimal(c.bubble.elapsedMs / c.merge.elapsedMs, 1)
                          << "x faster than bubble sort on this data.\n";
            } else {
                std::cout << "  * Bubble sort was " << TablePrinter::formatDecimal(c.merge.elapsedMs / c.bubble.elapsedMs, 1)
                          << "x faster than merge sort on this data.\n";
            }
        }
    }

    void printMemoryTable(const Comparison& c) {
        const int widths[] = { 14, 18, 11, 9, 14, 14 };
        const int width = tableWidth(widths);
        const std::string header[] = { "Algorithm", "List Footprint (B)", "Extra Nodes", "Max Depth", "Aux Memory (B)", "Peak Total (B)" };
        const std::string bubbleRow[] = {
            "Bubble Sort", std::to_string(c.listBytes), "0", std::to_string(c.bubble.maxDepth),
            std::to_string(c.bubble.auxBytes), std::to_string(c.listBytes + c.bubble.auxBytes) };
        const std::string mergeRow[] = {
            "Merge Sort", std::to_string(c.listBytes), "0", std::to_string(c.merge.maxDepth),
            std::to_string(c.merge.auxBytes), std::to_string(c.listBytes + c.merge.auxBytes) };

        TablePrinter::printHorizontalLine(width);
        printRow(header, widths, 1);
        TablePrinter::printHorizontalLine(width);
        printRow(bubbleRow, widths, 1);
        printRow(mergeRow, widths, 1);
        TablePrinter::printHorizontalLine(width);

        std::cout << "  * Both sorts relink the existing nodes, so the list itself takes the same memory and no node is allocated.\n"
                  << "  * Aux Memory is an estimate of the working variables (bubble) or recursion stack (merge) beyond the list.\n"
                  << "    Max Depth is the deepest recursion level reached (bubble sort is a loop, so 0).\n";
    }

    void printComplexityTable() {
        const int widths[] = { 14, 20, 12, 12, 20, 7 };
        const int width = tableWidth(widths);
        const std::string header[] = { "Algorithm", "Best Case", "Average", "Worst Case", "Extra Space", "Stable" };
        const std::string bubbleRow[] = { "Bubble Sort", "O(n) (no swaps)", "O(n^2)", "O(n^2)", "O(1)", "Yes" };
        const std::string mergeRow[] = { "Merge Sort", "O(n log n)", "O(n log n)", "O(n log n)", "O(log n) stack", "Yes" };

        TablePrinter::printHorizontalLine(width);
        printRow(header, widths, LEFT_ALIGN_ALL);     // every column left aligned
        TablePrinter::printHorizontalLine(width);
        printRow(bubbleRow, widths, LEFT_ALIGN_ALL);
        printRow(mergeRow, widths, LEFT_ALIGN_ALL);
        TablePrinter::printHorizontalLine(width);

        std::cout << "  * n = number of patient records. Bubble sort stops early when a sweep makes no swap, which gives its O(n) best case.\n";
    }

} // namespace

void SortingAnalysis::printSortingReport(const PatientList& patients, const std::string& datasetTitle,
                                         SortField field, bool ascending) {
    TablePrinter::printBanner("SORTING & PERFORMANCE ANALYSIS - " + datasetTitle);
    if (patients.isEmpty()) {
        std::cout << "  [Notice] No records to sort.\n";
        return;
    }

    const std::string description = "Sorted by " + PatientSorter::fieldName(field) + " (" + orderName(ascending) + ")";
    Comparison comparison = compare(patients, field, ascending);

    TablePrinter::displayPatientTable(comparison.sorted, description + " - first 10 records", 10);

    TablePrinter::printSectionTitle("PERFORMANCE - BUBBLE SORT vs MERGE SORT (measured)");
    printPerformanceTable(comparison);

    TablePrinter::printSectionTitle("MEMORY USAGE");
    printMemoryTable(comparison);

    TablePrinter::printSectionTitle("TIME COMPLEXITY");
    printComplexityTable();
}

void SortingAnalysis::printAllFieldsReport(const PatientList& patients, const std::string& datasetTitle, bool ascending) {
    TablePrinter::printBanner("SORTING & PERFORMANCE ANALYSIS (ALL FIELDS) - " + datasetTitle);
    if (patients.isEmpty()) {
        std::cout << "  [Notice] No records to sort.\n";
        return;
    }

    const SortField fields[] = { SortField::Age, SortField::VisitDuration, SortField::TotalCost };
    for (SortField field : fields) {
        TablePrinter::printSectionTitle("Sorted by " + PatientSorter::fieldName(field) + " (" + orderName(ascending) + ")");
        Comparison comparison = compare(patients, field, ascending);
        printPerformanceTable(comparison);
        std::cout << "\n";
        printMemoryTable(comparison);
    }

    TablePrinter::printSectionTitle("TIME COMPLEXITY");
    printComplexityTable();
}
