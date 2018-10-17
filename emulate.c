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

int emulate(char *binPath) {
    //binary Load
    binaryLoad(binPath);

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





