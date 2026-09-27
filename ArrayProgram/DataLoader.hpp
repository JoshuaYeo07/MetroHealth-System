#ifndef DATA_LOADER_HPP
#define DATA_LOADER_HPP

#include "PatientArray.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

/**
 * @class DataLoader
 * @brief Parses CSV datasets for Facility A, B, and C into PatientArray instances.
 */
class DataLoader {
private:
    /**
     * @brief Trims leading and trailing whitespace and carriage returns (\r, \n).
     */
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    /**
     * @brief Resolves relative path dynamically across common execution directories.
     */
    static std::string resolvePath(const std::string& filename) {
        std::string candidates[] = {
            "../Datasets/" + filename,
            "Datasets/" + filename,
            "../../Datasets/" + filename,
            filename
        };
        for (const auto& path : candidates) {
            std::ifstream test(path);
            if (test.good()) {
                return path;
            }
        }
        return "../Datasets/" + filename; // Default fallback
    }

public:
    /**
     * @brief Reads a CSV dataset file and populates the given PatientArray.
     * @param filename Name or relative path of the CSV file
     * @param targetArray Reference to the PatientArray where records will be stored
     * @return Number of records successfully loaded, or -1 if file could not be opened
     */
    static int loadCSV(const std::string& filename, PatientArray& targetArray) {
        std::string resolvedPath = resolvePath(filename);
        std::ifstream file(resolvedPath);

        if (!file.is_open()) {
            std::cerr << "[Error] Unable to open dataset file at: " << resolvedPath << "\n";
            return -1;
        }

        std::string line;
        // Read and discard header row
        if (!std::getline(file, line)) {
            return 0;
        }

        int loadedCount = 0;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string id, ageStr, careType, stayStr, costStr, visitsStr;

            if (std::getline(ss, id, ',') &&
                std::getline(ss, ageStr, ',') &&
                std::getline(ss, careType, ',') &&
                std::getline(ss, stayStr, ',') &&
                std::getline(ss, costStr, ',') &&
                std::getline(ss, visitsStr, ',')) {

                try {
                    int age = std::stoi(trim(ageStr));
                    int lengthOfStay = std::stoi(trim(stayStr));
                    double baseCost = std::stod(trim(costStr));
                    int visits = std::stoi(trim(visitsStr));

                    Patient p(trim(id), age, trim(careType), lengthOfStay, baseCost, visits);
                    targetArray.push_back(p);
                    loadedCount++;
                } catch (const std::exception& e) {
                    std::cerr << "[Warning] Skipping malformed row: " << line << " (" << e.what() << ")\n";
                }
            }
        }

        file.close();
        return loadedCount;
    }

    /**
     * @brief Helper to load all three facility datasets into distinct arrays.
     */
    static bool loadAllFacilities(PatientArray& facilityA, PatientArray& facilityB, PatientArray& facilityC) {
        int countA = loadCSV("dataset1 facility_a.csv", facilityA);
        int countB = loadCSV("dataset2 facility_b.csv", facilityB);
        int countC = loadCSV("dataset3_facility_c.csv", facilityC);

        return (countA >= 0 && countB >= 0 && countC >= 0);
    }
};

#endif // DATA_LOADER_HPP
