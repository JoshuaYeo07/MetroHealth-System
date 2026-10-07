#ifndef SEARCHING_PRINTER_HPP
#define SEARCHING_PRINTER_HPP

#include "PatientArray.hpp"
#include "Searching.hpp"
#include <string>

/**
 * @class SearchingPrinter
 * @brief Utilities to display searching performance tables and matching records (Task 7d).
 */
class SearchingPrinter {
public:
    /**
     * @brief Displays the four runs (2 algorithms x unsorted/sorted data) for one query.
     * @param datasetName Name of the facility or dataset
     * @param recordCount Number of records in the dataset
     */
    static void printComparisonReport(const SearchComparison& result, const std::string& datasetName,
                                      size_t recordCount);

    /**
     * @brief Displays the records returned by a search.
     * @param sortedLabel The sorted copy the records were taken from, e.g. "Sorted (Care Type)"
     * @param maxRows Maximum rows to display (0 for all rows)
     */
    static void printMatches(const PatientArray& matches, const SearchQuery& query,
                             const std::string& sortedLabel, size_t maxRows = 10);

    /**
     * @brief Prints the performance analysis, calculated from the measured results.
     */
    static void printObservations(const SearchComparison& result, size_t recordCount);
};

#endif // SEARCHING_PRINTER_HPP
