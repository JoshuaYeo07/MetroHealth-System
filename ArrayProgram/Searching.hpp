#ifndef SEARCHING_HPP
#define SEARCHING_HPP

#include "Analysis.hpp"
#include "PatientArray.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @enum SearchField
 * @brief Search criteria supported in Task 7a.
 */
enum class SearchField {
    AgeRange,   // minAge <= age <= maxAge (e.g. 61-100)
    CareType,   // care type is one of the selected types (e.g. Emergency or Rehabilitation)
    StayAbove   // lengthOfStay > stayThreshold (e.g. > 24 hours)
};

/**
 * @struct SearchQuery
 * @brief Describes which patient records a search should return.
 */
struct SearchQuery {
    SearchField field = SearchField::AgeRange;
    int minAge = 0;                       // AgeRange: lower bound (inclusive)
    int maxAge = 0;                       // AgeRange: upper bound (inclusive)
    bool careTypes[NUM_CARE_TYPES] = {};  // CareType: true for each selected care type
    int stayThreshold = 0;                // StayAbove: visit duration in hours (exclusive)
    std::string label;                    // Short description for tables, e.g. "Age 61-100"

    static SearchQuery byAgeGroup(int ageGroupIndex);
    static SearchQuery byCareTypes(const bool selected[NUM_CARE_TYPES]);
    static SearchQuery byStayAbove(int hours);

    /**
     * @brief Tests one patient record against the query (counted as one key comparison).
     */
    bool matches(const Patient& p) const;

    /**
     * @brief Builds a record that always satisfies the query, used as the sentinel.
     */
    Patient makeSentinel() const;
};

/**
 * @struct SearchMetrics
 * @brief Empirical metrics recorded during a searching experiment (Task 7c).
 */
struct SearchMetrics {
    std::string algorithmName;        // "Linear Search" or "Sentinel Linear Search"
    std::string dataOrder;            // "Unsorted", "Sorted (Age)" or "Sorted (Stay)"
    size_t matches = 0;               // Number of records that satisfied the query
    uint64_t keyComparisons = 0;      // Records tested against the query (including the sentinel)
    uint64_t boundChecks = 0;         // End-of-array checks (i < n or i == n)
    double avgTimeNanosec = 0.0;      // Time per search: median of TIMING_BATCHES batch averages
    size_t memoryFootprintBytes = 0;  // Searched array buffer: capacity * sizeof(Patient)
    size_t extraMemoryBytes = 0;      // Result index list (+ sentinel slot for Sentinel Linear Search)
    bool arrayGrew = false;           // True if the array had to grow to make room for the sentinel

    uint64_t totalComparisons() const { return keyComparisons + boundChecks; }
};

/**
 * @struct SearchComparison
 * @brief Bundles both algorithms' results on unsorted and sorted data for one query.
 */
struct SearchComparison {
    SearchQuery query;
    SearchMetrics linearUnsorted;
    SearchMetrics linearSorted;
    SearchMetrics sentinelUnsorted;
    SearchMetrics sentinelSorted;
};

/**
 * @class SearchEngine
 * @brief Zero-STL searching implementations and benchmark harness for Task 7.
 *
 * Both algorithms test every record, so they work on unsorted and sorted data alike.
 */
class SearchEngine {
public:
    // A single search over 600 records takes only microseconds, so searches are timed in batches
    // after an untimed warm-up; the median batch average ignores batches slowed by background activity
    static constexpr int WARM_UP_MILLISECONDS = 50;
    static constexpr int TIMING_BATCHES = 7;
    static constexpr int RUNS_PER_BATCH = 200;

    /**
     * @brief Linear Search: tests every record and checks the array bound before each one.
     * Time Complexity: O(n). Space Complexity: O(1) auxiliary (plus the result list).
     * @param outMatches Optional array that receives copies of the matching records
     */
    static SearchMetrics linearSearch(const PatientArray& arr, const SearchQuery& query,
                                      const std::string& dataOrder, PatientArray* outMatches = nullptr);

    /**
     * @brief Sentinel Linear Search: copies a matching record into the free slot after the last
     * record, so the inner loop needs no bound check. Grows the array first if it is full.
     * Time Complexity: O(n). Space Complexity: O(1) auxiliary (one sentinel slot, plus the result list).
     * @param outMatches Optional array that receives copies of the matching records
     */
    static SearchMetrics sentinelLinearSearch(PatientArray& arr, const SearchQuery& query,
                                              const std::string& dataOrder, PatientArray* outMatches = nullptr);

    /**
     * @brief Runs both algorithms on the unsorted and sorted versions of the same dataset.
     * @param sortedLabel Data order label for the sorted runs, e.g. "Sorted (Age)"
     * @param outMatches Optional array that receives the matching records in sorted order
     */
    static SearchComparison runComparison(PatientArray& unsorted, PatientArray& sorted,
                                          const std::string& sortedLabel, const SearchQuery& query,
                                          PatientArray* outMatches = nullptr);
};

#endif // SEARCHING_HPP
