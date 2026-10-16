# Evidence record contract

Records live in `.agents/research/client748/flows/<subsystem>/<flow>.md` and
follow the versioned template in that directory. New records use English
headings. The validator still accepts historical Portuguese headings while
existing records are migrated; do not use them in new or updated records.

## Required front matter

- `id`: stable kebab-case identifier;
- `title`: concrete transition investigated;
- `subsystem`: subsystem-map domain;
- `status`: `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`, or `CLIENT_TESTED`;
- `native_sha256`: analyzed `WYD.exe` hash or `UNRESOLVED`;
- `updated`: ISO date `YYYY-MM-DD`.

This record schema measures native knowledge; `IMPLEMENTED` is a delivery/catalog
state, not an accepted research-record status. Record implementation and check
results in the validation section without inventing a schema value.

## Required sections

Every non-template record contains `Question`, `Evidence boundary`,
`Native 7.48 flow`, `State and lifecycle`, `Wire, ABI, and resources`,
`Current mapping`, `Delta matrix`, `Decisions`, `Gaps`, and `Validation`.

At `TRACED`, resolve `Callers` and `Callees` in Ghidra. At `CONTRACT`, the
ABI/resource and decision must be testable. At `CLIENT_TESTED`, record the
scenario, executable artifact/hash, and observed result of the real client flow.
Tie automated parity expectations to independent native evidence; a regression
test asserting the current implementation is not proof of native behavior.
Record affected asset identities and owner/observer or relogin coverage when
they determine the result. User-reported proof closes only the reported scenario.

Status measures knowledge of the native flow. A coordinated extension without
a 7.48 equivalent must not invent a native function, caller, or promotion.
Document and test its new contract in the feature, using a research record only
for the legacy boundary it crosses. `LOCATED` blocks an incomplete native
parity claim, not an independent extension.

## Citations

Cite evidence with address/symbol and current source file, for example:

```text
Native: WYD.exe sha256=... FUN_0055890A @ 0x0055890A
Source: tmproject/.../CPSock.cpp :: CPSock::ReadMessage
Server: internal/game/security.go :: inboundPacketSizeAllowed
Asset: tmproject/client748/UI/FieldScene2.bin :: root 257
```

Do not paste extensive pseudocode. Record the interpretation, relevant
branches, and a reproducible way to inspect the evidence.
