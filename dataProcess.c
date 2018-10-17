//
// Created by el djebena on 01/06/2018.
//
#include "dataProcess.h"

const uint8_t lsl = 0x0;
const uint8_t lsr = 0x1;
const uint8_t asr = 0x2;
const uint8_t ror = 0x3;
const uint32_t opCodeMask = 0x1E00000;
const uint32_t andOpcode = 0x0;
const uint32_t eorOpcode = 0x200000;
const uint32_t subOpcode = 0x400000;
const uint32_t rsbOpcode = 0x600000;
const uint32_t addOpcode = 0x800000;
const uint32_t tstOpcode = 0x1000000;
const uint32_t teqOpcode = 0x1200000;
const uint32_t cmpOpcode = 0x1400000;
const uint32_t orrOpcode = 0x1800000;
const uint32_t movOpcode = 0x1A00000;

void arithmeticLogicUnit(uint32_t nextInstruction, uint32_t operand1, uint32_t operand2) {
    //perform arithmetic
    //if arithhmetic operation update CSPR C to carry out of ALU
    uint32_t allBytes = nextInstruction;
    uint8_t destRegisterNumber = allBytes >> 12 & 0xF;
    uint32_t *destRegister = &(machineState.registers[destRegisterNumber]);
    int32_t result;
    bool updateCSPR = ((nextInstruction >> 20) & 1);

    if ((allBytes & opCodeMask) == movOpcode) {
        //*destRegister = operand2;
        machineState.registers[destRegisterNumber] = operand2;
    } else if ((allBytes & opCodeMask) == eorOpcode) {
        result = operand1 ^ operand2;
        machineState.registers[destRegisterNumber] = result;
        if (updateCSPR) {
            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

        }
    } else if ((allBytes & opCodeMask) == subOpcode) {
        result = operand1 - operand2;
        machineState.registers[destRegisterNumber] = result;

        if (updateCSPR) {


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 29, !(result > (operand1)));

        }
    } else if ((allBytes & opCodeMask) == rsbOpcode) {
        result = operand2 - operand1;
        machineState.registers[destRegisterNumber] = result;
        if (updateCSPR) {

            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 29, !(result > (operand1)));

        }
    } else if ((allBytes & opCodeMask) == addOpcode) {
        result = operand1 + operand2;
        machineState.registers[destRegisterNumber] = result;
        if (updateCSPR) {

            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 29, (result < (operand1)));

        }
    } else if ((allBytes & opCodeMask) == tstOpcode) {
        result = operand1 & operand2;
        if (updateCSPR) {
            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);
        }
    } else if ((allBytes & opCodeMask) == teqOpcode) {
        result = operand1 ^ operand2;
        if (updateCSPR) {
            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

        }
    } else if ((allBytes & opCodeMask) == cmpOpcode) {
        result = operand1 - operand2;

        if (updateCSPR) {

            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 29, !(result > (operand1)));

        }
    } else if ((allBytes & opCodeMask) == orrOpcode) {
        result = operand1 | operand2;
        machineState.registers[destRegisterNumber] = result;
        if (updateCSPR) {
            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

        }
    } else {
        //and
        result = operand1 & operand2;
        machineState.registers[destRegisterNumber] = result;
        if (updateCSPR) {
            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 30, result == 0x0);


            machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 31, result >> 31);

        }
    }
}

static uint32_t rotr32(uint32_t n, unsigned int c) {
    const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);
    c &= mask;
    return (n>>c) | (n<<( (-c)&mask ));
}

uint32_t barrelShift(uint32_t nextInstruction) {

    uint32_t allBytes = nextInstruction;
    uint32_t operand2 = allBytes & 0xFFF;
    bool cCSPRBitSet;
    bool updateCSPR = (nextInstruction & (1 << 20));

    bool IisSet = ((allBytes & 0x2000000) == 0x2000000);
    if (IisSet) {
        //do this if I set
        uint8_t numberOfBitsToRotate = (operand2 >> 8) * 2;
        uint32_t immediate = operand2 & 0xFF;
        operand2 = rotr32(immediate, numberOfBitsToRotate);

        cCSPRBitSet = ((immediate >> (numberOfBitsToRotate - 1)) & 0x1) == 0x1;

    } else {
        //fetch register and shift if I not set
        uint32_t rmRegister = operand2 & 0xF;
        uint8_t shift = (operand2 >> 4);
        uint8_t shiftType = ((shift >> 1) & 0x3);
        uint8_t shiftOption = (shift & 0x1) & 1;

        if (shiftOption == 0) {
            uint32_t integer = (shift >> 3);
            operand2 = performShift(shiftType, machineState.registers[rmRegister], integer, &cCSPRBitSet);
        } else {
            uint32_t rsRegister = (shift >> 4);
            uint8_t bottomByteOfrsRegister = (machineState.registers[rsRegister]);
            operand2 = performShift(shiftType, machineState.registers[rmRegister], bottomByteOfrsRegister,
                                    &cCSPRBitSet);
        }
    }

    if (updateCSPR) {
        machineState.registers[CSPR] = setBit(machineState.registers[CSPR], 29, cCSPRBitSet);
    }

    return operand2;
}

uint32_t performShift(uint8_t shiftType, uint32_t contentToShiftOn, uint32_t amountToShiftBy, bool *isCarrySet) {
// >> operator is logical shift if int is unsigned and is arithmetic if int is signed.
    uint32_t operand2;
    if (shiftType == lsl) {
        *isCarrySet = ((((contentToShiftOn << (amountToShiftBy - 1)) & 0x80000000) >> 31) & 0x1) == 0x1;
        operand2 = contentToShiftOn << amountToShiftBy;

    } else if (shiftType == lsr) {
        *isCarrySet = ((contentToShiftOn >> (amountToShiftBy - 1)) & 0x1) == 0x1;
        operand2 = contentToShiftOn >> amountToShiftBy;
    } else if (shiftType == asr) {
        *isCarrySet = ((contentToShiftOn >> (amountToShiftBy - 1)) & 0x1) == 0x1;
        //cast to signed to force arithmetic shift.
        operand2 = (signed) contentToShiftOn << amountToShiftBy;
    } else {
        *isCarrySet = ((contentToShiftOn >> (amountToShiftBy - 1)) & 0x1) == 0x1;
        operand2 = rotr32(contentToShiftOn, amountToShiftBy);
    }
    return operand2;
}

void dataProcessExecute(uint32_t nextInstruction) {

    uint32_t operand1 = machineState.registers[(nextInstruction >> 16) & 0xF];
    uint32_t operand2 = barrelShift(nextInstruction);


    arithmeticLogicUnit(nextInstruction, operand1, operand2);
}


