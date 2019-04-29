cmake_minimum_required(VERSION 3.13)
project(zlvm_c C)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_C_STANDARD 11)

add_executable(zlvm_c src/main.c
        src/Value.h src/Common.h
        src/State.h
        src/Instruction.h src/Condition.h src/Opcode.h src/CPSR.h src/ALU.h
        src/VirtualMachine.h src/VirtualMachine.c src/ALU.c src/asm/asm.c src/asm/asm.h)