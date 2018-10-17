cmake_minimum_required(VERSION 3.10)
project(Emulator C)

set(CMAKE_C_STANDARD 99)

add_executable(emulate emulate.c emulate.h dataProcess.c dataProcess.h utility.c utility.h multiply.c multiply.h dataTransfer.c dataTransfer.h branch.c branch.h main.c main.h)