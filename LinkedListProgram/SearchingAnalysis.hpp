#ifndef SEARCHING_ANALYSIS_HPP
#define SEARCHING_ANALYSIS_HPP

#include "PatientList.hpp"
#include "PatientSearcher.hpp"
#include <string>

/**
 * @class SearchingAnalysis
 * @brief Task 7: runs one search query with linear search and sentinel linear search, on the
 * unsorted dataset and again on a sorted copy, then prints text tables comparing them: the matching
 * records, measured time, comparisons and end-of-list checks, memory overhead and time complexity.
 */
class SearchingAnalysis {
public:
    /**
     * @brief Runs one query and prints the matching records, the performance table, the memory
     * overhead table and the complexity table.
     * @param patients Dataset to search (left unchanged; the searches run on the experiment's own copies)
     * @param datasetTitle Heading shown on the report
     * @param criteria What to search for (age group, care type or visit duration threshold)
     * @param sortedAscending Order of the sorted copy: true for lowest first, false for highest first
     */
    static void printSearchReport(const PatientList& patients, const std::string& datasetTitle,
                                  const SearchCriteria& criteria, bool sortedAscending);
};

#endif
