#ifndef DATA_LOADER_HPP
#define DATA_LOADER_HPP

#include "PatientList.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

/**
 * @class DataLoader
 * @brief Parses CSV datasets for Facility A, B, and C into PatientList instances.
 */
class DataLoader {
private:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n"); // find first char
        if (first == std::string::npos) return ""; // npos = not found
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    static bool fileExists(const std::string& path) {
        std::ifstream test(path);
        return test.good();
    }

    static std::string resolvePath(const std::string& filename) {
        if (fileExists("../Datasets/" + filename)) return "../Datasets/" + filename;
        if (fileExists("Datasets/" + filename)) return "Datasets/" + filename;
        if (fileExists("../../Datasets/" + filename)) return "../../Datasets/" + filename;
        if (fileExists(filename)) return filename;
        return "../Datasets/" + filename;
    }

public:
    /**
     * @brief Reads a CSV dataset file and appends each record as a node of the given PatientList.
     * @param filename Name or relative path of the CSV file
     * @param targetList Reference to the PatientList where records will be stored
     * @return Number of records successfully loaded, or -1 if file could not be opened
     */
    static int loadCSV(const std::string& filename, PatientList& targetList) {
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
                    targetList.push_back(p);
                    loadedCount++;
                } catch (const std::exception& e) {
                    std::cerr << "[Warning] Skipping malformed row: " << line << " (" << e.what() << ")\n";
                }
            }
        }

        file.close();
        return loadedCount;
    }
};

#endif