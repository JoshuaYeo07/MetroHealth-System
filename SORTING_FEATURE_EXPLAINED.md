# Bubble Sort vs Merge Sort: what was added and how it works

This document explains the sorting feature added to `LinkedListProgram`. It is written for someone who is still learning C++, so each piece is explained in plain words, with analogies, and the C++ ideas it relies on are collected at the end.

The code has **not been compiled** yet (no compiler was available when it was written). The two sorting algorithms were checked by porting their logic line for line to Python and testing them on 3000 random lists, where both matched a reference stable sort. Build it in Visual Studio and run menu option `[4]` before relying on it.

---

## 1. The big picture

### What the feature does

From the main menu, option `[4] Sorting & Performance Analysis` lets you:

1. Pick a dataset (Facility A, B, C, or all combined).
2. Pick what to sort by: **Age**, **Visit Duration** (length of stay), **Total Medical Cost**, **Care Type**, or all of them.
3. Pick the order: ascending (lowest first) or descending (highest first).

The program then sorts a **copy** of the chosen patient list with **bubble sort** and again with **merge sort**, and prints text tables comparing them: the sorted records, how long each took, how much work each did, how much memory each needed, and the theoretical time complexity.

This covers the assignment items:

- a. Sort datasets by Age, Visit Duration, or Total Medical Cost
- c. Compare time complexity and memory usage between sorting methods
- d. Display sorting performance results as text-based tables in the console

### The train analogy

A `PatientList` is a **train**. Each carriage (`Node`) holds one patient and a coupling to the next carriage (`next`). You can only walk the train from the front.

Sorting means putting the carriages in a new order. Both algorithms do it by **re-coupling** the carriages (changing the `next` pointers). The patients are never copied or moved in memory.

- **Bubble sort:** walk along the train. Whenever two neighbouring carriages are in the wrong order, swap them. Repeat the walk until a whole walk finds nothing to swap.
- **Merge sort:** split the train in half, then split each half, and so on until every piece is a single carriage. Then join the pieces back together two at a time, always picking whichever front carriage should come first, like zipping two sorted queues into one.

### How the pieces fit

```
main.cpp (menu)
   |  user picks dataset, field, order
   v
SortingAnalysis (runs both sorts on copies, measures, prints tables)
   |  asks for a comparison rule + calls bubbleSort / mergeSort
   v
PatientSorter (the two algorithms, the comparison rules, the stats)
   |  uses getHead() and relink()
   v
PatientList (the linked list that already existed)
```

### Files

| File | New or edited | Role |
|---|---|---|
| `PatientSorter.hpp` | new | The two sorting algorithms, the comparison rules, `SortField`, `SortStats` |
| `SortingAnalysis.hpp` | new | Declares the two report functions |
| `SortingAnalysis.cpp` | new | Runs the comparison and prints all the tables |
| `main.cpp` | edited | New menu option, dataset picker helper, sorting menu |
| `LinkedListProgram.vcxproj` and `.filters` | edited | Register the new files so Visual Studio builds them |

`PatientList` itself was **not changed**. The sort uses two functions that were already there: `getHead()` (get the first node) and `relink(newHead)` (tell the list "this is the new first node, recompute `tail` and `count`").

---

## 2. `PatientSorter.hpp`: the sorting itself

This file is header-only (everything is defined inside the header), like `CareTypeStats.hpp` and `DataLoader.hpp`.

### 2.1 `enum class SortField`

```cpp
enum class SortField { Age, VisitDuration, TotalCost, CareType };
```

A named choice of what to sort by. Using an enum instead of the numbers 1, 2, 3 makes the code readable: `SortField::Age` says what it means.

### 2.2 `struct SortStats`

A small bundle of numbers describing what one sorting run did. It is a `struct` because it is just data (the same reason `CostSummary` is a struct).

| Field | Meaning |
|---|---|
| `elapsedMs` | How long the sort took, in milliseconds |
| `comparisons` | How many times two patients were compared |
| `moves` | How many nodes were re-coupled (see below) |
| `maxDepth` | Deepest level of recursion reached (0 for bubble sort, which uses a loop) |
| `auxBytes` | Estimated extra memory the algorithm needed beyond the list itself |

The constructor sets everything to 0, so a fresh `SortStats` starts clean.

What counts as a "move" differs between the algorithms, and this is stated under the performance table:

- Bubble sort: one swap of two neighbouring nodes = 1 move.
- Merge sort: one node placed into the merged output = 1 move (plus 1 when the leftover tail of a chain is attached in one step).

### 2.3 `class PatientSorter`

All members are `static`, so you never create a `PatientSorter` object. You call `PatientSorter::bubbleSort(...)` directly, like a toolbox.

#### The two type aliases

```cpp
using Node = PatientList::Node;
using Comparator = bool (*)(const Node*, const Node*);
```

- `Node` is just a shorter name for `PatientList::Node`.
- `Comparator` is a **function pointer type**: "a function that takes two node pointers and returns a bool". It is the same pattern `CareTypeStatList::sortBy` already uses.

**Analogy:** the sorting algorithm is a referee who needs a rulebook that answers one question: "should A come before B?" The rulebook is passed in. Change the rulebook (age, stay, cost, ascending, descending) and the same referee sorts a different way. Bubble sort and merge sort get **the same rulebook**, so the only thing that differs between runs is the algorithm. That makes the comparison fair.

#### The six comparison rules

```cpp
static bool ageAscending(const Node* a, const Node* b)  { return a->data.age < b->data.age; }
static bool ageDescending(const Node* a, const Node* b) { return a->data.age > b->data.age; }
// likewise stayAscending/stayDescending and costAscending/costDescending
```

Each returns `true` when `a` must come **strictly before** `b`.

- "Strictly" means `<`, not `<=`. If two patients are equal, neither comes before the other.
- That is what makes both sorts **stable**: patients that tie keep the order they had before (for example two patients of the same age stay in file order).
- `costAscending` and `costDescending` call `calculateTotalCost()` each time, because the cost is not stored in the patient. It is computed from stay x rate x visits.

#### `comparatorFor(field, ascending)`

Picks the right one of the six rules for a field and direction and returns it as a function pointer. This is how the menu choice ("by cost, descending") turns into the rulebook handed to the sorts.

#### `fieldName(field)`

Returns a display name ("Age", "Visit Duration (Length of Stay)", "Total Medical Cost") used in table headings.

#### `isSorted(list, comesBefore)`

A correctness checker. It walks the list once and returns `false` if any node comes strictly before the node ahead of it. The analysis code calls it after each sort and shows the result in the "Sorted OK" column, so a bug would show up as `NO` instead of silently printing wrong data.

---

### 2.4 `bubbleSort(list, comesBefore)`

**Idea:** sweep the chain from the front. Compare each pair of neighbours; if the second should come before the first, swap them. After one sweep the "best" remaining node has bubbled to the end of the unsorted part. Repeat until a sweep makes no swaps.

#### The variables

| Variable | Meaning |
|---|---|
| `head` | First node of the chain (changes if the first two nodes swap) |
| `lastSorted` | Everything from this node to the end is already in its final position, so sweeps stop before it |
| `swapped` | Did this sweep swap anything? If not, we are done |
| `previous` | The node just before `current`, needed to re-couple when swapping |
| `current`, `next` | The pair of neighbours being compared |

#### The swap, done by re-coupling

```
before:  previous -> current -> next -> rest
after:   previous -> next -> current -> rest
```

In code:

```cpp
current->next = next->next;   // current now points past next
next->next = current;         // next now points back to current
if (previous == nullptr) head = next; else previous->next = next;
previous = next;              // current has not moved, next is now behind it
```

Nothing is copied. Only `next` pointers change. After a swap, `current` stays the same node (it moved one place later), so it can be compared with its new neighbour straight away.

#### Why the sweeps get shorter

`lastSorted = current;` after each sweep marks the end of the walk as finished. The inner loop is `while (current->next != lastSorted)`, so each sweep stops one node earlier than the one before.

#### Early exit

If a whole sweep swaps nothing, the list is already in order and the outer `while (swapped)` ends. On an already-sorted list that means a single sweep, which is why the best case is O(n).

#### The finish

`list.relink(head)` hands the new first node back to the list, which recomputes `tail` and `count`. The timer covers all of it.

#### Cost

- Time: O(n) best case (already sorted), O(n^2) average and worst case. For n records it makes about n x n / 2 comparisons.
- Extra memory: O(1). A handful of pointers and one bool, estimated as `4 * sizeof(Node*) + sizeof(bool)` (33 bytes on a 64-bit build).

---

### 2.5 `mergeSort(list, comesBefore)` and its helpers

**Idea (divide and conquer):** a list of 0 or 1 nodes is already sorted. Otherwise cut it in half, sort each half the same way, then merge the two sorted halves into one sorted chain.

`mergeSort` itself is a short wrapper: start the timer, call `mergeSortChain` on the whole list, call `relink`, stop the timer, and fill in the memory estimate.

#### `mergeSortChain(head, comesBefore, stats, depth)` (private)

The recursive part. It takes the first node of a chain and returns the first node of the same chain, sorted.

1. Record how deep the recursion is (`depth`), keeping the maximum in `stats.maxDepth`.
2. **Base case:** if the chain has 0 or 1 nodes, return it unchanged.
3. **Find the middle** with a *slow and a fast pointer*. `fast` moves two nodes for every one that `slow` moves, so when `fast` reaches the end, `slow` is in the middle. (A linked list cannot jump to the middle by index, so this trick avoids counting first.)
4. **Cut** the chain: `second = slow->next; slow->next = nullptr;` now there are two separate chains.
5. Sort both halves by calling itself (with `depth + 1`).
6. Merge the two sorted halves with `mergeChains` and return the result.

#### `mergeChains(a, b, comesBefore, stats)` (private)

Takes two already-sorted chains and zips them into one.

**Analogy:** two queues, each sorted. Look at the person at the front of each queue and let the one who should go first through. Repeat until one queue is empty. Then everyone left in the other queue just follows.

In code:

- `mergedHead` and `mergedTail` build the output chain.
- While both chains have nodes: `if (comesBefore(b, a))` take from `b`, otherwise take from `a`. Taking from `a` on a tie keeps the sort stable.
- When one chain runs out, the rest of the other chain is already in order, so it is attached in a single step (counted as 1 move).

#### Cost

- Time: O(n log n) in every case. The list is halved about log2(n) times, and each level of merging touches every node once.
- Extra memory: O(log n), only the recursion stack. No nodes are allocated, because merging just re-couples existing nodes.
- The estimate is `maxDepth * MERGE_FRAME_BYTES + MERGE_STEP_BYTES`. For about 600 records the recursion goes roughly 11 levels deep, which is about 576 bytes on a 64-bit build.

#### The three private constants and `elapsedSince`

```cpp
BUBBLE_AUX_BYTES   = 4 * sizeof(Node*) + sizeof(bool)
MERGE_FRAME_BYTES  = 6 * sizeof(void*)     // one recursive call
MERGE_STEP_BYTES   = 6 * sizeof(void*)     // the merge call at the bottom
```

These are **estimates** built from the sizes of the variables each algorithm keeps, not measurements of the real stack. They are good enough to show the pattern (constant vs grows with log n) but are not exact byte counts, and the report says so.

`elapsedSince(start)` converts "now minus start" from the `<chrono>` clock into milliseconds.

### 2.6 `sortedCopy(source, field, ascending, algorithm)`

A convenience function for other parts of the program (for example a teammate's search experiment). It makes a deep copy of a list, sorts the copy and returns it. The original list is left unchanged, so the unsorted and sorted versions can be used side by side.

```cpp
PatientList byAge  = PatientSorter::sortedCopy(allPatients, SortField::Age, true);                         // merge sort (default)
PatientList byAge2 = PatientSorter::sortedCopy(allPatients, SortField::Age, true, SortAlgorithm::Bubble);  // bubble sort
```

- `SortAlgorithm` is a small enum (`Bubble` or `Merge`). Merge is the default because it is much faster; both give the identical order.
- **Memory cost:** every call allocates one extra list (about `source.getMemoryUsageBytes()`). The `const &` parameter only avoids copying the argument; the copy inside is deliberate. To sort without the extra list, call `mergeSort(list, ...)` directly on the list itself.
- The sorted data is **not stored anywhere** by the program. Each caller keeps its own sorted copy.

---

## 3. `SortingAnalysis.cpp` and `.hpp`: running the experiment and printing it

### 3.1 The two public functions (declared in the `.hpp`)

#### `printSortingReport(patients, datasetTitle, field, ascending)`

The report for one field. It prints, in order:

1. A banner with the dataset name.
2. The **first 10 sorted records**, using the existing `TablePrinter::displayPatientTable`.
3. The **performance table**.
4. The **memory table**.
5. The **time complexity table**.

If the dataset is empty it prints a notice and stops.

#### `printAllFieldsReport(patients, datasetTitle, ascending)`

Runs the comparison for all four fields (age, stay, cost, care type). For each field it prints the performance and memory tables, then prints the complexity table once at the end. It skips the sample records to keep the output short.

Both take `const PatientList&`, so the original list is never changed. Each algorithm sorts its own copy.

### 3.2 Helpers inside the `.cpp` (in an anonymous namespace)

The `namespace { ... }` block makes these helpers private to this file, like `static` functions.

#### `REPEATS = 20`

Sorting 200 to 600 records takes a tiny fraction of a millisecond, too short to time reliably in one go. Each algorithm is run 20 times and the times are averaged.

#### `struct Comparison`

Everything about one head-to-head run: both algorithms' `SortStats`, whether each result was correctly sorted, the record count, the list's memory footprint, and the sorted list itself (used for the sample table).

#### `measure(sort, source, comesBefore, sortedOut)`

Runs one algorithm 20 times:

1. Make a fresh deep copy of the source list (`PatientList working = source;`). This uses the Rule of 5 copy constructor. The copy is made **before** the sort starts its own timer, so copying is not timed.
2. Run the sort on the copy and add its time to a running total.
3. Keep the stats of the last run (comparisons and moves are the same every run).
4. On the last run, move the sorted list out through `sortedOut` (`std::move`, so no copying).
5. Return the stats with `elapsedMs` replaced by the average.

The `sort` parameter is a function pointer, so the same `measure` works for both `bubbleSort` and `mergeSort`.

Why fresh copies: if bubble sort ran twice on the same list, the second run would be on already-sorted data and would be unrealistically fast.

#### `compare(patients, field, ascending)`

1. Gets the comparison rule from `comparatorFor`.
2. Runs `measure` for bubble sort, then for merge sort, on the same data with the same rule.
3. Checks each result with `isSorted` and confirms no record was lost (`size()` equals the original).
4. Records the list's memory footprint using the existing `getMemoryUsageBytes()`.

#### `orderName(ascending)`

Returns "ascending" or "descending" for headings.

#### Table helpers: `tableWidth`, `printRow`, `LEFT_ALIGN_ALL`

- `tableWidth(widths)` adds up the column widths (plus the `| ` and ` ` around each cell) to get the full line width, so the `+-----+` border lines up with the rows.
- `printRow(cells, widths, rightAlignFrom)` prints one row. Columns from `rightAlignFrom` onwards are right-aligned (numbers); earlier ones are left-aligned (text). Text longer than its column is cut with `...` using the existing `TablePrinter::fit`.
- `LEFT_ALIGN_ALL` is passed when every column should be left-aligned (the complexity table is all text).

These two are **templates** (`template <size_t N>`). That just lets the same function work for rows of any number of columns: the compiler works out N from the array you pass in, and if the cells array and the widths array have different lengths it refuses to compile, which catches typos.

### 3.3 The three tables

#### Performance table (`printPerformanceTable`)

| Column | Meaning |
|---|---|
| Algorithm | Bubble Sort or Merge Sort |
| Records | How many patients were sorted |
| Avg Time (ms) | Average of 20 runs |
| Comparisons | How many times two patients were compared |
| Node Moves | Re-couplings, defined in section 2.2 |
| Sorted OK | YES if the result passed `isSorted` and kept every record |

Under it: a note on the 20-run average, what a move means, and one line saying which algorithm was faster and by how many times.

How to read it: bubble sort's comparison count is large (about n^2 / 2) and merge sort's is small (about n log n). On 600 records that is hundreds of thousands against a few thousand. Merge sort's node moves are also far fewer than bubble sort's swaps.

#### Memory table (`printMemoryTable`)

| Column | Meaning |
|---|---|
| List Footprint (B) | Memory the list itself uses: `sizeof(PatientList) + count * sizeof(Node)`. Identical for both, because sorting only re-couples nodes |
| Extra Nodes | Nodes allocated by the sort: 0 for both |
| Max Depth | Deepest recursion level (0 for bubble sort) |
| Aux Memory (B) | The estimate from section 2.5 |
| Peak Total (B) | List footprint + aux memory |

The honest conclusion this table shows: **neither algorithm needs significant extra memory on a linked list.** Bubble sort uses a constant handful of pointers. Merge sort uses a recursion stack that grows slowly (log n). The list itself dominates the total. Merge sort's disadvantage in memory is real but tiny here, and on arrays it would be much larger (arrays need a temporary buffer for merging; this is a good point for the array-versus-linked-list comparison in the report).

#### Complexity table (`printComplexityTable`)

A fixed theory table:

| Algorithm | Best | Average | Worst | Extra space | Stable |
|---|---|---|---|---|---|
| Bubble Sort | O(n) (no swaps) | O(n^2) | O(n^2) | O(1) | Yes |
| Merge Sort | O(n log n) | O(n log n) | O(n log n) | O(log n) stack | Yes |

(`n` is the number of patient records.) It sits next to the measured numbers so you can show that the measurements agree with the theory.

---

## 4. `main.cpp`: the menu changes

#### New include

`#include "SortingAnalysis.hpp"` so `main.cpp` can call the report functions. `PatientSorter.hpp` comes in through it, which is how `SortField` becomes available.

#### `chooseDataset(...)` (new helper)

The "pick Facility A, B, C or all combined" menu already existed inline inside `runAgeGroupMenu`. It was pulled out into this helper so the sorting menu can reuse it instead of duplicating the code. It returns a pointer to the chosen list (and sets the title through `titleOut`), or `nullptr` if the user picked "Back".

#### `runAgeGroupMenu(...)` (edited)

Now shorter: it calls `chooseDataset`, then `printAgeGroupReport`. Behaviour is the same as before.

#### `runSortingMenu(...)` (new)

1. Pick a dataset with `chooseDataset`.
2. Pick the field: `[1]` Age, `[2]` Visit Duration, `[3]` Total Medical Cost, `[4]` Care Type (alphabetical, extra field for Task 7), `[5]` all four fields, `[0]` back.
3. Pick the order: `[1]` ascending, `[2]` descending.
4. If `[5]` was chosen, call `printAllFieldsReport`. Otherwise turn the number into a `SortField` and call `printSortingReport`.

Every prompt uses the existing `readChoice`, which re-asks on bad input and returns -1 if input ends, so the new menu is handled the same way as the old ones.

#### The main menu

```
[1] Task 3 - Dataset Storage Summary & Sample Records
[2] Task 4 - Patient Demographic & Billing Categorization
[3] Task 5 - Healthcare Expenditure & Service Analysis
[4] Sorting & Performance Analysis (Bubble Sort vs Merge Sort)   <- new
[5] Run All Reports (Tasks 3 - 5)                                 <- was [4]
[0] Exit
```

"Run All Reports" moved from 4 to 5, and `readChoice(0, 5)` replaces `readChoice(0, 4)`. The sorting option is not part of "Run All Reports" because it asks you questions.

The `[4]` menu line has no "Task N" label because the assignment sheet numbering was not available when this was written. Rename it to match your assignment.

---

## 5. One full example, step by step

You choose: `[4]`, then `[1] Dataset 1`, then `[3] Total Medical Cost`, then `[2] Descending`.

1. `runSortingMenu` collects those choices and calls `printSortingReport(list, "Dataset 1 - ...", SortField::TotalCost, false)`.
2. `compare` asks `comparatorFor(TotalCost, false)` for `costDescending`.
3. `measure` runs bubble sort 20 times, each on a fresh copy of Dataset 1's 200 patients, and averages the time.
4. `measure` does the same for merge sort, keeping the final sorted list.
5. `isSorted` verifies both results.
6. The first 10 sorted patients are printed (highest cost first), then the performance, memory and complexity tables.
7. Dataset 1's original list is untouched, so the other reports are unaffected.

---

## 6. C++ ideas used in this feature

| Idea | Where | Plain meaning |
|---|---|---|
| Function pointer (`bool (*)(const Node*, const Node*)`) | `Comparator`, `SortFunction` | A variable that holds "a function to call later". Lets one algorithm work with many rules |
| `static` member function | Everything in `PatientSorter` | Belongs to the class, not to an object. Call it as `PatientSorter::name(...)` |
| `using X = ...` | `Node`, `Comparator`, `SortFunction` | Another name for an existing type |
| `enum class` | `SortField` | A set of named choices that can't be mixed up with plain numbers |
| `struct` | `SortStats`, `Comparison` | A bundle of related data |
| `constexpr` | The memory-estimate constants | A value fixed at compile time |
| Recursion | `mergeSortChain` | A function that calls itself on a smaller piece of the problem |
| Slow/fast pointers | Finding the middle of the chain | One pointer moves twice as fast, so when it reaches the end the slow one is at the middle |
| `std::chrono::steady_clock` | `elapsedSince` | A stopwatch for measuring elapsed time |
| `std::move` | `measure` | Hand the sorted list over without copying it |
| Deep copy | `PatientList working = source;` | Uses the Rule of 5 copy constructor to make an independent copy |
| Anonymous namespace | `SortingAnalysis.cpp` | Helpers visible only inside that file |
| Template | `tableWidth`, `printRow` | One function that adapts to different array sizes |
| `const T&` parameters | Almost everywhere | Read the caller's data without copying it or changing it |

---

## 7. Things to know before you rely on the numbers

- **Not compiled yet.** Build it and fix any compile error before using it. The sorting logic itself was tested in a Python port, not in C++.
- **Times are small and noisy.** With 200 to 600 records each sort takes a fraction of a millisecond. The 20-run average steadies it, but the exact milliseconds vary by machine and between runs. Comparisons and node moves are exact and repeatable, so lean on those too. Debug and Release builds also differ a lot; for the report, run in **Release**.
- **Memory is an estimate.** The "Aux Memory" figure comes from variable sizes, not from measuring the real stack. It shows the pattern, not exact bytes.
- **Bubble sort's best case** only shows up if the data is already sorted. The datasets are not, so expect its average-case behaviour.
- **Both sorts are stable**, so patients that tie on the sort field stay in their original order, and bubble and merge sort give the identical final order.
