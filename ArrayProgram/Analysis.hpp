#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP

#include "Patient.hpp"
#include "PatientArray.hpp"
#include <string>


const int NUM_AGE_GROUPS = 5;
const int NUM_CARE_TYPES = 6;

extern const char *AGE_GROUP_NAMES[NUM_AGE_GROUPS];
extern const char *CARE_TYPE_NAMES[NUM_CARE_TYPES];

struct CellStats {
  int patientCount = 0;
  double totalCost = 0.0;
  double totalHours = 0.0;
};

struct AnalysisResult {
  CellStats grid[NUM_AGE_GROUPS][NUM_CARE_TYPES];
};

int careTypeIndex(const std::string &careType);
void accumulate(const Patient &p, AnalysisResult &r);
AnalysisResult analyze(const PatientArray &arr);

CellStats ageGroupTotal(const AnalysisResult &r, int ag);
CellStats careTypeTotal(const AnalysisResult &r, int c);
CellStats datasetTotal(const AnalysisResult &r);
int mostPreferredCareType(const AnalysisResult &r, int ag);
AnalysisResult combine(const AnalysisResult &a, const AnalysisResult &b);

#endif