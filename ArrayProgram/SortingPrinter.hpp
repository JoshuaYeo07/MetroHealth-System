#ifndef SORTING_PRINTER_HPP
#define SORTING_PRINTER_HPP

#include "PatientArray.hpp"
#include "Sorting.hpp"
#include <string>

/**
 * @class SortingPrinter
 * @brief Utilities to display sorting performance comparison tables and sorted records previews.
 */
class SortingPrinter {
public:
    /**
     * @brief Displays the comprehensive benchmark table comparing Bubble Sort vs Merge Sort across criteria.
     * @param pairs Array of BenchmarkPair results
     * @param pairCount Number of criteria benchmarked
     * @param datasetName Name of the facility or dataset
     * @param recordCount Number of records in the dataset
     */
    static void printBenchmarkReport(const BenchmarkPair pairs[], size_t pairCount,
                                     const std::string& datasetName, size_t recordCount);

    /**
     * @brief Displays sample preview of top sorted records to visually verify sorting order.
     */
    static void printSortedPreview(const PatientArray& arr, const std::string& title,
                                   SortCriterion criterion, size_t sampleCount = 5);

    /**
     * @brief Prints technical and theoretical performance analysis for documentation / video presentation.
     */
    static void printAlgorithmicObservations();
};

#endif // SORTING_PRINTER_HPP
