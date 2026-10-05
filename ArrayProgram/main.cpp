#include "Analysis.hpp"
#include "AnalysisPrinter.hpp"
#include "DataLoader.hpp"
#include "Patient.hpp"
#include "PatientArray.hpp"
#include "Searching.hpp"
#include "SearchingPrinter.hpp"
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

// Helper to benchmark Bubble Sort vs Merge Sort across all 4 criteria on a dataset
static void runFacilitySortingBenchmark(const PatientArray &dataset, const std::string &facilityName) {
    BenchmarkPair pairs[4];
    PatientArray sortedByAge;
    PatientArray sortedByStay;
    PatientArray sortedByCost;
    PatientArray sortedByCareType;

    std::cout << "\nExecuting Sorting Benchmarks on " << facilityName << "...\n";

    pairs[0] = SortingEngine::runComparison(dataset, SortCriterion::Age, &sortedByAge);
    pairs[1] = SortingEngine::runComparison(dataset, SortCriterion::LengthOfStay, &sortedByStay);
    pairs[2] = SortingEngine::runComparison(dataset, SortCriterion::TotalCost, &sortedByCost);
    pairs[3] = SortingEngine::runComparison(dataset, SortCriterion::CareType, &sortedByCareType);

    // 1. Comparison table
    SortingPrinter::printBenchmarkReport(pairs, 4, facilityName, dataset.size());

    // 2. Previews confirming sorted order
    SortingPrinter::printSortedPreview(sortedByAge, "MetroHealth System", SortCriterion::Age, 5);
    SortingPrinter::printSortedPreview(sortedByStay, "MetroHealth System", SortCriterion::LengthOfStay, 5);
    SortingPrinter::printSortedPreview(sortedByCost, "MetroHealth System", SortCriterion::TotalCost, 5);
    SortingPrinter::printSortedPreview(sortedByCareType, "MetroHealth System", SortCriterion::CareType, 5);

    // 3. Technical observations and complexity analysis
    SortingPrinter::printAlgorithmicObservations();
}

// Reads one console line with surrounding whitespace removed; returns false on EOF
static bool readTrimmedLine(std::string &line) {
    if (!std::getline(std::cin, line)) {
        return false;
    }
    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        line.clear();
    } else {
        line = line.substr(start, line.find_last_not_of(" \t\r\n") - start + 1);
    }
    return true;
}

// Parses a whole number of up to 6 digits (rejects signs, letters and decimals)
static bool parseNonNegativeInt(const std::string &text, int &value) {
    if (text.empty() || text.size() > 6) {
        return false;
    }
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }
    value = std::stoi(text);
    return true;
}

// Runs Linear vs Sentinel Linear Search on unsorted and sorted data, then prints the report,
// the matching records and the performance analysis
static void runSearchExperiment(const SearchQuery &query, PatientArray &unsorted,
                                PatientArray &sortedByAge, PatientArray &sortedByStay) {
    // Stay queries use the array sorted by visit duration; age and care type queries use the age-sorted array
    bool useStaySort = (query.field == SearchField::StayAbove);
    PatientArray &sorted = useStaySort ? sortedByStay : sortedByAge;
    std::string sortedLabel = useStaySort ? "Sorted (Stay)" : "Sorted (Age)";

    PatientArray matches;
    SearchComparison result = SearchEngine::runComparison(unsorted, sorted, sortedLabel, query, &matches);

    SearchingPrinter::printComparisonReport(result, "MetroHealth System (All Facilities Combined)", unsorted.size());
    SearchingPrinter::printMatches(matches, query);
    SearchingPrinter::printObservations(result, unsorted.size());
}

// Sub-Menu for Task 7: Searching Experiments
static void handleTask7SubMenu(PatientArray &allPatients, PatientArray &sortedByAge, PatientArray &sortedByStay) {
    while (true) {
        std::cout << "\n====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "SEARCHING EXPERIMENTS - LINEAR vs SENTINEL LINEAR SEARCH (TASK 7)" << Color::RESET << "\n";
        std::cout << "====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 1 ]" << Color::RESET << ": Search by Age Group\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 2 ]" << Color::RESET << ": Search by Care Type\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 3 ]" << Color::RESET << ": Search by Visit Duration Threshold\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 0 ]" << Color::RESET << ": Return to Main Menu\n";
        std::cout << "====================================================================================\n";
        std::cout << "Enter your choice: ";

        std::string choice;
        if (!readTrimmedLine(choice)) {
            break;
        }
        if (choice.empty()) {
            continue;
        }

        if (choice == "1") {
            std::cout << "\nSelect an age group:\n";
            for (int ag = 0; ag < NUM_AGE_GROUPS; ++ag) {
                std::cout << "  " << Color::BOLD_GREEN << "[ " << ag + 1 << " ]" << Color::RESET << ": "
                          << AGE_GROUP_NAMES[ag] << "\n";
            }
            std::cout << "Enter your choice (1-" << NUM_AGE_GROUPS << "): ";

            std::string input;
            if (!readTrimmedLine(input)) {
                break;
            }
            if (input.size() == 1 && input[0] >= '1' && input[0] < '1' + NUM_AGE_GROUPS) {
                runSearchExperiment(SearchQuery::byAgeGroup(input[0] - '1'), allPatients, sortedByAge, sortedByStay);
            } else {
                std::cout << "\n[Error] Invalid age group '" << input << "'.\n";
            }
            pauseForUser("Press Enter to return to Sub-Menu...");
        } else if (choice == "2") {
            std::cout << "\nSelect one or more care types:\n";
            for (int c = 0; c < NUM_CARE_TYPES; ++c) {
                std::cout << "  " << Color::BOLD_GREEN << "[ " << c + 1 << " ]" << Color::RESET << ": "
                          << CARE_TYPE_NAMES[c] << "\n";
            }
            std::cout << "Enter the numbers separated by spaces (e.g. 1 5 for Emergency or Rehabilitation): ";

            std::string input;
            if (!readTrimmedLine(input)) {
                break;
            }
            bool selected[NUM_CARE_TYPES] = {};
            bool valid = true;
            bool anySelected = false;
            for (char ch : input) {
                if (ch >= '1' && ch < '1' + NUM_CARE_TYPES) {
                    selected[ch - '1'] = true;
                    anySelected = true;
                } else if (ch != ' ' && ch != ',') {
                    valid = false;
                }
            }

            if (valid && anySelected) {
                runSearchExperiment(SearchQuery::byCareTypes(selected), allPatients, sortedByAge, sortedByStay);
            } else {
                std::cout << "\n[Error] Invalid care type selection '" << input << "'.\n";
            }
            pauseForUser("Press Enter to return to Sub-Menu...");
        } else if (choice == "3") {
            std::cout << "\nFind patients whose visit duration is MORE than how many hours? (press Enter for 24): ";

            std::string input;
            if (!readTrimmedLine(input)) {
                break;
            }
            int hours = 24;
            if (input.empty() || parseNonNegativeInt(input, hours)) {
                runSearchExperiment(SearchQuery::byStayAbove(hours), allPatients, sortedByAge, sortedByStay);
            } else {
                std::cout << "\n[Error] Invalid number of hours '" << input << "'.\n";
            }
            pauseForUser("Press Enter to return to Sub-Menu...");
        } else if (choice == "0") {
            break;
        } else {
            std::cout << "\n[Error] Invalid choice '" << choice << "'. Please enter a number between 0 and 3.\n";
            pauseForUser("Press Enter to continue...");
        }
    }
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

    // 4. Pre-sorted copies for Task 7, so every search runs on both unsorted and sorted data
    PatientArray sortedByAge = allPatients;
    PatientArray sortedByStay = allPatients;
    SortingEngine::mergeSort(sortedByAge, SortCriterion::Age);
    SortingEngine::mergeSort(sortedByStay, SortCriterion::LengthOfStay);

    // 5. Interactive Menu Loop
    while (true) {
        std::cout << "\n====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "METROHEALTH SYSTEM - SMART HEALTHCARE & PATIENT MANAGEMENT" << Color::RESET << "\n";
        std::cout << "====================================================================================\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 1 ]" << Color::RESET << ": Program Implementation (Dataset Ingestion & Sample Previews)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 2 ]" << Color::RESET << ": Patient Demographic & Billing Categorization (Age Group Reports)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 3 ]" << Color::RESET << ": Healthcare Expenditure & Service Analysis (Care Types & Facility Comparisons)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 4 ]" << Color::RESET << ": Sorting Experiments (Bubble Sort & Merge Sort)\n";
        std::cout << "  " << Color::BOLD_GREEN << "[ 5 ]" << Color::RESET << ": Searching Experiments (Linear & Sentinel Linear Search)\n";
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
            runFacilitySortingBenchmark(allPatients, "All Facilities Combined");
            pauseForUser();
        } else if (input == "5") {
            // Task 7: Searching Experiments (Sub-Menu)
            handleTask7SubMenu(allPatients, sortedByAge, sortedByStay);
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
