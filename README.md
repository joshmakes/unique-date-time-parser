# Project Overview

#### This project implements a streaming, validated, deduplicating file processor for date-time strings (per the ISO 8601 standard).

#### It reads in an arbitrarily large input file, identifies valid date-time records, deduplicates them via an in-memory hash-indexed arena, and writes unique validated values to an output file.

### The program is written in C (11), built using a modular architecture, and designed to emphasize:
- Buffer-aware correctness
- Streaming safety (no reliance on fgets)
- Efficient uniqueness tracking
- Clean separation of parsing, validation, and storage logic

## Design Rationale
### Why a memory arena?

I implemented a custom arena-based hash index because:
- Deterministic memory footprint
- O(1) lookup/insert time in practice (FNV-1a hash + probe-forward strategy)
- Zero per-allocation overhead / fragmentation:
    - 100% up-front / initialization allocation
- Perfectly suited for fixed-size string storage

#### The arena acts as a validated, deduplicating set for accepted date-time values, and at any time is fully synchronized with the output file.

### Why buffered streaming instead of line-based I/O?

The intent was to:
- Showcase handling of partial read() results
- Correctly reconstruct strings split across buffer boundaries
- Test robustness under real pipeline behavior

This meant manually assembling lines using:
- a working input buffer
- a carry-over buffer for incomplete fragments

This decision paid off — it exposed subtle parsing issues that would never surface under higher-level I/O routines.

## Notes & Assumptions
#### This project was tested exclusively on Ubuntu 24.04, and includes a Makefile, sample input data, build recipes, and captured command-line output.

#### Delimiter is intended to be configurable (via the `#define`) assumed to be '\n' (comma was also tested and worked)
#### The arena is capped at 32,767 entries. Supplying 32,768 or more unique values returns `NO_SLOTS`.

> This limit was chosen so the arena occupies ~1 MB of heap memory. It can be increased if desired.

## Date-Time Values Follow One of Two Strict Formats:
```
1. YYYY-MM-DDThh:mm:ssZ
2. YYYY-MM-DDThh:mm:ss±hh:mm
```

### Strings must be fully reconstructed before validation — partial tails are not considered valid candidates.

### All identical values must map to exactly one entry in the output.

## Overall Workflow

### Below is the high-level pipeline executed for every dataset:

1. Initialize arena / Allocate fixed slot storage and metadata
2.  Open input and output files
3. Read raw bytes into configurable-size buffer via read(), stop on EOF or read error.
4. Parse buffer via bufferDedup() / detect delimiters ('\n')
5. Merge prior partial line via carryOverBuffer
6. Form one or more candidate strings
7. Validate candidates: using `isValidLen()` + `isValidDateTime()`
8. Deduplicate and store: using hash slot assignment in arena
9. Write canonical copy to output: only if the candidate is valid and unique.
10. Preserve incomplete trailing data, save leftover fragment to carryOverBuffer and combine on the next read cycle

#### This approach allows:
- Arbitrarily large input files
- Fragmented lines split across reads
- No dependency on OS guarantees of complete line  delivery
    - Although this project was tested exclusively on an Ubuntu VM

## Key Challenges & Solutions
- Handling split lines across reads
    - Solved with a carry-over buffer tracking tail data between read cycles
- Ensuring unique, correct reconstruction ordering
    - Solved by explicit merge logic and only evaluating candidates after assembly
- Deduplication without dynamic structures
- Solved using a hashed arena + open addressing and probe forward


## Major Bug – “Sunday Special: Slot 0 Black Hole”
#### During mass scaling beyond 10k lines, I discovered the arena would store records in slot 0 but `getData()` treated index 0 as invalid

#### This produced a fascinating symptom:
- ✔ System worked perfectly
- ✘ except exactly one line always disappeared

> Fix: allow index 0 as valid in retrieval
After correction, 100% of valid records flowed through correctly

#### This led to tracing through the codebase, from the parsing logic to the internals of the memory arena. Just for the underlying issue to be from a "handy" getter function :(

## Build & Run Instructions
> Requirements
> - GCC or Clang
> - GNU Make
> - POSIX environment -- Linux (Ubuntu 24.04 recommended)

### Build the project
```
make clean all && make run && make diff
```

> `run` and `diff` are optional make arguments to run the executable with the base tested set, and run the diff after the program is executed to compare the input and output files.


This will:
- Clean all existing builds / binaries / output files
- Compile all .c files
- Generate main executable
- (optional) `run` executes the program with the default input/output datasets
- (optional) `diff` compares input and output files and verifies uniqueness filtering

#### Run the executable
`make run` 

OR
```
./build/unique-date-time-parser ./data/input/generated-date-times.txt data/output/validated-date-times.txt
```

----
Example output:
#### Running with empty output file:
```
$ diff ./data/input/generated-date-times.txt ./data/output/validated-date-times.txt
1,32767d0
< 8894-44-51T24:42:67+16:35
< 8202-58-54T81:18:88-14:19
< 0262-00-19T65:92:95Z
< 5105-27-85T80:23:84Z
< 3310-65-95T24:44:44Z
< 7863-63-15T22:11:94-14:48
< 4984-44-82T25:25:95+99:85
< 0601-25-82T57:68:22+27:27
< 5960-17-11T96:81:27-33:52
< 3232-83-82T96:30:16Z
< 6998-40-35T48:25:24-32:65
< 5226-95-80T88:08:83-85:51
< 8969-63-83T77:30:36+30:79
< 9358-23-70T95:02:38-94:86
```

#### After building and running program. (The example test case has no invalid strings). I will provide example test cases that show the exclusion of invalid strings.

```
josh@jsh:/mnt/c/users/josh/repos/unique-date-time-parser$ make clean all run diff
Using input dataset: data/input/generated-date-times.txt
Using output dataset: data/output/validated-date-times.txt
Number of total bytes read: 797357, and total read() calls: 3115
Executing test: diffing input and output files:
✔ No differences detected
```

#### Here is an example showing the off-nominal case where there were numerous invalid datetime strings:
```
josh@jsh:/mnt/c/users/josh/repos/unique-date-time-parser$ make clean all run diff
Using input dataset: data/input/some-invalid-date-times.txt
Using output dataset: data/output/validated-date-times.txt
Invalid date/time: 'GINGERR-00TTTTTTTZZZ'
Invalid date/time: 'ZZZZZZZZZZZZZZZZZZZZZZZZZ'
Invalid date/time: 'ZZZZZZZZZZZZZZZZZZZZ'
Failed to allocate slot in Arena_t (error code): 4 - for candidate 5904-57-31T02:42:58+94:71
Number of total bytes read: 46538, and total read() calls: 182
Diffing input and output files:
✘ Differences found:
5d4
< 1743-97-41T97:92:17-99:111743-97-41T97:92:17-99:11
229,256d227
< 7798-50-07T12:15:
< 1211-27-
< 6913-14-
< 8957-03-47T05:60:
< 9269-20-06T71:30:
< 7514-42-52T58:39:
< 3748-36-
< 8113-51-
< 6844-65-
< 0802-95-
< 7942-03-
< 9592-70-
< 0242-71-
< 4502-96-
< 9731-29-
< 4130-36-
< 0418-06-
< 1273-74-
< 2682-94-01T71:52:
< 3634-21-
< 9878-12-
< 7252-55-
< 5679-41-
< 7148-06-13T80:92:
< 1904-17-
< 9416-84-13T34:50:
< 6300-59-
< 7802-31-
536,538d506
< GINGERR-00TTTTTTTZZZ
< ZZZZZZZZZZZZZZZZZZZZZZZZZ
< ZZZZZZZZZZZZZZZZZZZZ
1911d1878
< 7754-15-36T25:09
1941d1907
< 5904-57-31T02:42:58+94:71
```

## Testing
#### This project includes both functional validation tests and structure-level behavior checks to ensure the parser behaves correctly under different input conditions.

Testing Considerations:
- Correct parsing of valid date-time strings
- Rejection of malformed or incomplete values
- Deduplication correctness
    - Output file contains only valid values and never missing lines except when arena capacity is reached

- Arena fullness behavior (`NO_SLOTS`)
- Buffered reconstruction and carry-over correctness
- Unique values: duplicate, but otherwise valid datetime string should result in only one of the duplicate strings being output to file

### Included my tiny python script I used to generate the (valid) datetime strings
*Sample usage:*
```
josh@jsh:/mnt/c/users/josh/repos/unique-date-time-parser$ python3 ./test/gen_date_time_file.py ./data/input/new-generated-date-times.txt 32620
josh@jsh:/mnt/c/users/josh/repos/unique-date-time-parser$ make clean all && make run && make diff
Using input dataset: data/input/new-generated-date-times.txt
Using output dataset: data/output/validated-date-times.txt
Number of total bytes read: 767005, and total read() calls: 2997
Diffing input and output files:
✔ No differences detected
```

### To make off-nominal data, I simply modified the generated input file, and included my test sets inside the `data/input/` directory:
- For example: `some-invalid-date-times.txt` (self-explanatory)

#### *You can easily generate a new input datetime string and then change the Makefile to reference your input file.*

## Performance Notes

### The system was tested against ~32k unique values
### Arena behavior remained stable with full hash spread
### Buffered parsing remained correct even with deliberately small read buffers and intentionally fragmented input lines

