# ZL Virtual Machine

ZLVM is an experimental assembler and virtual machine for the ZL virtual CPU. The project is
written in C11 and contains both an assembler (`zlasm`) and an emulator (`zlvm`). Assembly source
can be translated in memory and executed by the emulator, or assembled separately for future
binary-loading workflows.

The project is under active development. The instruction set is largely implemented, and
`test.asm` now runs as the end-to-end reference program. The assembler and VM still have binary
format and feature-completeness work remaining; see
[Roadmap: Running `test.asm`](#roadmap-running-testasm) for the current status.

## Architecture

```text
Assembly source
      |
      v
 Lexer -> Parser -> Directive and label passes -> Instruction/data encoder
                                                        |
                                                        v
                                           ROM image / future .bin file
                                                        |
                                                        v
                                      ZL virtual CPU (registers, ALU, RAM)
                                                        |
                                                        v
                                              Interrupt-based host I/O
```

The repository has two main components:

- `asm/` implements tokenization, parsing, directives, labels, and instruction encoding. It builds
  the `zlasm` library and command-line program.
- `emulator/` implements the processor state, ALU, memory access, instruction dispatch, conditions,
  and interrupts. It builds the `zlvm` library and command-line program.

Public headers are in `include/`. The current VM exposes initialization, ROM loading, and execution
through `VirtualMachine.h`; the assembler exposes in-memory assembly through `asm/zlasm.h`.

For the complete virtual CPU reference, see [ZL Virtual CPU ISA](docs/ISA.md).

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

The standalone assembler supports an optional output path:

```sh
./build/asm/zlasm program.asm -o program.bin
```

Without `-o`, it replaces the input extension with `.bin`. The output is a raw little-endian ROM
image using the instruction and data layout defined in the [ISA reference](docs/ISA.md).

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

- Binary images have no header, format version, sections, entry-point metadata, or relocation
  records.
- Sections, alignment, explicit placement, entry selection, macros, external symbols, and linking
  are incomplete or metadata-only.

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
- [ ] Support `zlvm --binary test.bin` while retaining direct `zlvm test.asm` execution.
- [x] Remove the Debug-only parser path that executes the program and exits before writing a file.
- [x] Replace native-structure serialization with a fixed-width, explicitly endian-defined binary
  encoding shared by the assembler and VM loader.
- [ ] Prove source and binary execution are equivalent by comparing output, halt state, registers,
  and relevant memory.

### Milestone 3: Assembler and ISA completion

- [ ] Implement text/data sections, alignment, explicit locations, and entry-point selection.
- [ ] Define object-file and linker behavior for `.global` and `.extern`. The unused `factorial`
  declaration in `test.asm` does not block Milestones 1 or 2.
- [ ] Implement macros, syscall behavior, and structured diagnostics for invalid input.
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
