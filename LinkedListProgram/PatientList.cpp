#include "PatientList.hpp"

// Constructor - starts as an empty chain
PatientList::PatientList()
    : head(nullptr), tail(nullptr), count(0) {}

// Destructor - Walks the chain and frees every node to prevent memory leaks
PatientList::~PatientList() {
    clear();
}

// Copy Constructor (Deep Copy - allocates a fresh node for every record)
PatientList::PatientList(const PatientList& other)
    : head(nullptr), tail(nullptr), count(0) {
    appendAll(other);
}

// Copy Assignment Operator (Deep Copy with self-assignment protection)
PatientList& PatientList::operator=(const PatientList& other) {
    if (this != &other) {
        clear();
        appendAll(other);
    }
    return *this;
}

// Move Constructor (Transfers ownership of the node chain)
PatientList::PatientList(PatientList&& other) noexcept
    : head(other.head), tail(other.tail), count(other.count) {
    other.head = nullptr;
    other.tail = nullptr;
    other.count = 0;
}

// Move Assignment Operator
PatientList& PatientList::operator=(PatientList&& other) noexcept {
    if (this != &other) {
        clear();
        head = other.head;
        tail = other.tail;
        count = other.count;

        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }
    return *this;
}

// Element Access: O(n) - a linked list has no random access, so walk from head to the index
Patient& PatientList::at(size_t index) {
    if (index >= count) {
        throw std::out_of_range("Index out of bounds in PatientList::at()");
    }
    Node* current = head;
    for (size_t i = 0; i < index; ++i) {
        current = current->next;
    }
    return current->data;
}

const Patient& PatientList::at(size_t index) const {
    if (index >= count) {
        throw std::out_of_range("Index out of bounds in PatientList::at()");
    }
    const Node* current = head;
    for (size_t i = 0; i < index; ++i) {
        current = current->next;
    }
    return current->data;
}

// Direct node access for traversal-based sorting, searching and analysis algorithms
PatientList::Node* PatientList::getHead() {
    return head;
}

const PatientList::Node* PatientList::getHead() const {
    return head;
}

const PatientList::Node* PatientList::getTail() const {
    return tail;
}

// Appends a new node at the tail in O(1) (tail pointer avoids walking the whole chain)
void PatientList::push_back(const Patient& patient) {
    Node* node = new Node(patient);
    if (tail == nullptr) {
        head = node;
    } else {
        tail->next = node;
    }
    tail = node;
    ++count;
}

// Deep-copies every record of another list onto the end of this list
void PatientList::appendAll(const PatientList& other) {
    // Snapshot the length first so appending a list to itself still terminates
    size_t remaining = other.count;
    for (const Node* current = other.head; remaining > 0; current = current->next, --remaining) {
        push_back(current->data);
    }
}

// Removes the node at index: O(n) to locate the predecessor, O(1) to unlink (no element shifting)
void PatientList::removeAt(size_t index) {
    if (index >= count) {
        throw std::out_of_range("Index out of bounds in PatientList::removeAt()");
    }

    Node* removed;
    if (index == 0) {
        removed = head;
        head = head->next;
        if (head == nullptr) {
            tail = nullptr;
        }
    } else {
        Node* previous = head;
        for (size_t i = 0; i < index - 1; ++i) {
            previous = previous->next;
        }
        removed = previous->next;
        previous->next = removed->next;
        if (removed == tail) {
            tail = previous;
        }
    }

    delete removed;
    --count;
}

// Frees every node and resets the list to empty
void PatientList::clear() {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    tail = nullptr;
    count = 0;
}

// Links a temporary sentinel node after the last node and returns it. The tail pointer and the
// count are left alone, so tail still marks the last real node and detachSentinel can undo this in O(1).
PatientList::Node* PatientList::attachSentinel(const Patient& sentinelRecord) {
    Node* sentinel = new Node(sentinelRecord);
    if (tail == nullptr) {
        head = sentinel;
    } else {
        tail->next = sentinel;
    }
    return sentinel;
}

// Unlinks and frees the node added by attachSentinel, leaving the list exactly as it was before
void PatientList::detachSentinel(Node* sentinel) {
    if (tail == nullptr) {
        head = nullptr;
    } else {
        tail->next = nullptr;
    }
    delete sentinel;
}

// Adopts a re-ordered chain of this list's nodes and recomputes tail and count
void PatientList::relink(Node* newHead) {
    head = newHead;
    tail = nullptr;
    count = 0;
    for (Node* current = head; current != nullptr; current = current->next) {
        tail = current;
        ++count;
    }
}

// Size queries
size_t PatientList::size() const {
    return count;
}

bool PatientList::isEmpty() const {
    return count == 0;
}

// Calculates memory footprint in bytes: list header + one node (record + next pointer) per patient
size_t PatientList::getMemoryUsageBytes() const {
    return sizeof(PatientList) + (count * sizeof(Node));
}

// Builds a new list holding only the patients that fall in the given age group (Task 4a)
PatientList PatientList::filterByAgeGroup(AgeGroup group) const {
    PatientList result;
    for (const Node* current = head; current != nullptr; current = current->next) {
        if (current->data.getAgeGroup() == group) {
            result.push_back(current->data);
        }
    }
    return result;
}
