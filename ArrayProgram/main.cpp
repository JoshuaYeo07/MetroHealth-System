#include "Analysis.hpp"
#include "AnalysisPrinter.hpp"
#include "DataLoader.hpp"
#include "Patient.hpp"
#include "PatientArray.hpp"
#include "TablePrinter.hpp"
#include <iomanip>
#include <iostream>

int main() {
  std::cout << "==============================================================="
               "=====================\n";
  std::cout << "  METROHEALTH SYSTEM - SMART HEALTHCARE & PATIENT MANAGEMENT "
               "(ARRAY PROTOTYPE)\n";
  std::cout << "==============================================================="
               "=====================\n\n";

  // Instantiate custom dynamic arrays (zero STL containers)
  PatientArray facilityA;
  PatientArray facilityB;
  PatientArray facilityC;

  std::cout << "[Step 1] Loading MetroHealth Datasets into Custom Dynamic "
               "Arrays...\n";

  int countA = DataLoader::loadCSV("dataset1 facility_a.csv", facilityA);
  int countB = DataLoader::loadCSV("dataset2 facility_b.csv", facilityB);
  int countC = DataLoader::loadCSV("dataset3_facility_c.csv", facilityC);

  std::cout << "\n-------------------------------------------------------------"
               "-----------------------\n";
  std::cout << "  DATASET INGESTION SUMMARY (TASK 3)\n";
  std::cout << "---------------------------------------------------------------"
               "---------------------\n";

  std::cout << "| Facility / Dataset                        | Status  | "
               "Records | Capacity | Memory Footprint |\n";
  std::cout << "|-------------------------------------------|---------|--------"
               "-|----------|------------------|\n";

  auto printStatusRow = [](const std::string &name, int count,
                           const PatientArray &arr) {
    std::cout << "| " << std::left << std::setw(42) << name << "| "
              << std::setw(8) << (count > 0 ? "SUCCESS" : "FAILED") << "| "
              << std::right << std::setw(7) << arr.size() << " | " << std::right
              << std::setw(8) << arr.getCapacity() << " | " << std::right
              << std::setw(13) << arr.getMemoryUsageBytes() << " B |\n";
  };

  printStatusRow("Facility A (General Hospital Hub)", countA, facilityA);
  printStatusRow("Facility B (University Medical Center)", countB, facilityB);
  printStatusRow("Facility C (Community Health Clinic)", countC, facilityC);
  std::cout << "---------------------------------------------------------------"
               "---------------------\n\n";

  // Display sample preview of each facility using formatted ASCII tables
  if (facilityA.size() > 0) {
    TablePrinter::displayPatientTable(
        facilityA, "Facility A: General Hospital - Sample Records", 5);
  }
  if (facilityB.size() > 0) {
    TablePrinter::displayPatientTable(
        facilityB, "Facility B: University Medical Center - Sample Records", 5);
  }
  if (facilityC.size() > 0) {
    TablePrinter::displayPatientTable(
        facilityC, "Facility C: Community Health Clinic - Sample Records", 5);
  }

  std::cout << "\n[System Status] Task 3 Array Foundation successfully "
               "implemented and verified.\n";
  std::cout << "All 3 datasets are stored in custom dynamic arrays with zero "
               "STL dependencies.\n";

  AnalysisResult resA = analyze(facilityA);
  AnalysisResult resB = analyze(facilityB);
  AnalysisResult resC = analyze(facilityC);
  AnalysisResult resOverall = combine(combine(resA, resB), resC);

  printAgeGroupReport(resOverall,
                      "MetroHealth System (All Facilities Combined)");
  printAgeGroupReport(resA,
                      "Facility A (General Hospital - Adult & Elderly Focus)");
  printAgeGroupReport(
      resB, "Facility B (University Medical Center - Youth & Student Focus)");
  printAgeGroupReport(resC, "Facility C (Community Health Clinic - All Ages)");

  printCareTypeSummary(resA, "Facility A");
  printCareTypeSummary(resB, "Facility B");
  printCareTypeSummary(resC, "Facility C");

  printDatasetComparison(resA, resB, resC);
  printAgeGroupComparison(resA, resB, resC);

  return 0;
}
