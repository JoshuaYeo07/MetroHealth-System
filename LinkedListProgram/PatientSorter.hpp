#ifndef PATIENT_SORTER_HPP
#define PATIENT_SORTER_HPP

#include "PatientList.hpp"
#include <chrono>
#include <cstddef>
#include <string>

/**
 * @enum SortField
 * @brief The patient attribute a PatientList can be sorted by.
 */
enum class SortField {
    Age,
    VisitDuration,   // Length of Stay (hours)
    TotalCost        // Length of Stay x Base Cost Per Hour x Days Visits Per Year
};

/**
 * @enum SortAlgorithm
 * @brief Which sorting algorithm PatientSorter::sortedCopy should use.
 */
enum class SortAlgorithm {
    Bubble,
    Merge
};

/**
 * @struct SortStats
 * @brief What one run of a sorting algorithm did, used to compare the algorithms.
 */
struct SortStats {
    double elapsedMs;          // Wall-clock time of the sort
    long long comparisons;     // Calls to the comparison rule
    long long moves;           // Node relinks: one adjacent swap (bubble) or one node placed in the merged output (merge)
    size_t maxDepth;           // Deepest recursion level reached (0 for the iterative bubble sort)
    size_t auxBytes;           // Estimated working memory needed beyond the list itself

    SortStats() : elapsedMs(0.0), comparisons(0), moves(0), maxDepth(0), auxBytes(0) {}
};

/**
 * @class PatientSorter
 * @brief Bubble sort and merge sort for PatientList. Both sort by relinking the existing nodes,
 * so no Patient record is copied and no node is allocated or freed while sorting.
 *
 * A comparison rule is a plain function comesBefore(a, b) that returns true when a must come
 * strictly before b. Both algorithms take the same rule, so only the algorithm differs between runs.
 * Because the rule is strict, equal elements keep their original order (both sorts are stable).
 */
class PatientSorter {
public:
    using Node = PatientList::Node;
    using Comparator = bool (*)(const Node*, const Node*);

    // ---- Comparison rules ----
    static bool ageAscending(const Node* a, const Node* b)  { return a->data.age < b->data.age; }
    static bool ageDescending(const Node* a, const Node* b) { return a->data.age > b->data.age; }

    static bool stayAscending(const Node* a, const Node* b)  { return a->data.lengthOfStay < b->data.lengthOfStay; }
    static bool stayDescending(const Node* a, const Node* b) { return a->data.lengthOfStay > b->data.lengthOfStay; }

    static bool costAscending(const Node* a, const Node* b)  { return a->data.calculateTotalCost() < b->data.calculateTotalCost(); }
    static bool costDescending(const Node* a, const Node* b) { return a->data.calculateTotalCost() > b->data.calculateTotalCost(); }

    // Picks the comparison rule for a field and direction
    static Comparator comparatorFor(SortField field, bool ascending) {
        switch (field) {
        case SortField::Age:           return ascending ? ageAscending : ageDescending;
        case SortField::VisitDuration: return ascending ? stayAscending : stayDescending;
        case SortField::TotalCost:     return ascending ? costAscending : costDescending;
        }
        return ageAscending;
    }

    static std::string fieldName(SortField field) {
        switch (field) {
        case SortField::Age:           return "Age";
        case SortField::VisitDuration: return "Visit Duration (Length of Stay)";
        case SortField::TotalCost:     return "Total Medical Cost";
        }
        return "Unknown";
    }

    // True when no node comes strictly before the node ahead of it
    static bool isSorted(const PatientList& list, Comparator comesBefore) {
        const Node* current = list.getHead();
        while (current != nullptr && current->next != nullptr) {
            if (comesBefore(current->next, current)) {
                return false;
            }
            current = current->next;
        }
        return true;
    }

    /**
     * @brief Bubble sort: repeatedly sweeps the chain and swaps neighbours that are out of order.
     * Each sweep pushes the next-best node to the end of the unsorted part. Stops early after a
     * sweep with no swaps. O(n) best case, O(n^2) average and worst case, O(1) extra memory.
     */
    static SortStats bubbleSort(PatientList& list, Comparator comesBefore) {
        SortStats stats;
        if (list.size() < 2) {
            return stats;
        }

        auto start = std::chrono::steady_clock::now();

        Node* head = list.getHead();
        Node* lastSorted = nullptr;   // Nodes from here to the end are already in their final place
        bool swapped = true;
        while (swapped) {
            swapped = false;
            Node* previous = nullptr;
            Node* current = head;
            while (current->next != lastSorted) {
                Node* next = current->next;
                ++stats.comparisons;
                if (comesBefore(next, current)) {
                    // Swap current and next by relinking: previous -> next -> current -> (rest)
                    current->next = next->next;
                    next->next = current;
                    if (previous == nullptr) {
                        head = next;
                    } else {
                        previous->next = next;
                    }
                    previous = next;
                    swapped = true;
                    ++stats.moves;
                } else {
                    previous = current;
                    current = next;
                }
            }
            lastSorted = current;
        }
        list.relink(head);

        stats.elapsedMs = elapsedSince(start);
        stats.maxDepth = 0;
        stats.auxBytes = BUBBLE_AUX_BYTES;
        return stats;
    }

    /**
     * @brief Merge sort: splits the chain in half, sorts each half recursively, then merges the two
     * sorted halves by relinking nodes. O(n log n) in every case; the only extra memory is the
     * recursion stack, O(log n) deep.
     */
    static SortStats mergeSort(PatientList& list, Comparator comesBefore) {
        SortStats stats;
        if (list.size() < 2) {
            return stats;
        }

        auto start = std::chrono::steady_clock::now();

        Node* head = mergeSortChain(list.getHead(), comesBefore, stats, 1);
        list.relink(head);

        stats.elapsedMs = elapsedSince(start);
        stats.auxBytes = stats.maxDepth * MERGE_FRAME_BYTES + MERGE_STEP_BYTES;
        return stats;
    }

    /**
     * @brief Returns a sorted deep copy of a list and leaves the original unchanged, so the
     * unsorted and sorted versions can be used side by side (e.g. searching unsorted vs sorted data).
     * The copy costs one extra list of memory (about source.getMemoryUsageBytes()).
     * @param source List to copy and sort
     * @param field Age, Visit Duration or Total Medical Cost
     * @param ascending true for lowest first, false for highest first
     * @param algorithm Bubble or Merge (merge sort by default, it is much faster)
     */
    static PatientList sortedCopy(const PatientList& source, SortField field, bool ascending,
                                  SortAlgorithm algorithm = SortAlgorithm::Merge) {
        PatientList copy = source;
        Comparator rule = comparatorFor(field, ascending);
        if (algorithm == SortAlgorithm::Bubble) {
            bubbleSort(copy, rule);
        } else {
            mergeSort(copy, rule);
        }
        return copy;
    }

private:
    // Working variables of bubbleSort: previous, current, next, lastSorted pointers and the swapped flag
    static constexpr size_t BUBBLE_AUX_BYTES = 4 * sizeof(Node*) + sizeof(bool);
    // One mergeSortChain frame: head, second, left, right pointers + return address + saved frame pointer
    static constexpr size_t MERGE_FRAME_BYTES = 6 * sizeof(void*);
    // The mergeChains call at the bottom of the recursion: a, b, mergedHead, mergedTail, chosen + return address
    static constexpr size_t MERGE_STEP_BYTES = 6 * sizeof(void*);

    static double elapsedSince(std::chrono::steady_clock::time_point start) {
        return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    }

    // Sorts the chain starting at head and returns the new first node of the sorted chain
    static Node* mergeSortChain(Node* head, Comparator comesBefore, SortStats& stats, size_t depth) {
        if (depth > stats.maxDepth) {
            stats.maxDepth = depth;
        }
        if (head == nullptr || head->next == nullptr) {
            return head;
        }

        // Find the middle with a slow and a fast pointer, then cut the chain into two halves
        Node* slow = head;
        Node* fast = head->next;
        while (fast != nullptr && fast->next != nullptr) {
            slow = slow->next;
            fast = fast->next->next;
        }
        Node* second = slow->next;
        slow->next = nullptr;

        Node* left = mergeSortChain(head, comesBefore, stats, depth + 1);
        Node* right = mergeSortChain(second, comesBefore, stats, depth + 1);
        return mergeChains(left, right, comesBefore, stats);
    }

    // Merges two sorted chains into one. On a tie the node from the left chain goes first (keeps it stable)
    static Node* mergeChains(Node* a, Node* b, Comparator comesBefore, SortStats& stats) {
        Node* mergedHead = nullptr;
        Node* mergedTail = nullptr;

        while (a != nullptr && b != nullptr) {
            ++stats.comparisons;
            Node* chosen;
            if (comesBefore(b, a)) {
                chosen = b;
                b = b->next;
            } else {
                chosen = a;
                a = a->next;
            }
            if (mergedTail == nullptr) {
                mergedHead = chosen;
            } else {
                mergedTail->next = chosen;
            }
            mergedTail = chosen;
            ++stats.moves;
        }

        // One chain ran out: the rest of the other chain is already in order, so attach it in one step
        Node* rest = (a != nullptr) ? a : b;
        if (rest != nullptr) {
            if (mergedTail == nullptr) {
                mergedHead = rest;
            } else {
                mergedTail->next = rest;
            }
            ++stats.moves;
        } else if (mergedTail != nullptr) {
            mergedTail->next = nullptr;
        }
        return mergedHead;
    }
};

#endif