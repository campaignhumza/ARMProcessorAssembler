#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "emulate.h"

struct state {
    uint8_t memory[65536];
    uint32_t registers[15];
};

struct instruction {
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t cond;

 uint32_t allbytes;
 uint32_t swapped;
};

enum InstructionClass{DataProcess, Multiply, DataTransfer, Branch};

enum ShiftType{LSL, LSR, ASR, ROR};

struct state machineState;

bool isCSPR_N() {
    return (machineState.registers[CSPR] >> 31 & 1);
}

bool isCSPR_Z() {
    return (machineState.registers[CSPR]>> 30 & 1);
}

bool isCSPR_C() {
    return (machineState.registers[CSPR]>> 29 & 1);
}

bool isCSPR_V() {
    return (machineState.registers[CSPR]>> 28 & 1);
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

bool isCond(struct instruction* nextInstruction) {
    uint32_t allBytes = nextInstruction->allbytes;
    uint8_t cond = ((allBytes >> 28) & 0xF);

    switch(cond) {
        case eq  :
            return isCSPR_Z();
        case ne  :
            return !isCSPR_Z();
        case ge  :
            return isCSPR_N() == isCSPR_V();
        case lt  :
            return isCSPR_N() != isCSPR_V();
        case gt  :
            return !isCSPR_Z() && (isCSPR_N() == isCSPR_V());
        case le  :
            return (isCSPR_Z() || (isCSPR_N() != isCSPR_V()));
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

void arithmeticLogicUnit(struct instruction* nextInstruction,uint32_t operand1,uint32_t operand2,bool updateCSPR,uint32_t* carryOut) {
    //perform arithmetic
    //if arithhmetic operation update CSPR C to carry out of ALU
            uint32_t allBytes = nextInstruction->allbytes;
            uint8_t destRegisterAddress = allBytes >> 12 & 0xF;
            uint32_t * destRegister = &(machineState.registers[destRegisterAddress]);
            uint32_t result;

            if((allBytes & movMask) == movMask) {
                *destRegister = operand2;

            } else if((allBytes & eorMask) == eorMask) {
                result = operand1 ^ operand2;
                *destRegister = result;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState.registers[CSPR] |= (0x40000000);
                    }
                    machineState.registers[CSPR] |= (result & 0x80000000);

                }
            } else if((allBytes & subMask) == subMask) {
                result = operand1 - operand2;
                *destRegister = result;
                if(result > (operand2)) {
                    //underflow
                    *carryOut = 0;
                } else {
                    *carryOut = 1;
                }
            } else if((allBytes & rsbMask) == rsbMask) {
                result = operand2 - operand1;
                *destRegister = result;
                *destRegister = operand2 - operand1;
                if(result > (operand1)) {
                    //underflow
                    *carryOut = 0;
                } else {
                    *carryOut = 1;
                }
            } else if((allBytes & addMask) == addMask) {
                result = operand1 + operand2;
                *destRegister = operand2 + operand1;
                if(result < operand1) {
                    //overflow
                    *carryOut = 1;
                } else {
                    *carryOut = 0;
                }
            } else if((allBytes & tstMask) == tstMask) {
                result = operand1 & operand2;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState.registers[CSPR] |= (0x40000000);
                    }
                    machineState.registers[CSPR] |= (result & 0x80000000);
                }

            } else if((allBytes & teqMask) == teqMask) {
                result = operand1 ^ operand2;
                if(updateCSPR) {
                if(result == 0x0) {
                    machineState.registers[CSPR] |= (0x40000000);
                }
                machineState.registers[CSPR] |= (result & 0x80000000);

            }

            } else if((allBytes & cmpMask) == cmpMask) {
                result = operand1 - operand2;
                if(result == 0x0 && updateCSPR) {
                    machineState.registers[CSPR] |= (0x40000000);
                }
                if(result > (operand2)) {
                    //underflow
                    *carryOut = 0;
                } else {
                    *carryOut = 1;
                }
            } else if((allBytes & orrMask) == orrMask) {
                result = operand1 | operand2;
                *destRegister = result;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState.registers[CSPR] |= (0x40000000);
                    }
                    machineState.registers[CSPR] |= (result & 0x80000000);

                }
            } else {
                //mov
                result = operand1 & operand2;
                *destRegister = result;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState.registers[CSPR] |= (0x40000000);
                    }
                    machineState.registers[CSPR] |= (result & 0x80000000);

                }
            }
}

uint32_t performShift(uint8_t shiftType, uint32_t contentToShiftOn, uint32_t amountToShiftBy, uint32_t * carry) {

// >> operator is logaical shift if int is unisgned and is arithmetic if int is signed.
    uint32_t operand2;
    if((shiftType & lsl) == lsl) {
        *carry = ((contentToShiftOn << (amountToShiftBy-1)) & 0x80000000) >> 31;
        operand2 = contentToShiftOn << amountToShiftBy;
        return operand2;
    } else if((shiftType & lsr) == lsr) {
        *carry = (contentToShiftOn >> (amountToShiftBy-1)) & 1;
        operand2 = contentToShiftOn >> amountToShiftBy;
        return operand2;
    } else if((shiftType & asr) == asr) {
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

void barrelShift(struct instruction* nextInstruction,bool updateCSPR) {
    //perform shift
    //if logic operation update CSPR C to barrel carry out
    //continue to ALU
//get operand1 from register.
    uint32_t allBytes = nextInstruction->allbytes;
    uint32_t operand1 = machineState.registers[(allBytes>> 16) & 0xF];
    uint32_t operand2 = allBytes & 0xFFF;
    uint32_t carry;

    bool IisSet = ((nextInstruction->allbytes & 0x2000000) == 0x2000000);
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
            operand2 = performShift(shiftType,machineState.registers[rmRegister],integer,&carry);
        } else {
           uint32_t rsRegister = (shift >> 4);
           uint8_t bottomByteOfrsRegister = (machineState.registers[rsRegister]);
           operand2 = performShift(shiftType,machineState.registers[rmRegister],bottomByteOfrsRegister,&carry);
        }
    }

    arithmeticLogicUnit(nextInstruction,operand1,operand2,updateCSPR,&carry);

    if (updateCSPR) {
        machineState.registers[CSPR] |= (carry << 29);
    }
}

void dataProcessExecute(struct instruction* nextInstruction) {
    uint32_t allBytes = nextInstruction->allbytes;

    bool updateCSPR = (allBytes & (1<<20));

    barrelShift(nextInstruction,updateCSPR);
}

void multiplyExecute(struct instruction* nextInstruction) {

}

void dataTransferExecute(struct instruction* nextInstruction) {

}

void branchExecute(struct instruction* nextInstruction) {

}

void execute(struct instruction* nextInstruction) {

    if(isCond(nextInstruction)) {
        switch (getInstructionClass(nextInstruction)) {
            case DataProcess :
                dataProcessExecute(nextInstruction);
                break;
            case Multiply :
                multiplyExecute(nextInstruction);
                break;
            case DataTransfer :
                dataTransferExecute(nextInstruction);
                break;
            case Branch :
                branchExecute(nextInstruction);
                break;
        }
    }
}

struct instruction fetch() {
    uint8_t byte0 = machineState.memory[machineState.registers[13]];
    uint8_t byte1 = machineState.memory[machineState.registers[13]+1];
    uint8_t byte2 = machineState.memory[machineState.registers[13]+2];
    uint8_t byte3 = machineState.memory[machineState.registers[13]+3];

    //pc + 4
    machineState.registers[13] = machineState.registers[13]+4;

    struct instruction next = {byte0,byte1,byte2,byte3,((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0)};

    return next;
}

void printState() {
    printf("Registers:\n");
    for(int i = 0; i <15; i++) {
        if(i < 13) {
                printf("$%-3d:%10d (0x%08x)\n",i,machineState.registers[i],machineState.registers[i]);


        } else {
            if(i == 13) {
                printf("PC  :%10d (0x%08x)\n",machineState.registers[i],machineState.registers[i]);
            } else {
                printf("CSPR:%10d (0x%08x)\n",machineState.registers[i],machineState.registers[i]);
            }
        }
    }
    printf("Non-zero memory:\n");

    for(int i = 0; i < 65537; i++) {
        uint8_t byte0 = machineState.memory[i];
        uint8_t byte1 = machineState.memory[i+1];
        uint8_t byte2 = machineState.memory[i+2];
        uint8_t byte3 = machineState.memory[i+3];
        uint32_t allBytes = ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3);
        if(allBytes != HALT) {
            printf("0x%08x: 0x%08x\n",i,allBytes);
            i+=3;
        } else {
            break;
        }

    }
}

void binaryLoad(char* binPath) {
    FILE *fileptr;
    long filelen;

    fileptr = fopen(binPath, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);
    // Jump back to the beginning of the file

    for(int i =0; i < 65536; i++) {
        machineState.memory[i] = 0;
    }

    for(int i = 0; i <16; i++) {
        machineState.registers[i] = 0;
    }

    fread(&machineState.memory, filelen+1, 1, fileptr); // Read in the entire file
    fclose(fileptr);
}

int main(int argc, char *argv[]) {

    //binary Load
    binaryLoad(argv[1]);

    //fetch execute cycle
    struct instruction nextInstruction = fetch();
    while(nextInstruction.allbytes != HALT) {

        execute(&nextInstruction);
        nextInstruction = fetch();
    }
    machineState.registers[13] = machineState.registers[13]+4;

    //print state
    printState();

    return 0;
}





