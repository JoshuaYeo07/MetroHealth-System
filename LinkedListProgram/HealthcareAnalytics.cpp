#include "HealthcareAnalytics.hpp"
#include "CareTypeStats.hpp"
#include "TablePrinter.hpp"
#include <iostream>
#include <iomanip>

namespace {

    const int GROUP_TABLE_WIDTH = 90;

    CostSummary summarize(const PatientList& patients) {
        CostSummary summary;
        summary.addAll(patients);
        return summary;
    }

    // Totals for one age group without building a filtered copy of the list
    CostSummary summarizeAgeGroup(const PatientList& patients, AgeGroup group) {
        CostSummary summary;
        for (const PatientList::Node* current = patients.getHead(); current != nullptr; current = current->next) {
            if (current->data.getAgeGroup() == group) {
                summary.add(current->data);
            }
        }
        return summary;
    }

    // ---------------------------------------------------------------------------------------------
    // Task 4
    // ---------------------------------------------------------------------------------------------

    // Task 4b/4c for one age group: care type breakdown, most preferred care type, total and average cost
    void printAgeGroupTable(AgeGroup group, const PatientList& groupPatients) {
        std::cout << "\nAge Group " << ageGroupName(group) << "\n";
        TablePrinter::printRule('-', GROUP_TABLE_WIDTH);

        if (groupPatients.isEmpty()) {
            std::cout << "  No patients recorded in this age group for this dataset.\n";
            TablePrinter::printRule('-', GROUP_TABLE_WIDTH);
            return;
        }

    std::cout << std::left  << std::setw(20) << "Care Type"
            << std::right << std::setw(15) << "Patient Count"
            << std::setw(22) << "Total Cost (MYR)"
            << std::setw(33) << "Average Cost per Patient (MYR)" << "\n";
        TablePrinter::printRule('-', GROUP_TABLE_WIDTH);

        CareTypeStatList careTypes(groupPatients);
        careTypes.sortByPatientCount();
        for (const CareTypeStatList::Node* node = careTypes.getHead(); node != nullptr; node = node->next) {
        std::cout << std::left  << std::setw(20) << node->careType
                << std::right << std::setw(15) << node->summary.patientCount
                << std::setw(22) << TablePrinter::formatMoney(node->summary.totalCost)
                << std::setw(33) << TablePrinter::formatMoney(node->summary.averageCost()) << "\n";
        }
        TablePrinter::printRule('-', GROUP_TABLE_WIDTH);

        CostSummary total = summarize(groupPatients);
        int topCount = careTypes.highestPatientCount();
        std::cout << "Total Billing for Age Group : MYR " << TablePrinter::formatMoney(total.totalCost) << "\n";
        std::cout << "Patients in Age Group       : " << total.patientCount << "\n";
        std::cout << "Average Cost per Patient    : MYR " << TablePrinter::formatMoney(total.averageCost()) << "\n";
        std::cout << "Most Preferred Care Type    : " << careTypes.mostPreferred()
            << " (" << topCount << " of " << total.patientCount << " patients, "
            << TablePrinter::formatPercent(topCount, total.patientCount) << ")\n";
    }

    void printAgeGroupSummaryRow(const std::string& label, const CostSummary& summary, const std::string& preferred) {
    std::cout << std::left  << std::setw(42) << label
              << std::right << std::setw(8)  << summary.patientCount
            << "   " << std::left << std::setw(22) << TablePrinter::fit(preferred, 21)
            << std::right << std::setw(17) << TablePrinter::formatMoney(summary.totalCost)
            << std::setw(18) << TablePrinter::formatMoney(summary.averageCost()) << "\n";
    }

    // One-line-per-age-group overview of the whole dataset
    void printAgeGroupSummary(const PatientList& patients, const std::string& datasetTitle) {
        TablePrinter::printSectionTitle("AGE GROUP SUMMARY - " + datasetTitle);
    std::cout << std::left  << std::setw(42) << "Age Group"
              << std::right << std::setw(8)  << "Patients"
            << "   " << std::left << std::setw(22) << "Most Preferred"
            << std::right << std::setw(17) << "Total Cost (MYR)"
            << std::setw(18) << "Avg Cost/Patient" << "\n";
        TablePrinter::printRule();

        int categorized = 0;
        for (int g = 0; g < AGE_GROUP_COUNT; ++g) {
            AgeGroup group = static_cast<AgeGroup>(g);
            PatientList groupPatients = patients.filterByAgeGroup(group);
            CareTypeStatList careTypes(groupPatients);
            CostSummary summary = summarize(groupPatients);
            categorized += summary.patientCount;
            printAgeGroupSummaryRow(ageGroupName(group), summary, careTypes.mostPreferred());
        }
        TablePrinter::printRule();

        CareTypeStatList allCareTypes(patients);
        printAgeGroupSummaryRow("ALL AGE GROUPS", summarize(patients), allCareTypes.mostPreferred());
        TablePrinter::printRule();

        if (categorized < static_cast<int>(patients.size())) {
            std::cout << "  * " << (patients.size() - categorized)
                << " patient(s) with an age outside 0-100 were not assigned to an age group.\n";
        }
    }

    // ---------------------------------------------------------------------------------------------
    // Task 5
    // ---------------------------------------------------------------------------------------------

    // Task 5a: total medical billing per dataset
    void printBillingPerDataset(const Facility* facilities, const CostSummary& grand) {
        TablePrinter::printSectionTitle("TASK 5a - TOTAL MEDICAL BILLING COST PER DATASET");
        std::cout << std::left << std::setw(51) << "Dataset / Facility"
            << std::right << std::setw(9) << "Patients"
            << std::setw(20) << "Total Billing (MYR)"
            << std::setw(18) << "Avg/Patient (MYR)"
            << std::setw(12) << "% of Total" << "\n";
        TablePrinter::printRule();

        auto printRow = [&grand](const std::string& label, const CostSummary& summary) {
            std::cout << std::left << std::setw(51) << label
                << std::right << std::setw(9) << summary.patientCount
                << std::setw(20) << TablePrinter::formatMoney(summary.totalCost)
                << std::setw(18) << TablePrinter::formatMoney(summary.averageCost())
                << std::setw(12) << TablePrinter::formatPercent(summary.totalCost, grand.totalCost) << "\n";
            };

        for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
            printRow(facility->title(), summarize(facility->patients));
        }
        TablePrinter::printRule();
        printRow("ALL DATASETS", grand);
        TablePrinter::printRule();
    }

    void printCareTypeTable(const PatientList& patients, const std::string& title) {
        std::cout << "\n" << title << "\n";
        TablePrinter::printRule();
        std::cout << std::left << std::setw(16) << "Care Type"
            << std::right << std::setw(9) << "Patients"
            << std::setw(17) << "Total Cost (MYR)"
            << std::setw(23) << "Avg Cost/Patient (MYR)"
            << std::setw(17) << "Total Stay (hrs)"
            << std::setw(15) << "Avg Stay (hrs)"
            << std::setw(13) << "% of Billing" << "\n";
        TablePrinter::printRule();

        CostSummary total = summarize(patients);
        auto printRow = [&total](const std::string& label, const CostSummary& summary) {
            std::cout << std::left << std::setw(16) << label
                << std::right << std::setw(9) << summary.patientCount
                << std::setw(17) << TablePrinter::formatMoney(summary.totalCost)
                << std::setw(23) << TablePrinter::formatMoney(summary.averageCost())
                << std::setw(17) << summary.totalStayHours
                << std::setw(15) << TablePrinter::formatDecimal(summary.averageStay())
                << std::setw(13) << TablePrinter::formatPercent(summary.totalCost, total.totalCost) << "\n";
            };

        CareTypeStatList careTypes(patients);
        careTypes.sortByTotalCost();
        for (const CareTypeStatList::Node* node = careTypes.getHead(); node != nullptr; node = node->next) {
            printRow(node->careType, node->summary);
        }
        TablePrinter::printRule();
        printRow("TOTAL", total);
        TablePrinter::printRule();
    }

    // Task 5b: total medical cost grouped by care type, per dataset and combined
    void printCostByCareType(const Facility* facilities, const PatientList& allPatients) {
        TablePrinter::printSectionTitle("TASK 5b - TOTAL MEDICAL COST GROUPED BY CARE TYPE (highest expenditure first)");
        for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
            printCareTypeTable(facility->patients, facility->title());
        }
        printCareTypeTable(allPatients, "All Datasets Combined");
    }

    // Task 5c: expenditure and visit duration side by side for every dataset
    void printDatasetComparison(const Facility* facilities, const CostSummary& grand) {
        TablePrinter::printSectionTitle("TASK 5c - EXPENDITURE & VISIT DURATION COMPARISON ACROSS DATASETS");
        std::cout << std::left << std::setw(12) << "Dataset"
            << std::right << std::setw(9) << "Patients"
            << std::setw(13) << "Total Cost"
            << std::setw(17) << "Avg Cost/Patient"
            << std::setw(17) << "Total Stay (hrs)"
            << std::setw(15) << "Avg Stay (hrs)"
            << std::setw(13) << "Total Visits"
            << std::setw(14) << "Avg Visits/Yr" << "\n";
        TablePrinter::printRule();

        auto printRow = [](const std::string& label, const CostSummary& summary) {
            std::cout << std::left << std::setw(12) << label
                << std::right << std::setw(9) << summary.patientCount
                << std::setw(13) << TablePrinter::formatMoney(summary.totalCost)
                << std::setw(17) << TablePrinter::formatMoney(summary.averageCost())
                << std::setw(17) << summary.totalStayHours
                << std::setw(15) << TablePrinter::formatDecimal(summary.averageStay())
                << std::setw(13) << summary.totalVisits
                << std::setw(14) << TablePrinter::formatDecimal(summary.averageVisits()) << "\n";
            };

        for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
            printRow(facility->shortName, summarize(facility->patients));
        }
        TablePrinter::printRule();
        printRow("All Datasets", grand);
        TablePrinter::printRule();
    }

    enum class Metric { PatientCount, TotalCost, AverageCost, AverageStay };

    std::string formatMetric(const CostSummary& summary, Metric metric) {
        if (summary.patientCount == 0) {
            return "-";
        }
        switch (metric) {
        case Metric::PatientCount: return std::to_string(summary.patientCount);
        case Metric::TotalCost:    return TablePrinter::formatMoney(summary.totalCost);
        case Metric::AverageCost:  return TablePrinter::formatMoney(summary.averageCost());
        default:                   return TablePrinter::formatDecimal(summary.averageStay());
        }
    }

    // Task 5c: age group (rows) x dataset (columns) matrix of one metric
    void printAgeGroupMatrix(const std::string& title, Metric metric,
        const Facility* facilities, const PatientList& allPatients) {
        std::cout << "\n" << title << "\n";
        TablePrinter::printRule();
        std::cout << std::left << std::setw(42) << "Age Group" << std::right;
        for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
            std::cout << std::setw(17) << facility->shortName;
        }
        std::cout << std::setw(17) << "All Datasets" << "\n";
        TablePrinter::printRule();

        for (int g = 0; g < AGE_GROUP_COUNT; ++g) {
            AgeGroup group = static_cast<AgeGroup>(g);
            std::cout << std::left << std::setw(42) << ageGroupName(group) << std::right;
            for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
                std::cout << std::setw(17) << formatMetric(summarizeAgeGroup(facility->patients, group), metric);
            }
            std::cout << std::setw(17) << formatMetric(summarizeAgeGroup(allPatients, group), metric) << "\n";
        }
        TablePrinter::printRule();

        std::cout << std::left << std::setw(42) << "ALL AGE GROUPS" << std::right;
        for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
            std::cout << std::setw(17) << formatMetric(summarize(facility->patients), metric);
        }
        std::cout << std::setw(17) << formatMetric(summarize(allPatients), metric) << "\n";
        TablePrinter::printRule();
    }

    // Tracks the group with the highest value of some metric
    struct Leader {
        std::string label;
        CostSummary summary;
        double value;

        Leader() : value(-1.0) {}

        void consider(const std::string& candidate, const CostSummary& candidateSummary, double candidateValue) {
            if (candidateSummary.patientCount > 0 && candidateValue > value) {
                label = candidate;
                summary = candidateSummary;
                value = candidateValue;
            }
        }
    };

    void printFinding(const std::string& question, const Leader& leader, const std::string& detail) {
        std::cout << "  " << std::left << std::setw(44) << question << ": "
            << (leader.value < 0 ? "-" : leader.label + "  (" + detail + ")") << "\n";
    }

    // Task 5c: highest-cost and longest-stay dataset, age group and care type
    void printKeyFindings(const Facility* facilities, const PatientList& allPatients, const CostSummary& grand) {
        Leader facilityTotal, facilityAverage, facilityStay;
        for (const Facility* facility = facilities; facility != nullptr; facility = facility->next) {
            CostSummary summary = summarize(facility->patients);
            facilityTotal.consider(facility->shortName, summary, summary.totalCost);
            facilityAverage.consider(facility->shortName, summary, summary.averageCost());
            facilityStay.consider(facility->shortName, summary, summary.averageStay());
        }

        Leader groupTotal, groupAverage, groupStay;
        for (int g = 0; g < AGE_GROUP_COUNT; ++g) {
            AgeGroup group = static_cast<AgeGroup>(g);
            CostSummary summary = summarizeAgeGroup(allPatients, group);
            groupTotal.consider(ageGroupName(group), summary, summary.totalCost);
            groupAverage.consider(ageGroupName(group), summary, summary.averageCost());
            groupStay.consider(ageGroupName(group), summary, summary.averageStay());
        }

        Leader careTotal, careAverage, careStay;
        CareTypeStatList careTypes(allPatients);
        for (const CareTypeStatList::Node* node = careTypes.getHead(); node != nullptr; node = node->next) {
            careTotal.consider(node->careType, node->summary, node->summary.totalCost);
            careAverage.consider(node->careType, node->summary, node->summary.averageCost());
            careStay.consider(node->careType, node->summary, node->summary.averageStay());
        }

        auto share = [&grand](const Leader& leader) {
            return "MYR " + TablePrinter::formatMoney(leader.summary.totalCost) + ", "
                + TablePrinter::formatPercent(leader.summary.totalCost, grand.totalCost) + " of all billing";
            };
        auto perPatient = [](const Leader& leader) {
            return "MYR " + TablePrinter::formatMoney(leader.summary.averageCost()) + " per patient";
            };
        auto perVisit = [](const Leader& leader) {
            return TablePrinter::formatDecimal(leader.summary.averageStay()) + " hrs per visit";
            };

        TablePrinter::printSectionTitle("TASK 5c - KEY FINDINGS");
        printFinding("Dataset with highest total billing", facilityTotal, share(facilityTotal));
        printFinding("Dataset with highest avg cost per patient", facilityAverage, perPatient(facilityAverage));
        printFinding("Dataset with longest avg visit duration", facilityStay, perVisit(facilityStay));
        printFinding("Age group with highest total billing", groupTotal, share(groupTotal));
        printFinding("Age group with highest avg cost per patient", groupAverage, perPatient(groupAverage));
        printFinding("Age group with longest avg visit duration", groupStay, perVisit(groupStay));
        printFinding("Care type with highest total billing", careTotal, share(careTotal));
        printFinding("Care type with highest avg cost per patient", careAverage, perPatient(careAverage));
        printFinding("Care type with longest avg visit duration", careStay, perVisit(careStay));
        TablePrinter::printRule();
    }

} // namespace

void HealthcareAnalytics::printAgeGroupReport(const PatientList& patients, const std::string& datasetTitle) {
    TablePrinter::printBanner("TASK 4 - PATIENT DEMOGRAPHIC & BILLING CATEGORIZATION: " + datasetTitle);
    std::cout << "Cost         = Length of Stay x Base Cost Per Hour x Days Visits Per Year\n";
    std::cout << "Average Cost = Total Medical Cost in Age Group / Number of Patients in Age Group\n";

    for (int g = 0; g < AGE_GROUP_COUNT; ++g) {
        AgeGroup group = static_cast<AgeGroup>(g);
        // Task 4a: recategorize the dataset's patients into a separate linked list per age group
        PatientList groupPatients = patients.filterByAgeGroup(group);
        printAgeGroupTable(group, groupPatients);
    }

    printAgeGroupSummary(patients, datasetTitle);
}

void HealthcareAnalytics::printExpenditureReport(const Facility* facilities, const PatientList& allPatients) {
    TablePrinter::printBanner("TASK 5 - HEALTHCARE EXPENDITURE & SERVICE ANALYSIS (all amounts in MYR)");
    CostSummary grand = summarize(allPatients);

    printBillingPerDataset(facilities, grand);
    printCostByCareType(facilities, allPatients);
    printDatasetComparison(facilities, grand);

    TablePrinter::printSectionTitle("TASK 5c - COMPARISON ACROSS DATASETS AND AGE GROUPS");
    printAgeGroupMatrix("Patient Count by Age Group", Metric::PatientCount, facilities, allPatients);
    printAgeGroupMatrix("Total Expenditure by Age Group (MYR)", Metric::TotalCost, facilities, allPatients);
    printAgeGroupMatrix("Average Cost per Patient by Age Group (MYR)", Metric::AverageCost, facilities, allPatients);
    printAgeGroupMatrix("Average Visit Duration by Age Group (hrs per visit)", Metric::AverageStay, facilities, allPatients);

    printKeyFindings(facilities, allPatients, grand);
}
