#ifndef PATIENT_HPP
#define PATIENT_HPP

#include <string>
#include <iostream>
#include <iomanip>

/**
 * @enum AgeGroup
 * @brief Categorization of patients based on demographic criteria (Assignment Task 4a).
 */
enum class AgeGroup {
    PediatricsAndAdolescents,   // 0–17
    YoungAdultsStudents,        // 18–25
    WorkingAdultsEarlyCareer,   // 26–45
    WorkingAdultsLateCareer,    // 46–60
    SeniorCitizensGeriatric,    // 61–100
    Unknown                     // Invalid / out of range
};

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
     * @brief Calculates total medical cost produced by the patient (Task 4b).
     * Formula: Cost = Length of Stay x Base Cost Per Hour x Days Visits Per Year
     * @return Calculated medical cost in MYR
     */
    double calculateTotalCost() const {
        return static_cast<double>(lengthOfStay) * baseCostPerHour * static_cast<double>(daysVisitsPerYear);
    }

    /**
     * @brief Determines the demographic age group classification (Task 4a).
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
};

#endif // PATIENT_HPP
