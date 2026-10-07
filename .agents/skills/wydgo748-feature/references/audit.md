# WYD-Go audit guidance

Use this guidance for an audit or diagnosis, or when a concrete risk warrants
additional review. Do not perform a global audit for every feature. Define the
invariant and follow only the representations and inputs that reach it; expand
the scope when evidence demonstrates cross-cutting impact.

Review and diagnosis are read-only unless the user requests corrections.
Earlier reports and comments are search indexes, not current-code authority.
Reuse valid technical evidence according to [AGENTS.md](../../../../AGENTS.md).

## 1. Define the invariant

State the contract that must hold in one sentence. Examples:

- The server derives the result; the client sends only intent.
- An operation cannot duplicate an item after failure or relogin.
- The 7.48 wire preserves proven sizes and offsets.
- Closing UI releases modal state and input capture.
- A 1x1 item occupies one inventory cell but fills its equipped region.

Without a precise invariant, an audit lists differences without deciding which
ones are defects.

## 2. Map all representations

Search with `rg` and follow the complete path:

```text
wire <-> parser/handler <-> model/runtime <-> store <-> base data <-> client/UI
```

Record conversions, copies, derived values, caches, and legacy projections.
For a legacy client boundary, apply the native-evidence gate before claiming
that TMProject represents the native 7.48 contract. Reuse unchanged evidence;
internal contract-preserving changes do not require fresh native research.

## 3. Enumerate inputs and writers

Consider each input that reaches the invariant:

- Login, relogin, and initial loading.
- Action packets and alternative handlers.
- Ticks, AI, timers, callbacks, and hooks.
- Reload, administration, scripts, and fallbacks.
- Map transitions, disconnect, and rollback.
- UI input, clicks, hotkeys, Escape, close controls, and callbacks.

For multiple writers, demonstrate who serializes mutations. Gameplay state
must converge in `World`.

## 4. Follow the entire transition

For every input, verify identity, state, distance, ownership, and limits;
mutation in a copy or transactional state; persistence and failure handling;
publication to the owner and observers; temporary/modal cleanup; and the
result after relogin or packet repetition. Look for early returns between
mutation, persistence, and publication.

## 5. Look for bypasses

- An old handler remains registered.
- An alternative opcode or size is accepted.
- An exported function bypasses the new flow.
- A mock avoids the faulty parser, store, or World boundary.
- A modern control hides a missing 7.48 resource.
- A later-version branch remains reachable.
- A global scan substitutes for a local lookup.
- Failure leaves partial item, currency, modal, or observer state.

## 6. Review tests and evidence

Open tests and confirm that they traverse the relevant production path. A test
can pass while mocking the defective boundary. Evaluate success, rejection,
repetition/idempotency, persistence failure/rollback, disconnect/relogin,
owner/observer publication, malformed data and limits, byte-exact ABI layout,
and real interaction for visual/input defects as applicable. In a read-only
audit, record missing tests without implementing changes unasked.

Distinguish validation levels:

```text
IMPLEMENTED          the change exists
STATICALLY VERIFIED  relevant inspection/static build checks passed
AUTOMATED TESTED     relevant automated tests passed
CLIENT-TESTED        the actual built-client flow passed
```

Do not infer validation of unrelated areas from a higher level elsewhere.

## 7. Dead code and duplication

Global dead-code analysis is optional and limited to removal tasks or a
concrete suspicion of unreachable duplication. Start with callers and symbol
registrations. Reuse an installed/pinned tool and record its version; do not
automatically download `@latest` for each review.

Before removal, prove the absence of static and dynamic callers, inspect
opcode/table/callback/reflection/hook/script registration, exclude active
fallbacks and migrations, search equivalent names/IDs and indirect effects,
then remove in a focused patch and run the affected test/build.

Classify incomplete TODO paths as scaffolding, not dead code. A path is truly
dead only after its active replacement and unreachability are demonstrated.
Do not remove stubs without authorization. Clean `vet` or dead-code output
does not detect duplicate transactions; inspect copied grants, saves, and
rollbacks. Verify intentionally unused symbols against the current tree;
do not maintain an unverified exemption list of historical function names.

## Audit output

List concrete findings first, ordered by severity. Each finding needs exact
file/line, violated invariant, triggering scenario, impact, and evidence.
Use these classifications consistently:

```text
CONFIRMED     demonstrated in current code, data, or native evidence
PROBABLE      strong evidence; reproduction is still pending
HYPOTHESIS    a lead requiring further investigation
NOT A DEFECT  a difference explained by the applicable contract
```

Then record reviewed flows, missing prerequisites, and coverage gaps. Do not
claim full compatibility or total coverage without enumerating and testing
the criteria. Check whether current evidence contradicts an earlier report,
handoff, or comment.
