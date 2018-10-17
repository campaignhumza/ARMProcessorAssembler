//
// Created by el djebena on 17/10/2018.
//

#include "branch.h"


const int32_t branchOffSetMask = 0xFFFFFF;

void branchExecute(uint32_t nextInstruction) {
    uint32_t signedOffSet = (nextInstruction & branchOffSetMask);

    if ((signedOffSet >> 23) == 1) {
        signedOffSet = signedOffSet << 2;
        signedOffSet |= 0xFC000000;
        signedOffSet = (~(signedOffSet) & 0xFFFFFF) + 1;
        //signedOffSet |= 0x7FFFFF;
        //signedOffSet += 1;
        //signedOffSet = signedOffSet << 2;
        machineState.registers[PC] -= (signedOffSet);
    } else {
        signedOffSet = (signedOffSet) << 2;
        machineState.registers[PC] += (signedOffSet);
    }
    fetchedInstruction.isvalid = false;
}

