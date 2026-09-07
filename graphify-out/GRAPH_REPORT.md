# Graph Report - zlvm  (2026-09-07)

## Corpus Check
- Corpus is ~20,218 words - fits in a single context window. You may not need a graph.

## Summary
- 333 nodes · 802 edges · 18 communities (15 shown, 3 thin omitted)
- Extraction: 79% EXTRACTED · 21% INFERRED · 0% AMBIGUOUS · INFERRED: 168 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- VM Public API
- VM Execution
- CLI Diagnostics
- Assembler Pipeline
- CMake Targets
- Public Types
- Directives and Tokens
- Lexer
- Parser
- Emulator CLI
- Line Lists
- Label Tables
- Linked Lists
- Equivalence Tests
- ALU
- CI Workflow
- Target Configuration
- Memory Ownership

## God Nodes (most connected - your core abstractions)
1. `vm_run_instruction()` - 26 edges
2. `zlasm_assemble()` - 22 edges
3. `asm_malloc()` - 19 edges
4. `lexer_readToken()` - 17 edges
5. `asm_processDirectives()` - 16 edges
6. `asm_translate()` - 15 edges
7. `asm_calloc()` - 15 edges
8. `vm_set_state()` - 15 edges
9. `main()` - 15 edges
10. `directive_get_raw_data()` - 13 edges

## Surprising Connections (you probably didn't know these)
- `asm_processDirectives()` --calls--> `is_data_directive()`  [INFERRED]
  asm/src/Assembler.c → include/asm/Directive.h
- `asm_translate()` --calls--> `instruction_encode()`  [INFERRED]
  asm/src/Assembler.c → emulator/src/Instruction.c
- `directive_get_raw_data()` --calls--> `is_data_directive()`  [INFERRED]
  asm/src/Directive.c → include/asm/Directive.h
- `read_source()` --calls--> `zlasm_assemble()`  [INFERRED]
  emulator/main.c → asm/src/zlasm.c
- `main()` --calls--> `assemblySource()`  [INFERRED]
  tests/assembler_diagnostic_tests.c → asm/src/zlasm.c

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Assembly Execution Flow** — readme_assembly_to_execution_pipeline, docs_isa_assembly_language, docs_isa_instruction_encoding, docs_isa_machine_model, docs_isa_interrupts_and_syscalls [EXTRACTED 1.00]
- **test.asm End-to-End Validation** — readme_test_asm_roadmap, tests_cmakelists_test_asm_fixture, tests_cmakelists_execution_equivalence, tests_cmakelists_instruction_tests, tests_cmakelists_diagnostic_tests [EXTRACTED 1.00]
- **CMake Target Composition** — cmakelists_project_build, asm_cmakelists_zlasm, asm_cmakelists_zlasm_static, asm_cmakelists_zlasm_bin, emulator_cmakelists_zlvm, emulator_cmakelists_zlvm_static, emulator_cmakelists_zlvm_bin [EXTRACTED 1.00]

## Communities (18 total, 3 thin omitted)

### Community 0 - "VM Public API"
Cohesion: 0.13
Nodes (36): assemblySource(), byte, Condition, condition_to_string(), byte, Instruction, instruction_decode(), instruction_encode() (+28 more)

### Community 1 - "VM Execution"
Cohesion: 0.19
Nodes (37): byte, Condition, dword, Instruction, Operation, State, VirtualMachine, word (+29 more)

### Community 2 - "CLI Diagnostics"
Cohesion: 0.10
Nodes (29): byte, ZlasmDiagnostic, ZlasmResult, derive_output_path(), main(), print_diagnostic(), read_source(), write_binary() (+21 more)

### Community 3 - "Assembler Pipeline"
Cohesion: 0.12
Nodes (29): asm_addExternal(), asm_addGlobal(), asm_init(), asm_processDirectives(), asm_processLabels(), asm_translate(), byte, Condition (+21 more)

### Community 4 - "CMake Targets"
Cohesion: 0.09
Nodes (29): Developer Agent, Reviewer Agent, Project Boundaries, Repository Workflow, zlasm Shared Library, zlasm CLI, zlasm Static Library, ZLVM Project Build (+21 more)

### Community 5 - "Public Types"
Cohesion: 0.11
Nodes (4): DirectiveType, is_data_directive(), State, is_error()

### Community 6 - "Directives and Tokens"
Cohesion: 0.16
Nodes (24): byte, Directive, DirectiveType, dword, Token, directive_add_arg(), directive_free(), directive_get_raw_data() (+16 more)

### Community 7 - "Lexer"
Cohesion: 0.18
Nodes (24): Token, TokenList, TokenStream, is_dec_char(), is_digit_for_base(), is_eof(), is_ignored_char(), is_token_end() (+16 more)

### Community 8 - "Parser"
Cohesion: 0.20
Nodes (19): append_token(), Directive, Line, ParserContext, Statement, Token, TokenList, TokenStream (+11 more)

### Community 9 - "Emulator CLI"
Cohesion: 0.21
Nodes (11): byte, State, ZlasmDiagnostic, main(), print_diagnostic(), print_state(), read_binary(), read_source() (+3 more)

### Community 10 - "Line Lists"
Cohesion: 0.35
Nodes (10): Line, line_list_add(), line_list_free(), line_list_get_last(), line_list_init(), line_print(), lineStream_new(), lineStream_read() (+2 more)

### Community 11 - "Label Tables"
Cohesion: 0.64
Nodes (7): labelInfo_getIfExist(), labelInfo_getOrCreate(), labelTable_add(), labelTable_init(), labelTable_setOrCreate(), LabelInfo, LabelTable

### Community 12 - "Linked Lists"
Cohesion: 0.43
Nodes (7): linked_list_append(), linked_list_count(), linked_list_get_last(), linked_list_remove_first(), linked_list_remove_last(), LinkedList, LinkedListItem

### Community 13 - "Equivalence Tests"
Cohesion: 0.36
Nodes (7): byte, State, VirtualMachine, compare_states(), main(), read_file(), state_name()

### Community 14 - "ALU"
Cohesion: 0.67
Nodes (6): alu_compute(), alu_reset(), alu_set_signed_flags(), alu_set_unsigned_flags(), alu_setFlags(), ALU

## Knowledge Gaps
- **9 isolated node(s):** `CMake Single-Platform CI Workflow`, `CI Verification Sequence`, `Project Boundaries`, `zlvm_configure_target`, `Instruction Conditions` (+4 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **3 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `zlasm_assemble()` connect `CLI Diagnostics` to `VM Public API`, `Assembler Pipeline`, `Directives and Tokens`, `Lexer`, `Parser`, `Emulator CLI`?**
  _High betweenness centrality (0.278) - this node is a cross-community bridge._
- **Why does `assemblySource()` connect `VM Public API` to `CLI Diagnostics`, `Equivalence Tests`?**
  _High betweenness centrality (0.188) - this node is a cross-community bridge._
- **Why does `asm_translate()` connect `Assembler Pipeline` to `VM Public API`, `CLI Diagnostics`, `Directives and Tokens`, `Lexer`, `Label Tables`?**
  _High betweenness centrality (0.122) - this node is a cross-community bridge._
- **Are the 7 inferred relationships involving `vm_run_instruction()` (e.g. with `alu_reset()` and `test_decoded_instruction_validation()`) actually correct?**
  _`vm_run_instruction()` has 7 INFERRED edges - model-reasoned connections that need verification._
- **Are the 19 inferred relationships involving `zlasm_assemble()` (e.g. with `read_source()` and `asm_init()`) actually correct?**
  _`zlasm_assemble()` has 19 INFERRED edges - model-reasoned connections that need verification._
- **Are the 16 inferred relationships involving `asm_malloc()` (e.g. with `derive_output_path()` and `read_source()`) actually correct?**
  _`asm_malloc()` has 16 INFERRED edges - model-reasoned connections that need verification._
- **Are the 4 inferred relationships involving `lexer_readToken()` (e.g. with `asm_calloc()` and `asm_malloc()`) actually correct?**
  _`lexer_readToken()` has 4 INFERRED edges - model-reasoned connections that need verification._