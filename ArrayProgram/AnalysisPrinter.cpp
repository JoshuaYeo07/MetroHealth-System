#include "AnalysisPrinter.hpp"
#include <iomanip>
#include <iostream>


using std::cout;
using std::left;
using std::right;
using std::setw;

// Average that returns 0 instead of crashing when a group has no patients
static double safeAvg(double total, int n) { return n > 0 ? total / n : 0.0; }

void printAgeGroupReport(const AnalysisResult &r, const std::string &name) {
  cout << "\n" << std::string(84, '=') << "\n";
  cout << "  " << name << " - AGE GROUP BILLING ANALYSIS (TASK 4)\n";
  cout << std::string(84, '=') << "\n";
  cout << std::fixed << std::setprecision(2);

  for (int ag = 0; ag < NUM_AGE_GROUPS; ++ag) {
    CellStats t = ageGroupTotal(r, ag);
    cout << "\nAge Group: " << AGE_GROUP_NAMES[ag] << "\n";

    if (t.patientCount == 0) {
      cout << "  No patients in this age group.\n";
      continue;
    }

    cout << std::string(84, '-') << "\n";
    cout << left << setw(20) << "Care Type" << right << setw(15)
         << "Patient Count" << setw(20) << "Total Cost (MYR)" << setw(28)
         << "Avg Cost per Patient (MYR)" << "\n";
    cout << std::string(84, '-') << "\n";

    for (int c = 0; c < NUM_CARE_TYPES; ++c) {
      const CellStats &cell = r.grid[ag][c];
      if (cell.patientCount == 0)
        continue;
      cout << left << setw(20) << CARE_TYPE_NAMES[c] << right << setw(15)
           << cell.patientCount << setw(20) << cell.totalCost << setw(28)
           << safeAvg(cell.totalCost, cell.patientCount) << "\n";
    }

    cout << std::string(84, '-') << "\n";
    cout << "Most preferred care type          : "
         << CARE_TYPE_NAMES[mostPreferredCareType(r, ag)] << "\n";
    cout << "Total patients                    : " << t.patientCount << "\n";
    cout << "Total Billing for Age Group (MYR) : " << t.totalCost << "\n";
    cout << "Average Cost per Patient (MYR)    : "
         << safeAvg(t.totalCost, t.patientCount) << "\n";
  }
}

void printCareTypeSummary(const AnalysisResult &r, const std::string &name) {
  cout << "\n" << std::string(84, '=') << "\n";
  cout << "  " << name << " - TOTAL COST BY CARE TYPE (TASK 5b)\n";
  cout << std::string(84, '=') << "\n";
  cout << std::fixed << std::setprecision(2);
  cout << left << setw(20) << "Care Type" << right << setw(15)
       << "Patient Count" << setw(20) << "Total Cost (MYR)" << setw(22)
       << "Avg Stay (hrs)" << "\n";
  cout << std::string(84, '-') << "\n";

  for (int c = 0; c < NUM_CARE_TYPES; ++c) {
    CellStats t = careTypeTotal(r, c);
    if (t.patientCount == 0)
      continue;
    cout << left << setw(20) << CARE_TYPE_NAMES[c] << right << setw(15)
         << t.patientCount << setw(20) << t.totalCost << setw(22)
         << safeAvg(t.totalHours, t.patientCount) << "\n";
  }
  cout << std::string(84, '-') << "\n";
  cout << "Total Billing for Dataset (MYR): " << datasetTotal(r).totalCost
       << "\n";
}

void printDatasetComparison(const AnalysisResult &a, const AnalysisResult &b,
                            const AnalysisResult &c) {
  const AnalysisResult *res[3] = {&a, &b, &c};
  const char *names[3] = {"Facility A", "Facility B", "Facility C"};

  cout << "\n" << std::string(84, '=') << "\n";
  cout << "  DATASET COMPARISON (TASK 5a, 5c)\n";
  cout << std::string(84, '=') << "\n";
  cout << std::fixed << std::setprecision(2);
  cout << left << setw(14) << "Dataset" << right << setw(10) << "Patients"
       << setw(20) << "Total Cost (MYR)" << setw(22) << "Avg Cost/Patient"
       << setw(18) << "Avg Stay (hrs)" << "\n";
  cout << std::string(84, '-') << "\n";

  for (int i = 0; i < 3; ++i) {
    CellStats t = datasetTotal(*res[i]);
    cout << left << setw(14) << names[i] << right << setw(10) << t.patientCount
         << setw(20) << t.totalCost << setw(22)
         << safeAvg(t.totalCost, t.patientCount) << setw(18)
         << safeAvg(t.totalHours, t.patientCount) << "\n";
  }
  cout << std::string(84, '-') << "\n";
}

void printAgeGroupComparison(const AnalysisResult &a, const AnalysisResult &b,
                             const AnalysisResult &c) {
  const AnalysisResult *res[3] = {&a, &b, &c};
  const char *shortNames[NUM_AGE_GROUPS] = {"0-17", "18-25", "26-45", "46-60",
                                            "61-100"};

  cout << "\n" << std::string(84, '=') << "\n";
  cout << "  AGE GROUP COMPARISON ACROSS DATASETS (TASK 5c)\n";
  cout << std::string(84, '=') << "\n";
  cout << std::fixed << std::setprecision(2);
  cout << left << setw(12) << "Age Group" << right << setw(12) << "A Cost"
       << setw(12) << "B Cost" << setw(12) << "C Cost" << setw(12) << "A Stay"
       << setw(12) << "B Stay" << setw(12) << "C Stay" << "\n";
  cout << std::string(84, '-') << "\n";

  for (int ag = 0; ag < NUM_AGE_GROUPS; ++ag) {
    cout << left << setw(12) << shortNames[ag] << right;

    for (int i = 0; i < 3; ++i) { // total cost columns
      CellStats t = ageGroupTotal(*res[i], ag);
      if (t.patientCount == 0)
        cout << setw(12) << "-";
      else
        cout << setw(12) << t.totalCost;
    }
    for (int i = 0; i < 3; ++i) { // average stay columns
      CellStats t = ageGroupTotal(*res[i], ag);
      if (t.patientCount == 0)
        cout << setw(12) << "-";
      else
        cout << setw(12) << safeAvg(t.totalHours, t.patientCount);
    }
    cout << "\n";
  }
  cout << std::string(84, '-') << "\n";
  cout << "(Costs in MYR; Stay = average length of stay in hours; '-' = no "
          "patients)\n";
}