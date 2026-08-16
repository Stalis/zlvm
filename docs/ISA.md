# ZL Virtual CPU Instruction Set Architecture

This document describes the virtual CPU and assembly language implemented by the current ZLVM
source tree. ZLVM is experimental: several declarations are only partially implemented, and the
current binary representation is not a stable interchange format. Such cases are marked below.

## Machine Model

The processor uses 32-bit machine words and exposes 32 registers. Instructions are logically 64
bits wide. The emulator has a 4 KiB ROM followed by runtime-sized RAM; the command-line emulator
currently creates 4 KiB of RAM. The first 256 bytes of RAM are used as stack capacity.

| Logical range | Purpose |
| --- | --- |
| `0x0000`-`0x0fff` | 4096-byte ROM containing instructions and static data |
| `0x1000` onward | RAM; command-line range is `0x1000`-`0x1fff` |
| `0x1000`-`0x10ff` | Intended 256-byte stack region |

The VM initializes `pc` to zero, and initializes both `bp` and `sp` to `0x1000`. A push increments
`sp` by four and then writes a word; a pop reads a word and then decrements `sp` by four.

## Instruction Encoding

The logical instruction layout is:

| Bits | Width | Field |
| --- | ---: | --- |
| `0..7` | 8 | Opcode |
| `8..15` | 8 | Condition |
| `16..23` | 8 | First register |
| `24..31` | 8 | Second register |
| `32..63` | 32 | Immediate value or absolute target address |

The current assembler creates an `Instruction` C bitfield structure and copies its eight bytes
directly into the output. Bitfield allocation order, padding, and byte order are implementation
defined, so this table expresses the intended logical format rather than a portable file format.
A stable binary format must define byte offsets and endianness explicitly.

## Registers

Registers can be written using an alias or a numeric spelling such as `$r10`. Numeric registers are
range-checked, and an unknown alias terminates assembly with a diagnostic.

| Index | Alias | Intended role |
| ---: | --- | --- |
| 0 | `$zero` | Constant zero; declared read-only, but writes are not currently prevented |
| 1 | `$at` | Assembler temporary |
| 2 | `$v0` | Procedure result |
| 3 | `$v1` | Procedure result |
| 4 | `$v2` | Procedure result |
| 5 | `$v3` | Procedure result |
| 6 | `$a0` | Procedure argument |
| 7 | `$a1` | Procedure argument |
| 8 | `$a2` | Procedure argument |
| 9 | `$a3` | Procedure argument |
| 10 | `$t0` | Caller-managed temporary |
| 11 | `$t1` | Caller-managed temporary |
| 12 | `$t2` | Caller-managed temporary |
| 13 | `$t3` | Caller-managed temporary |
| 14 | `$t4` | Caller-managed temporary |
| 15 | `$t5` | Caller-managed temporary |
| 16 | `$t6` | Caller-managed temporary |
| 17 | `$t7` | Caller-managed temporary |
| 18 | `$s0` | Function-across temporary |
| 19 | `$s1` | Function-across temporary |
| 20 | `$s2` | Function-across temporary |
| 21 | `$s3` | Function-across temporary |
| 22 | `$s4` | Function-across temporary |
| 23 | `$s5` | Function-across temporary |
| 24 | `$s6` | Function-across temporary |
| 25 | `$s7` | Function-across temporary |
| 26 | `$k0` | Reserved for an OS kernel |
| 27 | `$sc` | Syscall selector |
| 28 | `$lp` | Link pointer used by `JMPAL` and `RET` |
| 29 | `$sp` | Stack pointer |
| 30 | `$bp` | Stack base pointer |
| 31 | `$pc` | Program counter |

Writing a byte or halfword updates only that member of the register's `Value` union on the current
host. Code should not assume a portable union layout.

## Processor Flags and State

Arithmetic and comparison operations update the current program status register (CPSR).

| Flag | Meaning |
| --- | --- |
| `N` | Result is negative when interpreted as a 32-bit signed word |
| `Z` | Result is zero |
| `C` | Unsigned carry/overflow indication |
| `V` | Signed overflow indication |
| `S` | Last ALU operation used a signed variant |
| `ST` | VM execution state |

Execution states include normal, halted, out of memory, stack overflow, stack underflow, invalid
condition, and invalid opcode. `S_HALTED` has numeric value zero and is the successful terminal
state used as the command-line exit code.

### Conditions

A condition mnemonic can follow any opcode. If omitted, the instruction is unconditional. The
assembler accepts the aliases shown in the third column.

| Code | Condition | Accepted spelling | Runtime status |
| ---: | --- | --- | --- |
| 0 | Unconditional | `un` or omitted | Implemented |
| 1 | Zero set | `zs`, `eq` | Implemented |
| 2 | Zero clear | `zc`, `ne` | Implemented |
| 3 | Negative set | `ns`, `mi` | Implemented |
| 4 | Negative clear | `nc`, `pl` | Implemented |
| 5 | Overflow set | `vs` | Implemented |
| 6 | Overflow clear | `vc` | Implemented |
| 7 | Carry set / unsigned higher-or-same | `cs`, `hs` | Implemented |
| 8 | Carry clear / unsigned lower | `cc`, `lo` | Implemented |
| 9 | Signed-operation flag set | `ss` | Implemented |
| 10 | Signed-operation flag clear | `sc` | Implemented |
| 11 | Unsigned higher | No parser spelling (`uh` is printable) | Runtime implemented |
| 12 | Unsigned lower-or-same | No parser spelling (`ul` is printable) | Runtime implemented |
| 13 | Signed less-than | `lt` | Implemented as `N != V` |
| 14 | Signed less-than-or-equal | `le` | Implemented as `Z || (N != V)` |
| 15 | Signed greater-than | `gt` | Implemented as `!Z && (N == V)` |
| 16 | Signed greater-than-or-equal | `ge` | Implemented as `N == V` |

An unsupported runtime condition changes the VM state to `S_ERR_INVALID_CONDITION`.

## Instruction Set

Opcode mnemonics are normalized to uppercase before lookup. In the syntax column, `rd` is the
first register, `rs` is the second register, and `imm` is a 32-bit immediate or absolute label
address. All target addresses are byte addresses in ROM.

| Code | Mnemonic | Syntax | Current behavior |
| ---: | --- | --- | --- |
| 0 | `NOP` | `nop` | No operation |
| 1 | `POP` | `pop` | Discard one word from the stack |
| 2 | `POPR` | `popr rd` | Pop a word into `rd` |
| 3 | `PUSHR` | `pushr rs` | Push the first register's word value |
| 4 | `PUSHI` | `pushi imm` | Push an immediate word |
| 5 | `DUP` | `dup` | Duplicate the top stack word |
| 6 | `MOVR` | `movr rd, rs` | Copy `rs` to `rd` |
| 7 | `MOVI` | `movi rd, imm` | Copy `imm` to `rd` |
| 8 | `ADDR` | `addr rd, rs` | `rd = rd + rs` |
| 9 | `SUBR` | `subr rd, rs` | `rd = rd - rs` |
| 10 | `MULR` | `mulr rd, rs` | `rd = rd * rs` |
| 11 | `DIVR` | `divr rd, rs` | `rd = rd / rs` (no divide-by-zero guard) |
| 12 | `MODR` | `modr rd, rs` | `rd = rd % rs` (no divide-by-zero guard) |
| 13 | `ADDI` | `addi rd, imm` | `rd = rd + imm` |
| 14 | `SUBI` | `subi rd, imm` | `rd = rd - imm` |
| 15 | `MULI` | `muli rd, imm` | `rd = rd * imm` |
| 16 | `DIVI` | `divi rd, imm` | `rd = rd / imm` (no divide-by-zero guard) |
| 17 | `MODI` | `modi rd, imm` | `rd = rd % imm` (no divide-by-zero guard) |
| 18 | `INT` | `int imm` | Invoke an emulator interrupt |
| 19 | `SYSCALL` | `syscall` | Dispatch using `$sc`; no services implemented |
| 20 | `JMP` | `jmp imm` | Set `pc` to an absolute byte address |
| 21 | `NOT` | `not rd` | `rd = ~rd` |
| 22 | `ANDR` | `andr rd, rs` | `rd = rd & rs` |
| 23 | `ORR` | `orr rd, rs` | `rd = rd \| rs` |
| 24 | `XORR` | `xorr rd, rs` | `rd = rd ^ rs` |
| 25 | `NANDR` | `nandr rd, rs` | `rd = ~(rd & rs)` |
| 26 | `NORR` | `norr rd, rs` | `rd = ~(rd \| rs)` |
| 27 | `ANDI` | `andi rd, imm` | `rd = rd & imm` |
| 28 | `ORI` | `ori rd, imm` | `rd = rd \| imm` |
| 29 | `XORI` | `xori rd, imm` | `rd = rd ^ imm` |
| 30 | `NANDI` | `nandi rd, imm` | `rd = ~(rd & imm)` |
| 31 | `NORI` | `nori rd, imm` | `rd = ~(rd \| imm)` |
| 32 | `INC` | `inc rd` | Increment `rd` |
| 33 | `DEC` | `dec rd` | Decrement `rd` |
| 34 | `LOADB` | `loadb rd, rs, imm` | Load a byte from `rs + imm` into `rd` |
| 35 | `STOREB` | `storeb rd, rs, imm` | Store the low byte of `rd` at `rs + imm` |
| 36 | `LOADH` | `loadh rd, rs, imm` | Load a 16-bit halfword from `rs + imm` |
| 37 | `STOREH` | `storeh rd, rs, imm` | Store the low halfword of `rd` at `rs + imm` |
| 38 | `LOADW` | `loadw rd, rs, imm` | Load a 32-bit word from `rs + imm` |
| 39 | `STOREW` | `storew rd, rs, imm` | Store the word in `rd` at `rs + imm` |
| 40 | `JMPAL` | `jmpal imm` | Save the next `pc` in `$lp`, then jump |
| 41 | `RET` | `ret` | Set `pc` to `$lp` |
| 42 | `CMPR` | `cmpr rd, rs` | Set flags from unsigned `rd - rs`; discard result |
| 43 | `CMPI` | `cmpi rd, imm` | Set flags from unsigned `rd - imm`; discard result |
| 44 | `CMPSR` | `cmpsr rd, rs` | Set flags from intended signed `rd - rs`; discard result |
| 45 | `CMPSI` | `cmpsi rd, imm` | Set flags from intended signed `rd - imm`; discard result |
| 46 | `ADDSR` | `addsr rd, rs` | Intended signed `rd = rd + rs` |
| 47 | `SUBSR` | `subsr rd, rs` | Intended signed `rd = rd - rs` |
| 48 | `MULSR` | `mulsr rd, rs` | Intended signed `rd = rd * rs` |
| 49 | `DIVSR` | `divsr rd, rs` | Intended signed `rd = rd / rs` |
| 50 | `MODSR` | `modsr rd, rs` | Intended signed `rd = rd % rs` |
| 51 | `ADDSI` | `addsi rd, imm` | Intended signed `rd = rd + imm` |
| 52 | `SUBSI` | `subsi rd, imm` | Intended signed `rd = rd - imm` |
| 53 | `MULSI` | `mulsi rd, imm` | Intended signed `rd = rd * imm` |
| 54 | `DIVSI` | `divsi rd, imm` | Intended signed `rd = rd / imm` |
| 55 | `MODSI` | `modsi rd, imm` | Intended signed `rd = rd % imm` |

The signed implementations currently convert unsigned register words to a 64-bit signed type
without explicit 32-bit sign extension. Their negative-value and overflow behavior should be
treated as incomplete.

## Interrupts and Syscalls

`INT` services are implemented directly by the emulator:

| Code | Inputs | Result |
| ---: | --- | --- |
| `0x01` | Low byte of `$a0` | Write `$a0 + 0x60` as one character to standard output |
| `0x02` | Low byte of `$a0` | Write `$a0` as one character to standard output |
| `0x10` | Standard input | Read one character into `$v0` |
| `0xff` | None | Set the VM state to `S_HALTED` |

Unknown interrupts currently do nothing. `SYSCALL` reads its selector from `$sc`, but no syscall
services are implemented.

## Assembly Language

### Statements and labels

The general statement form is:

```text
opcode [condition] [register [, register]] [, immediate-or-label]
```

Examples:

```asm
loop:
    loadb $a0, $t0
    cmpi  $a0, 0
    jmp zs #end
    inc   $t0
    jmp   #loop
end:
    int   0xFF
```

- A label definition ends with `:`.
- A label reference begins with `#`.
- A register begins with `$`.
- `;` begins a comment that continues to the end of the line.
- Commas are optional separators to the parser but should be used for readability.
- `.proc name` prefixes labels inside the procedure as `name.label`; `.endproc` ends that context.

### Literals

| Form | Example |
| --- | --- |
| Decimal integer | `4096` |
| Hexadecimal integer | `0xff`, `0XABCD` |
| Octal integer | `0o77` |
| Binary integer | `0b1111_0101` |
| Character | `'A'`, `'\n'` |
| String (data directives only) | `"Hello"` |

Underscores are accepted as separators inside numeric literals. Character escapes currently
include `\n`, `\r`, `\t`, `\a`, `\f`, `\v`, `\b`, and `\\`. String escape processing is not
implemented.

### Directives

| Directive | Intended purpose | Current status |
| --- | --- | --- |
| `.section name` | Select a section | Accepted and removed; no layout effect |
| `.global symbol` | Export a symbol | Stored as metadata; no linker uses it |
| `.extern symbol [...]` | Import a symbol | First argument stored as metadata; no linker uses it |
| `.align value` | Align following data | Accepted and ignored |
| `.entry symbol` | Select the entry point | Stored as metadata; VM still starts at address zero |
| `.locate address` | Move the output location | Accepted and ignored |
| `.ascii values...` | Emit strings/bytes without a terminator | Implemented |
| `.asciiz values...` | Emit values followed by NUL | Implemented |
| `.byte values...` | Emit 8-bit values | Implemented |
| `.hword values...` | Emit 16-bit values | Implemented |
| `.word values...` | Emit 32-bit values | Implemented |
| `.dword values...` | Emit 64-bit values | Implemented |
| `.space size` | Reserve zero-initialized bytes | Implemented |
| `.proc name` / `.endproc` | Scope local labels | Implemented |
| `.macro` / `.endmacro` | Define a macro | Markers are removed; macro expansion is not implemented |

All numeric data is currently copied in host byte order. This will change when the portable binary
format in the project roadmap is defined.

## Known Compatibility Constraints

- Binary images are compatible only with the compiler/ABI/endianness combination that produced
  the VM until explicit serialization is implemented.
- Division and modulo do not guard against zero divisors.
- The zero register is not enforced as read-only.
- Error reporting often exits immediately rather than returning structured diagnostics.

The prioritized implementation sequence and acceptance criteria are maintained in the
[README roadmap](../README.md#roadmap-running-testasm).
