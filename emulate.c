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

void arithmeticLogicUnit(struct state* machineState,struct instruction* nextInstruction) {
    //perform arithmetic
    //if arithhmetic operation update CSPR C to carry out of ALU
}

int32_t barrelShift(struct state* machineState,struct instruction* nextInstruction) {
    //perform shift
    //if logic operation update CSPR C to barrel carry out
    //continue to ALU
    //do this if I set
    uint32_t operand2 = nextInstruction->allbytes & 0xFFF;
    uint8_t numberOfBitsToRotate = operand2 >> 8;
    uint32_t immediate = operand2 & 0xFF;
    uint32_t shiftedOperand = rotr32(immediate,numberOfBitsToRotate);

    //fetch register and shift if I not set


    arithmeticLogicUnit(machineState,nextInstruction);

}

void dataProcessExecute(struct state* machineState,struct instruction* nextInstruction) {
    uint32_t allBytes = nextInstruction->allbytes;
    uint32_t rnRegister = machineState->registers[(allBytes>> 16) & 0xF];

    barrelShift(machineState,nextInstruction);
    uint32_t rdRegister = machineState->registers[(allBytes>> 12) & 0xF];

    //update CSPR_N to 31st bit of dest register
    machineState->registers[CSPR] |= (rdRegister & 0x80000000);

    //set CSPR_Z if dest register is all zero
    if(rdRegister == 0x0) {
        machineState->registers[CSPR] |= (0x40000000);
    }
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





