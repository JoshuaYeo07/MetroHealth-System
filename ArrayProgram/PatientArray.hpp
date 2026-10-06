#ifndef PATIENT_ARRAY_HPP
#define PATIENT_ARRAY_HPP

#include "Patient.hpp"
#include <cstddef>
#include <stdexcept>

/**
 * @class PatientArray
 * @brief Custom Dynamic Array for storing Patient records without using STL containers.
 *
 * Implements manual dynamic heap memory management:
 * - Dynamic capacity doubling (amortized O(1) push_back)
 * - Contiguous memory layout for O(1) random access
 * - Strict adherence to the Rule of 5 (deep copy & move semantics)
 * - Memory footprint tracking for Task 8 performance analysis
 */
class PatientArray {
private:
    Patient* data;        // Pointer to dynamically allocated contiguous heap buffer
    size_t count;         // Current number of patient elements stored
    size_t capacity;      // Total allocated capacity

    /**
     * @brief Internal helper to resize the dynamic array buffer.
     * @param newCapacity The new capacity to allocate
     */
    void reallocate(size_t newCapacity);

public:
    // Constructors & Destructor
    explicit PatientArray(size_t initialCapacity = 16);
    ~PatientArray();

    // Copy Semantics (Rule of 5)
    PatientArray(const PatientArray& other);
    PatientArray& operator=(const PatientArray& other);

    // Move Semantics (Rule of 5)
    PatientArray(PatientArray&& other) noexcept;
    PatientArray& operator=(PatientArray&& other) noexcept;

    // Element Access
    Patient& operator[](size_t index);
    const Patient& operator[](size_t index) const;
    Patient& at(size_t index);
    const Patient& at(size_t index) const;

    // Modifiers
    void push_back(const Patient& patient);
    void removeAt(size_t index);
    void clear();
    void reserve(size_t newCapacity);

    // Capacity & Status
    size_t size() const;
    size_t getCapacity() const;
    bool isEmpty() const;

    // Performance & Memory Analysis (Task 8)
    size_t getMemoryUsageBytes() const;

    // Low-level pointer access for sorting and swapping algorithms
    Patient* rawData();
    const Patient* rawData() const;
};

#endif // PATIENT_ARRAY_HPP
