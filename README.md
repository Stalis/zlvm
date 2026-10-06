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

## Current Limitations

- Raw `.bin` files remain the compatibility output; versioned object/executable images are exposed
  through the `ZlImage` codec and VM image loader.
- The versioned executable-image workflow is separate from raw `.bin` compatibility mode.

## Roadmap: Running `test.asm`

[`test.asm`](test.asm) is the canonical target program. It exercises procedure-local labels,
conditional control flow, calls and returns, the stack, RAM access, strings, numeric data
directives, character output, and halting. Work should proceed in the following order.

### Milestone 1: Reliable direct execution

- [x] Fix both command-line source readers to allocate space for a terminating NUL byte, never
  store `EOF`, detect read errors, and release the source buffer after assembly.
- [x] Allocate a complete parser `LineList` and initialize every temporary `TokenList` link.
- [x] Make data-directive encoding safe across `realloc`, advance numeric output by the emitted
  type width, and initialize `.space` deterministically.
- [x] Keep each encoded instruction alive until it is copied, initialize all fields, and retain
  output offsets if the translation buffer moves during growth.
- [x] NUL-terminate procedure-local labels and make label lookup compare complete names instead of
  matching prefixes.
- [x] Stop RAM writes from falling through into ROM, validate every byte of multi-byte memory
  accesses, and preserve ROM during stack operations.
- [x] Add `test.asm` as an automated CTest integration fixture and verify it in Debug, Release, and
  sanitizer builds.

Milestone 1 is complete: `zlvm test.asm` has no sanitizer-detected invalid memory access, prints the
characters encoded by the program (`Hello, World!\n\nBye!\n\n`), reaches `S_HALTED`, stores
`0x1024` at logical address `5192`, and does not change ROM as a side effect of RAM or stack writes.

### Milestone 2: Stable binary workflow

- [x] Support `zlasm test.asm -o test.bin` without modifying `argv` storage and report input/output
  failures with a nonzero exit status.
- [x] Support `zlvm --binary test.bin` while retaining direct `zlvm test.asm` execution.
- [x] Remove the Debug-only parser path that executes the program and exits before writing a file.
- [x] Replace native-structure serialization with a fixed-width, explicitly endian-defined binary
  encoding shared by the assembler and VM loader.
- [x] Prove source and binary execution are equivalent by comparing output, halt state, registers,
  and relevant memory.

### Milestone 3: Assembler and ISA completion

- [x] Make the assembler emit versioned text/data sections, alignment, explicit locations, and
  entry-point selection.
- [x] Define the versioned object-file contract for `.global` and `.extern`, and resolve symbols
  through the static linker. The unused `factorial` declaration in `test.asm` remains valid.
- [ ] Implement syscall behavior and remaining structured diagnostics for invalid input.
- [ ] Keep the ISA reference and integration tests synchronized with every completed feature.

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

## Development status

The assembler and emulator support direct source execution and standalone binary ROM images. The end-to-end reference program exercises control flow, procedures, stack and RAM access, strings, data directives, macros, character output, and clean VM halting.

Remaining work includes versioned executable images, sections and alignment, entry-point metadata, relocation, external symbols, and linking. Active work is tracked in [GitHub Issues](../../issues).
