# Performance-Aware Programming Course Homework

This repository contains my homework for the [Performance-Aware Programming course by Casey Muratori](https://www.computerenhance.com/p/table-of-contents).

## Project Status

This project now uses the **sim86 shared library** for 8086 instruction decoding, allowing me to focus on the simulation aspects of the course rather than implementing the decoder from scratch.

### Branches

- `main` - Original implementation attempt
- `my-original-implementation` - Preserved copy of my original decoder work
- `use-shared-library` - **Current working branch** using sim86 shared library

## Building and Running

### Prerequisites
- macOS with Clang/GCC
- The sim86 shared library files (included):
  - `libsim86.dylib` - Compiled shared library
  - `sim86_shared.h` - Header file with API definitions

### Compile
```bash
clang++ -o sim86 sim86.cpp -L. -lsim86
```

### Run
```bash
./sim86
```

This will decode the instructions from `listing_0039_more_movs` and output the assembly.

## Shared Library Functions Used

- `Sim86_Decode8086Instruction()` - Main decoder function
- `Sim86_RegisterNameFromOperand()` - Convert register info to strings
- `Sim86_MnemonicFromOperationType()` - Convert operation types to instruction names

## Goals

- **Learn C/C++ and Performance-Aware Programming**  
  I aim to use the summer of 2025 to improve my C/C++ skills and complete this course.

- **Timeframe**  
  I have 6 weeks before starting my external minor: [Systems Programming in C++](https://www.kiesopmaat.nl/modules/avans/ATD/141714/).

- **Study Plan**  
  - **Daily Commitment:** 4 hours per day  
  - **Total Duration:** 6 weeks × 7 days = 42 days  
  - **Total Hours:** 4 hours/day × 42 days = **168 hours**

### Reference

Here is the [GitHub](https://github.com/cmuratori/computer_enhance) repository for Casey Muratori's solutions and related materials for the Computer Enhance course.
