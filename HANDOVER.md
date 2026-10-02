# Handover: getting unsorted and sorted patient data (linked list program)

For: whoever is doing **Task 7 (Searching Experiments)** on the linked list side.
Everything below is in `LinkedListProgram/`, branch `feat/jack-linked-list`.

For a full explanation of how the sorting works, see `SORTING_FEATURE_EXPLAINED.md`.

> The sorting code has **not been compiled yet** (no compiler was available when it was written). Build in Visual Studio first. The algorithm logic itself was tested separately and gave correct stable sorts on random data.

---

## 1. TL;DR

```cpp
#include "PatientSorter.hpp"

// UNSORTED: the original lists, in CSV order (never modified by the sorting feature)
//   allPatients            -> all 600 patients from datasets 1-3 combined
//   facility->patients     -> one dataset (200 patients)

// SORTED: nothing is stored, you ask for a sorted copy
PatientList sortedByAge = PatientSorter::sortedCopy(allPatients, SortField::Age, true);
```

- `allPatients` stays unsorted. `sortedByAge` is a separate, sorted list.
- The sorted data is **not saved anywhere** (not in a file, not in a global). Each caller makes its own copy.

---

## 2. Where the unsorted data lives

In `main()` in `main.cpp`:

| Variable | What it holds | Order |
|---|---|---|
| `facilityA`, `facilityB`, `facilityC` | One dataset each. Chained: A -> B -> C through `facility->next` | CSV file order |
| `facility->patients` | That facility's `PatientList` (200 patients each) | CSV file order |
| `allPatients` | Every patient from all three datasets (a deep copy, built with `appendAll`) | A's rows, then B's, then C's, each in CSV order |

These are local variables in `main()`, so they are not global. To use them, **pass them as parameters** to your function, the same way the existing menus do:

```cpp
static void runSearchMenu(const Facility* facilities, const PatientList& allPatients);
```

and call it from a new case in the main menu `switch`.

To let the user pick a dataset, reuse the existing helper in `main.cpp`:

```cpp
std::string title;
const PatientList* dataset = chooseDataset(facilities, allPatients, "Select dataset to search:", title);
if (dataset == nullptr) return;     // user chose "Back"
// *dataset is the chosen PatientList (unsorted), title is its heading
```

---

## 3. How to get sorted data

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
PatientList byAge   = PatientSorter::sortedCopy(*dataset, SortField::Age, true);
PatientList byStay  = PatientSorter::sortedCopy(*dataset, SortField::VisitDuration, true);
PatientList byCost  = PatientSorter::sortedCopy(*dataset, SortField::TotalCost, false);   // highest cost first
PatientList slow    = PatientSorter::sortedCopy(*dataset, SortField::Age, true, SortAlgorithm::Bubble);
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

## 4. Rules to avoid breaking things

1. **Don't call `mergeSort` / `bubbleSort` directly on `allPatients` or `facility->patients`.** They sort in place, so every other report would suddenly see sorted data. Sort a copy (`sortedCopy` does this for you).
2. **Each sorted copy costs one extra list of memory** (about `list.getMemoryUsageBytes()`). That is real overhead and is worth reporting in 7c. Make the copy inside your function so it is freed when the function returns, instead of keeping many alive.
3. **Take lists as `const PatientList&`** in your search functions. It avoids copying and stops accidental changes.
4. `PatientSorter` only works on `PatientList` (linked list). It cannot sort `PatientArray`.

---

## 5. Reading a list (for your search code)

```cpp
for (const PatientList::Node* current = list.getHead(); current != nullptr; current = current->next) {
    const Patient& p = current->data;
    // p.age, p.careType, p.lengthOfStay, p.calculateTotalCost(), p.getAgeGroup() ...
}
```

- `list.size()` and `list.isEmpty()` are O(1).
- `list.at(i)` is O(n) (it walks from the head). **Don't use it inside a loop**, it turns the loop into O(n^2). Use the `getHead()` / `next` walk above.
- To print results as a table, `TablePrinter::displayPatientTable(list, "title", maxRows)` already exists. To show matches, push them into a new `PatientList` with `push_back` and pass that to it.

### Example: search unsorted vs sorted (7b)

```cpp
void runAgeSearchExperiment(const PatientList& unsorted) {
    PatientList sortedByAge = PatientSorter::sortedCopy(unsorted, SortField::Age, true);

    // your own search function, run on both so the results and the step counts can be compared
    // searchAgeRange(unsorted, 61, 100);
    // searchAgeRange(sortedByAge, 61, 100);
}
```

---

## 6. Things that are NOT done yet

- **No sort by care type.** `SortField` only has Age, VisitDuration and TotalCost. To add it: add a rule in `PatientSorter.hpp`

  ```cpp
  static bool careTypeAscending(const Node* a, const Node* b) { return a->data.careType < b->data.careType; }
  ```

  add `CareType` to `enum class SortField`, and add a case to `comparatorFor` and `fieldName`.
- **Nothing for the array program.** `PatientSorter` is linked-list only, and `ArrayProgram` has no sorting. For 7c (array vs linked list) the array side needs its own sort and search.
- **Binary search on a linked list** has no real speed-up, because a linked list has no index jump (finding the middle means walking the nodes). A sorted linked list still helps a *linear* search stop early once it passes the target. This is a useful point for the array vs linked list comparison.
- The sorting menu has no "Task N" label because the assignment numbering was not available. Rename it if needed.

---

## 7. Files to look at

| File | Why |
|---|---|
| `PatientSorter.hpp` | `sortedCopy`, `bubbleSort`, `mergeSort`, `SortField`, `SortStats` |
| `PatientList.hpp` | The list: `getHead()`, `size()`, `push_back()`, copy constructor |
| `TablePrinter.hpp` | Existing table helpers (`displayPatientTable`, `formatDecimal`, `formatMoney`, `printHorizontalLine`) |
| `main.cpp` | Menus, `chooseDataset`, where `allPatients` and the facilities are created |
| `SortingAnalysis.cpp` | A working example of timing two algorithms on fresh copies and printing text tables, a good pattern to copy for the search experiment |
