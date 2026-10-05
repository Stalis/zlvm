# Code Graph

This repository uses GitNexus for local code navigation and impact analysis. The index is generated
from the working tree and is not committed.

## Initialize or refresh

Run from the repository root:

```sh
node .gitnexus/run.cjs analyze --index-only --pdg
node .gitnexus/run.cjs status
```

Use `--force` after changing the analyzer version or index configuration. The `.gitnexus/` directory
is local state and can be recreated at any time.

## Useful queries

Use the GitNexus MCP tools for focused code questions:

- `query` finds execution flows related to a concept.
- `context` shows callers, callees, and process participation for a symbol.
- `impact` estimates what depends on a symbol before editing it.
- `trace` finds the shortest call path between two symbols.
- `cypher` queries the graph directly; read the repository schema first.

For the opcode pipeline, useful anchors include `Opcode`, `asm_translate`, `instruction_encode`,
`instruction_decode`, `vm_run_instruction`, and `vm_do_operation`.
