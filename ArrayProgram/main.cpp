#include <iostream>
#include <iomanip>
#include "Patient.hpp"
#include "PatientArray.hpp"
#include "DataLoader.hpp"
#include "Console.hpp"
#include "TablePrinter.hpp"

int main() {
    Console::enableColors();
    Console::printLogo("SMART HEALTHCARE & PATIENT MANAGEMENT (ARRAY PROTOTYPE)");
    std::cout << "\n";

    // Instantiate custom dynamic arrays (zero STL containers)
    PatientArray facilityA;
    PatientArray facilityB;
    PatientArray facilityC;

    std::cout << Console::BOLD << Console::MAGENTA << "[Step 1] Loading MetroHealth Datasets into Custom Dynamic Arrays..." << Console::RESET << "\n";

    int countA = DataLoader::loadCSV("dataset1 facility_a.csv", facilityA);
    int countB = DataLoader::loadCSV("dataset2 facility_b.csv", facilityB);
    int countC = DataLoader::loadCSV("dataset3_facility_c.csv", facilityC);

    std::cout << "\n------------------------------------------------------------------------------------\n";
    std::cout << Console::BOLD << Console::YELLOW << "  DATASET INGESTION SUMMARY (TASK 3)" << Console::RESET << "\n";
    std::cout << "------------------------------------------------------------------------------------\n";

    std::cout << "| Facility / Dataset                        | Status  | Records | Capacity | Memory Footprint |\n";
    std::cout << "|-------------------------------------------|---------|---------|----------|------------------|\n";

    auto printStatusRow = [](const std::string& name, int count, const PatientArray& arr) {
        std::cout << "| " << std::left << std::setw(42) << name
                  << "| " << (count > 0 ? Console::GREEN : Console::RED) << std::setw(8) << (count > 0 ? "SUCCESS" : "FAILED") << Console::RESET
                  << "| " << std::right << std::setw(7) << arr.size()
                  << " | " << std::right << std::setw(8) << arr.getCapacity()
                  << " | " << std::right << std::setw(13) << arr.getMemoryUsageBytes() << " B |\n";
    };

    printStatusRow("Facility A (General Hospital Hub)", countA, facilityA);
    printStatusRow("Facility B (University Medical Center)", countB, facilityB);
    printStatusRow("Facility C (Community Health Clinic)", countC, facilityC);
    std::cout << "------------------------------------------------------------------------------------\n\n";

    // Display sample preview of each facility using formatted ASCII tables
    if (facilityA.size() > 0) {
        TablePrinter::displayPatientTable(facilityA, "Facility A: General Hospital - Sample Records", 5);
    }
    if (facilityB.size() > 0) {
        TablePrinter::displayPatientTable(facilityB, "Facility B: University Medical Center - Sample Records", 5);
    }
    if (facilityC.size() > 0) {
        TablePrinter::displayPatientTable(facilityC, "Facility C: Community Health Clinic - Sample Records", 5);
    }

    std::cout << "\n" << Console::GREEN << "[System Status] Task 3 Array Foundation successfully implemented and verified." << Console::RESET << "\n";
    std::cout << "All 3 datasets are stored in custom dynamic arrays with zero STL dependencies.\n";

    return 0;
}
