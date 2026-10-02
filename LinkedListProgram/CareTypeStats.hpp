#ifndef CARE_TYPE_STATS_HPP
#define CARE_TYPE_STATS_HPP

#include "PatientList.hpp"
#include <cstddef>
#include <string>

/**
 * @struct CostSummary
 * @brief Running totals for a group of patients (a care type, an age group, a dataset, ...).
 */
struct CostSummary {
    int patientCount;
    double totalCost;            // Sum of Length of Stay x Base Cost Per Hour x Days Visits Per Year
    long long totalStayHours;    // Sum of Length of Stay (visit duration in hours)
    long long totalVisits;       // Sum of Days Visits Per Year

    CostSummary() : patientCount(0), totalCost(0.0), totalStayHours(0), totalVisits(0) {}

    void add(const Patient& patient) {
        ++patientCount;
        totalCost += patient.calculateTotalCost();
        totalStayHours += patient.lengthOfStay;
        totalVisits += patient.daysVisitsPerYear;
    }

    void addAll(const PatientList& patients) {
        for (const PatientList::Node* current = patients.getHead(); current != nullptr; current = current->next) {
            add(current->data);
        }
    }

    // Average Cost = Total Medical Cost in Group / Number of Patients in Group
    double averageCost() const {
        return patientCount > 0 ? totalCost / patientCount : 0.0;
    }

    double averageStay() const {
        return patientCount > 0 ? static_cast<double>(totalStayHours) / patientCount : 0.0;
    }

    double averageVisits() const {
        return patientCount > 0 ? static_cast<double>(totalVisits) / patientCount : 0.0;
    }
};

/**
 * @class CareTypeStatList
 * @brief Singly linked list with one node per distinct Care Type, accumulating a CostSummary.
 *
 * Built by traversing a PatientList once: each patient is added to its care type's node,
 * and a new node is appended the first time a care type is seen.
 */
class CareTypeStatList {
public:
    struct Node {
        std::string careType;
        CostSummary summary;
        Node* next;

        explicit Node(const std::string& type) : careType(type), next(nullptr) {}
    };

private:
    Node* head;
    Node* tail;
    size_t count;

    // Stable insertion sort that relinks nodes; comesBefore(a, b) is true when a must precede b
    void sortBy(bool (*comesBefore)(const Node*, const Node*)) {
        Node* sorted = nullptr;
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            if (sorted == nullptr || comesBefore(current, sorted)) {
                current->next = sorted;
                sorted = current;
            } else {
                Node* scan = sorted;
                while (scan->next != nullptr && !comesBefore(current, scan->next)) {
                    scan = scan->next;
                }
                current->next = scan->next;
                scan->next = current;
            }
            current = next;
        }
        head = sorted;
        tail = head;
        while (tail != nullptr && tail->next != nullptr) {
            tail = tail->next;
        }
    }

    static bool hasMorePatients(const Node* a, const Node* b) {
        if (a->summary.patientCount != b->summary.patientCount) {
            return a->summary.patientCount > b->summary.patientCount;
        }
        return a->careType < b->careType;
    }

    static bool hasHigherCost(const Node* a, const Node* b) {
        return a->summary.totalCost > b->summary.totalCost;
    }

public:
    CareTypeStatList() : head(nullptr), tail(nullptr), count(0) {}

    explicit CareTypeStatList(const PatientList& patients) : head(nullptr), tail(nullptr), count(0) {
        recordAll(patients);
    }

    ~CareTypeStatList() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    // Aggregation lists are built and printed in place; copying is not needed
    CareTypeStatList(const CareTypeStatList&) = delete;
    CareTypeStatList& operator=(const CareTypeStatList&) = delete;

    // Adds one patient to its care type's node, appending a new node for an unseen care type
    void record(const Patient& patient) {
        Node* node = head;
        while (node != nullptr && node->careType != patient.careType) {
            node = node->next;
        }
        if (node == nullptr) {
            node = new Node(patient.careType);
            if (tail == nullptr) {
                head = node;
            } else {
                tail->next = node;
            }
            tail = node;
            ++count;
        }
        node->summary.add(patient);
    }

    void recordAll(const PatientList& patients) {
        for (const PatientList::Node* current = patients.getHead(); current != nullptr; current = current->next) {
            record(current->data);
        }
    }

    // Most requested first (ties broken alphabetically)
    void sortByPatientCount() {
        sortBy(hasMorePatients);
    }

    // Highest expenditure first
    void sortByTotalCost() {
        sortBy(hasHigherCost);
    }

    const Node* getHead() const {
        return head;
    }

    size_t size() const {
        return count;
    }

    int highestPatientCount() const {
        int best = 0;
        for (const Node* node = head; node != nullptr; node = node->next) {
            if (node->summary.patientCount > best) {
                best = node->summary.patientCount;
            }
        }
        return best;
    }

    // Most preferred / requested care type; tied care types are joined with " / "
    std::string mostPreferred() const {
        int best = highestPatientCount();
        if (best == 0) {
            return "-";
        }
        std::string result;
        for (const Node* node = head; node != nullptr; node = node->next) {
            if (node->summary.patientCount == best) {
                if (!result.empty()) {
                    result += " / ";
                }
                result += node->careType;
            }
        }
        return result;
    }
};

#endif