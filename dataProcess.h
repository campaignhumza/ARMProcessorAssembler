//
// Created by el djebena on 01/06/2018.
//

#ifndef EMULATOR_DATAPROCESS_H
#define EMULATOR_DATAPROCESS_H
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>


struct state {
    uint8_t memory[65536];
    uint32_t registers[15];
};


static uint32_t rotr32(uint32_t n, unsigned int c);
static uint32_t performShift(uint8_t shiftType, uint32_t contentToShiftOn, uint32_t amountToShiftBy,bool* isCarrySet);
uint32_t barrelShift(struct state machineState,uint32_t nextInstruction);

#define CSPR 14



#endif //EMULATOR_DATAPROCESS_H
