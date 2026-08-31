# Repository Instructions

## Workflow

- Implement each GitHub issue in a branch named `dev/<issue_number>-<short_issue_name>`.
- Follow `docs/CODING_CONVENTIONS.md` for new or substantially changed code; do not reformat
  unrelated legacy code.
- Update `docs/ISA.md` in the same change when modifying opcodes, registers, conditions,
  interrupts, directives, memory layout, or instruction encoding.
- Treat SSH-agent, credential-provider, and access-timeout failures from Git-provider commands as
  temporary. Show the sanitized error, ask the user to restore access and confirm a retry, wait, and
  rerun the exact command. Escalate again if it repeats; stop only when the user declines or the
  failure is not temporary.

## Build and Test

- CMake 4.4.2 or newer is required. Match CI order:

  ```sh
  make format-check
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
  ctest --test-dir build -C Release --output-on-failure
  ```

- Run one test by its exact CTest name, for example:

  ```sh
  ctest --test-dir build -R '^zlvm\.instructions$' --output-on-failure
  ```

- `make format` rewrites every tracked or unignored C/header file; format only changed files when a
  repository-wide rewrite is not intended.
- Debug builds define `DEBUG` and emit additional VM diagnostics, so compare output only within the
  same build configuration.

## Project Boundaries

- `emulator/main.c` is the `zlvm` CLI; it links the assembler and executes source assembled in
  memory. `asm/main.c` is the standalone `zlasm` CLI.
- Treat `include/VirtualMachine.h` and `include/asm/zlasm.h` as the public API boundaries; keep test
  and implementation-only access in the corresponding `src/` trees.
- `test.asm` is the canonical assembly-to-execution fixture. Preserve the acceptance criteria in
  the README roadmap when changing that path.
