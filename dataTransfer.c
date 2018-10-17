//
// Created by el djebena on 17/10/2018.
//
#include "dataTransfer.h"

void dataTransferExecute(uint32_t nextInstruction) {

    bool c_CSPR_BitSet = false;
    uint32_t offset = nextInstruction & 0xFFF;
    if (((nextInstruction >> 25) & 1) == 1) {
        uint32_t rmRegister = offset & 0xF;
        uint8_t shift = (offset >> 4);
        uint8_t shiftType = ((shift >> 1) & 0x3);
        uint8_t shiftOption = (shift & 0x1) & 1;

        if (shiftOption == 0) {
            uint32_t integer = (shift >> 3);
            offset = performShift(shiftType, machineState.registers[rmRegister], integer, &c_CSPR_BitSet);
        } else {
            uint32_t rsRegister = (shift >> 4);
            uint8_t bottomByteOfrsRegister = (machineState.registers[rsRegister]);
            offset = performShift(shiftType, machineState.registers[rmRegister], bottomByteOfrsRegister,
                                  &c_CSPR_BitSet);
        }
    }

    uint8_t RnRegister = (nextInstruction >> 16) & 0xF;
    uint8_t RdRegister = (nextInstruction >> 12) & 0xF;
    bool PisSet = ((nextInstruction >> 24) & 1) == 1;
    bool UisSet = ((nextInstruction >> 23) & 1) == 1;
    bool LisSet = ((nextInstruction >> 20) & 1) == 1;
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint32_t word;
    if (RnRegister == PC) {
        //offset += 8;
    }
    if (PisSet) {
        if (LisSet) {
            if (UisSet) {
                byte0 = machineState.memory[machineState.registers[RnRegister] + offset];
                byte1 = machineState.memory[machineState.registers[RnRegister] + offset + 1];
                byte2 = machineState.memory[machineState.registers[RnRegister] + offset + 2];
                byte3 = machineState.memory[machineState.registers[RnRegister] + offset + 3];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] = word;
            } else {
                byte0 = machineState.memory[machineState.registers[RnRegister] - offset];
                byte1 = machineState.memory[machineState.registers[RnRegister] - (offset - 1)];
                byte2 = machineState.memory[machineState.registers[RnRegister] - (offset - 2)];
                byte3 = machineState.memory[machineState.registers[RnRegister] - (offset - 3)];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] = word;
            }
        } else {
            if (UisSet) {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister] + offset] = byte0;
                machineState.memory[machineState.registers[RnRegister] + offset + 1] = byte1;
                machineState.memory[machineState.registers[RnRegister] + offset + 2] = byte2;
                machineState.memory[machineState.registers[RnRegister] + offset + 3] = byte3;
            } else {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister] - offset] = byte0;
                machineState.memory[machineState.registers[RnRegister] - (offset + 1)] = byte1;
                machineState.memory[machineState.registers[RnRegister] - (offset + 2)] = byte2;
                machineState.memory[machineState.registers[RnRegister] - (offset + 3)] = byte3;
            }
        }
    } else {
        if (LisSet) {
            if (UisSet) {
                byte0 = machineState.memory[machineState.registers[RnRegister]];
                byte1 = machineState.memory[machineState.registers[RnRegister] + 1];
                byte2 = machineState.memory[machineState.registers[RnRegister] + 2];
                byte3 = machineState.memory[machineState.registers[RnRegister] + 3];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] = word;
                machineState.registers[RnRegister] += offset;
            } else {
                byte0 = machineState.memory[machineState.registers[RnRegister]];
                byte1 = machineState.memory[machineState.registers[RnRegister] + 1];
                byte2 = machineState.memory[machineState.registers[RnRegister] + 2];
                byte3 = machineState.memory[machineState.registers[RnRegister] + 3];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] = word;
                machineState.registers[RnRegister] -= offset;
            }
        } else {
            if (UisSet) {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister]] = byte0;
                machineState.memory[machineState.registers[RnRegister] + 1] = byte1;
                machineState.memory[machineState.registers[RnRegister] + 2] = byte2;
                machineState.memory[machineState.registers[RnRegister] + 3] = byte3;
                machineState.registers[RnRegister] += offset;
            } else {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister]] = byte0;
                machineState.memory[machineState.registers[RnRegister] + 1] = byte1;
                machineState.memory[machineState.registers[RnRegister] + 2] = byte2;
                machineState.memory[machineState.registers[RnRegister] + 3] = byte3;
                machineState.registers[RnRegister] -= offset;
            }
        }
    }
}

