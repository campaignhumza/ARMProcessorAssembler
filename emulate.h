//
// Created by el djebena on 26/05/2018.
//

#ifndef EMULATOR_EMULATE_H
#define EMULATOR_EMULATE_H


#define CSPR 14
#define PC 13

const uint32_t HALT = 0x00000000;

const uint8_t eq = 0x0;
const uint8_t ne = 0x1;
const uint8_t ge = 0xA;
const uint8_t lt = 0xB;
const uint8_t gt = 0xC;
const uint8_t le = 0xD;
const uint8_t al = 0xE;

const uint32_t dataProcMask = 0x0;
const uint32_t multMask = 0x90;
const uint32_t dataTransMask = 0x8000000;
const uint32_t branchMask = 0xA000000;

const uint32_t andMask = 0x0;
const uint32_t eorMask = 0x200000;
const uint32_t subMask = 0x400000;
const uint32_t rsbMask = 0xC00000;
const uint32_t addMask = 0x800000;
const uint32_t tstMask = 0x2000000;
const uint32_t teqMask = 0x2400000;
const uint32_t cmpMask = 0x2800000;
const uint32_t orrMask = 0x3000000;
const uint32_t movMask = 0x1A00000;

const uint8_t lsl = 0x0;
const uint8_t lsr = 0x1;
const uint8_t asr = 0x2;
const uint8_t ror = 0x3;

//struct instruction fetchNext(struct state* machineState);
#endif //EMULATOR_EMULATE_H
