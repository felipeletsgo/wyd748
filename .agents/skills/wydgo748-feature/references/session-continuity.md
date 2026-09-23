# Efficient continuity between sessions

Use this reference when resuming unfinished work. One-time entry, repetition
limits, and validation gates are in [AGENTS.md](../../../../AGENTS.md); this
reference covers only how to keep evidence and the next step reusable.

## Resume from current state

Open only the relevant handoff, if one exists, and compare it with the initial
working-tree check. If `HEAD` or the diff shows that work was incorporated,
reverted, or superseded, discard the old assumption. Do not reconstruct the
historical diff. Without a handoff, use the task's file or contract; do not
create a handoff merely to start.

Choose the next step from current state: a patch, focused test, or query that
closes an identified gap. Do not recover the entire conversation or list tasks
when the scope is clear. Do not repeat entry checks when switching skills.

## Reuse results

Record the connection between evidence and its inputs when needed to decide
whether a result remains valid. Do not calculate every hash by default.

| Evidence | Recheck when |
| --- | --- |
| Native identity | The file was replaced, metadata differs, or the recorded identity is insufficient; confirm its hash before reusing addresses. |
| Census/triage | The corpus, catalog, or triage tool changed; a new root can be located from a seed without repeating the census. |
| Ghidra export | The binary, project, or script changed, or a question is not covered by the valid export. |
| Reference reading | Content changed, is unavailable in context, or does not cover the current decision. |
| Validated evidence record | The record, template, schema, or validator changed. |
| Test/build | Relevant code, data, dependencies, toolchain, or environment changed; include consumers and the batch integration gate. |
| Client flow | The executable, assets, server contract, or relevant scenario changed; identify the candidate actually executed. |

Logs, PIDs, and line numbers are volatile: check them only when needed for the
next action. Missing evidence or a skipped test remains pending. Identify a
reused earlier result as earlier, not as a new run.

## Minimal handoff

Use the [template](../../../handoffs/TEMPLATE.md) when unfinished work needs
continuity. Update the scope's existing record instead of creating one per
patch:

- objective and boundaries;
- observed `HEAD` and active files, distinguishing others' changes;
- confirmed decision/evidence and its canonical path;
- last validation: command, result, and relevant inputs/environment;
- gaps and pending acceptance criteria;
- next executable step and blocking condition, if any.

Update the record when closing a batch or stopping unfinished work if these
items changed materially. Do not record activity that added no knowledge.
Links replace dumps, copied code, and transcripts.

A handoff is not a lock or stronger evidence than the implementation. Preserve
concurrent work; ask for direction only on a real conflict. Remove the record
only when work is complete and its durable knowledge is already in the
canonical location.
