cmake_minimum_required(VERSION 3.13)
project(zlvm_c C)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_C_STANDARD 11)

add_executable(zlvm
        src/main.c

        src/common/Value.h
        src/common/Types.h
        src/common/State.h
        src/common/Instruction.h
        src/common/Condition.h
        src/common/Opcode.h
        src/common/Error.h
        src/common/Registers.h

        src/emulator/CPSR.h
        src/emulator/ALU.h              src/emulator/ALU.c
        src/emulator/VirtualMachine.h   src/emulator/VirtualMachine.c

        src/asm/asm.c        src/asm/asm.h
        src/asm/Token.c      src/asm/Token.h
        src/asm/Lexer.c      src/asm/Lexer.h