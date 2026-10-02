#include "Analysis.hpp"

const char *AGE_GROUP_NAMES[NUM_AGE_GROUPS] = {
    "0-17: Pediatrics & Adolescents",
    "18-25: Young Adults / University Students",
    "26-45: Working Adults (Early Career)",
    "46-60: Working Adults (Late Career)",
    "61-100: Senior Citizens / Geriatric Care"};

const char *CARE_TYPE_NAMES[NUM_CARE_TYPES] = {
    "Emergency",   "Outpatient",     "Inpatient",
    "Vaccination", "Rehabilitation", "Routine Checkup"};

int careTypeIndex(const std::string &t) {
  for (int i = 0; i < NUM_CARE_TYPES; ++i)
    if (t == CARE_TYPE_NAMES[i])
      return i;
  return -1;
}

// Adds ONE patient into the grid (row = age group, column = care type)
void accumulate(const Patient &p, AnalysisResult &r) {
  int a = static_cast<int>(p.getAgeGroup()); // enum order = grid row order
  int c = careTypeIndex(p.careType);
  if (a < 0 || a >= NUM_AGE_GROUPS || c < 0)
    return; // skips Unknown/bad data

  r.grid[a][c].patientCount += 1;
  r.grid[a][c].totalCost += p.calculateTotalCost();
  r.grid[a][c].totalHours += p.lengthOfStay;
}

AnalysisResult analyze(const PatientArray &arr) {
  AnalysisResult r;
  for (size_t i = 0; i < arr.size(); ++i)
    accumulate(arr[i], r);
  return r;
}

CellStats ageGroupTotal(const AnalysisResult &r, int ag) {
  CellStats t;
  for (int c = 0; c < NUM_CARE_TYPES; ++c) {
    t.patientCount += r.grid[ag][c].patientCount;
    t.totalCost += r.grid[ag][c].totalCost;
    t.totalHours += r.grid[ag][c].totalHours;
  }
  return t;
}

CellStats careTypeTotal(const AnalysisResult &r, int c) {
  CellStats t;
  for (int a = 0; a < NUM_AGE_GROUPS; ++a) {
    t.patientCount += r.grid[a][c].patientCount;
    t.totalCost += r.grid[a][c].totalCost;
    t.totalHours += r.grid[a][c].totalHours;
  }
  return t;
}

CellStats datasetTotal(const AnalysisResult &r) {
  CellStats t;
  for (int a = 0; a < NUM_AGE_GROUPS; ++a) {
    CellStats row = ageGroupTotal(r, a);
    t.patientCount += row.patientCount;
    t.totalCost += row.totalCost;
    t.totalHours += row.totalHours;
  }
  return t;
}

int mostPreferredCareType(const AnalysisResult &r, int ag) {
  int best = -1, bestCount = 0;
  for (int c = 0; c < NUM_CARE_TYPES; ++c)
    if (r.grid[ag][c].patientCount > bestCount) {
      bestCount = r.grid[ag][c].patientCount;
      best = c;
    }
  return best;
}

AnalysisResult combine(const AnalysisResult &a, const AnalysisResult &b) {
  AnalysisResult res = a;
  for (int ag = 0; ag < NUM_AGE_GROUPS; ++ag) {
    for (int c = 0; c < NUM_CARE_TYPES; ++c) {
      res.grid[ag][c].patientCount += b.grid[ag][c].patientCount;
      res.grid[ag][c].totalCost += b.grid[ag][c].totalCost;
      res.grid[ag][c].totalHours += b.grid[ag][c].totalHours;
    }
  }
  return res;
}