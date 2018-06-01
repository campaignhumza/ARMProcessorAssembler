//
// Created by el djebena on 01/06/2018.
//
#include "utility.h"

const uint8_t eq = 0x0;
const uint8_t ne = 0x1;
const uint8_t ge = 0xA;
const uint8_t lt = 0xB;
const uint8_t gt = 0xC;
const uint8_t le = 0xD;
const uint8_t al = 0xE;


bool isCond(uint32_t nextInstruction, uint32_t CSPR) {
    uint8_t cond = ((nextInstruction >> 28) & 0xF);

    switch(cond) {
        case eq  :
            return isCSPR_Z(CSPR);
        case ne  :
            return !isCSPR_Z(CSPR);
        case ge  :
            return isCSPR_N(CSPR) == isCSPR_V(CSPR);
        case lt  :
            return isCSPR_N(CSPR) != isCSPR_V(CSPR);
        case gt  :
            return !isCSPR_Z(CSPR) && (isCSPR_N(CSPR) == isCSPR_V(CSPR));
        case le  :
            return (isCSPR_Z(CSPR) || (isCSPR_N(CSPR) != isCSPR_V(CSPR)));
        case al  :
            return true;
        default:
            return false;
    }
}

bool isCSPR_N(uint32_t CSPR) {
    return (CSPR >> 31 & 1);
}

bool isCSPR_Z(uint32_t CSPR) {
    return (CSPR>> 30 & 1);
}

bool isCSPR_C(uint32_t CSPR) {
    return (CSPR>> 29 & 1);
}

bool isCSPR_V(uint32_t CSPR) {
    return (CSPR>> 28 & 1);
}
