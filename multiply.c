//
// Created by el djebena on 01/06/2018.
//

#include "multiply.h"

void multiplyExecute(uint32_t nextInstruction) {
    uint8_t RdRegisterNumber = (nextInstruction >> 16) & 0xF;
    uint8_t RmRegisterNumber = nextInstruction & 0xF;
    uint8_t RsRegisterNumber = (nextInstruction >> 8) & 0xF;
    uint8_t RnRegisterNumber = (nextInstruction >> 12) & 0XF;

    bool isAccumulate = ((nextInstruction >> 21) & 1) == 1;
    if (isAccumulate) {
        machineState.registers[RdRegisterNumber] =
                (machineState.registers[RmRegisterNumber] * machineState.registers[RsRegisterNumber]) +
                machineState.registers[RnRegisterNumber];
    } else {
        machineState.registers[RdRegisterNumber] = (machineState.registers[RmRegisterNumber] *
                                                    machineState.registers[RsRegisterNumber]);
    }

    machineState.registers[CSPR] |= (machineState.registers[RdRegisterNumber] == 0) << 30;
    machineState.registers[CSPR] |= ((machineState.registers[RdRegisterNumber] & 0x80000000) >> 2);

}