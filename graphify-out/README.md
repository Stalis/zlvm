# Graphify Artifacts

`graphify-out/opcode-overlay.json` restores the `Opcode` enum and its documented, assembler, and VM-dispatch relationships that Graphify cannot extract from the `FOREACH_OPCODE` X-macro.

After refreshing Graphify, run:

```sh
python3 tools/apply_opcode_overlay.py
```

The script derives every opcode and source location from the repository and fails if the macro, ISA table, or dispatch cases diverge.
