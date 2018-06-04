#include "emulate.h"
#include "dataProcess.h"

struct state machineState;

void arithmeticLogicUnit(uint32_t nextInstruction,uint32_t operand1,uint32_t operand2) {
    //perform arithmetic
    //if arithhmetic operation update CSPR C to carry out of ALU
            uint32_t allBytes = nextInstruction;
            uint8_t destRegisterNumber = allBytes >> 12 & 0xF;
            uint32_t * destRegister = &(machineState.registers[destRegisterNumber]);
            int32_t result;
    bool updateCSPR = (nextInstruction & (1<<20));

            if((allBytes & opCodeMask) == movOpcode) {
                *destRegister = operand2;

            } else if((allBytes & opCodeMask) == eorOpcode) {
                result = operand1 ^ operand2;
                *destRegister = result;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState.registers[CSPR] |= (0x40000000);
                    }
                    machineState.registers[CSPR] |= (result & 0x80000000);

                }
            } else if((allBytes & opCodeMask) == subOpcode) {
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
            } else if((allBytes & opCodeMask) == rsbOpcode) {
                result = operand2 - operand1;
                *destRegister = result;
                if (updateCSPR) {
                    if (result > (operand1)) {
                        //underflow
                        machineState.registers[CSPR] |= (0 << 30);
                    } else {
                        machineState.registers[CSPR] |= (1 << 30);
                    }
                }
            } else if((allBytes & opCodeMask) == addOpcode) {
                result = operand1 + operand2;
                *destRegister = result;
                if (updateCSPR) {
                    if (result < operand1) {
                        //overflow
                        machineState.registers[CSPR] |= (1 << 30);
                    } else {
                        machineState.registers[CSPR] |= (0 << 30);
                    }
                }
            } else if((allBytes & opCodeMask) == tstOpcode) {
                result = operand1 & operand2;
                if(updateCSPR) {
                    if(result == 0x0) {
                        machineState.registers[CSPR] |= (0x40000000);
                    }
                    machineState.registers[CSPR] |= (result & 0x80000000);
                }

            } else if((allBytes & opCodeMask) == teqOpcode) {
                result = operand1 ^ operand2;
                if(updateCSPR) {
                if(result == 0x0) {
                    machineState.registers[CSPR] |= (0x40000000);
                }
                machineState.registers[CSPR] |= (result & 0x80000000);

            }

            } else if((allBytes & opCodeMask) == cmpOpcode) {
                result = operand1 - operand2;
                if(result == 0x0 && updateCSPR) {
                    //set Z
                    machineState.registers[CSPR] |= (0x40000000);
                }

                if (updateCSPR) {
                    if (result > (operand1)) {
                        //underflow
                        //unset C
                        machineState.registers[CSPR] |= (0 << 29);
                    } else {
                        machineState.registers[CSPR] |= (1 << 29);
                    }

                    if(result < 0) {
                        //set N
                        machineState.registers[CSPR] |= ((result >> 31) & 1) << 31;
                    }
                }
            } else if((allBytes & opCodeMask) == orrOpcode) {
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

    bool c_CSPR_BitSet = false;
    uint32_t offset = nextInstruction & 0xFFF;
    if(((nextInstruction >> 25) & 1) == 1) {
        uint32_t rmRegister = offset & 0xF;
        uint8_t shift = (offset >> 4);
        uint8_t shiftType = ((shift >> 1) & 0x3);
        uint8_t shiftOption =  (shift & 0x1) & 1;

        if(shiftOption == 1) {
            uint32_t integer = (shift >> 3);
            offset = performShift(shiftType,machineState.registers[rmRegister],integer,&c_CSPR_BitSet);
        } else {
            uint32_t rsRegister = (shift >> 4);
            uint8_t bottomByteOfrsRegister = (machineState.registers[rsRegister]);
            offset = performShift(shiftType,machineState.registers[rmRegister],bottomByteOfrsRegister,&c_CSPR_BitSet);
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
    if(RnRegister == PC) {
        offset += 4;
    }
    if(PisSet) {
        if(LisSet) {
            if(UisSet) {
                byte0 = machineState.memory[machineState.registers[RnRegister] + offset];
                byte1 = machineState.memory[machineState.registers[RnRegister] + offset + 1];
                byte2 = machineState.memory[machineState.registers[RnRegister] + offset + 2];
                byte3 = machineState.memory[machineState.registers[RnRegister] + offset + 3];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] =  word;
            } else {
                byte0 = machineState.memory[machineState.registers[RnRegister] - offset];
                byte1 = machineState.memory[machineState.registers[RnRegister] - (offset + 1)];
                byte2 = machineState.memory[machineState.registers[RnRegister] - (offset + 2)];
                byte3 = machineState.memory[machineState.registers[RnRegister] - (offset + 3)];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] = word;
            }
        } else {
            if(UisSet) {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister] + offset] =  byte0;
                machineState.memory[machineState.registers[RnRegister] + offset + 1] =  byte1;
                machineState.memory[machineState.registers[RnRegister] + offset + 2] =  byte2;
                machineState.memory[machineState.registers[RnRegister] + offset + 3] =  byte3;
            } else {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister] - offset] =  byte0;
                machineState.memory[machineState.registers[RnRegister] - (offset + 1)] =  byte1;
                machineState.memory[machineState.registers[RnRegister] - (offset + 2)] =  byte2;
                machineState.memory[machineState.registers[RnRegister] - (offset + 3)] =  byte3;
            }
        }
    } else {
        if(LisSet) {
            if(UisSet) {
                byte0 = machineState.memory[machineState.registers[RnRegister]];
                byte1 = machineState.memory[machineState.registers[RnRegister] + 1];
                byte2 = machineState.memory[machineState.registers[RnRegister] + 2];
                byte3 = machineState.memory[machineState.registers[RnRegister] + 3];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] =  word;
                machineState.registers[RnRegister] += offset;
            } else {
                byte0 = machineState.memory[machineState.registers[RnRegister]];
                byte1 = machineState.memory[machineState.registers[RnRegister] + 1];
                byte2 = machineState.memory[machineState.registers[RnRegister] + 2];
                byte3 = machineState.memory[machineState.registers[RnRegister] + 3];
                word = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);
                machineState.registers[RdRegister] =  word;
                machineState.registers[RnRegister] -= offset;
            }
        } else {
            if(UisSet) {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8 ;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister]] =  byte0;
                machineState.memory[machineState.registers[RnRegister] + 1] =  byte1;
                machineState.memory[machineState.registers[RnRegister] + 2] =  byte2;
                machineState.memory[machineState.registers[RnRegister]+ 3] =  byte3;
                machineState.registers[RnRegister] += offset;
            } else {
                byte0 = machineState.registers[RdRegister];
                byte1 = machineState.registers[RdRegister] >> 8 ;
                byte2 = machineState.registers[RdRegister] >> 16;
                byte3 = machineState.registers[RdRegister] >> 24;
                machineState.memory[machineState.registers[RnRegister]] =  byte0;
                machineState.memory[machineState.registers[RnRegister] + 1] =  byte1;
                machineState.memory[machineState.registers[RnRegister] + 2] =  byte2;
                machineState.memory[machineState.registers[RnRegister]+ 3] =  byte3;
                machineState.registers[RnRegister] -= offset;
            }
        }
    }
}

void branchExecute(uint32_t nextInstruction) {
    int32_t signedOffSet = ((nextInstruction & branchOffSetMask)+1) << 2;

    machineState.registers[PC] += (signedOffSet);
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

    if((instruction & branchMask) == branchInstruction) {
        return Branch;
    }
    if((instruction & multMask) == multInstruction) {
        return Multiply;
    }
    if((instruction & dataTransMask) == dataTransInstruction) {
        return DataTransfer;
    } else {
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
    for(int i = 0; i <=16; i++) {
        if(i < 13) {
                printf("$%-3d:%10d (0x%08x)\n",i,machineState.registers[i],machineState.registers[i]);


        } else {
            if(i >= 15) {
                if (i == 15) {
                    printf("PC  :%10d (0x%08x)\n", machineState.registers[i], machineState.registers[i]);
                } else {
                    printf("CSPR:%10d (0x%08x)\n", machineState.registers[i], machineState.registers[i]);
                }
            }
        }
    }
    printf("Non-zero memory:\n");

    for(int i = 0; i < 65535; i++) {
        uint8_t byte0 = machineState.memory[i];
        uint8_t byte1 = machineState.memory[i+1];
        uint8_t byte2 = machineState.memory[i+2];
        uint8_t byte3 = machineState.memory[i+3];
        uint32_t allBytes = ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3);
        if(allBytes != HALT) {
            printf("0x%08x: 0x%08x\n",i,allBytes);

        }
        i+=3;

    }
}

void binaryLoad(char* binPath) {

    FILE* fileptr;
    long filelen;

    fileptr = fopen(binPath, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);
    // Jump back to the beginning of the file

    //cleanup the memory
    for(int i =0; i < 65535; i++) {
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

    struct state stateVar = machineState;
    //fetch execute cycle
    uint32_t nextInstruction = fetch();
    while(nextInstruction != HALT) {

        execute(nextInstruction, decode(nextInstruction));
        printState();
        nextInstruction = fetch();
    }

    machineState.registers[PC] = machineState.registers[PC]+4;

    //print state
    printState();

    return 0;
}





