# Graph Report - zlvm  (2026-09-07)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 390 nodes · 1031 edges · 16 communities (13 shown, 3 thin omitted)
- Extraction: 84% EXTRACTED · 16% INFERRED · 0% AMBIGUOUS · INFERRED: 168 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `19cf1116`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- instruction_tests.c
- VirtualMachineInternal.c
- zlasm_assemble
- instruction_decode
- zlasm Shared Library
- Types.h
- directive_get_raw_data
- Lexer.c
- Parser.c
- main
- vm_do_operation
- LabelTable.c
- ALU.c
- CI Verification Sequence
- zlvm_configure_target
- Memory Ownership

## God Nodes (most connected - your core abstractions)
1. `vm_do_operation()` - 63 edges
2. `Opcode` - 61 edges
3. `validate_operands()` - 60 edges
4. `ZL Instruction Set` - 60 edges
5. `vm_run_instruction()` - 26 edges
6. `zlasm_assemble()` - 22 edges
7. `asm_malloc()` - 19 edges
8. `lexer_readToken()` - 17 edges
9. `asm_processDirectives()` - 16 edges
10. `asm_translate()` - 16 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `assemblySource()`  [INFERRED]
  tests/assembler_diagnostic_tests.c → asm/src/zlasm.c
- `decode_instruction()` --calls--> `instruction_decode()`  [INFERRED]
  tests/instruction_tests.c → emulator/src/Instruction.c
- `test_instruction_codec_validation()` --calls--> `instruction_decode()`  [INFERRED]
  tests/instruction_tests.c → emulator/src/Instruction.c
- `asm_translate()` --calls--> `instruction_encode()`  [INFERRED]
  asm/src/Assembler.c → emulator/src/Instruction.c
- `test_instruction_codec()` --calls--> `instruction_encode()`  [INFERRED]
  tests/instruction_tests.c → emulator/src/Instruction.c

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Assembly Execution Flow** — readme_assembly_to_execution_pipeline, docs_isa_assembly_language, docs_isa_instruction_encoding, docs_isa_machine_model, docs_isa_interrupts_and_syscalls [EXTRACTED 1.00]
- **CMake Target Composition** — cmakelists_project_build, asm_cmakelists_zlasm, asm_cmakelists_zlasm_static, asm_cmakelists_zlasm_bin, emulator_cmakelists_zlvm, emulator_cmakelists_zlvm_static, emulator_cmakelists_zlvm_bin [EXTRACTED 1.00]
- **test.asm End-to-End Validation** — readme_test_asm_roadmap, tests_cmakelists_test_asm_fixture, tests_cmakelists_execution_equivalence, tests_cmakelists_instruction_tests, tests_cmakelists_diagnostic_tests [EXTRACTED 1.00]

## Communities (16 total, 3 thin omitted)

### Community 0 - "instruction_tests.c"
Cohesion: 0.13
Nodes (36): assemblySource(), byte, vm_destroy(), vm_get_state(), byte, State, VirtualMachine, compare_states() (+28 more)

### Community 1 - "VirtualMachineInternal.c"
Cohesion: 0.20
Nodes (35): byte, Condition, dword, Instruction, State, VirtualMachine, word, print_registers() (+27 more)

### Community 2 - "zlasm_assemble"
Cohesion: 0.07
Nodes (52): byte, ZlasmDiagnostic, ZlasmResult, derive_output_path(), main(), print_diagnostic(), read_source(), write_binary() (+44 more)

### Community 3 - "instruction_decode"
Cohesion: 0.24
Nodes (10): Condition, condition_to_string(), byte, Instruction, instruction_decode(), instruction_encode(), instruction_print(), Opcode (+2 more)

### Community 4 - "zlasm Shared Library"
Cohesion: 0.11
Nodes (25): Developer Agent, Reviewer Agent, Project Boundaries, Repository Workflow, zlasm Shared Library, zlasm CLI, zlasm Static Library, ZLVM Project Build (+17 more)

### Community 5 - "Types.h"
Cohesion: 0.09
Nodes (11): DirectiveType, is_data_directive(), linked_list_append(), linked_list_count(), linked_list_get_last(), linked_list_remove_first(), linked_list_remove_last(), State (+3 more)

### Community 6 - "directive_get_raw_data"
Cohesion: 0.16
Nodes (24): byte, Directive, DirectiveType, dword, Token, directive_add_arg(), directive_free(), directive_get_raw_data() (+16 more)

### Community 7 - "Lexer.c"
Cohesion: 0.18
Nodes (24): Token, TokenList, TokenStream, is_dec_char(), is_digit_for_base(), is_eof(), is_ignored_char(), is_token_end() (+16 more)

### Community 8 - "Parser.c"
Cohesion: 0.13
Nodes (29): Line, line_list_add(), line_list_free(), line_list_get_last(), line_list_init(), line_print(), lineStream_new(), lineStream_read() (+21 more)

### Community 9 - "main"
Cohesion: 0.21
Nodes (11): byte, State, ZlasmDiagnostic, main(), print_diagnostic(), print_state(), read_binary(), read_source() (+3 more)

### Community 10 - "vm_do_operation"
Cohesion: 0.11
Nodes (66): Opcode, Statement, validate_operands(), Instruction Conditions, ZL Instruction Set, Interrupts and Syscalls, Operation, vm_do_operation() (+58 more)

### Community 11 - "LabelTable.c"
Cohesion: 0.64
Nodes (7): labelInfo_getIfExist(), labelInfo_getOrCreate(), labelTable_add(), labelTable_init(), labelTable_setOrCreate(), LabelInfo, LabelTable

### Community 14 - "ALU.c"
Cohesion: 0.67
Nodes (6): alu_compute(), alu_reset(), alu_set_signed_flags(), alu_set_unsigned_flags(), alu_setFlags(), ALU

## Knowledge Gaps
- **9 isolated node(s):** `CI Verification Sequence`, `CMake Single-Platform CI Workflow`, `zlvm_configure_target`, `Assembler Diagnostic Tests`, `Instruction Tests` (+4 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **3 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `zlasm_assemble()` connect `zlasm_assemble` to `instruction_tests.c`, `directive_get_raw_data`, `Lexer.c`, `Parser.c`, `main`?**
  _High betweenness centrality (0.218) - this node is a cross-community bridge._
- **Why does `asm_translate()` connect `zlasm_assemble` to `instruction_decode`, `directive_get_raw_data`, `Lexer.c`, `vm_do_operation`, `LabelTable.c`?**
  _High betweenness centrality (0.213) - this node is a cross-community bridge._
- **Why does `validate_operands()` connect `vm_do_operation` to `zlasm_assemble`?**
  _High betweenness centrality (0.178) - this node is a cross-community bridge._
- **What connects `CI Verification Sequence`, `CMake Single-Platform CI Workflow`, `zlvm_configure_target` to the rest of the system?**
  _9 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `instruction_tests.c` be split into smaller, more focused modules?**
  _Cohesion score 0.12802275960170698 - nodes in this community are weakly interconnected._
- **Should `zlasm_assemble` be split into smaller, more focused modules?**
  _Cohesion score 0.07175141242937853 - nodes in this community are weakly interconnected._
- **Should `zlasm Shared Library` be split into smaller, more focused modules?**
  _Cohesion score 0.10666666666666667 - nodes in this community are weakly interconnected._