#include "Analysis.hpp"
#include "AnalysisPrinter.hpp"
#include "DataLoader.hpp"
#include "Patient.hpp"
#include "PatientArray.hpp"
#include "Sorting.hpp"
#include "SortingPrinter.hpp"
#include "TablePrinter.hpp"
#include <iomanip>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
// Enables ANSI escape code color support in Windows console
static void enableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
}
#endif

// ANSI Color Escape Sequences
namespace Color {
    const std::string RESET      = "\033[0m";
    const std::string BOLD_GREEN = "\033[1;32m";
}

// Helper to pause and let the user view the report before returning to the menu
static void pauseForUser(const std::string &prompt = "Press Enter to return to the Main Menu...") {
    std::cout << "\n" << prompt;
    std::string dummy;
    std::getline(std::cin, dummy);
}

// Sub-Menu for Task 4: Patient Demographic & Billing Categorization
static void handleTask4SubMenu(const AnalysisResult &resOverall,
                               const AnalysisResult &resA,
                               const AnalysisResult &resB,
                               const AnalysisResult &resC) {
    while (true) {
        std::cout << "\n====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "AGE GROUP BILLING ANALYSIS - DEMOGRAPHIC REPORTS (TASK 4)" << Color::RESET << "\n";
        std::cout << "====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 1 ]" << Color::RESET << ": All Facilities Combined\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 2 ]" << Color::RESET << ": Facility A (General Hospital - Adult & Elderly Focus)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 3 ]" << Color::RESET << ": Facility B (University Medical Center - Youth & Student Focus)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 4 ]" << Color::RESET << ": Facility C (Community Health Clinic - All Ages)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 0 ]" << Color::RESET << ": Return to Main Menu\n";
        std::cout << "====================================================================================\n";
        std::cout << "Enter your choice: ";

        std::string subInput;
        if (!std::getline(std::cin, subInput)) {
            break;
        }

        size_t start = subInput.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue;
        }
        subInput = subInput.substr(start, subInput.find_last_not_of(" \t\r\n") - start + 1);

        if (subInput == "1") {
            printAgeGroupReport(resOverall, "MetroHealth System (All Facilities Combined)");
            pauseForUser("Press Enter to return to Sub-Menu...");
        } else if (subInput == "2") {
            printAgeGroupReport(resA, "Facility A (General Hospital - Adult & Elderly Focus)");
            pauseForUser("Press Enter to return to Sub-Menu...");
        } else if (subInput == "3") {
            printAgeGroupReport(resB, "Facility B (University Medical Center - Youth & Student Focus)");
            pauseForUser("Press Enter to return to Sub-Menu...");
        } else if (subInput == "4") {
            printAgeGroupReport(resC, "Facility C (Community Health Clinic - All Ages)");
            pauseForUser("Press Enter to return to Sub-Menu...");
        } else if (subInput == "0") {
            break;
        } else {
            std::cout << "\n[Error] Invalid choice '" << subInput << "'. Please enter a number between 0 and 4.\n";
            pauseForUser("Press Enter to continue...");
        }
    }
}

// Helper to benchmark Bubble Sort vs Merge Sort across all 3 criteria on a dataset
static void runFacilitySortingBenchmark(const PatientArray &dataset, const std::string &facilityName) {
    BenchmarkPair pairs[3];
    PatientArray sortedByAge;
    PatientArray sortedByStay;
    PatientArray sortedByCost;

    std::cout << "\nExecuting Sorting Benchmarks on " << facilityName << "...\n";

    pairs[0] = SortingEngine::runComparison(dataset, SortCriterion::Age, &sortedByAge);
    pairs[1] = SortingEngine::runComparison(dataset, SortCriterion::LengthOfStay, &sortedByStay);
    pairs[2] = SortingEngine::runComparison(dataset, SortCriterion::TotalCost, &sortedByCost);

    // 1. Comparison table
    SortingPrinter::printBenchmarkReport(pairs, 3, facilityName, dataset.size());

    // 2. Previews confirming sorted order
    SortingPrinter::printSortedPreview(sortedByAge, facilityName, SortCriterion::Age, 5);
    SortingPrinter::printSortedPreview(sortedByStay, facilityName, SortCriterion::LengthOfStay, 5);
    SortingPrinter::printSortedPreview(sortedByCost, facilityName, SortCriterion::TotalCost, 5);

    // 3. Technical observations and complexity analysis
    SortingPrinter::printAlgorithmicObservations();
}


int main() {
#ifdef _WIN32
    enableVirtualTerminalProcessing();
#endif

    // 1. Instantiate custom dynamic arrays (zero STL containers)
    PatientArray facilityA;
    PatientArray facilityB;
    PatientArray facilityC;

    std::cout << "Loading MetroHealth Datasets into Custom Dynamic Arrays...\n";
    int countA = DataLoader::loadCSV("dataset1 facility_a.csv", facilityA);
    int countB = DataLoader::loadCSV("dataset2 facility_b.csv", facilityB);
    int countC = DataLoader::loadCSV("dataset3_facility_c.csv", facilityC);

    if (countA <= 0 || countB <= 0 || countC <= 0) {
        std::cerr << "[Warning] One or more datasets failed to load. Please check file paths.\n";
    }

    // 2. Pre-combine all 600 records across the network for system-wide sorting & searching
    PatientArray allPatients;
    for (size_t i = 0; i < facilityA.size(); ++i) allPatients.push_back(facilityA[i]);
    for (size_t i = 0; i < facilityB.size(); ++i) allPatients.push_back(facilityB[i]);
    for (size_t i = 0; i < facilityC.size(); ++i) allPatients.push_back(facilityC[i]);

    // 3. Pre-compute analysis grids (5x6 fixed 2D arrays, zero STL)
    AnalysisResult resA = analyze(facilityA);
    AnalysisResult resB = analyze(facilityB);
    AnalysisResult resC = analyze(facilityC);
    AnalysisResult resOverall = combine(combine(resA, resB), resC);

    // 3. Interactive Menu Loop
    while (true) {
        std::cout << "\n====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "METROHEALTH SYSTEM - SMART HEALTHCARE & PATIENT MANAGEMENT" << Color::RESET << "\n";
        std::cout << "====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 1 ]" << Color::RESET << ": Program Implementation (Dataset Ingestion & Sample Previews)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 2 ]" << Color::RESET << ": Patient Demographic & Billing Categorization (Age Group Reports)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 3 ]" << Color::RESET << ": Healthcare Expenditure & Service Analysis (Care Types & Facility Comparisons)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 4 ]" << Color::RESET << ": Sorting Experiments (Bubble Sort & Merge Sort)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 5 ]" << Color::RESET << ": Searching Experiments (Linear & Binary Search)   [Next Phase]\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 0 ]" << Color::RESET << ": Exit Program\n";
        std::cout << "====================================================================================\n";
        std::cout << "Enter your choice: ";

        std::string input;
        if (!std::getline(std::cin, input)) {
            break; // Handle EOF / stream close
        }

        // Trim leading and trailing whitespace
        size_t start = input.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue; // User just hit enter, show menu again
        }
        input = input.substr(start, input.find_last_not_of(" \t\r\n") - start + 1);

        if (input == "1") {
            // Task 3: Ingestion Summary & Sample Tables
            std::cout << "\n------------------------------------------------------------------------------------\n";
            std::cout << "  DATASET INGESTION SUMMARY (TASK 3)\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            std::cout << "| Facility / Dataset                        | Status  | Records | Capacity | Memory Footprint |\n";
            std::cout << "|-------------------------------------------|---------|---------|----------|------------------|\n";

            auto printStatusRow = [](const std::string &name, int count, const PatientArray &arr) {
                std::cout << "| " << std::left << std::setw(42) << name << "| "
                          << std::setw(8) << (count > 0 ? "SUCCESS" : "FAILED") << "| "
                          << std::right << std::setw(7) << arr.size() << " | " << std::right
                          << std::setw(8) << arr.getCapacity() << " | " << std::right
                          << std::setw(13) << arr.getMemoryUsageBytes() << " B |\n";
            };

            printStatusRow("Facility A (General Hospital Hub)", countA, facilityA);
            printStatusRow("Facility B (University Medical Center)", countB, facilityB);
            printStatusRow("Facility C (Community Health Clinic)", countC, facilityC);
            std::cout << "------------------------------------------------------------------------------------\n";

            if (facilityA.size() > 0) {
                TablePrinter::displayPatientTable(facilityA, "Facility A: General Hospital - Sample Records", 5);
            }
            if (facilityB.size() > 0) {
                TablePrinter::displayPatientTable(facilityB, "Facility B: University Medical Center - Sample Records", 5);
            }
            if (facilityC.size() > 0) {
                TablePrinter::displayPatientTable(facilityC, "Facility C: Community Health Clinic - Sample Records", 5);
            }

            pauseForUser();
        } else if (input == "2") {
            // Task 4: Demographic & Billing Categorization Reports (Sub-Menu)
            handleTask4SubMenu(resOverall, resA, resB, resC);
        } else if (input == "3") {
            // Task 5: Healthcare Expenditure & Service Analysis
            printCareTypeSummary(resA, "Facility A");
            printCareTypeSummary(resB, "Facility B");
            printCareTypeSummary(resC, "Facility C");

            printDatasetComparison(resA, resB, resC);
            printAgeGroupComparison(resA, resB, resC);

            pauseForUser();
        } else if (input == "4") {
            // Task 6: Sorting Experiments (All Facilities Combined - 600 Records)
            runFacilitySortingBenchmark(allPatients, "MetroHealth System (All Facilities Combined)");
            pauseForUser();
        } else if (input == "5") {
            // Task 7: Searching Experiments (Placeholder)
            std::cout << "\n====================================================================================\n";
            std::cout << "  TASK 7: SEARCHING EXPERIMENTS (NEXT PHASE)\n";
            std::cout << "====================================================================================\n";
            std::cout << "Planned Features:\n";
            std::cout << "  - Algorithms: Linear Search vs. Binary Search (Unsorted vs. Sorted)\n";
            std::cout << "  - Filter Queries: Age Group (e.g., 61-100), Specific Care Type, Visit Duration (>24 hrs)\n";
            std::cout << "  - Performance Metrics: Execution time (microseconds) & Search steps count\n";

            pauseForUser();
        } else if (input == "0") {
            std::cout << "\nExiting MetroHealth System. Thank you!\n";
            break;
        } else {
            std::cout << "\n[Error] Invalid choice '" << input << "'. Please enter a number between 0 and 5.\n";
            pauseForUser();
        }
    }

    return 0;
}
