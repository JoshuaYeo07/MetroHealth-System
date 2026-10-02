#ifndef ANALYSIS_PRINTER_HPP
#define ANALYSIS_PRINTER_HPP

#include "Analysis.hpp"
#include <string>


// Task 4 + 5d: per-age-group tables for one dataset
void printAgeGroupReport(const AnalysisResult &r,
                         const std::string &datasetName);

// Task 5b: total cost grouped by care type for one dataset
void printCareTypeSummary(const AnalysisResult &r,
                          const std::string &datasetName);

// Task 5a + 5c: compare the three datasets
void printDatasetComparison(const AnalysisResult &a, const AnalysisResult &b,
                            const AnalysisResult &c);

// Task 5c: compare age groups across the three datasets
void printAgeGroupComparison(const AnalysisResult &a, const AnalysisResult &b,
                             const AnalysisResult &c);

#endif // ANALYSIS_PRINTER_HPP