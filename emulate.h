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
#define PC 15

enum InstructionClass{DataProcess, Multiply, DataTransfer, Branch};

enum ShiftType{LSL, LSR, ASR, ROR};

void arithmeticLogicUnit(uint32_t nextInstruction,uint32_t operand1,uint32_t operand2);
void dataProcessExecute(struct state machineState, uint32_t nextInstruction);

const uint32_t HALT = 0x00000000;

const uint32_t dataProcMask = 0x0;
const uint32_t multMask = 0xFC000F0;
const uint32_t multInstruction = 0x90;
const uint32_t dataTransMask = 0xC600000;
const uint32_t dataTransInstruction = 0x4000000;
const uint32_t branchMask = 0xF000000;
const uint32_t branchInstruction = 0xA000000;

const uint32_t opCodeMask = 0x1E00000;
const uint32_t andOpcode = 0x0;
const uint32_t eorOpcode = 0x200000;
const uint32_t subOpcode = 0x400000;
const uint32_t rsbOpcode = 0x600000;
const uint32_t addOpcode = 0x800000;
const uint32_t tstOpcode = 0x1000000;
const uint32_t teqOpcode = 0x2400000;
const uint32_t cmpOpcode = 0x1400000;
const uint32_t orrOpcode = 0x1800000;
const uint32_t movOpcode = 0x1A00000;

const int32_t branchOffSetMask = 0xFFFFFF;

//struct instruction fetchNext(struct state* machineState);
#endif //EMULATOR_EMULATE_H
