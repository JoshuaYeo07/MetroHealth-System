#ifndef SORTING_ANALYSIS_HPP
#define SORTING_ANALYSIS_HPP

#include "PatientList.hpp"
#include "PatientSorter.hpp"
#include <string>

/**
 * @class SortingAnalysis
 * @brief Sorts a dataset with bubble sort and merge sort and prints text tables comparing the two:
 * the sorted records, measured time / comparisons / moves, memory usage and time complexity.
 */
class SortingAnalysis {
public:
    /**
     * @brief Sorts one dataset by one field with both algorithms and prints the sorted sample,
     * the performance table, the memory table and the complexity table.
     * @param patients Dataset to sort (left unchanged; each algorithm sorts its own copy)
     * @param datasetTitle Heading shown on the report
     * @param field Age, Visit Duration or Total Medical Cost
     * @param ascending true for lowest first, false for highest first
     */
    static void printSortingReport(const PatientList& patients, const std::string& datasetTitle,
                                   SortField field, bool ascending);

    /**
     * @brief Runs the comparison for all four fields on one dataset and prints a performance and
     * memory table for each, followed by the complexity table.
     */
    static void printAllFieldsReport(const PatientList& patients, const std::string& datasetTitle, bool ascending);
};

#endif