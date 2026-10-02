#ifndef PATIENT_HPP
#define PATIENT_HPP

#include <string>

/**
 * @enum AgeGroup
 * @brief Categorization of patients based on demographic criteria.
 */
enum class AgeGroup {
    PediatricsAndAdolescents,   // 0-17
    YoungAdultsStudents,        // 18-25
    WorkingAdultsEarlyCareer,   // 26-45
    WorkingAdultsLateCareer,    // 46-60
    SeniorCitizensGeriatric,    // 61-100
    Unknown                     // Invalid / out of range
};

// Number of valid age groups. AgeGroup values 0 .. AGE_GROUP_COUNT-1 are iterated in ascending age order.
const int AGE_GROUP_COUNT = 5;

/**
 * @brief Age range label of an age group, e.g. "18-25".
 */
inline std::string ageGroupRange(AgeGroup group) {
    switch (group) {
        case AgeGroup::PediatricsAndAdolescents: return "0-17";
        case AgeGroup::YoungAdultsStudents:      return "18-25";
        case AgeGroup::WorkingAdultsEarlyCareer: return "26-45";
        case AgeGroup::WorkingAdultsLateCareer:  return "46-60";
        case AgeGroup::SeniorCitizensGeriatric:  return "61-100";
        default:                                 return "N/A";
    }
}

/**
 * @brief Descriptive label of an age group, e.g. "Young Adults / University Students".
 */
inline std::string ageGroupDescription(AgeGroup group) {
    switch (group) {
        case AgeGroup::PediatricsAndAdolescents: return "Pediatrics & Adolescents";
        case AgeGroup::YoungAdultsStudents:      return "Young Adults / University Students";
        case AgeGroup::WorkingAdultsEarlyCareer: return "Working Adults (Early Career)";
        case AgeGroup::WorkingAdultsLateCareer:  return "Working Adults (Late Career)";
        case AgeGroup::SeniorCitizensGeriatric:  return "Senior Citizens / Geriatric Care";
        default:                                 return "Unknown Age Bracket";
    }
}

/**
 * @brief Full age group name for table output, e.g. "18-25: Young Adults / University Students".
 */
inline std::string ageGroupName(AgeGroup group) {
    if (group == AgeGroup::Unknown) {
        return ageGroupDescription(group);
    }
    return ageGroupRange(group) + ": " + ageGroupDescription(group);
}

/**
 * @struct Patient
 * @brief Represents a patient record collected across MetroHealth facilities.
 */
struct Patient {
    std::string patientId;
    int age;
    std::string careType;
    int lengthOfStay;          // Visit duration in hours
    double baseCostPerHour;     // Billing rate in MYR/hr
    int daysVisitsPerYear;      // Average visits/days per year

    // Default constructor
    Patient()
        : patientId(""), age(0), careType(""), lengthOfStay(0),
          baseCostPerHour(0.0), daysVisitsPerYear(0) {}

    // Parameterized constructor
    Patient(std::string id, int a, std::string care, int stay, double rate, int visits)
        : patientId(id), age(a), careType(care), lengthOfStay(stay),
          baseCostPerHour(rate), daysVisitsPerYear(visits) {}

    /**
     * @brief Calculates total medical cost produced by the patient.
     * Formula: Cost = Length of Stay x Base Cost Per Hour x Days Visits Per Year
     * @return Calculated medical cost in MYR
     */
    double calculateTotalCost() const {
        return static_cast<double>(lengthOfStay) * baseCostPerHour * static_cast<double>(daysVisitsPerYear);
    }

    /**
     * @brief Determines the demographic age group classification.
     * @return AgeGroup enum value
     */
    AgeGroup getAgeGroup() const {
        if (age >= 0 && age <= 17) {
            return AgeGroup::PediatricsAndAdolescents;
        } else if (age >= 18 && age <= 25) {
            return AgeGroup::YoungAdultsStudents;
        } else if (age >= 26 && age <= 45) {
            return AgeGroup::WorkingAdultsEarlyCareer;
        } else if (age >= 46 && age <= 60) {
            return AgeGroup::WorkingAdultsLateCareer;
        } else if (age >= 61 && age <= 100) {
            return AgeGroup::SeniorCitizensGeriatric;
        }
        return AgeGroup::Unknown;
    }

    /**
     * @brief Converts the patient's age group to a human-readable string for table output.
     * @return Descriptive age group string
     */
    std::string getAgeGroupName() const {
        return ageGroupName(getAgeGroup());
    }
};

#endif
