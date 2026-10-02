#ifndef HEALTHCARE_ANALYTICS_HPP
#define HEALTHCARE_ANALYTICS_HPP

#include "Facility.hpp"
#include "PatientList.hpp"
#include <string>

/**
 * @class HealthcareAnalytics
 * @brief Console reports, computed purely by traversing singly linked lists.
 */
class HealthcareAnalytics {
public:
    /**
     * @brief Task 4: recategorizes patients into age groups and, per group, prints the care type
     * breakdown, most preferred care type, total medical cost and average cost per patient.
     * @param patients Dataset to categorize
     * @param datasetTitle Heading shown on the report
     */
    static void printAgeGroupReport(const PatientList& patients, const std::string& datasetTitle);

    /**
     * @brief Task 5: billing per dataset, cost grouped by care type, and expenditure / visit
     * duration comparison across datasets and age groups.
     * @param facilities Head of the facility chain (A -> B -> C)
     * @param allPatients All datasets combined
     */
    static void printExpenditureReport(const Facility* facilities, const PatientList& allPatients);
};

#endif
