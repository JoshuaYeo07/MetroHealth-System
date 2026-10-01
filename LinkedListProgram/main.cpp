#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "Patient.hpp"
#include "PatientList.hpp"
#include "Facility.hpp"
#include "DataLoader.hpp"
#include "TablePrinter.hpp"
#include "HealthcareAnalytics.hpp"

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

// Reads a menu number in [minChoice, maxChoice]; returns -1 when input ends
static int readChoice(int minChoice, int maxChoice) {
    std::string line;
    while (true) {
        std::cout << "Enter choice (" << minChoice << "-" << maxChoice << "): ";
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

// Task 4 dataset picker: each facility in the chain, then all datasets combined
static void runAgeGroupMenu(const Facility* facilities, const PatientList& allPatients) {
    std::cout << "\n  Select dataset for age group categorization:\n";
    int option = 0;
    for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
        std::cout << "    [" << ++option << "] " << facility->title() << "\n";
    }
    int combinedOption = option + 1;
    std::cout << "    [" << combinedOption << "] All Datasets Combined\n";
    std::cout << "    [0] Back to main menu\n";

    int choice = readChoice(0, combinedOption);
    if (choice <= 0) {
        return;
    }
    if (choice == combinedOption) {
        HealthcareAnalytics::printAgeGroupReport(allPatients, "All Datasets Combined");
        return;
    }

    const Facility* selected = facilities;
    for (int i = 1; i < choice; ++i) {
        selected = selected->next;
    }
    HealthcareAnalytics::printAgeGroupReport(selected->patients, selected->title());
}

static void runAllReports(const Facility* facilities, const PatientList& allPatients) {
    printStorageSummary(facilities);
    for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
        HealthcareAnalytics::printAgeGroupReport(facility->patients, facility->title());
    }
    HealthcareAnalytics::printAgeGroupReport(allPatients, "All Datasets Combined");
    HealthcareAnalytics::printExpenditureReport(facilities, allPatients);
}

int main() {
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
        std::cout << "  [1] Task 3 - Dataset Storage Summary & Sample Records\n";
        std::cout << "  [2] Task 4 - Patient Demographic & Billing Categorization (by Age Group)\n";
        std::cout << "  [3] Task 5 - Healthcare Expenditure & Service Analysis\n";
        std::cout << "  [4] Run All Reports (Tasks 3 - 5)\n";
        std::cout << "  [0] Exit\n";

        switch (readChoice(0, 4)) {
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
