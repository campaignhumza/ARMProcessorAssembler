#include "emulate.h"
#include "dataProcess.h"

struct state machineState;

void arithmeticLogicUnit(uint32_t nextInstruction,uint32_t operand1,uint32_t operand2) {
    //perform arithmetic
    //if arithhmetic operation update CSPR C to carry out of ALU
            uint32_t allBytes = nextInstruction;
            uint8_t destRegisterAddress = allBytes >> 12 & 0xF;
            uint32_t * destRegister = &(machineState.registers[destRegisterAddress]);
            uint32_t result;
    bool updateCSPR = (nextInstruction & (1<<20));

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
                if (updateCSPR) {
                    if (result > (operand2)) {
                        //underflow
                        machineState.registers[CSPR] |= (0 << 30);
                    } else {
                        machineState.registers[CSPR] |= (1 << 30);
                    }
                }
            } else if((allBytes & rsbMask) == rsbMask) {
                result = operand2 - operand1;
                *destRegister = result;
                *destRegister = operand2 - operand1;
                if (updateCSPR) {
                    if (result > (operand1)) {
                        //underflow
                        machineState.registers[CSPR] |= (0 << 30);
                    } else {
                        machineState.registers[CSPR] |= (1 << 30);
                    }
                }
            } else if((allBytes & addMask) == addMask) {
                result = operand1 + operand2;
                *destRegister = operand2 + operand1;
                if (updateCSPR) {
                    if (result < operand1) {
                        //overflow
                        machineState.registers[CSPR] |= (1 << 30);
                    } else {
                        machineState.registers[CSPR] |= (0 << 30);
                    }
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
                if (updateCSPR) {
                    if (result > (operand2)) {
                        //underflow
                        machineState.registers[CSPR] |= (0 << 30);
                    } else {
                        machineState.registers[CSPR] |= (1 << 30);
                    }
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
                //and
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

void multiplyExecute(struct state * machineState,uint32_t nextInstruction) {
    uint8_t RdRegisterNumber = (nextInstruction >> 16) & 0xF;
    uint8_t RmRegisterNumber = nextInstruction & 0xF;
    uint8_t RsRegisterNumber = (nextInstruction >> 8) & 0xF;
    uint8_t RnRegisterNumber = (nextInstruction >> 12) &0XF;

    bool isAccumulate = ((nextInstruction >> 21) & 1) == 1;
    if(isAccumulate) {
        machineState->registers[RdRegisterNumber] = (machineState->registers[RmRegisterNumber] * machineState->registers[RsRegisterNumber]) + machineState->registers[RnRegisterNumber] ;
    } else {
        machineState->registers[RdRegisterNumber] = (machineState->registers[RmRegisterNumber] * machineState->registers[RsRegisterNumber]);
    }

    machineState->registers[CSPR] |= (machineState->registers[RdRegisterNumber] == 0) << 30;
    machineState->registers[CSPR] |= ((machineState->registers[RdRegisterNumber] & 0x80000000) >>2);

}

void dataTransferExecute(uint32_t nextInstruction) {

}

void branchExecute(uint32_t nextInstruction) {

}

void dataProcessExecute(struct state machineState, uint32_t nextInstruction) {

    uint32_t operand1 = machineState.registers[(nextInstruction>> 16) & 0xF];
    uint32_t operand2 = barrelShift(machineState,nextInstruction);

    arithmeticLogicUnit(nextInstruction,operand1,operand2);

}

void execute(uint32_t nextInstruction, enum InstructionClass instructionClass) {

    if(isCond(nextInstruction, machineState.registers[CSPR])) {
        switch (instructionClass) {
            case DataProcess :
                dataProcessExecute(machineState,nextInstruction);
                break;
            case Multiply :
                multiplyExecute(&machineState,nextInstruction);
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

enum InstructionClass decode(uint32_t instruction) {

    if((instruction & branchMask) == branchMask) {
        return Branch;
    }
    if((instruction & multMask) == multInstruction) {
        return Multiply;
    }
    if((instruction & dataTransMask) == dataTransMask) {
        return DataTransfer;
    }else
    {
        return DataProcess;
    }
}

uint32_t fetch() {
    //Get instruction from memory byte by byte
    uint8_t byte0 = machineState.memory[machineState.registers[PC]];
    uint8_t byte1 = machineState.memory[machineState.registers[PC]+1];
    uint8_t byte2 = machineState.memory[machineState.registers[PC]+2];
    uint8_t byte3 = machineState.memory[machineState.registers[PC]+3];

    //pc + 4
    machineState.registers[PC] = machineState.registers[PC]+4;

    //concat bytes into 32 bit instruction
    uint32_t nextInstruction = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);

    return nextInstruction;
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
    uint32_t nextInstruction = fetch();
    while(nextInstruction != HALT) {

        execute(nextInstruction, decode(nextInstruction));
        nextInstruction = fetch();
    }

    machineState.registers[PC] = machineState.registers[PC]+4;

    //print state
    printState();

    return 0;
}





