//
// Created by el djebena on 01/06/2018.
//

#ifndef EMULATOR_DATAPROCESS_H
#define EMULATOR_DATAPROCESS_H
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "emulate.h"


void arithmeticLogicUnit(uint32_t nextInstruction, uint32_t operand1, uint32_t operand2);
uint32_t barrelShift(uint32_t nextInstruction);
uint32_t performShift(uint8_t shiftType, uint32_t contentToShiftOn, uint32_t amountToShiftBy, bool *isCarrySet);
void dataProcessExecute(uint32_t nextInstruction);

#endif //EMULATOR_DATAPROCESS_H
