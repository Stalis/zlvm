# ZL Virtual Machine

[![CMake CI](https://github.com/Stalis/zlvm/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/Stalis/zlvm/actions/workflows/cmake-single-platform.yml)

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
- Section-aware relocatable objects and static linking
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

ZLVM supports direct execution from assembly source, standalone raw ROM images, and a section-aware object and linking workflow. The assembler can emit relocatable `.zlo` objects, while `zllink` resolves symbols and relocations and produces versioned `.zle` executable images with a configurable entry point.

The end-to-end test suite covers control flow, procedures, stack and RAM access, strings, data directives, macros, object linking, character output, and clean VM halting. Further ISA and toolchain development is tracked in [GitHub Issues](../../issues).

## Requirements

- CMake 4.4.2 or newer
- A C11 compiler

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The primary executables are generated at:

- `build/emulator/zlvm`
- `build/asm/zlasm`
- `build/linker/zllink`

An installation can be staged with:

```sh
cmake --install build --prefix install
```

## Usage

The intended direct source workflow is:

```sh
./build/emulator/zlvm test.asm
```

`zlvm` assembles the source in memory, loads the resulting image into ROM, starts execution at ROM
address zero, and runs until the VM halts or enters an error state.

To execute an image assembled separately, use the explicit `--binary` option:

```sh
./build/asm/zlasm program.asm -o program.bin
./build/emulator/zlvm --binary program.bin
```

The standalone assembler supports an optional output path:

```sh
./build/asm/zlasm program.asm -o program.bin
```

Without `-o`, it replaces the input extension with `.bin`. The output is a raw little-endian ROM
image using the instruction and data layout defined in the [ISA reference](docs/ISA.md).

To produce and link versioned images:

```sh
./build/asm/zlasm -c program.asm -o program.zlo
./build/linker/zllink -o program.zle program.zlo
```

Use `--entry symbol` with `zllink` to override `.entry`; otherwise the first object entry symbol or
`start` is selected. The resulting executable image can be loaded through the `ZlImage` and
`vm_loadImage` APIs.

## Assembly Example

```asm
start:
    movi $a0, 'A'
    int  0x02
    int  0xFF
```

Registers use a `$` prefix, label references use `#`, and `;` starts a comment. Opcodes are
case-insensitive because the assembler normalizes them before translation. See the
[ISA reference](docs/ISA.md#assembly-language) for operands, literals, conditions, and directives.

## Project Documentation

- [ZL Virtual CPU ISA](docs/ISA.md)
- [Coding Conventions](docs/CODING_CONVENTIONS.md)
- [BSD 2-Clause License](LICENSE)

## Contributing

Before contributing, read the [coding conventions](docs/CODING_CONVENTIONS.md). New behavior should
include focused tests, and changes to instructions, registers, conditions, directives, interrupts,
or binary encoding must update the ISA reference.

## License

ZLVM is available under the [BSD 2-Clause License](LICENSE).
