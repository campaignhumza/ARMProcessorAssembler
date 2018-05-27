#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "emulate.h"

struct state {
    uint8_t memory[65536];
    uint32_t registers[17];
};

struct instruction {
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t cond;

 uint32_t allbytes;
};

enum InstructionClass{DataProcess, Multiply, DataTransfer, Branch};

enum ShiftType{LSL, LSR, ASR, ROR};


struct state machineState;


bool isCSPR_N(struct state* machineState) {
    return (machineState->registers[16] >> 32 & 1);
}

bool isCSPR_Z(struct state* machineState) {
    return (machineState->registers[16]>> 31 & 1);
}

bool isCSPR_C(struct state* machineState) {
    return (machineState->registers[16]>> 30 & 1);
}

bool isCSPR_V(struct state* machineState) {
    return (machineState->registers[16]>> 29 & 1);
}

enum InstructionClass getInstructionClass(struct instruction* nextInstruction) {

    uint32_t allBytes = nextInstruction->allbytes;
    if((allBytes & dataProcMask) == dataProcMask) {
        return DataProcess;
    }
    if((allBytes & multMask) == multMask) {
        return Multiply;
    }
    if((allBytes & dataTransMask) == dataTransMask) {
        return DataTransfer;
    }else
        {
        return Branch;
    }
}


bool isCond(struct state* machineState,struct instruction* nextInstruction) {
    uint8_t byte0 = nextInstruction->byte0;
    uint8_t cond = (byte0 >> 4);

    switch(cond) {
        case eq  :
            return isCSPR_Z(machineState);
        case ne  :
            return !isCSPR_Z(machineState);
        case ge  :
            return isCSPR_N(machineState) == isCSPR_V(machineState);
        case lt  :
            return isCSPR_N(machineState) != isCSPR_V(machineState);
        case gt  :
            return !isCSPR_Z(machineState) && (isCSPR_N(machineState) == isCSPR_V(machineState));
        case le  :
            return (isCSPR_Z(machineState) || (isCSPR_N(machineState) != isCSPR_V(machineState)));
        case al  :
            return true;
        default:
            return false;
    }
}

static inline uint32_t rotr32(uint32_t n, unsigned int c)
{
    const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);

    // assert ( (c<=mask) &&"rotate by type width or more");
    c &= mask;
    return (n>>c) | (n<<( (-c)&mask ));
}
size_t highestOneBitPosition(uint32_t a) {
    size_t bits=0;
    while (a!=0) {
        ++bits;
        a>>=1;
    };
    return bits;
}

bool additionWillProduceCarry(uint32_t a, uint32_t b) {
    size_t a_bits=highestOneBitPosition(a), b_bits=highestOneBitPosition(b);
    return (a_bits<32 && b_bits<32);
}

bool subtractionWillUseBorrow(uint32_t a, uint32_t b) {
    size_t a_bits=highestOneBitPosition(a), b_bits=highestOneBitPosition(b);
    return (a_bits<32 && b_bits<32);
}

void arithmeticLogicUnit(struct state* machineState,struct instruction* nextInstruction,uint32_t operand1,uint32_t operand2,bool updateCSPR,uint32_t* carryOut) {
    //perform arithmetic
    //if arithhmetic operation update CSPR C to carry out of ALU
            uint32_t allBytes = nextInstruction->allbytes;
            uint8_t destRegisterAddress = allBytes >> 12 & 0xF;
            uint32_t * destRegister = &(machineState->registers[destRegisterAddress]);
            uint32_t result;

            if(allBytes & andMask == andMask) {

                result = operand1 & operand2;
                *destRegister = result;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState->registers[CSPR] |= (0x40000000);
                    }
                    machineState->registers[CSPR] |= (result & 0x80000000);

                }
            } else if(allBytes & eorMask == eorMask) {
                result = operand1 ^ operand2;
                *destRegister = result;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState->registers[CSPR] |= (0x40000000);
                    }
                    machineState->registers[CSPR] |= (result & 0x80000000);

                }
            } else if(allBytes & subMask == subMask) {
                result = operand1 - operand2;
                *destRegister = result;
                if(result > (operand2)) {
                    //underflow
                    *carryOut = 0;
                } else {
                    *carryOut = 1;
                }
            } else if(allBytes & rsbMask == rsbMask) {
                result = operand2 - operand1;
                *destRegister = result;
                *destRegister = operand2 - operand1;
                if(result > (operand1)) {
                    //underflow
                    *carryOut = 0;
                } else {
                    *carryOut = 1;
                }
            } else if(allBytes & addMask == addMask) {
                result = operand1 + operand2;
                *destRegister = operand2 + operand1;
                if(result < operand1) {
                    //overflow
                    *carryOut = 1;
                } else {
                    *carryOut = 0;
                }
            } else if(allBytes & tstMask == tstMask) {
                result = operand1 & operand2;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState->registers[CSPR] |= (0x40000000);
                    }
                    machineState->registers[CSPR] |= (result & 0x80000000);
                }

            } else if(allBytes & teqMask == teqMask) {
                result = operand1 ^ operand2;
                if(updateCSPR) {
                if(result == 0x0) {
                    machineState->registers[CSPR] |= (0x40000000);
                }
                machineState->registers[CSPR] |= (result & 0x80000000);

            }

            } else if(allBytes & cmpMask == cmpMask) {
                result = operand1 - operand2;
                if(result == 0x0 && updateCSPR) {
                    machineState->registers[CSPR] |= (0x40000000);
                }
                if(result > (operand2)) {
                    //underflow
                    *carryOut = 0;
                } else {
                    *carryOut = 1;
                }
            } else if(allBytes & orrMask == orrMask) {
                result = operand1 | operand2;
                *destRegister = result;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState->registers[CSPR] |= (0x40000000);
                    }
                    machineState->registers[CSPR] |= (result & 0x80000000);

                }
            } else {
                //mov
                *destRegister = operand2;
            }
}

void barrelShift(struct state* machineState,struct instruction* nextInstruction,bool updateCSPR) {
    //perform shift
    //if logic operation update CSPR C to barrel carry out
    //continue to ALU
//get operand1 from register.
    uint32_t operand1 = machineState->registers[(nextInstruction->allbytes >> 16 & 0xF)];
    uint32_t operand2 = nextInstruction->allbytes & 0xFFF;
    uint32_t carry;

    bool IisSet = (nextInstruction->allbytes & 0x2000000 == 0x2000000);
    if(IisSet) {
        //do this if I set
        uint8_t numberOfBitsToRotate = (operand2 >> 8) * 2;
        uint32_t immediate = operand2 & 0xFF;
        uint32_t shiftedOperand = rotr32(immediate, numberOfBitsToRotate);

        if (updateCSPR) {
            carry = (immediate >> (numberOfBitsToRotate - 1)) & 1;
        }

    } else {
        //fetch register and shift if I not set
        uint32_t rmRegister = operand2 & 0xF;
        uint8_t shift = (operand2 >> 4);
        uint8_t shiftType = ((shift >> 1) & 0x3);
        uint8_t shiftOption =  (shift & 0x1) & 1;


        if(shiftOption == 1) {
            uint32_t integer = (shift >> 3);
            operand2 = performShift(shiftType,machineState->registers[rmRegister],integer,&carry);
        } else {
           uint32_t rsRegister = (shift >> 4);
           uint8_t bottomByteOfrsRegister = machineState->registers[rsRegister];
           operand2 = performShift(shiftType,machineState->registers[rmRegister],bottomByteOfrsRegister,&carry);
        }
    }

    arithmeticLogicUnit(machineState,nextInstruction,operand1,operand2,updateCSPR,&carry);

    if (updateCSPR) {
        machineState->registers[CSPR] |= (carry << 29);
    }

}

uint32_t performShift(uint8_t shiftType, uint32_t contentToShiftOn, uint32_t amountToShiftBy, uint32_t * carry) {

// >> operator is logaical shift if int is unisgned and is arithmetic if int is signed.
    uint32_t operand2;
    if(shiftType & lsl == lsl) {
        *carry = ((contentToShiftOn << (amountToShiftBy-1)) & 0x80000000) >> 31;
        operand2 = contentToShiftOn << amountToShiftBy;
        return operand2;
    } else if(shiftType & lsr == lsr) {
        *carry = (contentToShiftOn >> (amountToShiftBy-1)) & 1;
        operand2 = contentToShiftOn >> amountToShiftBy;
        return operand2;
    } else if(shiftType & asr == asr) {
        *carry = (contentToShiftOn >> (amountToShiftBy-1)) & 1;
        //cast to signed to force arithmetic shift.
        operand2 = (signed)contentToShiftOn << amountToShiftBy;
        return operand2;
    } else {
        *carry = (contentToShiftOn >> (amountToShiftBy-1)) & 1;
        operand2 = rotr32(contentToShiftOn,amountToShiftBy);
        return operand2;
    }
}

void dataProcessExecute(struct state* machineState,struct instruction* nextInstruction) {
    uint32_t allBytes = nextInstruction->allbytes;
    uint32_t rnRegister = machineState->registers[(allBytes>> 16) & 0xF];

    bool updateCSPR = (allBytes >> 20 & 0x1) == 0X1;

    barrelShift(machineState,nextInstruction,updateCSPR);



}

void multiplyExecute(struct state* machineState,struct instruction* nextInstruction) {

}

void dataTransferExecute(struct state* machineState,struct instruction* nextInstruction) {

}

void branchExecute(struct state* machineState,struct instruction* nextInstruction) {

}

void execute(struct state* machineState, struct instruction* nextInstruction) {
    uint8_t byte0 = nextInstruction->byte0;
    uint8_t cond = (byte0 >> 4);


    if(isCond(machineState,nextInstruction)) {
        switch (getInstructionClass(nextInstruction)) {
            case DataProcess :
                dataProcessExecute(machineState,nextInstruction);
                break;
            case Multiply :
                multiplyExecute(machineState,nextInstruction);
                break;
            case DataTransfer :
                dataTransferExecute(machineState,nextInstruction);
                break;
            case Branch :
                branchExecute(machineState,nextInstruction);
                break;
        }
    }

}

struct instruction fetch(struct state* machineState) {


    uint8_t byte0 = machineState->memory[machineState->registers[15]];
    uint8_t byte1 = machineState->memory[machineState->registers[15]+1];
    uint8_t byte2 = machineState->memory[machineState->registers[15]+2];
    uint8_t byte3 = machineState->memory[machineState->registers[15]+3];

    //pc + 4
    machineState->registers[15] = machineState->registers[15]+4;

    struct instruction next = {byte0,byte1,byte2,byte3,((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3)};


    return next;
}

int main(int argc, char *argv[]) {

    //execute each instruction;

    struct state machineState;
    FILE *fileptr;
    long filelen;

    fileptr = fopen(argv[1], "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);
    // Jump back to the beginning of the file

    for(int i =0; i < 65536; i++) {
        machineState.memory[i] = 0;
    }



    for(int i = 0; i <17; i++) {
        machineState.registers[i] = 0;
    }

    fread(&machineState.memory, filelen+1, 1, fileptr); // Read in the entire file
    fclose(fileptr);


    struct instruction nextInstruction = fetch(&machineState);
    while(nextInstruction.allbytes != HALT) {

        execute(&machineState,&nextInstruction);
        printf("%x\n",nextInstruction.allbytes);
        nextInstruction = fetch(&machineState);
    }

    return 0;
}





