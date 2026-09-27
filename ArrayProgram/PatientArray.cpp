#include "PatientArray.hpp"

// Constructor with optional initial capacity
PatientArray::PatientArray(size_t initialCapacity)
    : data(nullptr), count(0), capacity(initialCapacity) {
    if (capacity > 0) {
        data = new Patient[capacity];
    }
}

// Destructor - Reclaims allocated heap memory to prevent memory leaks
PatientArray::~PatientArray() {
    delete[] data;
    data = nullptr;
    count = 0;
    capacity = 0;
}

// Internal reallocation: Allocates larger block, copies elements, frees old memory
void PatientArray::reallocate(size_t newCapacity) {
    if (newCapacity < count) {
        newCapacity = count;
    }

    Patient* newData = new Patient[newCapacity];
    for (size_t i = 0; i < count; ++i) {
        newData[i] = data[i];
    }

    delete[] data;
    data = newData;
    capacity = newCapacity;
}

// Copy Constructor (Deep Copy)
PatientArray::PatientArray(const PatientArray& other)
    : data(nullptr), count(other.count), capacity(other.capacity) {
    if (capacity > 0) {
        data = new Patient[capacity];
        for (size_t i = 0; i < count; ++i) {
            data[i] = other.data[i];
        }
    }
}

// Copy Assignment Operator (Deep Copy with self-assignment protection)
PatientArray& PatientArray::operator=(const PatientArray& other) {
    if (this != &other) {
        delete[] data;
        count = other.count;
        capacity = other.capacity;
        if (capacity > 0) {
            data = new Patient[capacity];
            for (size_t i = 0; i < count; ++i) {
                data[i] = other.data[i];
            }
        } else {
            data = nullptr;
        }
    }
    return *this;
}

// Move Constructor (Transfers ownership of raw heap pointer)
PatientArray::PatientArray(PatientArray&& other) noexcept
    : data(other.data), count(other.count), capacity(other.capacity) {
    other.data = nullptr;
    other.count = 0;
    other.capacity = 0;
}

// Move Assignment Operator
PatientArray& PatientArray::operator=(PatientArray&& other) noexcept {
    if (this != &other) {
        delete[] data;
        data = other.data;
        count = other.count;
        capacity = other.capacity;

        other.data = nullptr;
        other.count = 0;
        other.capacity = 0;
    }
    return *this;
}

// Element Access: Unchecked operator[] for O(1) performance
Patient& PatientArray::operator[](size_t index) {
    return data[index];
}

const Patient& PatientArray::operator[](size_t index) const {
    return data[index];
}

// Element Access: Bounds-checked at()
Patient& PatientArray::at(size_t index) {
    if (index >= count) {
        throw std::out_of_range("Index out of bounds in PatientArray::at()");
    }
    return data[index];
}

const Patient& PatientArray::at(size_t index) const {
    if (index >= count) {
        throw std::out_of_range("Index out of bounds in PatientArray::at()");
    }
    return data[index];
}

// Appends an element to the dynamic array; expands capacity if full (2x factor)
void PatientArray::push_back(const Patient& patient) {
    if (count >= capacity) {
        size_t newCap = (capacity == 0) ? 16 : capacity * 2;
        reallocate(newCap);
    }
    data[count++] = patient;
}

// Removes element at specified index by shifting elements left
void PatientArray::removeAt(size_t index) {
    if (index >= count) {
        throw std::out_of_range("Index out of bounds in PatientArray::removeAt()");
    }
    for (size_t i = index; i < count - 1; ++i) {
        data[i] = data[i + 1];
    }
    --count;
}

// Clears contents without deallocating buffer
void PatientArray::clear() {
    count = 0;
}

// Size and capacity queries
size_t PatientArray::size() const {
    return count;
}

size_t PatientArray::getCapacity() const {
    return capacity;
}

bool PatientArray::isEmpty() const {
    return count == 0;
}

// Calculates exact memory footprint in bytes (Task 8 analysis)
size_t PatientArray::getMemoryUsageBytes() const {
    return sizeof(PatientArray) + (capacity * sizeof(Patient));
}

// Direct pointer access for algorithms
Patient* PatientArray::rawData() {
    return data;
}

const Patient* PatientArray::rawData() const {
    return data;
}
