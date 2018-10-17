#include "emulate.h"

MachineState machineState = {};
FetchedInstruction fetchedInstruction= {};
DecodedInstruction decodedInstruction = {};

const uint32_t HALT = 0x00000000;
const uint32_t dataProcMask = 0x0;
const uint32_t multMask = 0xFC000F0;
const uint32_t multInstruction = 0x90;
const uint32_t dataTransMask = 0xC600000;
const uint32_t dataTransInstruction = 0x4000000;
const uint32_t branchMask = 0xF000000;
const uint32_t branchInstruction = 0xA000000;
const int32_t branchOffSetMask = 0xFFFFFF;

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
}

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





