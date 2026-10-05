#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "Patient.hpp"
#include "PatientList.hpp"
#include "Facility.hpp"
#include "DataLoader.hpp"
#include "TablePrinter.hpp"
#include "CareTypeStats.hpp"
#include "HealthcareAnalytics.hpp"
#include "SortingAnalysis.hpp"
#include "SearchingAnalysis.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// ANSI colours: light green for the logo, red for the exit/back menu options; reset back to the default console colour afterwards
static const char* const LIGHT_GREEN = "\033[92m";
static const char* const RED = "\033[91m";
static const char* const COLOR_RESET = "\033[0m";

// "MetroHealth" logo, generated with pyfiglet's "standard" font and stored as plain ASCII
static void printLogo() {
#ifdef _WIN32
    // Windows consoles only render ANSI colour codes once virtual terminal processing is enabled
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (console != INVALID_HANDLE_VALUE && GetConsoleMode(console, &mode)) {
        SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
    std::cout << LIGHT_GREEN << R"LOGO(
 __  __      _             _   _            _ _   _
|  \/  | ___| |_ _ __ ___ | | | | ___  __ _| | |_| |__
| |\/| |/ _ \ __| '__/ _ \| |_| |/ _ \/ _` | | __| '_ \
| |  | |  __/ |_| | | (_) |  _  |  __/ (_| | | |_| | | |
|_|  |_|\___|\__|_|  \___/|_| |_|\___|\__,_|_|\__|_| |_|

)LOGO" << COLOR_RESET;
}

// Task 3: how each dataset is stored in its singly linked list, followed by sample records
static void printStorageSummary(const Facility* facilities) {
    TablePrinter::printBanner("TASK 3 - DATASET STORAGE SUMMARY (CUSTOM SINGLY LINKED LISTS)");

    auto printRow = [](const std::string& name, const std::string& status, const std::string& records,
        const std::string& nodeSize, const std::string& memory, const std::string& chain) {
        std::cout << "| " << std::left  << std::setw(24) << name
                << "| " << std::setw(8) << status
                  << "| " << std::right << std::setw(7)  << records
                << " | " << std::setw(12) << nodeSize
                << " | " << std::setw(16) << memory
                  << " | " << std::left  << std::setw(17) << chain << " |\n";
        };

    TablePrinter::printHorizontalLine(101);
    printRow("Facility / Dataset", "Status", "Records", "Node Size", "Memory Footprint", "Head -> Tail");
    TablePrinter::printHorizontalLine(101);
    for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
        const PatientList& list = facility->patients;
        std::string chain = list.isEmpty()
            ? "(empty)"
            : list.getHead()->data.patientId + " -> " + list.getTail()->data.patientId;
        printRow(facility->datasetLabel + " - " + facility->shortName,
            facility->loadResult > 0 ? "SUCCESS" : "FAILED",
            std::to_string(list.size()),
            std::to_string(sizeof(PatientList::Node)) + " B",
            std::to_string(list.getMemoryUsageBytes()) + " B",
            chain);
    }
    TablePrinter::printHorizontalLine(101);
    std::cout << "  * Each node = one Patient record + one 'next' pointer. Nodes are allocated individually on the heap,\n"
        << "    appended in O(1) through the tail pointer, and reached only by traversing from the head.\n";

    for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
        TablePrinter::displayPatientTable(facility->patients, facility->title() + " - Sample Records", 5);
    }
}

// Reads a whole number in [minChoice, maxChoice]; returns -1 when input ends
static int readChoice(int minChoice, int maxChoice, const std::string& prompt = "Enter choice") {
    std::string line;
    while (true) {
        std::cout << prompt << " (" << minChoice << "-" << maxChoice << "): ";
        if (!std::getline(std::cin, line)) {
            return -1;
        }
        std::istringstream in(line);
        int choice;
        char extra;
        if (in >> choice && !(in >> extra) && choice >= minChoice && choice <= maxChoice) {
            return choice;
        }
        std::cout << "  [Invalid] Please enter a number from " << minChoice << " to " << maxChoice << ".\n";
    }
}

// Dataset picker: each facility in the chain, then all datasets combined.
// Returns the chosen list (and its heading in titleOut), or nullptr when the user goes back.
static const PatientList* chooseDataset(const Facility* facilities, const PatientList& allPatients,
    const std::string& heading, std::string& titleOut) {
    std::cout << "\n  " << heading << "\n";
    int option = 0;
    for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
        std::cout << "    [" << ++option << "] " << facility->title() << "\n";
    }
    int combinedOption = option + 1;
    std::cout << "    [" << combinedOption << "] All Datasets Combined\n";
    std::cout << "    " << RED << "[0] Back to main menu" << COLOR_RESET << "\n";

    int choice = readChoice(0, combinedOption);
    if (choice <= 0) {
        return nullptr;
    }
    if (choice == combinedOption) {
        titleOut = "All Datasets Combined";
        return &allPatients;
    }

    const Facility* selected = facilities;
    for (int i = 1; i < choice; ++i) {
        selected = selected->next;
    }
    titleOut = selected->title();
    return &selected->patients;
}

// Task 4 dataset picker
static void runAgeGroupMenu(const Facility* facilities, const PatientList& allPatients) {
    std::string title;
    const PatientList* selected = chooseDataset(facilities, allPatients, "Select dataset for age group categorization:", title);
    if (selected != nullptr) {
        HealthcareAnalytics::printAgeGroupReport(*selected, title);
    }
}

// Sorting: pick a dataset, the field to sort by and the order, then compare bubble sort with merge sort
static void runSortingMenu(const Facility* facilities, const PatientList& allPatients) {
    std::string title;
    const PatientList* dataset = chooseDataset(facilities, allPatients, "Select dataset to sort:", title);
    if (dataset == nullptr) {
        return;
    }

    std::cout << "\n  Sort by:\n";
    std::cout << "    [1] Age\n";
    std::cout << "    [2] Visit Duration (Length of Stay)\n";
    std::cout << "    [3] Total Medical Cost\n";
    std::cout << "    [4] Care Type (alphabetical)\n";
    std::cout << "    [5] All fields (comparison)\n";
    std::cout << "    " << RED << "[0] Back to main menu" << COLOR_RESET << "\n";
    int fieldChoice = readChoice(0, 5);
    if (fieldChoice <= 0) {
        return;
    }

    std::cout << "\n  Order:\n";
    std::cout << "    [1] Ascending (lowest first)\n";
    std::cout << "    [2] Descending (highest first, Z-A for care type)\n";
    int orderChoice = readChoice(1, 2);
    if (orderChoice <= 0) {
        return;
    }
    bool ascending = (orderChoice == 1);

    if (fieldChoice == 5) {
        SortingAnalysis::printAllFieldsReport(*dataset, title, ascending);
        return;
    }
    SortField field = SortField::Age;
    if (fieldChoice == 2) {
        field = SortField::VisitDuration;
    } else if (fieldChoice == 3) {
        field = SortField::TotalCost;
    } else if (fieldChoice == 4) {
        field = SortField::CareType;
    }
    SortingAnalysis::printSortingReport(*dataset, title, field, ascending);
}

// Longest visit duration threshold the search menu accepts, in hours
static const int MAX_STAY_THRESHOLD_HOURS = 1000;

// Highest total medical cost threshold the search menu accepts, in MYR
static const int MAX_COST_THRESHOLD_MYR = 1000000;

// Asks how the sorted copy used by a search should be ordered. The order decides whether the matching
// records end up near the head or near the tail, and so how early the search on sorted data can stop.
// Returns 1 for ascending, 2 for descending, or -1 when input ends.
static int chooseSortedOrder() {
    std::cout << "\n  Order of the sorted data:\n";
    std::cout << "    [1] Ascending (youngest, A-Z, shortest visit or lowest cost first)\n";
    std::cout << "    [2] Descending (oldest, Z-A, longest visit or highest cost first)\n";
    return readChoice(1, 2);
}

// Task 7: pick a dataset, what to search for and how the sorted data is ordered, then compare
// linear search with sentinel linear search on the unsorted data and on sorted data
static void runSearchingMenu(const Facility* facilities, const PatientList& allPatients) {
    std::string title;
    const PatientList* dataset = chooseDataset(facilities, allPatients, "Select dataset to search:", title);
    if (dataset == nullptr) {
        return;
    }

    std::cout << "\n  Search by:\n";
    std::cout << "    [1] Age Group\n";
    std::cout << "    [2] Care Type\n";
    std::cout << "    [3] Visit Duration Threshold (visits longer than a number of hours)\n";
    std::cout << "    [4] Total Medical Cost Threshold (total cost above an amount in MYR)\n";
    std::cout << "    " << RED << "[0] Back to main menu" << COLOR_RESET << "\n";
    int searchChoice = readChoice(0, 4);
    if (searchChoice <= 0) {
        return;
    }

    SearchCriteria criteria;
    if (searchChoice == 1) {
        std::cout << "\n  Age group:\n";
        for (int g = 0; g < AGE_GROUP_COUNT; ++g) {
            std::cout << "    [" << (g + 1) << "] " << ageGroupName(static_cast<AgeGroup>(g)) << "\n";
        }
        int groupChoice = readChoice(1, AGE_GROUP_COUNT);
        if (groupChoice <= 0) {
            return;
        }
        criteria = SearchCriteria::forAgeGroup(static_cast<AgeGroup>(groupChoice - 1));
    } else if (searchChoice == 2) {
        // Offer the care types that actually occur in the chosen dataset
        CareTypeStatList careTypes(*dataset);
        if (careTypes.size() == 0) {
            std::cout << "  [Notice] No records to search.\n";
            return;
        }
        std::cout << "\n  Care type:\n";
        int option = 0;
        for (const CareTypeStatList::Node* node = careTypes.getHead(); node != nullptr; node = node->next) {
            std::cout << "    [" << ++option << "] " << node->careType
                      << " (" << node->summary.patientCount << " patients)\n";
        }
        int careChoice = readChoice(1, option);
        if (careChoice <= 0) {
            return;
        }
        const CareTypeStatList::Node* selected = careTypes.getHead();
        for (int i = 1; i < careChoice; ++i) {
            selected = selected->next;
        }
        criteria = SearchCriteria::forCareType(selected->careType);
    } else if (searchChoice == 3) {
        std::cout << "\n";
        int hours = readChoice(0, MAX_STAY_THRESHOLD_HOURS, "  Find visits longer than how many hours?");
        if (hours < 0) {
            return;
        }
        criteria = SearchCriteria::forStayLongerThan(hours);
    } else {
        std::cout << "\n";
        int amount = readChoice(0, MAX_COST_THRESHOLD_MYR, "  Find total medical costs above how many MYR?");
        if (amount < 0) {
            return;
        }
        criteria = SearchCriteria::forCostAbove(amount);
    }

    int orderChoice = chooseSortedOrder();
    if (orderChoice <= 0) {
        return;
    }
    SearchingAnalysis::printSearchReport(*dataset, title, criteria, orderChoice == 1);
}

// Runs every report that needs no input, in menu order. The sorting report, which normally asks what
// to sort by, uses all datasets combined with preset choices. The searching experiment is not
// included: it has to be told what to search for, so it only runs from its own menu option.
static void runAllReports(const Facility* facilities, const PatientList& allPatients) {
    const std::string combinedTitle = "All Datasets Combined";

    printStorageSummary(facilities);
    for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
        HealthcareAnalytics::printAgeGroupReport(facility->patients, facility->title());
    }
    HealthcareAnalytics::printAgeGroupReport(allPatients, combinedTitle);
    HealthcareAnalytics::printExpenditureReport(facilities, allPatients);

    // Sorting: bubble sort vs merge sort on all three fields, lowest first
    SortingAnalysis::printAllFieldsReport(allPatients, combinedTitle, true);
}

int main() {
    printLogo();
    std::cout << "====================================================================================\n";
    std::cout << "  METROHEALTH SYSTEM - SMART HEALTHCARE & PATIENT MANAGEMENT (LINKED LIST PROTOTYPE)\n";
    std::cout << "====================================================================================\n\n";

    // The three datasets are chained as a singly linked list: A -> B -> C
    Facility facilityA("Dataset 1", "Facility A", "Facility A (General Hospital Hub)", "dataset1 facility_a.csv");
    Facility facilityB("Dataset 2", "Facility B", "Facility B (University Medical Center)", "dataset2 facility_b.csv");
    Facility facilityC("Dataset 3", "Facility C", "Facility C (Community Health Clinic)", "dataset3_facility_c.csv");
    facilityA.next = &facilityB;
    facilityB.next = &facilityC;
    Facility* facilities = &facilityA;

    // Every patient from every dataset, for cross-facility analysis
    PatientList allPatients;

    std::cout << "[Step 1] Loading MetroHealth Datasets into Custom Singly Linked Lists...\n";
    for (Facility* facility = facilities; facility != nullptr; facility = facility->next) {
        facility->loadResult = DataLoader::loadCSV(facility->fileName, facility->patients);
        allPatients.appendAll(facility->patients);
        std::cout << "  " << std::left << std::setw(52) << facility->title()
            << (facility->loadResult >= 0 ? std::to_string(facility->loadResult) + " records loaded" : "FAILED")
            << "\n";
    }

    bool running = true;
    while (running) {
        std::cout << "\n------------------------------------------------------------------------------------\n";
        std::cout << "  MAIN MENU\n";
        std::cout << "------------------------------------------------------------------------------------\n";
        std::cout << "  [1] Dataset Storage Summary & Sample Records\n";
        std::cout << "  [2] Patient Demographic & Billing Categorization (by Age Group)\n";
        std::cout << "  [3] Healthcare Expenditure & Service Analysis\n";
        std::cout << "  [4] Sorting & Performance Analysis (Bubble Sort vs Merge Sort)\n";
        std::cout << "  [5] Searching Experiments (Linear Search vs Sentinel Linear Search)\n";
        std::cout << "  [6] Run All Reports\n";
        std::cout << "  " << RED << "[0] Exit" << COLOR_RESET << "\n";

        switch (readChoice(0, 6)) {
        case 1:
            printStorageSummary(facilities);
            break;
        case 2:
            runAgeGroupMenu(facilities, allPatients);
            break;
        case 3:
            HealthcareAnalytics::printExpenditureReport(facilities, allPatients);
            break;
        case 4:
            runSortingMenu(facilities, allPatients);
            break;
        case 5:
            runSearchingMenu(facilities, allPatients);
            break;
        case 6:
            runAllReports(facilities, allPatients);
            break;
        default:
            running = false;
            break;
        }
    }

    std::cout << "\n[System Status] MetroHealth linked list session ended. All nodes released.\n";
    return 0;
}