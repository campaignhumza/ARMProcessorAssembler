//
// Created by el djebena on 01/06/2018.
//

#ifndef EMULATOR_UTILITY_H
#define EMULATOR_UTILITY_H
#include <stdint.h>
#include <stdbool.h>

bool isCSPR_N(uint32_t CSPR);
bool isCSPR_Z(uint32_t CSPR);
bool isCSPR_C(uint32_t CSPR);
bool isCSPR_V(uint32_t CSPR);
bool isCond(uint32_t nextInstruction, uint32_t CSPR);
#endif //EMULATOR_UTILITY_H
