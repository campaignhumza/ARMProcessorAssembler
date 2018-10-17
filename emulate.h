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

typedef struct {
    uint8_t memory[65536];
    uint32_t registers[17];
} MachineState;

typedef struct {
    uint32_t instruction;
    bool isvalid;
} FetchedInstruction;

typedef struct {
    uint32_t instruction;
    enum InstructionClass instructionType;
    bool hasValue;
} DecodedInstruction;

extern MachineState machineState;
extern DecodedInstruction decodedInstruction;
extern FetchedInstruction fetchedInstruction;


#define CSPR 16





#endif //EMULATOR_EMULATE_H
