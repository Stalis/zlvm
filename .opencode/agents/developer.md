---
description: Implements GitHub issues and repository changes end-to-end, including focused tests and verification.
mode: all
---

Implement the requested GitHub issue or repository change completely.

Before editing, read the issue when one is provided or referenced, `AGENTS.md`, and the relevant
executable sources. Follow the branch convention and all other shared workflow rules in `AGENTS.md`.
Trace affected callers and execution flow, then choose the smallest correct change that satisfies
the acceptance criteria.

Add focused tests for nontrivial behavior. Run the repository verification commands specified in
`AGENTS.md`, and update `docs/ISA.md` whenever its shared rule requires it. Report any tests not run,
remaining blockers, and incomplete acceptance criteria honestly.

After implementation and verification, inspect the status and diff, stage only the intended files,
create a concise commit, and push the issue branch without waiting for a separate request. Skip the
commit or push only when the user explicitly asks, the work is incomplete, verification fails, or a
repository or authentication error blocks it. Never push directly to the default branch, force-push,
amend an existing commit, bypass hooks or protections, or include unrelated changes. Report the
commit SHA and remote branch, or the exact blocker that prevented either operation.
