#include "Searching.hpp"
#include <chrono>
#include <stdexcept>

namespace {
    // Auxiliary-memory constants, using the same rule as Sorting.cpp:
    // count working pointers, indices, flags, and temporary objects.
    // Exclude pass-through parameters (query, comparison counters), instrumentation
    // (the timing loop), and dynamic string storage.

    // Scan workspace: data and result-buffer pointers; n, i, found counters
    constexpr size_t SCAN_VARIABLE_BYTES = sizeof(Patient*) + sizeof(size_t*) + 3 * sizeof(size_t);

    // Age group boundaries in the same order as AGE_GROUP_NAMES (Task 4a)
    const int AGE_GROUP_MIN[NUM_AGE_GROUPS] = {0, 18, 26, 46, 61};
    const int AGE_GROUP_MAX[NUM_AGE_GROUPS] = {17, 25, 45, 60, 100};

    // Linear Search core: the bound check (i < n) runs before every record
    size_t linearScan(const Patient* data, size_t n, const SearchQuery& query, size_t* out,
                      uint64_t& keyComparisons, uint64_t& boundChecks) {
        size_t found = 0;
        for (size_t i = 0;; ++i) {
            ++boundChecks;
            if (i >= n) {
                break;
            }
            ++keyComparisons;
            if (query.matches(data[i])) {
                out[found++] = i;
            }
        }
        return found;
    }

    // Sentinel Linear Search core: data[n] holds a record that always matches, so the inner
    // loop is guaranteed to stop without a bound check. The bound is only checked after each match.
    size_t sentinelScan(const Patient* data, size_t n, const SearchQuery& query, size_t* out,
                        uint64_t& keyComparisons, uint64_t& boundChecks) {
        size_t found = 0;
        size_t i = 0;
        while (true) {
            ++keyComparisons;
            while (!query.matches(data[i])) {
                ++i;
                ++keyComparisons;
            }
            ++boundChecks;
            if (i == n) {
                break; // Stopped on the sentinel, not on a real record
            }
            out[found++] = i++;
        }
        return found;
    }

    // Median via insertion sort (the number of batches is tiny)
    double median(double values[], int count) {
        for (int i = 1; i < count; ++i) {
            double key = values[i];
            int j = i - 1;
            while (j >= 0 && values[j] > key) {
                values[j + 1] = values[j];
                --j;
            }
            values[j + 1] = key;
        }
        return values[count / 2];
    }

    // Records the comparison counts from one run, then times repeated runs in batches
    template <typename Scan>
    void measure(Scan scan, SearchMetrics& m) {
        m.matches = scan(m.keyComparisons, m.boundChecks);

        uint64_t ignoredKeys = 0;
        uint64_t ignoredBounds = 0;
        volatile size_t sink = 0; // Stops the compiler from optimising the timed loops away

        // Untimed warm-up: lets the CPU clock speed and caches settle before measuring
        auto warmUpEnd = std::chrono::high_resolution_clock::now()
                         + std::chrono::milliseconds(SearchEngine::WARM_UP_MILLISECONDS);
        while (std::chrono::high_resolution_clock::now() < warmUpEnd) {
            sink = sink + scan(ignoredKeys, ignoredBounds);
        }

        double batchAverages[SearchEngine::TIMING_BATCHES];
        for (int b = 0; b < SearchEngine::TIMING_BATCHES; ++b) {
            auto start = std::chrono::high_resolution_clock::now();
            for (int r = 0; r < SearchEngine::RUNS_PER_BATCH; ++r) {
                sink = sink + scan(ignoredKeys, ignoredBounds);
            }
            auto end = std::chrono::high_resolution_clock::now();
            batchAverages[b] = std::chrono::duration<double, std::micro>(end - start).count()
                               / SearchEngine::RUNS_PER_BATCH;
        }
        m.avgTimeMicrosec = median(batchAverages, SearchEngine::TIMING_BATCHES);
    }

    // Copies the matching records (by index) into the caller's array, if one was given
    void copyMatches(const PatientArray& arr, const size_t* indices, size_t count, PatientArray* outMatches) {
        if (outMatches == nullptr) {
            return;
        }
        outMatches->clear();
        for (size_t i = 0; i < count; ++i) {
            outMatches->push_back(arr[indices[i]]);
        }
    }
}

SearchQuery SearchQuery::byAgeGroup(int ageGroupIndex) {
    if (ageGroupIndex < 0 || ageGroupIndex >= NUM_AGE_GROUPS) {
        throw std::out_of_range("Invalid age group index in SearchQuery::byAgeGroup()");
    }
    SearchQuery q;
    q.field = SearchField::AgeRange;
    q.minAge = AGE_GROUP_MIN[ageGroupIndex];
    q.maxAge = AGE_GROUP_MAX[ageGroupIndex];
    q.label = "Age " + std::to_string(q.minAge) + "-" + std::to_string(q.maxAge);
    return q;
}

SearchQuery SearchQuery::byCareTypes(const bool selected[NUM_CARE_TYPES]) {
    SearchQuery q;
    q.field = SearchField::CareType;
    for (int c = 0; c < NUM_CARE_TYPES; ++c) {
        q.careTypes[c] = selected[c];
        if (selected[c]) {
            if (!q.label.empty()) {
                q.label += " or ";
            }
            q.label += CARE_TYPE_NAMES[c];
        }
    }
    return q;
}

SearchQuery SearchQuery::byStayAbove(int hours) {
    SearchQuery q;
    q.field = SearchField::StayAbove;
    q.stayThreshold = hours;
    q.label = "Stay > " + std::to_string(hours) + " hrs";
    return q;
}

SearchQuery SearchQuery::byCostAbove(int myr) {
    SearchQuery q;
    q.field = SearchField::CostAbove;
    q.costThreshold = myr;
    q.label = "Total Cost > " + std::to_string(myr) + " MYR";
    return q;
}

bool SearchQuery::matches(const Patient& p) const {
    switch (field) {
        case SearchField::AgeRange:
            return p.age >= minAge && p.age <= maxAge;
        case SearchField::CareType: {
            int c = careTypeIndex(p.careType);
            return c >= 0 && careTypes[c];
        }
        case SearchField::StayAbove:
            return p.lengthOfStay > stayThreshold;
        case SearchField::CostAbove:
            return p.calculateTotalCost() > costThreshold;
    }
    return false;
}

Patient SearchQuery::makeSentinel() const {
    Patient sentinel;
    sentinel.patientId = "SENTINEL";
    sentinel.age = minAge;                      // Satisfies AgeRange
    sentinel.lengthOfStay = stayThreshold + 1;  // Satisfies StayAbove
    sentinel.baseCostPerHour = costThreshold + 1.0;
    sentinel.daysVisitsPerYear = 1;             // Satisfies CostAbove: cost >= 1 hr x (threshold + 1) x 1
    for (int c = 0; c < NUM_CARE_TYPES; ++c) {  // Satisfies CareType
        if (careTypes[c]) {
            sentinel.careType = CARE_TYPE_NAMES[c];
            break;
        }
    }
    return sentinel;
}

SearchMetrics SearchEngine::linearSearch(const PatientArray& arr, const SearchQuery& query,
                                         const std::string& dataOrder, PatientArray* outMatches) {
    SearchMetrics m;
    m.dataStructure = "Dynamic Array";
    m.algorithmName = "Linear Search";
    m.dataOrder = dataOrder;
    m.timeComplexity = "O(n)";
    m.memoryFootprintBytes = arr.getMemoryUsageBytes(); // Container object plus allocated patient slots

    const Patient* data = arr.rawData();
    size_t n = arr.size();
    size_t bufferSlots = n > 0 ? n : 1;
    size_t* indices = new size_t[bufferSlots]; // Worst case: every record matches

    // Workspace: scan variables + result index buffer
    m.auxiliaryMemoryBytes = SCAN_VARIABLE_BYTES + bufferSlots * sizeof(size_t);

    measure([&](uint64_t& keys, uint64_t& bounds) {
        return linearScan(data, n, query, indices, keys, bounds);
    }, m);

    copyMatches(arr, indices, m.matches, outMatches);
    delete[] indices;
    return m;
}

SearchMetrics SearchEngine::sentinelLinearSearch(PatientArray& arr, const SearchQuery& query,
                                                 const std::string& dataOrder, PatientArray* outMatches) {
    SearchMetrics m;
    m.dataStructure = "Dynamic Array";
    m.algorithmName = "Sentinel Linear Search";
    m.dataOrder = dataOrder;
    m.timeComplexity = "O(n)";
    m.memoryFootprintBytes = arr.getMemoryUsageBytes(); // Container object plus allocated patient slots

    const Patient sentinel = query.makeSentinel();
    if (!query.matches(sentinel)) {
        // Without a matching sentinel the scan would run past the end of the array
        throw std::invalid_argument("Sentinel record does not satisfy the search query");
    }

    // The sentinel needs one free slot after the last record; grow (2x) only if the array is full
    size_t n = arr.size();
    size_t growthBytes = 0;
    if (arr.getCapacity() <= n) {
        size_t oldCapacity = arr.getCapacity();
        arr.reserve(oldCapacity == 0 ? 16 : oldCapacity * 2);
        growthBytes = (arr.getCapacity() - oldCapacity) * sizeof(Patient);
        m.arrayGrew = true;
    }

    Patient* data = arr.rawData();
    size_t bufferSlots = n > 0 ? n : 1;
    size_t* indices = new size_t[bufferSlots];

    // Workspace: scan variables + result index buffer + the temporary sentinel record.
    // The sentinel's slot lies in spare capacity that Mem Footprint already counts;
    // if the array was full, the capacity it had to add is counted instead.
    m.auxiliaryMemoryBytes = SCAN_VARIABLE_BYTES + bufferSlots * sizeof(size_t) + sizeof(Patient) + growthBytes;

    measure([&](uint64_t& keys, uint64_t& bounds) {
        // Placed on every search so its cost is part of the timing. Slot n lies beyond size(),
        // so the array's records are unchanged.
        data[n] = sentinel;
        return sentinelScan(data, n, query, indices, keys, bounds);
    }, m);

    copyMatches(arr, indices, m.matches, outMatches);
    delete[] indices;
    return m;
}

SearchComparison SearchEngine::runComparison(PatientArray& unsorted, PatientArray& sorted,
                                             const std::string& sortedLabel, const SearchQuery& query,
                                             PatientArray* outMatches) {
    SearchComparison result;
    result.query = query;
    result.linearUnsorted = linearSearch(unsorted, query, "Unsorted");
    result.linearSorted = linearSearch(sorted, query, sortedLabel, outMatches);
    result.sentinelUnsorted = sentinelLinearSearch(unsorted, query, "Unsorted");
    result.sentinelSorted = sentinelLinearSearch(sorted, query, sortedLabel);
    return result;
}
