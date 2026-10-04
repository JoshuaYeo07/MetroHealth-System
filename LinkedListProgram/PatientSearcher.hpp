#ifndef PATIENT_SEARCHER_HPP
#define PATIENT_SEARCHER_HPP

#include "PatientList.hpp"
#include "PatientSorter.hpp"
#include <cstddef>
#include <string>

/**
 * @enum SearchField
 * @brief The patient attribute a search looks at.
 */
enum class SearchField {
    Age,             // Age group, e.g. 61-100
    CareType,        // One specific care type, e.g. Emergency
    VisitDuration    // Length of Stay above a threshold, e.g. > 24 hours
};

/**
 * @enum DataOrder
 * @brief Whether the list given to a search is in loaded order or sorted by the search field.
 */
enum class DataOrder {
    Unsorted,
    Sorted
};

/**
 * @enum SearchAlgorithm
 * @brief Which searching algorithm PatientSearcher::search should use.
 */
enum class SearchAlgorithm {
    Linear,
    SentinelLinear
};

/**
 * @struct SearchCriteria
 * @brief What one search is looking for: an age group, one care type, or visits longer than a threshold.
 */
struct SearchCriteria {
    SearchField field;
    int minAge;             // Age: youngest matching age
    int maxAge;             // Age: oldest matching age
    std::string careType;   // CareType: the care type to match
    int thresholdHours;     // VisitDuration: visits longer than this match
    std::string label;      // Shown on report headings, e.g. "Visit Duration > 24 hours"

    SearchCriteria() : field(SearchField::Age), minAge(0), maxAge(0), thresholdHours(0) {}

    // Every patient whose age falls in the group, e.g. 61-100
    static SearchCriteria forAgeGroup(AgeGroup group) {
        SearchCriteria criteria;
        criteria.field = SearchField::Age;
        criteria.minAge = ageGroupMinAge(group);
        criteria.maxAge = ageGroupMaxAge(group);
        criteria.label = "Age Group " + ageGroupName(group);
        return criteria;
    }

    // Every patient who received exactly this care type, e.g. Emergency
    static SearchCriteria forCareType(const std::string& careType) {
        SearchCriteria criteria;
        criteria.field = SearchField::CareType;
        criteria.careType = careType;
        criteria.label = "Care Type = " + careType;
        return criteria;
    }

    // Every patient whose visit lasted longer than the threshold, e.g. > 24 hours
    static SearchCriteria forStayLongerThan(int hours) {
        SearchCriteria criteria;
        criteria.field = SearchField::VisitDuration;
        criteria.thresholdHours = hours;
        criteria.label = "Visit Duration > " + std::to_string(hours) + (hours == 1 ? " hour" : " hours");
        return criteria;
    }

    // True when the patient is one of the records this search is looking for
    bool matches(const Patient& patient) const {
        switch (field) {
        case SearchField::Age:           return patient.age >= minAge && patient.age <= maxAge;
        case SearchField::CareType:      return patient.careType == careType;
        case SearchField::VisitDuration: return patient.lengthOfStay > thresholdHours;
        }
        return false;
    }

    // A made-up record that is guaranteed to match, whichever field is searched.
    // Sentinel linear search places it after the last real record.
    Patient sentinelRecord() const {
        Patient sentinel;
        sentinel.patientId = "SENTINEL";
        sentinel.age = minAge;
        sentinel.careType = careType;
        sentinel.lengthOfStay = thresholdHours + 1;
        return sentinel;
    }
};

/**
 * @struct SearchStats
 * @brief What one run of a search did, used to compare the algorithms and unsorted with sorted data.
 */
struct SearchStats {
    double elapsedMicros;      // Average wall-clock time of one search in microseconds (filled in by the code that times it)
    long long comparisons;     // Records checked against the search criteria (each one is one step along the chain)
    long long endChecks;       // Times the search tested whether it had reached the end of the list
    size_t matchCount;         // Records that satisfy the criteria
    size_t auxBytes;           // Estimated working variables needed beyond the list itself
    size_t sentinelBytes;      // Heap memory of the temporary sentinel node (0 for plain linear search)

    SearchStats() : elapsedMicros(0.0), comparisons(0), endChecks(0), matchCount(0), auxBytes(0), sentinelBytes(0) {}
};

/**
 * @class PatientSearcher
 * @brief Linear search and sentinel linear search for PatientList, usable on unsorted and on sorted data.
 *
 * Both algorithms find ALL records that satisfy a SearchCriteria, count their comparisons and
 * end-of-list checks, and append the matches to matchesOut when one is given (pass nullptr to only
 * count). They visit the records in the same order and report the same matches; what differs is
 * how each one knows that it has reached the end of the list.
 */
class PatientSearcher {
public:
    using Node = PatientList::Node;

    // ---- Sorted data ----
    // The searches do not sort anything themselves. Sorted data comes from PatientSorter::sortedCopy,
    // which returns a sorted deep copy and leaves the original list unchanged.

    // The field a list has to be sorted by before a sorted search on this search field can be run on it
    static SortField sortFieldFor(SearchField field) {
        switch (field) {
        case SearchField::Age:           return SortField::Age;
        case SearchField::VisitDuration: return SortField::VisitDuration;
        case SearchField::CareType:      return SortField::CareType;
        }
        return SortField::Age;
    }

    static std::string sortOrderName(SearchField field, bool ascending) {
        switch (field) {
        case SearchField::Age:           return ascending ? "Age (youngest first)" : "Age (oldest first)";
        case SearchField::VisitDuration: return ascending ? "Visit Duration (shortest first)" : "Visit Duration (longest first)";
        case SearchField::CareType:      return ascending ? "Care Type (A-Z)" : "Care Type (Z-A)";
        }
        return "Unknown";
    }

    // ---- Searching ----

    static std::string algorithmName(SearchAlgorithm algorithm) {
        switch (algorithm) {
        case SearchAlgorithm::Linear:         return "Linear Search";
        case SearchAlgorithm::SentinelLinear: return "Sentinel Linear Search";
        }
        return "Unknown";
    }

    /**
     * @brief Linear search: walks the chain from the head and checks each record against the criteria.
     * Before every record it also has to test whether the chain has ended (current != nullptr).
     *
     * Unsorted data: a match can be anywhere, so every record has to be checked. Always n comparisons.
     *
     * Sorted data (sorted by the search field, in either direction): the matching records sit next
     * to each other as one group. Once that group has been reached and a record that does not match
     * follows it, no later record can match, so the search stops early. Best case k + 1 comparisons
     * (the k matches are at the head), worst case n (the matches are at the tail, or there are none).
     *
     * @param list List to search
     * @param criteria What to search for
     * @param order DataOrder::Sorted only if the list is sorted by the criteria's field
     * @param matchesOut Receives a copy of every matching record, when given
     */
    static SearchStats linearSearch(const PatientList& list, const SearchCriteria& criteria, DataOrder order,
                                    PatientList* matchesOut = nullptr) {
        SearchStats stats;
        stats.auxBytes = LINEAR_AUX_BYTES;

        bool groupReached = false;      // Has the first matching record been found yet?
        const Node* current = list.getHead();
        while (current != nullptr) {
            ++stats.comparisons;
            if (criteria.matches(current->data)) {
                groupReached = true;
                recordMatch(current, stats, matchesOut);
            } else if (order == DataOrder::Sorted && groupReached) {
                break;                  // The group of matches has ended
            }
            current = current->next;
        }

        // The loop tested "current != nullptr" once before every record it checked, plus one last
        // time when it ran off the end of the chain (but not when it stopped early)
        stats.endChecks = stats.comparisons + (current == nullptr ? 1 : 0);
        return stats;
    }

    /**
     * @brief Sentinel linear search: first links a sentinel node, a made-up record that is guaranteed
     * to match, after the last real record. The walk is then certain to hit a match before the chain
     * ends, so it does not have to test for the end of the list at every record. It only asks "is
     * this the sentinel?" when it finds a match, and removes the sentinel again when it is done.
     *
     * It checks the same records as linear search (plus the sentinel itself, one extra comparison),
     * but makes one end-of-list check per match instead of one per record. The price is one
     * temporary node. On sorted data it stops early exactly as linear search does.
     *
     * The list is taken by non-const reference because the sentinel is linked to it while the search
     * runs. It is back in its original state when the function returns.
     *
     * @param list List to search (temporarily one node longer)
     * @param criteria What to search for
     * @param order DataOrder::Sorted only if the list is sorted by the criteria's field
     * @param matchesOut Receives a copy of every matching record, when given
     */
    static SearchStats sentinelLinearSearch(PatientList& list, const SearchCriteria& criteria, DataOrder order,
                                            PatientList* matchesOut = nullptr) {
        SearchStats stats;
        stats.auxBytes = SENTINEL_AUX_BYTES;
        stats.sentinelBytes = sizeof(Node);

        Node* sentinel = list.attachSentinel(criteria.sentinelRecord());

        bool groupReached = false;      // Has the first matching record been found yet?
        const Node* current = list.getHead();
        while (true) {
            ++stats.comparisons;
            if (!criteria.matches(current->data)) {
                if (order == DataOrder::Sorted && groupReached) {
                    break;                      // The group of matches has ended
                }
                current = current->next;        // No end-of-list test: the sentinel ahead always matches
            } else {
                // A match is either a real record or the sentinel that marks the end
                ++stats.endChecks;
                if (current == sentinel) {
                    break;                      // Every real record has been checked
                }
                groupReached = true;
                recordMatch(current, stats, matchesOut);
                current = current->next;
            }
        }

        list.detachSentinel(sentinel);
        return stats;
    }

    // Runs the chosen algorithm, so that code comparing the algorithms can treat them the same way
    static SearchStats search(SearchAlgorithm algorithm, PatientList& list, const SearchCriteria& criteria,
                              DataOrder order, PatientList* matchesOut = nullptr) {
        if (algorithm == SearchAlgorithm::SentinelLinear) {
            return sentinelLinearSearch(list, criteria, order, matchesOut);
        }
        return linearSearch(list, criteria, order, matchesOut);
    }

private:
    // linearSearch: the current pointer and the groupReached flag
    static constexpr size_t LINEAR_AUX_BYTES = sizeof(Node*) + sizeof(bool);
    // sentinelLinearSearch: the current and sentinel pointers, and the groupReached flag
    static constexpr size_t SENTINEL_AUX_BYTES = 2 * sizeof(Node*) + sizeof(bool);

    static void recordMatch(const Node* node, SearchStats& stats, PatientList* matchesOut) {
        ++stats.matchCount;
        if (matchesOut != nullptr) {
            matchesOut->push_back(node->data);
        }
    }
};

#endif
