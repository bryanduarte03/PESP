# PESP Instruction Set Architecture (ISA)

This document describes the instruction set for the PESP CPU and its pipelined version (PESPv2).

---

## Overview

- 32 general-purpose registers: R0–R31
- Fixed 32-bit instruction width
- Register-based architecture
- Immediate values supported
- Designed for simple pipelined execution

---

## Instruction Format

General format:

[31:28] opcode  
[27:23] rd  
[22:18] rs1  
[17:13] rs2  
[12:0]  immediate  

Note: Not all fields are used by every instruction.

---

## Registers

- R0 is always zero (read-only)
- R1–R31 are general-purpose registers

---

## Arithmetic Instructions

ADD   rd, rs1, rs2    ; rd = rs1 + rs2  
SUB   rd, rs1, rs2    ; rd = rs1 - rs2  
AND   rd, rs1, rs2    ; rd = rs1 & rs2  
OR    rd, rs1, rs2    ; rd = rs1 | rs2  

---

## Immediate Instructions

LDI   rd, imm         ; rd = immediate  

---

## Memory Instructions

LD    rd, rs1, imm    ; rd = MEM[rs1 + imm]  
ST    rs2, rs1, imm   ; MEM[rs1 + imm] = rs2  

---

## Branch Instructions

BEQ   rs1, rs2, imm   ; if (rs1 == rs2) PC = imm  
BNE   rs1, rs2, imm   ; if (rs1 != rs2) PC = imm  

---

## Jump Instructions

JMP   imm             ; PC = imm  
JCD   rs1, imm        ; if (rs1 == 0) PC = imm  

---

## System Instructions

HLT                   ; halt execution  

---

## Pipeline Behavior (PESPv2)

- 5-stage pipeline: IF → ID → EX → MEM → WB  
- Forwarding reduces data hazards  
- Load-use hazards cause a 1-cycle stall  
- Branches and jumps cause pipeline flush  

---

## Example Program

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

## Notes

- Immediate values use 13 bits  
- Instruction encoding varies by type  
- Designed for educational CPU design and simulation
