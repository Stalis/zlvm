# ZLVM Coding Conventions

These conventions define the style for new ZLVM code and for code that is substantially modified.
They do not require repository-wide reformatting, and established public API names should remain
compatible unless a deliberate migration is approved.

## Language and Portability

- Use standard C11.
- Prefer fixed-width project types (`byte`, `hword`, `word`, and `dword`) where the width is part of
  the VM or binary contract. Use `size_t` for host object sizes and indexes.
- Do not depend on C bitfield layout, union type-punning layout, host endianness, or structure
  padding in persistent or public binary formats.
- Do not use identifiers reserved by C or the implementation, including names beginning with two
  underscores or an underscore followed by an uppercase letter.
- Do not redefine standard allocation or C library function names with macros.
- Prefer bounded formatting such as `snprintf` over `sprintf`.
- Check conversions when narrowing from a wider integer type.

## Formatting

- Indent with four spaces. Do not use tabs for indentation.
- Use K&R brace placement:

  ```c
  if (condition) {
      perform_action();
  } else {
      handle_failure();
  }
  ```

- Use braces around every `if`, `else`, loop, and `switch` body, including single statements.
- Prefer lines no longer than 100 characters; 120 characters is the hard limit.
- Put one statement per line and one declaration per line when declarations have different roles.
- Place one space after control-flow keywords and around binary operators. Do not put a space
  between a function name and `(`.
- Write pointers with the asterisk next to the variable: `VirtualMachine *vm`.
- Use blank lines to separate logical phases, not every statement.
- Keep `switch` cases indented one level inside the switch and their bodies one additional level.

## Naming

### Functions

Use subsystem-prefixed `snake_case` names:

```c
void vm_initialize(VirtualMachine *vm, size_t ram_size);
byte asm_read_token(AssemblerContext *context);
```

Choose a stable, short subsystem prefix such as `vm_`, `asm_`, `lexer_`, `parser_`, `alu_`, or
`instruction_`. Internal helpers that cannot conflict may use an unprefixed descriptive
`snake_case` name when they are `static`.

### Variables and fields

- Use descriptive `snake_case` for local variables, parameters, and structure fields.
- Avoid `this`, single-letter variables outside small loops, and parameter names beginning with
  underscores.
- Name counts with a `_count` suffix, byte sizes with `_size`, indexes with `_index`, and boolean
  predicates with verbs such as `is_`, `has_`, or `can_`.

### Types and constants

- Use `PascalCase` for `struct`, `union`, `enum`, and `typedef` names: `VirtualMachine`,
  `AssemblerContext`, `Instruction`.
- Use subsystem-prefixed `UPPER_SNAKE_CASE` for enum members, constants, and macros:
  `VM_ROM_SIZE`, `ASM_TOKEN_ID`, `OPCODE_ADD_REGISTER`.
- Include units in names when ambiguity is possible, for example `timeout_ms` or
  `instruction_size_bytes`.

Existing names such as `ZLVM_ROM_SIZE`, `R_PC`, and `C_ZERO_SET` remain valid public compatibility
names. Apply the new convention when introducing a replacement API rather than renaming them as an
unrelated cleanup.

## Headers and Includes

- Give every header a unique project-prefixed guard, for example `ZLVM_ASM_PARSER_H`.
- A public header must include the headers that define every type it exposes.
- Order includes in implementation files as:
  1. the matching header;
  2. C standard library headers;
  3. other project headers.
- Separate include groups with one blank line and sort within each group.
- Do not use relative paths that traverse the source tree from a C file; configure include
  directories in CMake and include the public path.
- Keep declarations in public headers minimal. Put implementation-only declarations in internal
  headers or make them `static` in the implementation file.

## Functions and Control Flow

- Give each function one focused responsibility.
- Validate public function pointers and sizes before dereferencing or copying.
- Return errors to the caller where recovery is possible; reserve process termination for the
  command-line boundary.
- Prefer early returns for invalid input and error paths when they reduce nesting.
- Do not use `goto` to share ordinary control flow. A local cleanup label is acceptable when it
  makes multi-resource failure handling safer.
- Do not rely on fallthrough in a `switch` unless it is intentional, documented, and annotated
  with a recognized fallthrough comment.
- Guard division, modulo, shifts, and address arithmetic against invalid or overflowing operands.

## Memory and Ownership

- State ownership in public API comments for every allocated pointer: who allocates it, who frees
  it, and how its size is communicated.
- Pair every successful allocation with a clear release path.
- Store the result of `realloc` in a temporary pointer until success is confirmed. Recompute any
  interior pointers from offsets after the allocation moves.
- Allocate string storage including the terminating NUL byte.
- Initialize linked-list nodes and aggregate objects before use, normally with `calloc` or a
  dedicated initializer.
- Use `sizeof *pointer` when allocating an object through a pointer:

  ```c
  LineList *lines = calloc(1, sizeof *lines);
  ```

- Do not hide allocation, deallocation, or pointer assignment inside macros.
- Use explicit byte encoders/decoders for VM images instead of copying native structures.

## Errors and Diagnostics

- Library code should return a status and preserve enough context for the caller to report the
  problem.
- Command-line diagnostics go to standard error and include the file path plus line and column when
  available.
- A command succeeds with exit status zero and fails with a nonzero status. VM `S_HALTED` is a
  successful terminal state.
- Check all file open, read, write, flush, and close operations.
- Error messages should state what failed and, where useful, the offending value.

## Comments and Documentation

- Explain intent, invariants, ownership, format constraints, and non-obvious decisions. Do not
  repeat the code in prose.
- Public functions and types should have concise documentation for inputs, outputs, side effects,
  ownership, and error behavior.
- Write comments and documentation in English.
- Use actionable TODOs with a subject, for example `TODO(assembler): implement section layout`.
- Changes to opcodes, registers, conditions, interrupts, directives, memory layout, or instruction
  encoding must update `docs/ISA.md` in the same change.

## CMake and Warnings

- Use target-scoped commands such as `target_compile_options`, `target_compile_definitions`, and
  `target_include_directories`; avoid modifying global `CMAKE_C_FLAGS`.
- Declare C11 requirements with target properties or `target_compile_features`.
- Build project code with at least `-Wall -Wextra` on GCC and Clang and the corresponding warning
  level on MSVC.
- Do not suppress a warning globally to accommodate one site. Fix the site or scope the suppression
  to the smallest justified target or source file.
- Keep platform-specific options behind compiler and platform checks.

## Testing Expectations

- Add focused unit tests for parser, encoder, ALU, condition, memory, and instruction changes.
- Add an integration fixture when a change affects the assembly-to-execution path.
- Test normal behavior, boundary values, malformed input, and relevant failure states.
- Run Debug and Release builds. Memory-management changes should also run under AddressSanitizer
  and UndefinedBehaviorSanitizer on a supported compiler.
- `test.asm` is the canonical end-to-end program; changes affecting it must preserve the acceptance
  criteria in the README roadmap.

## Example

```c
State vm_load_program(VirtualMachine *vm, const byte *program, size_t program_size) {
    if (vm == NULL || program == NULL) {
        return VM_ERROR_INVALID_ARGUMENT;
    }

    if (program_size > VM_ROM_SIZE) {
        return VM_ERROR_OUT_OF_MEMORY;
    }

    memcpy(vm->rom, program, program_size);
    return VM_STATE_NORMAL;
}
```

This example demonstrates the intended formatting and naming. It is not a declaration of a current
public API.
