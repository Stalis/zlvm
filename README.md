# ZL Virtual Machine

ZLVM is a C11 assembler and virtual machine for a custom 32-bit CPU architecture. It includes a standalone assembler (`zlasm`), an emulator (`zlvm`), a documented instruction set, and automated integration and sanitizer tests.

Assembly programs can be executed directly from source or compiled into a fixed-width, little-endian ROM image and loaded separately by the emulator.

## Key capabilities

- Custom 32-bit machine model with 32 registers
- Fixed-width 64-bit instruction encoding
- Lexer, parser, labels, directives, and macro expansion
- Arithmetic, conditional execution, calls, returns, and stack operations
- Separate ROM and RAM address spaces
- Interrupt-based host I/O
- Direct source execution and standalone binary workflow
- Structured assembler diagnostics
- Verified source/binary execution equivalence
- Documented ISA and automated Debug, Release, and sanitizer testing

## Architecture

```text
Assembly source
      |
      v
Lexer -> Parser -> Directives, labels and macros -> Encoder
                                                     |
                                                     v
                                              ROM image
                                                     |
                                                     v
                                Virtual CPU: registers, ALU,
                                      ROM, RAM and stack
                                                     |
                                                     v
                                      Interrupt-based host I/O
```

The repository contains two primary components:

- `asm/` — tokenization, parsing, macro expansion, directives, labels, diagnostics, and instruction encoding.
- `emulator/` — processor state, ALU, memory access, instruction dispatch, conditions, stack operations, and interrupts.

See the [ZL Virtual CPU ISA](docs/ISA.md) for the complete machine and assembly-language reference.

## Development status

The assembler and emulator support direct source execution and standalone binary ROM images. The end-to-end reference program exercises control flow, procedures, stack and RAM access, strings, data directives, macros, character output, and clean VM halting.

Remaining work includes versioned executable images, sections and alignment, entry-point metadata, relocation, external symbols, and linking. Active work is tracked in [GitHub Issues](../../issues).
