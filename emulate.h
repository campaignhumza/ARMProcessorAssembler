//
// Created by el djebena on 26/05/2018.
//

#ifndef EMULATOR_EMULATE_H
#define EMULATOR_EMULATE_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "utility.h"
#include "dataProcess.h"
#define PC 13

enum InstructionClass{DataProcess, Multiply, DataTransfer, Branch};

enum ShiftType{LSL, LSR, ASR, ROR};

void arithmeticLogicUnit(uint32_t nextInstruction,uint32_t operand1,uint32_t operand2);
void dataProcessExecute(struct state machineState, uint32_t nextInstruction);

const uint32_t HALT = 0x00000000;

const uint32_t dataProcMask = 0x0;
const uint32_t multMask = 0x90;
const uint32_t dataTransMask = 0x8000000;
const uint32_t branchMask = 0xA000000;

const uint32_t andMask = 0x0;
const uint32_t eorMask = 0x200000;
const uint32_t subMask = 0x400000;
const uint32_t rsbMask = 0xC00000;
const uint32_t addMask = 0x800000;
const uint32_t tstMask = 0x1000000;
const uint32_t teqMask = 0x2400000;
const uint32_t cmpMask = 0x2800000;
const uint32_t orrMask = 0x3000000;
const uint32_t movMask = 0x1A00000;

//struct instruction fetchNext(struct state* machineState);
#endif //EMULATOR_EMULATE_H
