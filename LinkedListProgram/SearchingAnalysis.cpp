#include "SearchingAnalysis.hpp"
#include "TablePrinter.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>

namespace {

    // One search of a few hundred nodes takes about a microsecond, too short to time on its own,
    // so each search is timed over this many runs and the time is averaged
    const int SEARCH_REPEATS = 2000;

    const char* const UNSORTED = "Unsorted";
    const char* const SORTED = "Sorted";

    // Result of running one query with both algorithms, on unsorted data and on sorted data
    struct Experiment {
        SearchStats linearUnsorted;       // Linear search on the data in loaded order
        SearchStats sentinelUnsorted;     // Sentinel linear search on the data in loaded order
        SearchStats linearSorted;         // Linear search on the sorted copy
        SearchStats sentinelSorted;       // Sentinel linear search on the sorted copy
        std::string sortOrder;            // How the sorted copy is ordered, e.g. "Age (youngest first)"
        size_t recordCount;
        size_t listBytes;                 // Footprint of the dataset list
        size_t sortedCopyBytes;           // Footprint of the sorted copy the sorted searches need
        PatientList matches;              // The matching records in loaded order (found on the unsorted data)
        PatientList sortedMatches;        // The same records in sorted order (found on the sorted data)

        Experiment() : recordCount(0), listBytes(0), sortedCopyBytes(0) {}
    };

    // Runs one algorithm on one list and returns its counts together with its average time per search
    SearchStats measure(SearchAlgorithm algorithm, PatientList& list, const SearchCriteria& criteria,
                        DataOrder order, PatientList* matchesOut) {
        // Counted run: gives the exact comparisons and end checks, and fills the result list when one is given
        SearchStats stats = PatientSearcher::search(algorithm, list, criteria, order, matchesOut);

        // Timed runs: no result list is built, so only the search itself is timed.
        // Adding up every run's match count uses each result, so the compiler cannot skip the repeats.
        size_t matchesSeen = 0;
        auto start = std::chrono::steady_clock::now();
        for (int run = 0; run < SEARCH_REPEATS; ++run) {
            matchesSeen += PatientSearcher::search(algorithm, list, criteria, order, nullptr).matchCount;
        }
        auto end = std::chrono::steady_clock::now();

        stats.elapsedMicros = std::chrono::duration<double, std::micro>(end - start).count() / SEARCH_REPEATS;
        stats.matchCount = matchesSeen / SEARCH_REPEATS;
        return stats;
    }

    Experiment runExperiment(const PatientList& patients, const SearchCriteria& criteria, bool sortedAscending) {
        Experiment experiment;
        experiment.recordCount = patients.size();
        experiment.listBytes = patients.getMemoryUsageBytes();
        experiment.sortOrder = PatientSearcher::sortOrderName(criteria.field, sortedAscending);

        // Every search runs on the experiment's own copies, because sentinel linear search links a
        // temporary node to the list it searches and the loaded dataset must never be modified.
        // Both algorithms search the same copy, so their times can be compared fairly.
        // The copies are freed when this function returns.

        // Unsorted data: a copy of the dataset in the order it was loaded
        PatientList unsortedCopy = patients;
        experiment.linearUnsorted = measure(SearchAlgorithm::Linear, unsortedCopy, criteria,
                                            DataOrder::Unsorted, &experiment.matches);
        experiment.sentinelUnsorted = measure(SearchAlgorithm::SentinelLinear, unsortedCopy, criteria,
                                              DataOrder::Unsorted, nullptr);

        // Sorted data: a merge-sorted copy of the dataset, from PatientSorter::sortedCopy
        const SortField sortField = PatientSearcher::sortFieldFor(criteria.field);
        PatientList sortedCopy = PatientSorter::sortedCopy(patients, sortField, sortedAscending);
        experiment.sortedCopyBytes = sortedCopy.getMemoryUsageBytes();
        experiment.linearSorted = measure(SearchAlgorithm::Linear, sortedCopy, criteria,
                                          DataOrder::Sorted, &experiment.sortedMatches);
        experiment.sentinelSorted = measure(SearchAlgorithm::SentinelLinear, sortedCopy, criteria,
                                            DataOrder::Sorted, nullptr);
        return experiment;
    }

    // ---- Table helpers ----

    // One table cell: "| " + the text padded (or cut) to the column width + " "
    std::string cell(const std::string& text, size_t width, bool rightAlign) {
        std::string shown = TablePrinter::fit(text, width);
        std::string padding(width - shown.size(), ' ');
        return "| " + (rightAlign ? padding + shown : shown + padding) + " ";
    }

    // Prints the header row between two border lines that are exactly as wide as the row
    void printTableHeader(const std::string& headerRow) {
        TablePrinter::printHorizontalLine(static_cast<int>(headerRow.size()));
        std::cout << headerRow << "\n";
        TablePrinter::printHorizontalLine(static_cast<int>(headerRow.size()));
    }

    // "40.00%": how much smaller 'value' is than 'baseline'
    std::string percentFewer(long long value, long long baseline) {
        return TablePrinter::formatPercent(static_cast<double>(baseline - value), static_cast<double>(baseline));
    }

    void printMatchSummary(const Experiment& e) {
        std::cout << "  * Matched " << e.linearUnsorted.matchCount << " of " << e.recordCount << " records ("
                  << TablePrinter::formatPercent(static_cast<double>(e.linearUnsorted.matchCount),
                                                 static_cast<double>(e.recordCount)) << ").\n";
    }

    // ---- Performance table ----

    std::string performanceRow(const std::string& algorithm, const std::string& data, const std::string& matches,
                               const std::string& comparisons, const std::string& endChecks,
                               const std::string& time, const std::string& sameResult) {
        return cell(algorithm, 22, false) + cell(data, 8, false) + cell(matches, 7, true)
             + cell(comparisons, 11, true) + cell(endChecks, 10, true) + cell(time, 13, true)
             + cell(sameResult, 11, false) + "|";
    }

    std::string performanceRow(SearchAlgorithm algorithm, const std::string& data, const SearchStats& stats,
                               const std::string& sameResult) {
        return performanceRow(PatientSearcher::algorithmName(algorithm), data, std::to_string(stats.matchCount),
                              std::to_string(stats.comparisons), std::to_string(stats.endChecks),
                              TablePrinter::formatDecimal(stats.elapsedMicros, 4), sameResult);
    }

    void printPerformanceTable(const Experiment& e) {
        auto sameAsBaseline = [&e](const SearchStats& stats) {
            return std::string(stats.matchCount == e.linearUnsorted.matchCount ? "YES" : "NO");
        };

        const std::string header = performanceRow("Algorithm", "Data", "Matches", "Comparisons", "End Checks",
                                                  "Avg Time (us)", "Same Result");
        printTableHeader(header);
        std::cout << performanceRow(SearchAlgorithm::Linear, UNSORTED, e.linearUnsorted, "(baseline)") << "\n"
                  << performanceRow(SearchAlgorithm::SentinelLinear, UNSORTED, e.sentinelUnsorted,
                                    sameAsBaseline(e.sentinelUnsorted)) << "\n"
                  << performanceRow(SearchAlgorithm::Linear, SORTED, e.linearSorted,
                                    sameAsBaseline(e.linearSorted)) << "\n"
                  << performanceRow(SearchAlgorithm::SentinelLinear, SORTED, e.sentinelSorted,
                                    sameAsBaseline(e.sentinelSorted)) << "\n";
        TablePrinter::printHorizontalLine(static_cast<int>(header.size()));

        std::cout << "  * n = " << e.recordCount << " records. Comparisons = records checked against the query; each one is one step along the chain.\n"
                  << "  * End Checks = times the search tested whether it had reached the end of the list. Linear search tests\n"
                  << "    before every record. Sentinel linear search tests only when it finds a match, because the sentinel\n"
                  << "    node it links after the last record always matches (that node is its one extra comparison).\n"
                  << "  * Avg Time is the average of " << SEARCH_REPEATS << " searches, in microseconds (us). Building the result list is not\n"
                  << "    timed; creating and removing the sentinel node is.\n"
                  << "  * Same Result = found as many matching records as linear search on the unsorted data.\n";

        // Unsorted vs sorted data
        if (e.linearSorted.comparisons < e.linearUnsorted.comparisons) {
            std::cout << "  * Sorted vs unsorted: on sorted data the search stopped after " << e.linearSorted.comparisons
                      << " of " << e.recordCount << " records ("
                      << percentFewer(e.linearSorted.comparisons, e.linearUnsorted.comparisons) << " fewer\n"
                      << "    comparisons): the matching records sit together, so the search stops as soon as that group ends.\n";
        } else if (e.linearUnsorted.matchCount == 0) {
            std::cout << "  * Sorted vs unsorted: sorting saved no comparisons, because there are no matching records and the\n"
                      << "    search still had to check every record.\n";
        } else {
            std::cout << "  * Sorted vs unsorted: sorting saved no comparisons, because the matching records run to the end of the\n"
                      << "    sorted list and the search still had to check every record.";
            // A group at the tail in one direction is at the head in the other (unless every record matches)
            if (e.linearUnsorted.matchCount < e.recordCount) {
                std::cout << " The opposite order would put them first.";
            }
            std::cout << "\n";
        }

        // Sentinel linear search vs linear search
        if (e.sentinelUnsorted.endChecks < e.linearUnsorted.endChecks) {
            std::cout << "  * Sentinel vs linear: on unsorted data the sentinel search made " << e.sentinelUnsorted.endChecks
                      << (e.sentinelUnsorted.endChecks == 1 ? " end check" : " end checks")
                      << " instead of " << e.linearUnsorted.endChecks << "\n"
                      << "    (" << percentFewer(e.sentinelUnsorted.endChecks, e.linearUnsorted.endChecks) << " fewer), and "
                      << e.sentinelSorted.endChecks << " instead of " << e.linearSorted.endChecks << " on sorted data.\n";
        } else {
            std::cout << "  * Sentinel vs linear: the sentinel search saved no end checks for this query, because every record it\n"
                      << "    visited was a match.\n";
        }

        // Whether the saved end checks also saved time is reported as measured
        const std::string times = TablePrinter::formatDecimal(e.sentinelUnsorted.elapsedMicros, 4) + " us against "
                                + TablePrinter::formatDecimal(e.linearUnsorted.elapsedMicros, 4) + " us on unsorted data";
        if (e.sentinelUnsorted.elapsedMicros < e.linearUnsorted.elapsedMicros) {
            std::cout << "    It was also faster: " << times << ".\n";
        } else {
            std::cout << "    It was not faster, though: " << times << ". On a list this small,\n"
                      << "    creating and freeing the sentinel node costs more than the end checks it saves.\n";
        }
    }

    // ---- Memory overhead table ----

    std::string memoryRow(const std::string& algorithm, const std::string& data, const std::string& sortedCopy,
                          const std::string& sentinel, const std::string& workingVariables, const std::string& total) {
        return cell(algorithm, 22, false) + cell(data, 8, false) + cell(sortedCopy, 15, true)
             + cell(sentinel, 12, true) + cell(workingVariables, 16, true) + cell(total, 18, true) + "|";
    }

    // sortedCopyBytes is the sorted copy the search needs on top of the dataset (0 for a search on unsorted data)
    std::string memoryRow(SearchAlgorithm algorithm, const std::string& data, size_t sortedCopyBytes,
                          const SearchStats& stats) {
        return memoryRow(PatientSearcher::algorithmName(algorithm), data, std::to_string(sortedCopyBytes),
                         std::to_string(stats.sentinelBytes), std::to_string(stats.auxBytes),
                         std::to_string(sortedCopyBytes + stats.sentinelBytes + stats.auxBytes));
    }

    void printMemoryTable(const Experiment& e) {
        const std::string header = memoryRow("Algorithm", "Data", "Sorted Copy (B)", "Sentinel (B)",
                                             "Working Vars (B)", "Total Overhead (B)");
        printTableHeader(header);
        std::cout << memoryRow(SearchAlgorithm::Linear, UNSORTED, 0, e.linearUnsorted) << "\n"
                  << memoryRow(SearchAlgorithm::SentinelLinear, UNSORTED, 0, e.sentinelUnsorted) << "\n"
                  << memoryRow(SearchAlgorithm::Linear, SORTED, e.sortedCopyBytes, e.linearSorted) << "\n"
                  << memoryRow(SearchAlgorithm::SentinelLinear, SORTED, e.sortedCopyBytes, e.sentinelSorted) << "\n";
        TablePrinter::printHorizontalLine(static_cast<int>(header.size()));

        const size_t nodeBytes = sizeof(PatientList::Node);
        const size_t pointerBytes = sizeof(PatientList::Node*);
        std::cout << "  * Overhead = memory needed on top of the dataset list, which is " << e.listBytes << " B: "
                  << e.recordCount << " nodes x " << nodeBytes << " B\n"
                  << "    (" << (nodeBytes - pointerBytes) << " B patient record + " << pointerBytes
                  << " B next pointer) + " << sizeof(PatientList) << " B list header. The next pointers alone take "
                  << (e.recordCount * pointerBytes) << " B,\n"
                  << "    a cost a contiguous array does not have.\n"
                  << "  * Searching sorted data needs a sorted copy of the list, merge sorted by " << e.sortOrder << ".\n"
                  << "  * Sentinel = the one temporary node the sentinel search links after the tail and frees when it is done.\n"
                  << "  * Working Vars is an estimate of the pointers and flags each search keeps.\n"
                  << "  * This experiment runs every search on its own copy of the dataset, so the loaded lists are never\n"
                  << "    modified, not even by the sentinel node. A search run directly on a list needs no such copy, so it is\n"
                  << "    not counted above. Neither is the result list (" << e.matches.size() << " matches, "
                  << e.matches.getMemoryUsageBytes() << " B), which is the same for every search.\n";
    }

    // ---- Complexity table ----

    std::string complexityRow(const std::string& algorithm, const std::string& comparisons, const std::string& endChecks,
                              const std::string& linkedListNeeds, const std::string& arrayNeeds) {
        return cell(algorithm, 22, false) + cell(comparisons, 11, false) + cell(endChecks, 12, false)
             + cell(linkedListNeeds, 17, false) + cell(arrayNeeds, 20, false) + "|";
    }

    void printComplexityTable() {
        const std::string header = complexityRow("Algorithm", "Comparisons", "End Checks",
                                                 "Linked List Needs", "Array Needs (theory)");
        printTableHeader(header);
        std::cout << complexityRow(PatientSearcher::algorithmName(SearchAlgorithm::Linear), "O(n)", "1 per record",
                                   "Nothing extra", "Nothing extra") << "\n"
                  << complexityRow(PatientSearcher::algorithmName(SearchAlgorithm::SentinelLinear), "O(n)", "1 per match",
                                   "1 temporary node", "1 spare slot") << "\n";
        TablePrinter::printHorizontalLine(static_cast<int>(header.size()));

        std::cout << "  * n = records in the list, k = matching records. On sorted data both algorithms stop when the group of\n"
                  << "    matches ends: k + 1 comparisons at best (matches at the head), n at worst (at the tail, or none).\n"
                  << "  * Both algorithms make the same comparisons on a linked list as on an array. An array is usually faster\n"
                  << "    per step, because its records sit side by side in memory, while linked list nodes are separate heap\n"
                  << "    blocks reached through pointers.\n"
                  << "  * On a linked list the sentinel is a node linked after the tail, which the tail pointer makes O(1).\n"
                  << "    On an array it is written into a spare slot after the last element.\n"
                  << "  * The array column is theory; its measured figures come from the array program.\n";
    }

} // namespace

void SearchingAnalysis::printSearchReport(const PatientList& patients, const std::string& datasetTitle,
                                          const SearchCriteria& criteria, bool sortedAscending) {
    TablePrinter::printBanner("TASK 7 - SEARCHING EXPERIMENT - " + datasetTitle);
    if (patients.isEmpty()) {
        std::cout << "  [Notice] No records to search.\n";
        return;
    }

    Experiment experiment = runExperiment(patients, criteria, sortedAscending);

    std::cout << "Query         : " << criteria.label << "\n"
              << "Algorithms    : " << PatientSearcher::algorithmName(SearchAlgorithm::Linear) << ", "
              << PatientSearcher::algorithmName(SearchAlgorithm::SentinelLinear) << "\n"
              << "Unsorted data : the dataset in the order it was loaded\n"
              << "Sorted data   : a merge-sorted copy of the dataset, ordered by " << experiment.sortOrder << "\n";

    TablePrinter::displayPatientTable(experiment.matches, "SEARCH RESULTS - UNSORTED DATA (in loaded order)", 10);
    TablePrinter::displayPatientTable(experiment.sortedMatches,
                                      "SEARCH RESULTS - SORTED DATA, ordered by " + experiment.sortOrder, 10);
    printMatchSummary(experiment);
    std::cout << "  * Both tables hold the same matching records; only their order differs. Records that tie on the\n"
              << "    sort field keep their loaded order, because merge sort is stable.\n";

    TablePrinter::printSectionTitle("PERFORMANCE - LINEAR vs SENTINEL LINEAR SEARCH ON UNSORTED AND SORTED DATA (measured)");
    printPerformanceTable(experiment);

    TablePrinter::printSectionTitle("MEMORY OVERHEAD");
    printMemoryTable(experiment);

    TablePrinter::printSectionTitle("TIME COMPLEXITY - LINEAR vs SENTINEL LINEAR SEARCH");
    printComplexityTable();
}
