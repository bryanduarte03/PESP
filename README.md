# PESP CPU Project

A custom CPU architecture and simulator written in C, featuring both a single-cycle design (PESP) and an enhanced pipelined implementation (PESPv2), along with a custom assembler.

---

## Overview

This project explores CPU design concepts including:

- Instruction set architecture (ISA)
- Pipeline design
- Data hazards and forwarding
- Control hazards and flushing
- Assembly-to-machine-code translation

---

## Components

### PESP (Original CPU)
- Basic instruction execution model
- Custom ISA
- Register-based architecture

### PESPv2 (Pipelined CPU)
- 5-stage pipeline:
  - IF (Fetch)
  - ID (Decode)
  - EX (Execute)
  - MEM (Memory)
  - WB (Writeback)
- Data hazard handling:
  - Forwarding (bypassing)
  - Load-use hazard detection with stall insertion
- Control hazard handling:
  - Branch and jump flushing
- Supported control instructions:
  - BEQ / BNE
  - JUMP
  - JCD (jump if condition zero)

### Assembler
- Converts .asm programs into .bin machine code
- Supports arithmetic, load/store, branches, jumps, labels, and immediates

---

## Build Instructions

Build assembler:
gcc -std=c11 -Wall -Wextra -g assembler/PESPAssembler.c -o assembler

Build original simulator:
gcc -std=c11 -Wall -Wextra -g simulator/PESP.c -o pesp

Build pipelined simulator:
gcc -std=c11 -Wall -Wextra -g simulator/PESPv2.c -o pespv2

---

## Usage

Assemble a program:
./assembler programs/test.asm

Run on pipelined CPU:
./pespv2 programs/test.bin

---

## Example

Assembly:
LDI R1, 5
LDI R2, 5
BEQ R1, R2, 4
LDI R3, 99
HLT

Behavior:
- Branch is taken
- LDI R3, 99 is flushed
- Demonstrates control hazard handling

---

## Pipeline Features (PESPv2)

- Instruction-level parallelism via pipelining
- Forwarding to reduce stalls
- Load-use hazard detection with automatic stalling
- Branch and jump flush mechanism
- Correct handling of wrong-path instructions

---

## Future Improvements

- Branch prediction
- Superscalar execution
- Performance metrics (CPI, IPC)
- Memory hierarchy (cache simulation)
- Pipeline visualization tools

---

## Author

Bryan Duarte

---

## Notes

This project demonstrates core computer architecture concepts including pipelining, hazard detection, and ISA design.
