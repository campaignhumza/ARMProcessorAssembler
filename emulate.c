#include "emulate.h"
//#include "dataProcess.h"

MachineState machineState;
//uint32_t fetchedInstruction.instruction;

typedef struct {
    uint32_t instruction;
    bool isvalid;
} FetchedInstruction;

typedef struct {
    uint32_t instruction;
    enum InstructionClass instructionType;
    bool hasValue;
} DecodedInstruction;

DecodedInstruction decodedInstruction = {0, 0};
FetchedInstruction fetchedInstruction;

const uint8_t lsl = 0x0;
const uint8_t lsr = 0x1;
const uint8_t asr = 0x2;
const uint8_t ror = 0x3;

uint32_t setBit(uint32_t number, int bitPos, int bitNewValue) {
    uint32_t bin = number;
    if (bitNewValue) {
        bin |= 1 << bitPos;
        return bin;
    } else {
        bin &= ~(1 << bitPos);
        return bin;
    }

}

uint32_t barrelShift(uint32_t nextInstruction) {

    uint32_t allBytes = nextInstruction;
    uint32_t operand2 = allBytes & 0xFFF;
    bool c_CSPR_BitSet;
    bool updateCSPR = (nextInstruction & (1 << 20));

    bool IisSet = ((allBytes & 0x2000000) == 0x2000000);
    if (IisSet) {
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
        uint8_t shiftOption = (shift & 0x1) & 1;

        if (shiftOption == 0) {
            uint32_t integer = (shift >> 3);
            operand2 = performShift(shiftType, machineState.registers[rmRegister], integer, &c_CSPR_BitSet);
        } else {
            uint32_t rsRegister = (shift >> 4);
            uint8_t bottomByteOfrsRegister = (machineState.registers[rsRegister]);
            operand2 = performShift(shiftType, machineState.registers[rmRegister], bottomByteOfrsRegister,
                                    &c_CSPR_BitSet);
        }
    }

    if (updateCSPR) {
        machineState.registers[CSPR] |= (c_CSPR_BitSet << 30);
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

static uint32_t rotr32(uint32_t n, unsigned int c) {
    const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
    c &= mask;
    return (n >> c) | (n << ((-c) & mask));
}

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

uint32_t fetch() {
    //Get instruction from memory byte by byte
    uint8_t byte0 = machineState.memory[machineState.registers[PC]];
    uint8_t byte1 = machineState.memory[machineState.registers[PC] + 1];
    uint8_t byte2 = machineState.memory[machineState.registers[PC] + 2];
    uint8_t byte3 = machineState.memory[machineState.registers[PC] + 3];


    //pc + 4
    machineState.registers[PC] = machineState.registers[PC] + 4;

    //concat bytes into 32 bit instruction
    uint32_t nextInstruction = ((byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0);

    return nextInstruction;
}

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

void dataProcessExecute(uint32_t nextInstruction) {

    uint32_t operand1 = machineState.registers[(nextInstruction >> 16) & 0xF];
    uint32_t operand2 = barrelShift(nextInstruction);

    arithmeticLogicUnit(nextInstruction, operand1, operand2);
}

void decode(FetchedInstruction nextInstruction) {


    fetchedInstruction.instruction = fetch();

    enum InstructionClass instructionType;
    if ((nextInstruction.instruction & branchMask) == branchInstruction) {
        instructionType = Branch;
        fetchedInstruction.isvalid = true;
    } else if ((nextInstruction.instruction & multMask) == multInstruction) {
        instructionType = Multiply;
        fetchedInstruction.isvalid = true;
    } else if ((nextInstruction.instruction & dataTransMask) == dataTransInstruction) {
        instructionType = DataTransfer;
        fetchedInstruction.isvalid = true;
    } else {
        instructionType = DataProcess;
        fetchedInstruction.isvalid = true;
    }

    decodedInstruction.instructionType = instructionType;
    decodedInstruction.instruction = nextInstruction.instruction;

    if (nextInstruction.isvalid) {
        decodedInstruction.hasValue = true;
    } else {
        decodedInstruction.hasValue = false;
    }

}

void execute(DecodedInstruction decodedInstruction) {
    if (decodedInstruction.hasValue) {


        uint32_t nextInstruction = decodedInstruction.instruction;
        enum InstructionClass instructionClass = decodedInstruction.instructionType;

        if (isCond(nextInstruction, machineState.registers[CSPR])) {
            switch (instructionClass) {
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
    decode(fetchedInstruction);
} //111111111111111111111100

void printState() {
    printf("Registers:\n");
    for (int i = 0; i <= 16; i++) {
        if (i < 13) {
            printf("$%-3d:%11d (0x%08x)\n", i, machineState.registers[i], machineState.registers[i]);


        } else {
            if (i >= 15) {
                if (i == 15) {
                    printf("PC  :%11d (0x%08x)\n", machineState.registers[i], machineState.registers[i]);
                } else {
                    printf("CSPR:%11d (0x%08x)\n", machineState.registers[i], machineState.registers[i]);
                }
            }
        }
    }
    printf("Non-zero memory:\n");

    for (int i = 0; i < 65535; i++) {
        uint8_t byte0 = machineState.memory[i];
        uint8_t byte1 = machineState.memory[i + 1];
        uint8_t byte2 = machineState.memory[i + 2];
        uint8_t byte3 = machineState.memory[i + 3];
        uint32_t allBytes = ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3);
        if (allBytes != HALT) {
            printf("0x%08x: 0x%08x\n", i, allBytes);

        }
        i += 3;

    }
}

void binaryLoad(char *binPath) {

    FILE *fileptr;
    long filelen;

    fileptr = fopen(binPath, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);
    // Jump back to the beginning of the file

    //cleanup the memory
    for (int i = 0; i < 65535; i++) {
        machineState.memory[i] = 0;
    }

    for (int i = 0; i < 16; i++) {
        machineState.registers[i] = 0;
    }

    fread(&machineState.memory, filelen + 1, 1, fileptr); // Read in the entire file
    fclose(fileptr);
}

int main(int argc, char *argv[]) {

    //binary Load
    binaryLoad(argv[1]);

    //struct state stateVar = machineState;
    //fetch execute cycle
    fetchedInstruction.instruction = fetch();
    fetchedInstruction.isvalid = true;
    decode(fetchedInstruction);
    while (!decodedInstruction.hasValue || decodedInstruction.instruction != HALT) {

        execute(decodedInstruction);

    }

    //print state
    printState();

    return 0;
}





