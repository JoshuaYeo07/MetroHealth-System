#ifndef PATIENT_LIST_HPP
#define PATIENT_LIST_HPP

#include "Patient.hpp"
#include <cstddef>
#include <stdexcept>

/**
 * @class PatientList
 * @brief Custom Singly Linked List for storing Patient records without using STL containers.
 *
 * Implements manual node-based heap memory management:
 * - One heap-allocated node per patient, chained through a single forward pointer
 * - Head and tail pointers for O(1) push_back during dataset ingestion
 * - Sequential O(n) traversal (no random access) - the key contrast with an array
 * - Memory footprint tracking
 */
class PatientList {
public:
    /**
     * @struct Node
     * @brief A single link in the chain: one patient record plus a pointer to the next node.
     */
    struct Node {
        Patient data;
        Node* next;

        explicit Node(const Patient& patient) : data(patient), next(nullptr) {}
    };

private:
    Node* head;           // First node in the chain (nullptr when empty)
    Node* tail;           // Last node in the chain, kept for O(1) appends
    size_t count;         // Current number of patient nodes stored

public:
    // Constructors & Destructor
    PatientList();
    ~PatientList();

    // Copy Semantics (Rule of 5)
	PatientList(const PatientList& other); // Copy constructor - Construct by copying others
	PatientList& operator=(const PatientList& other); // Copy assignment - Assign by copying others
    /*
    if (this != &other) {   // 1. Is b the same list as a? No, so continue
        clear();            // 2. Delete b's old nodes (Tom is gone)
        appendAll(other);   // 3. Copy a's patients into b (Ali, Siti, Raj)
    }
    return *this;           // 4. Return b
    */

    // Move Semantics (Rule of 5)
    PatientList(PatientList&& other) noexcept; // Construct by stealing others
	PatientList& operator=(PatientList&& other) noexcept; // Assign by stealing others
    /*
    if (this != &other) {
        clear();                 // free old nodes
        head = other.head;       // steal other's chain
        tail = other.tail;
        count = other.count;
        other.head = nullptr;    // leave other empty
        other.tail = nullptr;
        other.count = 0;
    }
    return *this;
    */

    // Element Access (sequential traversal from head)
    Patient& at(size_t index);
    const Patient& at(size_t index) const;
    Node* getHead();
    const Node* getHead() const;
    const Node* getTail() const;

    // Modifiers
    void push_back(const Patient& patient);
    void appendAll(const PatientList& other);
    void removeAt(size_t index);
    void clear();

    // Re-attaches the list to a re-ordered chain of its own nodes
    void relink(Node* newHead);

    // Capacity & Status
    size_t size() const;
    bool isEmpty() const;

    // Performance & Memory Analysis
    size_t getMemoryUsageBytes() const;

    // Demographic Categorization
    PatientList filterByAgeGroup(AgeGroup group) const;
};

#endif
