---
id: cargo-gold-transfer-confirmations
title: Native Cargo gold confirmations move both balances with a 16-byte envelope
subsystem: transport
status: CONTRACT
native_sha256: 8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593
updated: 2026-09-30
---

# Cargo gold transfer confirmations

## Question

Which received envelope and balance transition must the buildable client
preserve for deposits (`0x388`) and withdrawals (`0x387`)? Does the current
server persist both balances before publishing that transition?

## Evidence boundary

- **USED:** read-only `references/client748/WYD.exe`, SHA-256 above, x86
  image base `0x00400000`, recovered Ghidra project
  `tmproject/build/native-research/WYD748.gpr`, program `WYD.exe`.
- **USED:** unchanged prior instruction exports for field receiver
  `FUN_00492e7d`, size policy `FUN_0055890a`, and the field receiver table.
  Existing coverage was reused without exporting those roots again.
- **USED:** new read-only, no-analysis inspection of `FUN_00488ea9` and
  `FUN_00488f09`, and [focused instruction evidence](../../exports/cargo-gold-confirmations.tsv).
  Each consumer export reports 25 instructions and 11 references, with the
  expected program hash and no script error. Decompiler inspection completed
  with `inspection_complete`. Full exports/logs remain ignored build artifacts.
- **USED:** `CargoGoldTransferContract.h`, `ReceivedPacketDispatch.h`,
  `TMFieldScene::OnPacketDeposit`/`OnPacketWithdraw`, and existing
  `ReceivedPacketDispatchTests.cpp` coverage.
- **USED:** `wydgo748/internal/game/handlers.go::onCargoGold`, `saveAccount`,
  wire builders/tests, and new `cargo_gold_contract_test.go` regressions.
- **USED:** [world-state parameter contracts](world-state-parameter-contracts.md)
  for the subsequent authoritative `0x339` snapshot. Gold in `0x337` is
  independently checked by the new encrypted response test.
- **NOT APPLICABLE:** assets and external legacy implementations; neither
  determines this payload. TMProject 7.69 is secondary source architecture.
- **CONTRADICTORY:** the previous unresolved native consumer status; the
  recovered project proves both received branches and balance mutations.

## Native 7.48 flow

### Observable entry

Incoming field packets carry the opcode word at `+4`. Deposit and withdrawal
reach distinct consumers; both read the DWORD at `+12` and adjust Cargo and
character gold oppositely. This proves a received confirmation, not just
a request builder.

### Callers

`FUN_00492e7d` compares `0x388` at `0x004932f2` and calls
`FUN_00488ea9` at `0x00493301`; it compares `0x387` at `0x00493314`
and calls `FUN_00488f09` at `0x00493323`.
The field receiver's table/lifecycle proof is reused from the
[sale envelope record](legacy-sale-confirmation-envelope.md): table
`0x005a4294`, receiver slot `0x005a4298`, constructor/destructor references
`0x00434407`/`0x004358fb`.

The size-policy computed jump at `0x00558ad3` reaches cases `0x005593c8`
(`0x387`) and `0x005593e1` (`0x388`). Each compares the size word
at `+0` against `0x10`. No new complete network call-chain trace for
that size validator is claimed.

### Main function

Both consumers are x86 `__thiscall` receivers with one packet pointer
argument and `RET 4` (`0x00488f06` and `0x00488f66`).
At manager base `DAT_013b71e8`, Cargo gold occupies `+0xc58` and
character gold `+0x704` in this native binary, not the candidate ABI.
Deposit adds the packet DWORD to Cargo and subtracts it from character gold;
withdrawal reverses those operations. The arithmetic works on 32-bit bit
patterns; it does not establish native server-side signedness or valid ranges.

### Callees

Each calls `FUN_004431e4` with argument zero and the same scene receiver
after mutation, then returns 1. Current source maps this to
`UpdateScoreUI(0)`. Downstream control identities are not newly traced
or required for the envelope/balance contract.

### Outputs and errors

Native size cases set a rejection flag for a declared size other than 16.
The candidate also checks actual length and matching opcodes before the
lengthless callback. Invalid frames do not reach consumers or modify borrowed
bytes. Native consumers perform unconditional local arithmetic; that does
not authorize bypassing server rejection or persistence.

## State and lifecycle

### Transition matrix

| Event/state | Precondition | Function/call | Resulting state | Side effects | Error/exit |
| --- | --- | --- | --- | --- | --- |
| Deposit confirmation | Valid 16-byte `0x388` | `FUN_00488ea9` | Cargo increases; character decreases | UI refresh | Candidate rejects invalid envelope first |
| Withdrawal confirmation | Valid 16-byte `0x387` | `FUN_00488f09` | Cargo decreases; character increases | UI refresh | Candidate rejects invalid envelope first |
| Server request | Live player, visible nearby banker, valid balances/amount | `onCargoGold` | Both balances persisted | Confirmation, Cargo snapshot, character snapshot | Reject invalid operation |
| Save failure | Mutation attempted | `saveAccount` error | Both balances restored | Error panel only | No success snapshots |
| Repeated request | Preconditions revalidated | `onCargoGold` | New transfer per successful request | Separate save/publication | No transaction ID or idempotence claim |

### Vtables, vptrs, and receivers

The native field receiver uses the table proof cited above; both balance
consumers are direct calls. No new virtual dispatch is introduced.

### Ownership

The receive buffer is borrowed for the callback and never retained. Manager
balances belong to existing application state. World owns server mutation;
persistence receives an account snapshot.

### Partial failure

The server restores both balances on synchronous save failure and publishes
only an error panel. New tests inspect the save boundary to prove no response
preceded persistence, and test two failures followed by success in each direction.

### Cleanup and teardown

These consumers allocate no visual or transport resources. The gate adds
no cleanup; scene/manager teardown remains with existing owners.

### Shutdown

No independent shutdown state exists here. Disconnect stops delivery;
pending network delivery at shutdown is not proven by this record.

### Logout and relogin

Cargo gold is supplied by the next character-list state and character gold
by authoritative character state. A previous delta is not required to
reconstruct either balance. Executed relogin remains pending.

## Wire, ABI, and resources

`0x387/0x388`, C<->S, exactly 16 bytes, standard header alignment:

| Field | Offset | Width | Interpretation |
| --- | ---: | ---: | --- |
| Header size | 0 | 2 | Unsigned declared size, 16 |
| Header opcode | 4 | 2 | Unsigned discriminator |
| Header recipient | 6 | 2 | Current confirmations use `SceneField` |
| Amount | 12 | 4 | Native DWORD; current server validates as `uint32` |

No asset dependency or wire field is introduced. Amounts and both resulting
balances obey the server's existing 2,000,000,000 gold cap; that limit is
not newly inferred from native client arithmetic.

## Current mapping

### Buildable source

`CargoGoldTransferContract.h` centralizes opcodes, size and offset.
Existing ABI assertions match `MSG_STANDARDPARM` and the exact receive gate.
Both `TMFieldScene` consumers match native directions and DWORD offset.
Only the header comment is translated/clarified in this batch; no executable
client behavior changes or new product build are required.

### WYD-Go

`onCargoGold` validates player, live HP, exact buffer size, banker,
nonzero amount, source balance and destination cap. It mutates both balances,
saves the account, then sends `CargoGoldTransfer`, `UpdateCargoGold` and
`UpdateEtc`, in that order. Save failure rolls back balances and sends no
success confirmation.

## Delta matrix

| Claim | Native 7.48 | Previous record | Current source/server | Decision |
| --- | --- | --- | --- | --- |
| Received envelope | Two consumers and exact 16-byte size cases | Native evidence unresolved | Same gate/DWORD offset | Record `PARIDADE_NATIVA`; no ABI rewrite |
| Balance directions | Deposit adds Cargo/subtracts character; withdrawal reverses | Active implementation only | Same transitions | Preserve proven implementation |
| Persistence authority | Native server not studied | Basic success/rejection tests | Save before publication and rollback | Add focused regressions; no native server claim |

## Decisions

- Classify recovered envelope and balance transitions as `PARIDADE_NATIVA`.
  Retain the existing fail-closed gate's `MODERNIZACAO_COMPATIVEL`
  classification; research maturity does not change implementation.
- Preserve server validation and subsequent snapshots without duplicating
  economy rules in the client.
- Keep both directions together because their ABI/lifecycle match.
- Update the existing record to `CONTRACT`, not a duplicate handoff or
  `CLIENT_TESTED`.

## Gaps

- Actual UI, rejection display, disconnect and relogin have not been executed
  in the built DirectX client.
- Injected store failures and snapshot assertions do not replace live
  PostgreSQL commit/rollback or crash-durability validation.
- The unchanged failure diagnostic in `handlers.go` still contains Portuguese.
  Untouched language debt prevents a global English completion/release-ready claim.

## Validation

- **STATICALLY VERIFIED:** read-only Ghidra consumer inspection and focused
  instruction exports; previous receiver/size/table exports reused.
- **AUTOMATED TESTED:** three new `TestCargoGoldContract*` tests (24 leaf
  cases): both directions, persistence/publication ordering and decrypted
  responses, repetition, two save failures/retry, zero/excess amount,
  insufficient funds, destination cap, dead/out-of-world players,
  invisible/distant banker, truncated/oversized buffers. Wire tests
  `TestCargoGoldTransfer748Layout` and `TestUpdateCargoGold748Layout` pass.
  The dead-player fixture uses `setPlayerCurHP`, not the obsolete score-only
  projection; the initial fixture failure did not establish a product bug.
- Existing C++ tests cover both envelopes, all prefixes, excess, null,
  discriminants and immutability. The preceding batch passed 58,666 architecture
  checks. No client semantic change invalidates that result; it is reused.
- **CLIENT_TESTED:** not claimed. No installation, launch, live database test
  or concurrency claim is made by this batch.
