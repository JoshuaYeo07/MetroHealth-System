#ifndef FACILITY_HPP
#define FACILITY_HPP

#include "PatientList.hpp"
#include <string>

/**
 * @struct Facility
 * @brief One MetroHealth dataset: facility labels plus its patients stored in a singly linked list.
 *
 * Facilities are themselves chained through `next`, so every report can traverse all
 * datasets as a singly linked list instead of indexing an array.
 */
struct Facility {
    std::string datasetLabel;   // e.g. "Dataset 1"
    std::string shortName;      // e.g. "Facility A"
    std::string name;           // e.g. "Facility A (General Hospital Hub)"
    std::string fileName;       // CSV file under Datasets/
    PatientList patients;
    int loadResult;             // Records loaded, or -1 if the file could not be opened
    Facility* next;

    Facility(const std::string& dataset, const std::string& shortLabel,
             const std::string& fullName, const std::string& csvFile)
        : datasetLabel(dataset), shortName(shortLabel), name(fullName), fileName(csvFile),
          loadResult(0), next(nullptr) {}

    // Title used on report headings, e.g. "Dataset 1 - Facility A (General Hospital Hub)"
    std::string title() const {
        return datasetLabel + " - " + name;
    }
};

#endif // FACILITY_HPP
