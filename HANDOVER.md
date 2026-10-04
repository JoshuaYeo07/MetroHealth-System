# Handover: getting unsorted and sorted patient data (linked list program)

For: whoever is doing **Task 7 (Searching Experiments)** on the linked list side.
Everything below is in `LinkedListProgram/`, branch `feat/jack-linked-list`.

For a full explanation of how the sorting works, see `SORTING_FEATURE_EXPLAINED.md`.

---

## 1. How to get sorted data

`PatientSorter.hpp` has `sortedCopy`. It deep-copies a list, sorts the copy and returns it. **The original is not changed.**

```cpp
static PatientList sortedCopy(const PatientList& source, SortField field, bool ascending,
                              SortAlgorithm algorithm = SortAlgorithm::Merge);
```

| Parameter | Options |
|---|---|
| `field` | `SortField::Age`, `SortField::VisitDuration` (length of stay, hours), `SortField::TotalCost` |
| `ascending` | `true` = lowest first, `false` = highest first |
| `algorithm` | `SortAlgorithm::Merge` (default, O(n log n)) or `SortAlgorithm::Bubble` (O(n^2)) |

Examples:

```cpp
PatientList byAge          = PatientSorter::sortedCopy(*dataset, SortField::Age, true);
PatientList byStay         = PatientSorter::sortedCopy(*dataset, SortField::VisitDuration, true);
PatientList byCost         = PatientSorter::sortedCopy(*dataset, SortField::TotalCost, false);   // highest cost first
PatientList byBubbleSort   = PatientSorter::sortedCopy(*dataset, SortField::Age, true, SortAlgorithm::Bubble);
```

Both algorithms give the **identical order** (same rules, both stable), so the choice only changes how long sorting takes, not the data.

### Which sort field for which search (7a)

| Search | Sort the data by |
|---|---|
| Age group (e.g. 61-100) | `SortField::Age` |
| Visit duration threshold (e.g. > 24 hours) | `SortField::VisitDuration` |
| Specific care type (e.g. Emergency) | **not available yet**, see section 6 |

### If you also want the sort's own numbers (time, comparisons...)

Use the algorithms directly on your own copy. They return a `SortStats`:

```cpp
PatientList copy = *dataset;                       // your own deep copy
SortStats stats = PatientSorter::mergeSort(copy,
    PatientSorter::comparatorFor(SortField::Age, true));
// copy  -> now sorted (it is changed in place, because mergeSort takes PatientList&)
// stats -> stats.elapsedMs, stats.comparisons, stats.moves, stats.maxDepth, stats.auxBytes
```

`bubbleSort` works the same way.

---

## 2. Rules to avoid breaking things

1. **Don't call `mergeSort` / `bubbleSort` directly on `allPatients` or `facility->patients`.** They sort in place, so every other report would suddenly see sorted data. Sort a copy (`sortedCopy` does this for you).
2. **Each sorted copy costs one extra list of memory** (about `list.getMemoryUsageBytes()`). That is real overhead and is worth reporting in 7c. Make the copy inside your function so it is freed when the function returns, instead of keeping many alive.
3. **Take lists as `const PatientList&`** in your search functions. It avoids copying and stops accidental changes.
4. `PatientSorter` only works on `PatientList` (linked list). It cannot sort `PatientArray`.

---