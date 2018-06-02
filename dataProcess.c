//
// Created by el djebena on 01/06/2018.
//
#include "dataProcess.h"

const uint8_t lsl = 0x0;
const uint8_t lsr = 0x1;
const uint8_t asr = 0x2;
const uint8_t ror = 0x3;

uint32_t barrelShift(struct state machineState,uint32_t nextInstruction) {

    uint32_t allBytes = nextInstruction;
    uint32_t operand2 = allBytes & 0xFFF;
    bool c_CSPR_BitSet;
    bool updateCSPR = (nextInstruction & (1<<20));

    bool IisSet = ((allBytes & 0x2000000) == 0x2000000);
    if(IisSet) {
        //do this if I set
        uint8_t numberOfBitsToRotate = (operand2 >> 8) * 2;
        uint32_t immediate = operand2 & 0xFF;
        operand2 = rotr32(immediate, numberOfBitsToRotate);

        c_CSPR_BitSet = ((immediate >> (numberOfBitsToRotate - 1)) & 0x1) == 0x1;

    } else {
        //fetch register and shift if I not set
        uint32_t rmRegister = operand2 & 0xF;
        uint8_t shift = (operand2 >> 4);
        uint8_t shiftType = ((shift >> 1) & 0x3);
        uint8_t shiftOption =  (shift & 0x1) & 1;

        if(shiftOption == 0) {
            uint32_t integer = (shift >> 3);
            operand2 = performShift(shiftType,machineState.registers[rmRegister],integer,&c_CSPR_BitSet);
        } else {
            uint32_t rsRegister = (shift >> 4);
            uint8_t bottomByteOfrsRegister = (machineState.registers[rsRegister]);
            operand2 = performShift(shiftType,machineState.registers[rmRegister],bottomByteOfrsRegister,&c_CSPR_BitSet);
        }
    }

    if (updateCSPR) {
        machineState.registers[CSPR] |= (c_CSPR_BitSet << 30);
    }

    return operand2;
}

uint32_t performShift(uint8_t shiftType, uint32_t contentToShiftOn, uint32_t amountToShiftBy,bool* isCarrySet) {

// >> operator is logical shift if int is unsigned and is arithmetic if int is signed.
    uint32_t operand2;
    if((shiftType & lsl) == lsl) {
        *isCarrySet = ((((contentToShiftOn << (amountToShiftBy-1)) & 0x80000000) >> 31) & 0x1) == 0x1;
        operand2 = contentToShiftOn << amountToShiftBy;

    } else if((shiftType & lsr) == lsr) {
        *isCarrySet = ((contentToShiftOn >> (amountToShiftBy-1)) & 0x1) == 0x1;
        operand2 = contentToShiftOn >> amountToShiftBy;
    } else if((shiftType & asr) == asr) {
        *isCarrySet = ((contentToShiftOn >> (amountToShiftBy-1)) & 0x1) == 0x1;
        //cast to signed to force arithmetic shift.
        operand2 = (signed)contentToShiftOn << amountToShiftBy;
    } else {
        *isCarrySet = ((contentToShiftOn >> (amountToShiftBy-1)) & 0x1) == 0x1;
        operand2 = rotr32(contentToShiftOn,amountToShiftBy);
    }
    return operand2;
}

static uint32_t rotr32(uint32_t n, unsigned int c) {
    const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);
    c &= mask;
    return (n>>c) | (n<<( (-c)&mask ));
}
