# MetroHealth System

**Comparative Analysis of Arrays and Singly Linked Lists for Sorting and Searching in a Smart Healthcare & Hospital Patient Management System**

CT077-3-2-DSTR (Data Structures) group assignment, Asia Pacific University of Technology and Innovation.

MetroHealth runs three healthcare facilities. This project loads their patient records and builds the same set of reports twice: once on a **custom dynamic array** and once on a **custom singly linked list**. Both versions sort and search the data, record how long each operation takes and how much memory it uses, and print the results as text tables. Running the two programs side by side shows where each data structure does better.

No STL containers (`<vector>`, `<list>`, `<map>`, ...) are used anywhere. Every container in the project is written from scratch, as the assignment requires.

---

## Contents

- [Quick start](#quick-start)
- [Project structure](#project-structure)
- [Datasets](#datasets)
- [Business rules](#business-rules)
- [Program 1: Array program](#program-1-array-program)
- [Program 2: Linked list program](#program-2-linked-list-program)
- [Array vs linked list at a glance](#array-vs-linked-list-at-a-glance)
- [How performance is measured](#how-performance-is-measured)
- [Assignment task coverage](#assignment-task-coverage)
- [Further documentation](#further-documentation)
- [Contributing](#contributing)
- [Submission notes](#submission-notes)

---

## Quick start

### Visual Studio (recommended)

1. Open `MetroHealth-System.slnx` in Visual Studio 2026 (platform toolset `v145`, C++20).
2. In Solution Explorer, right-click **ArrayProgram** or **LinkedListProgram** and choose **Set as Startup Project**.
3. Press **Ctrl+F5** to build and run.

The solution builds for `x64` and `x86`, in `Debug` and `Release`.

### Command line (g++)

Run these from the repository root:

```bash
g++ -std=c++20 -O2 ArrayProgram/*.cpp -o ArrayProgram.exe
```

```bash
g++ -std=c++20 -O2 LinkedListProgram/*.cpp -o LinkedListProgram.exe
```

```bash
./ArrayProgram.exe
```

```bash
./LinkedListProgram.exe
```

On Windows with MinGW/MSYS2, add `-static` to the compile commands if the program crashes on start or reports a missing DLL. This happens when another `libstdc++` is earlier on `PATH`.

Build in `Release` (or with `-O2`) for timing results. Debug builds are much slower and give misleading numbers.

### Where the program looks for the CSV files

Both programs search for the dataset files in this order, relative to the working directory:

1. `../Datasets/<file>`
2. `Datasets/<file>`
3. `../../Datasets/<file>`
4. `<file>`

Running from the repository root, or from inside `ArrayProgram/` or `LinkedListProgram/` (the Visual Studio default), works without changes. If a file cannot be found, the program prints an error with the path it tried.

---

## Project structure

```text
MetroHealth-System/
├── MetroHealth-System.slnx            Visual Studio solution (both projects)
├── Datasets/
│   ├── dataset1 facility_a.csv        Facility A: General Hospital (200 records)
│   ├── dataset2 facility_b.csv        Facility B: University Medical Center (200 records)
│   └── dataset3_facility_c.csv        Facility C: Community Health Clinic (200 records)
│
├── ArrayProgram/                      Program 1: custom dynamic array
│   ├── main.cpp                       Menus, loading, wiring of all tasks
│   ├── Patient.hpp                    Patient record, cost formula, age group
│   ├── PatientArray.hpp/.cpp          Dynamic array container (capacity doubling, Rule of 5)
│   ├── DataLoader.hpp                 CSV parser into PatientArray
│   ├── TablePrinter.hpp               Patient record table
│   ├── Analysis.hpp/.cpp              5 x 6 age-group x care-type statistics grid
│   ├── AnalysisPrinter.hpp/.cpp       Task 4 and Task 5 reports
│   ├── Sorting.hpp/.cpp               Bubble sort, merge sort, benchmark harness
│   ├── SortingPrinter.hpp/.cpp        Task 6 benchmark tables and previews
│   ├── Searching.hpp/.cpp             Linear search, sentinel linear search, timing harness
│   └── SearchingPrinter.hpp/.cpp      Task 7 result, performance and analysis tables
│
├── LinkedListProgram/                 Program 2: custom singly linked list
│   ├── main.cpp                       Menus, loading, wiring of all tasks
│   ├── Patient.hpp                    Patient record, cost formula, age group helpers
│   ├── PatientList.hpp/.cpp           Singly linked list container (head + tail, Rule of 5)
│   ├── Facility.hpp                   One dataset; facilities are chained as a linked list
│   ├── DataLoader.hpp                 CSV parser into PatientList
│   ├── TablePrinter.hpp               Banners, rules, patient record table
│   ├── CareTypeStats.hpp              CostSummary + linked list of per-care-type totals
│   ├── HealthcareAnalytics.hpp/.cpp   Task 4 and Task 5 reports
│   ├── PatientSorter.hpp              Bubble sort and merge sort by relinking nodes
│   ├── SortingAnalysis.hpp/.cpp       Task 6 performance, memory and complexity tables
│   ├── PatientSearcher.hpp            Linear search and sentinel linear search
│   └── SearchingAnalysis.hpp/.cpp     Task 7 result, performance and memory tables
│
├── HANDOVER.md                        How to get sorted copies of a PatientList for searching
├── SORTING_FEATURE_EXPLAINED.md       Walkthrough of the linked list sorting feature
└── Assignment Task 1 [30 marks].pdf   Assignment brief
```

---

## Datasets

Each CSV file has a header row and 200 patient records (600 in total).

```text
PatientID,Age,CareType,LengthOfStay,BaseCostPerHour,DaysVisitsPerYear
PT1001,42,Emergency,12,150.0,2
```

| Column | Type | Meaning |
|---|---|---|
| `PatientID` | text | Unique identifier, e.g. `PT1001` (A), `PT2001` (B), `PT3001` (C) |
| `Age` | int | Patient age in years |
| `CareType` | text | `Emergency`, `Inpatient`, `Outpatient`, `Rehabilitation`, `Routine Checkup` or `Vaccination` |
| `LengthOfStay` | int | Visit duration in hours |
| `BaseCostPerHour` | decimal | Billing rate for the care type, in MYR per hour |
| `DaysVisitsPerYear` | int | Average number of visits or days per year |

| Dataset | Facility | Population |
|---|---|---|
| Dataset 1 | Facility A: General Hospital (Metropolitan Hub) | Adult and elderly patients |
| Dataset 2 | Facility B: University Medical Center | Students, young adults and children |
| Dataset 3 | Facility C: Community Health Clinic | Residents of all ages |

Rows that fail to parse are skipped with a warning; the rest of the file still loads.

---

## Business rules

**Total medical cost per patient**

```text
Cost = Length of Stay x Base Cost Per Hour x Days Visits Per Year
```

**Average cost per patient in a group**

```text
Average Cost = Total Medical Cost in Group / Number of Patients in Group
```

**Age groups**

| Range | Group |
|---|---|
| 0-17 | Pediatrics & Adolescents |
| 18-25 | Young Adults / University Students |
| 26-45 | Working Adults (Early Career) |
| 46-60 | Working Adults (Late Career) |
| 61-100 | Senior Citizens / Geriatric Care |

Ages outside 0-100 fall into an `Unknown` group and are left out of age group reports.

---

## Program 1: Array program

All records are held in `PatientArray`, a dynamic array on one contiguous heap buffer. It starts with 16 slots and doubles its capacity when full, so appending is amortised O(1) and any record can be reached by index in O(1). It implements the Rule of 5 (deep copy and move) and reports its own memory use (`getMemoryUsageBytes()`).

At start-up the program loads the three facilities, combines them into one 600-record array, precomputes every statistic and makes four merge-sorted copies (by age, stay, cost and care type) for the search experiments.

### Main menu

| Option | Report | Task |
|---|---|---|
| `1` | **Program Implementation**: load status, record count, capacity and memory footprint of each array, then 5 sample records per facility | 3 |
| `2` | **Demographic & Billing Categorization**: sub-menu to pick all facilities combined, or Facility A, B or C. For each age group: patient count, total and average cost per care type, the most preferred care type and the group's total billing | 4 |
| `3` | **Healthcare Expenditure & Service Analysis**: total cost by care type for each facility, a dataset comparison (total billing, patients, average cost, average stay) and an age group comparison across datasets | 5 |
| `4` | **Sorting Experiments**: bubble sort vs merge sort on all 600 records, by Age, Length of Stay, Total Cost and Care Type. Prints a benchmark table, a preview of each sorted result and a complexity discussion | 6, 8 |
| `5` | **Searching Experiments**: sub-menu (see below) | 7, 8 |
| `0` | Exit | |

Task 4 and 5 statistics come from `Analysis.cpp`, which fills a fixed `5 x 6` grid (age group x care type) of patient count, total cost and total hours in a single pass. Facility grids are added together to get the combined result.

### Sorting (Task 6)

| Algorithm | How it works on the array | Time | Extra space |
|---|---|---|---|
| Bubble sort | Swaps neighbouring `Patient` records; stops early after a pass with no swaps | O(n) best, O(n²) average and worst | O(1) |
| Merge sort | Top-down; one `n`-slot scratch buffer is allocated once and reused by every merge | O(n log n) in all cases | O(n) buffer + O(log n) stack |

All four sorts are ascending (A-Z for care type). Both algorithms are stable. Each one runs on its own copy of the data, so they always start from the same unsorted order. The table reports execution time, comparisons, memory footprint, auxiliary memory and peak memory for each algorithm.

### Searching (Task 7)

The sub-menu offers four searches over all 600 records:

| Option | Search | Input |
|---|---|---|
| `1` | Age group | Pick one of the five groups |
| `2` | Care type | One or more care types, e.g. `1 5` for Emergency **or** Rehabilitation |
| `3` | Visit duration | Stays longer than *N* hours (default 24) |
| `4` | Total medical cost | Cost above *N* MYR (default 10000) |

Each search runs four times: linear search and sentinel linear search, each on the unsorted data and on the copy sorted by the searched field.

- **Linear search** checks the array bound (`i < n`) before every record.
- **Sentinel linear search** writes a record that always matches into the spare slot after the last element (`data[n]`), so the inner loop needs no bound check. The bound is checked only after each match. If the array has no spare slot, it grows first, and the report shows the extra memory that cost.

Both algorithms test every record, on unsorted and sorted data alike. Sorting groups the matches together in the output but does not shorten the array scan. The report shows the matching records, key comparisons, bound checks, average time, memory footprint and auxiliary memory, followed by an analysis of the results.

---

## Program 2: Linked list program

All records are held in `PatientList`, a singly linked list with one heap-allocated node per patient. It keeps both `head` and `tail` pointers, so appending is O(1), but reaching a record means walking from the head (O(n)). It implements the Rule of 5 and reports its own memory use (`getMemoryUsageBytes()`).

The three datasets themselves form a linked list too: each `Facility` holds its `PatientList` and a `next` pointer (A → B → C), and every report walks that chain instead of indexing an array. Per-care-type totals are gathered in `CareTypeStatList`, another linked list with one node per distinct care type.

### Main menu

| Option | Report | Task |
|---|---|---|
| `1` | **Dataset Storage Summary & Sample Records**: load status, records, node size, memory footprint and head → tail IDs of each list, then 5 sample records per facility | 3 |
| `2` | **Demographic & Billing Categorization**: pick Facility A, B, C or all combined. Builds a separate list per age group, then prints count, total and average cost per care type, the most preferred care type and a summary across groups | 4 |
| `3` | **Healthcare Expenditure & Service Analysis**: billing per dataset, cost by care type, dataset comparison, age group matrices (patient count, total cost, average cost, average stay) and key findings | 5 |
| `4` | **Sorting & Performance Analysis**: pick a dataset, a field and an order, then compare bubble sort and merge sort | 6, 8 |
| `5` | **Searching Experiments**: pick a dataset, a search and the order of the sorted copy, then compare linear and sentinel linear search | 7, 8 |
| `6` | **Run All Reports**: options 1-3 for every dataset plus the sorting report on all fields (combined data, ascending). Searching is left out because it needs input | |
| `0` | Exit | |

Menu options are colour-coded by the kind of choice (dataset, field, value, order, back), and the same colours are used in every menu.

### Sorting (Task 6)

Sort fields: Age, Visit Duration, Total Medical Cost, Care Type (alphabetical), or **All fields** for a side-by-side comparison. Order: ascending or descending.

| Algorithm | How it works on the list | Time | Extra space |
|---|---|---|---|
| Bubble sort | Swaps neighbouring nodes by relinking their `next` pointers; stops early after a pass with no swaps | O(n) best, O(n²) average and worst | O(1) |
| Merge sort | Finds the middle with slow/fast pointers, splits the chain, sorts each half recursively and merges by relinking | O(n log n) in all cases | O(log n) recursion stack |

Neither algorithm copies a `Patient` record or allocates a node; they only change pointers. Both are stable. The report prints a performance table (average time, comparisons, node moves, a sorted-order check), a memory table (recursion depth, footprint, auxiliary and peak memory) and a complexity table.

`PatientSorter::sortedCopy(...)` returns a sorted deep copy and leaves the original list untouched. The search experiments use it to build their sorted data. See [HANDOVER.md](HANDOVER.md).

### Searching (Task 7)

| Option | Search | Input |
|---|---|---|
| `1` | Age group | Pick one of the five groups |
| `2` | Care type | Pick one care type; the menu lists only the types present in the chosen dataset, with patient counts |
| `3` | Visit duration | Stays longer than *N* hours (0-1000) |
| `4` | Total medical cost | Cost above *N* MYR (0-1,000,000) |

You also choose whether the sorted copy is ascending or descending, which decides whether the matches sit near the head or near the tail.

- **Linear search** tests `current != nullptr` before every node.
- **Sentinel linear search** links a temporary node that always matches after the tail (O(1)), so the walk needs no end-of-list test per node. It checks for the end only when it finds a match, then unlinks the sentinel.
- **On sorted data** both searches stop as soon as the group of matching records ends, so they can finish well before the tail.

All searches run on the experiment's own copies, so the loaded datasets are never modified. The report prints the matches in loaded order and in sorted order, a performance table (time, comparisons, end-of-list checks), a memory overhead table (including the sorted copy and the sentinel node) and a complexity table.

---

## Array vs linked list at a glance

| | Array program | Linked list program |
|---|---|---|
| Container | `PatientArray`: one contiguous buffer, capacity doubles when full | `PatientList`: one heap node per record, `head` + `tail` |
| Access by position | O(1) | O(n) walk from the head |
| Append | Amortised O(1); occasional O(n) copy on growth | O(1) via `tail` |
| Memory overhead | Unused spare capacity | One `next` pointer per node, plus per-allocation overhead |
| Facilities | Three separate arrays | Chained `Facility` nodes (A → B → C) |
| Bubble sort | Swaps whole `Patient` records | Relinks two nodes; no record is copied |
| Merge sort | O(n) scratch buffer | O(log n) stack only; no buffer |
| Sentinel | Written into the spare slot after the last element | Temporary node linked after the tail |
| Search on sorted data | Scans every record | Stops when the group of matches ends |
| Dataset selection | Fixed: all 600 records for sorting and searching | Any facility or all combined |

---

## How performance is measured

| Measurement | Array program | Linked list program |
|---|---|---|
| Sort time | One run per algorithm, measured in microseconds and shown in milliseconds | Average of 20 runs, each on a fresh copy, in milliseconds |
| Search time | 50 ms untimed warm-up, then 7 batches of 200 runs; the median batch average, in microseconds | Average of 2000 runs, in microseconds |
| Comparisons | Every comparison between two records (sort) or every record tested (search) | Same |
| Bound / end checks | `i < n` or `i == n` checks | `current != nullptr` checks |
| Memory footprint | Array object + capacity x `sizeof(Patient)` | List object + number of nodes x `sizeof(Node)` |
| Auxiliary memory | Estimated from working variables, buffers and recursion frames | Estimated from working variables, recursion frames and the sentinel node |

Timings depend on the machine, the compiler and the build configuration. Compare the two programs on the same machine with the same configuration. Comparison and step counts are exact and do not vary between runs.

---

## Assignment task coverage

| Task | Requirement | Array program | Linked list program |
|---|---|---|---|
| 3 | Store the three datasets in each data structure | Menu 1 | Menu 1 |
| 4 | Age group categorization, preferred care type, total and average cost | Menu 2 | Menu 2 |
| 5 | Billing per dataset, cost by care type, comparisons across datasets and age groups | Menu 3 | Menu 3 |
| 6 | Sort by age, stay or cost; compare time and memory | Menu 4 | Menu 4 |
| 7 | Search by age group, care type and stay threshold on unsorted vs sorted data | Menu 5 | Menu 5 |
| 8 | Record execution time and memory for all sorts and searches | Menus 4-5 | Menus 4-5 |
| 9 | Compare costs and service preferences across datasets and age groups | Menus 2-3 (comparison tables) | Menus 2-3 (comparison tables and key findings) |

Sorting by care type and searching by total cost threshold go beyond the brief. They were added so every search field has a matching sorted copy.

---

## Further documentation

- [HANDOVER.md](HANDOVER.md): how to get sorted copies of a `PatientList` and the rules for using them in search code.
- [SORTING_FEATURE_EXPLAINED.md](SORTING_FEATURE_EXPLAINED.md): line-by-line walkthrough of the linked list bubble sort and merge sort, the sorting report and the menu changes.
- `Assignment Task 1 [30 marks].pdf`: the original assignment brief and marking criteria.

---

## Contributing

- Work on a personal feature branch (`feat/<name>`) and open a pull request into `main`.
- Do not use STL containers. Write any new container yourself.
- Keep the array program and the linked list program independent; they do not share source files.
- Never sort `allPatients` or a facility's list in place. Sort a copy so other reports still see the loaded order.
- Output must be text tables in the console. No GUI or chart libraries.

---

## Submission notes

- The ZIP for Moodle contains only the `.cpp`, `.hpp` and `.csv` files, named `<GroupNo>_<LeaderID>_<Member1ID>_<Member2ID>_<Member3ID>.zip`.
- The demo video (max 30 minutes, under 200 MB) and the Word report use the same naming format with `.mp4` and `.docx`.
- All three are due Friday of Week 9, 5:00 pm.
