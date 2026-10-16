# Native 7.48 catalog and coverage strategy

## Purpose and artifacts

Maintain a reproducible native census and turn it into functional knowledge
by observable transition. The historical corpus contains 4,146 functions;
that is a recorded corpus count, not an invariant of every binary or analysis.
Existence and location do not prove understanding.

| Artifact | Role | Behavioral authority? |
| --- | --- | --- |
| `inventory/functions.tsv` | Decompilation index, source references, preliminary relations | No |
| `inventory/ghidra-functions.tsv` | Exported Ghidra structural catalog | Not alone |
| `inventory/research-queue.tsv` | Historical candidates and reasons | Only after evidence review |
| `flows/<subsystem>/<transition>.md` | Transition evidence | Within its proven boundary and state |
| `handoffs/client748-research-program.md` | Operational continuity and next entry | No |

Before a new campaign, verify binary identity and catalog coverage. During
continuation, reuse that result when recorded inputs and analysis are unchanged.
A binary hash change invalidates old addresses until remapped; a cardinality
change may instead reflect changed analysis. Resolve the cause and consistency
before interpreting affected entries. Do not regenerate or rehash an unchanged
immutable corpus merely to resume work.

## Work unit

Trace only the path that decides the requested delta:

```text
Live source function or observable event/input/packet
  -> candidate and caller/callback
  -> native 7.48 function
  -> callees and data
  -> mutated object/global/state
  -> output, rejection, closure and applicable relogin
  -> source/TMProject equivalent
  -> adaptation decision and validation
```

Group helpers by transition. Prefer source-to-native searches, confirmed seeds
and call-graph propagation. Do not create one record per function or wait for
the whole census before adapting a critical flow whose contract is proven.

## Ranking lanes

The triage script uses existing `functions.tsv` fields deterministically.
Its score is a search heuristic, never evidence:

- documented evidence: prioritize closing the remaining gap;
- direct source reference: high priority for active divergence risk;
- many callers or entry references: root/dispatcher priority;
- many callees: smaller component-boundary bonus;
- no direct caller but data/control references: callback lane;
- thunk: small penalty to avoid starting with wrappers.

Keep the reason and next action with each row. Break ties by hexadecimal
address so unchanged inputs produce the same queue.

## Coverage and closure

1. Census: entries present and unique, valid schema, recorded binary identity.
2. Triage: lane, reason, score and selected root.
3. `TRACED`: native entry, callers/callees, state, effects, errors and applicable
   teardown/relogin are resolved in Ghidra.
4. `CONTRACT`: wire, ABI, offsets, packing, signedness, loaders/resources and
   decisions are testable.
5. Adaptation: change only the proven parity claim. Keep internal modernization
   or a coordinated extension distinct from native knowledge.
6. Validation: focused static/automated checks and, only when actually executed,
   `CLIENT_TESTED` with the built candidate's exact path, hash and scenario.
   The normal installed executable is `tmproject/client748/WYD.exe`, not a
   historical `project.exe` assumed to be current.

A cataloged function may remain `LOCATED`. That blocks an unsupported parity
claim, not a deliberately new extension. A function without a direct caller
needs indirect xrefs, vtable/callback evidence or runtime proof before its
role becomes a native contract.

Line counts do not prove full native coverage. Such a claim requires decisions
and records for critical flows and explicit classification of remaining wrappers,
runtime helpers, unreachable code and gaps. Never hide unknowns as understood.
Record confirmed facts, status promotions, adaptation/validation, the exact
blocker and the next concrete entry point; do not duplicate unchanged logs.
