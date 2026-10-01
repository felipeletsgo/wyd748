# Versioned WYD 7.48 client research

This directory preserves reproducible technical evidence for the 7.48 client
parity program. Recover the native executable's observable flow and compare
it with the current client and WYD-Go before changing a legacy boundary.
Conversation, source comments, and TMProject 7.69+ can suggest a research
seed; they do not establish the contract.

## Resume efficiently

1. Read the root `AGENTS.md` and the applicable research skill once.
2. Read only the handoff relevant to the selected transition. The program
   and parity handoffs are historical continuity aids, not the current queue.
3. Check `git status --short`, HEAD, the scoped diff, and the canonical
   record. Current source and records override stale handoff facts.
4. Reuse evidence and validation with unchanged inputs. Recalculate native
   identity only when its recorded path/metadata changed or the identity is
   insufficient; candidate hashes, logs, dumps, and line numbers are volatile.
5. Select the concrete missing transition and query only that gap. Use
   Ghidra when existing evidence cannot answer it. Update the existing record
   and validate the changed batch once; do not restart the census or builds.

The recorded native reference identity is:

```text
references/client748/WYD.exe
8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
```

The active candidate is `tmproject/client748/project.exe`; a source commit
or a `-NoDeploy` build does not update it. Bind any executed-client result
to the actual executable and assets used, not to a historical handoff hash.

## Organization and maturity

- `flows/TEMPLATE.md`: template for an observable transition.
- `flows/<subsystem>/`: one record per transition; avoid a generic inventory
  record combining opening, dragging, use, equipment, sale, and rollback.
- `exports/`: focused reproducible evidence tied to a record. Pass decisive
  functions, tables, and slots explicitly to `ExportWydFlow.java`; broad
  exploratory exports remain regenerable ignored caches.
- [Scene-transition evidence ledger](inventory/scene-transition-evidence-log.md):
  questions, interpretations, and remaining gaps from the historical
  lifecycle slices. It does not promote a canonical flow by itself.
- [Evidence schema](../../skills/wyd-client748-research/references/evidence-record.md):
  required sections, citations, and maturity gates.
- `.agents/handoffs/`: operational continuity, next action, and risks.
  A handoff never replaces the canonical record or native project.

## Selected flow index

This is a selected index, not the complete census. Record front matter is
authoritative. The entries below were reconciled with their records on
2026-09-30; future promotions must update both. The central
[documentation map](../../../DOCS/documentation-map.md) inventories all
versioned documentation and exports.

| Flow | Evidence state | Record |
| --- | --- | --- |
| Opcode size gate | `LOCATED` | [flows/transport/packet-size-gate.md](flows/transport/packet-size-gate.md) |
| Animation-array byte probe `0x1C1/0x2C2` | `CONTRACT` | [flows/transport/bone-animation-array-probe.md](flows/transport/bone-animation-array-probe.md) |
| Control focus, IME, and lifecycle | `TRACED` | [flows/ui/control-focus-ime-lifecycle.md](flows/ui/control-focus-ime-lifecycle.md) |
| Scene transition and switching | `LOCATED` | [flows/lifecycle/scene-transition.md](flows/lifecycle/scene-transition.md) |
| TCP disconnect and return to server selection | `CONTRACT` | [flows/transport/socket-disconnect-return-selectserver.md](flows/transport/socket-disconnect-return-selectserver.md) |
| Field reconstruction after server migration | `CONTRACT` | [flows/lifecycle/field-scene-rebuild-after-server-move.md](flows/lifecycle/field-scene-rebuild-after-server-move.md) |
| Character logout, selection, and relogin | `CONTRACT` | [flows/lifecycle/character-logout-selectchar-relogin.md](flows/lifecycle/character-logout-selectchar-relogin.md) |
| Application close and global shutdown | `CONTRACT` | [flows/lifecycle/application-close-global-shutdown.md](flows/lifecycle/application-close-global-shutdown.md) |
| AutoKick filter update and consumption `0x2C8` | `TRACED` | [flows/transport/autokick-filter-update.md](flows/transport/autokick-filter-update.md) |
| Bidirectional local chat `0x333` | `CONTRACT` | [flows/ui/local-chat-message.md](flows/ui/local-chat-message.md) |
| Scene text notice `0x101` | `CONTRACT` | [flows/ui/message-panel-text.md](flows/ui/message-panel-text.md) |
| Billing notice `0x194` | `CONTRACT` | [flows/ui/billing-notice.md](flows/ui/billing-notice.md) |
| Indexed/parameterized notices `0x105/0x106` | `UNMAPPED` (coordinated extension) | [flows/ui/indexed-parameterized-message-extension.md](flows/ui/indexed-parameterized-message-extension.md) |
| F shortcut for equipment-matched consumables | `CONTRACT` | [flows/ui/equipped-item-matched-consumable-shortcut.md](flows/ui/equipped-item-matched-consumable-shortcut.md) |
| E shortcut for special potions | `CONTRACT` | [flows/ui/special-potion-shortcut.md](flows/ui/special-potion-shortcut.md) |
| TOTO list loader | `CONTRACT` | [flows/ui/toto-list-loader.md](flows/ui/toto-list-loader.md) |
| TOTO selection, keyboard, and closure | `TRACED` | [flows/ui/toto-selection-close.md](flows/ui/toto-selection-close.md) |
| TOTO ticket purchase and materialization | `CONTRACT` | [flows/transport/toto-buy.md](flows/transport/toto-buy.md) |
| Gamble/Jackpot betting, roll, and result | `CONTRACT` | [flows/ui/gamble-jackpot.md](flows/ui/gamble-jackpot.md) |
| Skill-master opening, rendering, and purchase | `CONTRACT` | [flows/ui/skill-master-purchase.md](flows/ui/skill-master-purchase.md) |
| Grid-item visual bounds and scale | `TRACED` | [flows/ui/grid-item-mesh-scale.md](flows/ui/grid-item-mesh-scale.md) |
| Shop and Inventory side-by-side layout | `TRACED` | [flows/ui/shop-inventory-layout.md](flows/ui/shop-inventory-layout.md) |
| Trade and Inventory side-by-side layout | `TRACED` | [flows/ui/trade-inventory-layout.md](flows/ui/trade-inventory-layout.md) |
| Trade offer/closure consumers and emitters `0x383/0x384` | `TRACED` | [flows/transport/trade-session-envelope.md](flows/transport/trade-session-envelope.md) |
| First trade-check visual acknowledgement `0x386` | `CONTRACT` | [flows/transport/trade-check-confirmation-contract.md](flows/transport/trade-check-confirmation-contract.md) |
| Shared AutoTrade, Cargo, and Inventory positioning | `TRACED` | [flows/ui/auto-trade-inventory-layout.md](flows/ui/auto-trade-inventory-layout.md) |
| Six ItemMix panels with Inventory | `TRACED` | [flows/ui/native-mix-inventory-layout.md](flows/ui/native-mix-inventory-layout.md) |
| Bottom-right system-menu initial position and toggle | `TRACED` | [flows/ui/system-menu-initial-layout.md](flows/ui/system-menu-initial-layout.md) |
| Party-panel layout and lifecycle | `CONTRACT` | [flows/ui/party-panel-layout-lifecycle.md](flows/ui/party-panel-layout-lifecycle.md) |
| Motion/emote send and application `0x36A` | `CONTRACT` | [flows/transport/motion-emote-roundtrip.md](flows/transport/motion-emote-roundtrip.md) |
| Missing-entity recovery `0x369` | `CONTRACT` | [flows/transport/missing-entity-request.md](flows/transport/missing-entity-request.md) |
| Restart/recall request `0x289` | `CONTRACT` | [flows/transport/restart-recall-request.md](flows/transport/restart-recall-request.md) |
| Periodic keepalive `0x3A0` | `CONTRACT` | [flows/transport/keepalive-ping.md](flows/transport/keepalive-ping.md) |
| Change-city request `0x291` | `CONTRACT` | [flows/transport/change-city-request.md](flows/transport/change-city-request.md) |
| Portal request `0x290` | `CONTRACT` | [flows/transport/req-teleport.md](flows/transport/req-teleport.md) |
| Air transport `0xAD9` | `CONTRACT` | [flows/transport/airmove-contract.md](flows/transport/airmove-contract.md) |
| NPC interaction `0x28B` | `CONTRACT` | [flows/transport/use-npc-request.md](flows/transport/use-npc-request.md) |
| Guild-member removal `0x28C` | `CONTRACT` | [flows/transport/guild-deprivate-request.md](flows/transport/guild-deprivate-request.md) |
| Guild war and alliance `0xE0E/0xE12` | `CONTRACT` | [flows/transport/guild-relations-request.md](flows/transport/guild-relations-request.md) |
| Cross-channel war letters `0xED7/0xED8` | `CONTRACT` | [flows/transport/server-war-letter-contract.md](flows/transport/server-war-letter-contract.md) |
| Authoritative skill-belt snapshot `0x378` | `CONTRACT` | [flows/transport/short-skill-snapshot-contract.md](flows/transport/short-skill-snapshot-contract.md) |
| Action/ActionStop/Illusion envelope `0x366/0x367/0x368` | `CONTRACT` | [flows/transport/action-frame-contract.md](flows/transport/action-frame-contract.md) |
| Variable attack envelope `0x39D/0x39E/0x36C` | `CONTRACT` | [flows/transport/attack-frame-envelope.md](flows/transport/attack-frame-envelope.md) |
| Move/purchase confirmations `0x376/0x379` | `CONTRACT` | [flows/transport/inventory-transaction-confirmations.md](flows/transport/inventory-transaction-confirmations.md) |
| Cargo gold deposit/withdrawal confirmations `0x388/0x387` | `CONTRACT` | [flows/transport/cargo-gold-transfer-confirmations.md](flows/transport/cargo-gold-transfer-confirmations.md) |
| AutoTrade publication/query envelope `0x397` | `CONTRACT` | [flows/transport/auto-trade-envelope.md](flows/transport/auto-trade-envelope.md) |
| Celestial Capsule query/response envelope `0x2CD/0xDC3` | `CONTRACT` | [flows/transport/capsule-info-envelope.md](flows/transport/capsule-info-envelope.md) |
| Zone-challenge confirmation `0x28F` | `CONTRACT` | [flows/transport/challenge-confirm-request.md](flows/transport/challenge-confirm-request.md) |
| Player Ctrl+right-click interaction menu | `TRACED` | [flows/ui/player-interaction-menu-lifecycle.md](flows/ui/player-interaction-menu-lifecycle.md) |
| Quest-panel layout and lifecycle | `TRACED` | [flows/ui/quest-panel-layout-lifecycle.md](flows/ui/quest-panel-layout-lifecycle.md) |
| Character, Skill, and Inventory side-by-side layout | `TRACED` | [flows/ui/feature-panel-layout.md](flows/ui/feature-panel-layout.md) |
| Character Att Speed, C.POINT, HOLD, and Kingdom updates | `TRACED` | [flows/ui/character-stat-fields-update.md](flows/ui/character-stat-fields-update.md) |
| Character-selection panel position | `TRACED` | [flows/ui/select-character-layout.md](flows/ui/select-character-layout.md) |
| Character-deletion confirmation and password | `CONTRACT` | [flows/ui/select-character-delete-password.md](flows/ui/select-character-delete-password.md) |
| Server-selection position and lifecycle | `TRACED` | [flows/ui/server-selection-layout-lifecycle.md](flows/ui/server-selection-layout-lifecycle.md) |
| Premium Firework display `0x3CA` | `CONTRACT` | [flows/ui/premium-firework-display.md](flows/ui/premium-firework-display.md) |
| C.C state/control synchronization | `TRACED` | [flows/ui/cc-auto-combat-state-sync.md](flows/ui/cc-auto-combat-state-sync.md) |
| PK Mode toggle and authoritative application `0x399` | `CONTRACT` | [flows/ui/pk-mode-toggle-lifecycle.md](flows/ui/pk-mode-toggle-lifecycle.md) |
| PvP death Held EXP debt lifecycle | `CONTRACT` | [flows/combat/pvp-death-held-exp-lifecycle.md](flows/combat/pvp-death-held-exp-lifecycle.md) |
| Native `SetMyHumanMagic` no-op | ``STATICALLY_EVIDENCED`` | [inventory/set-my-human-magic-noop.md](inventory/set-my-human-magic-noop.md) |

Evidence maturity is not feature availability. `LOCATED` does not authorize
an unsupported parity change; `TRACED` establishes the recorded flow, not
every neighboring behavior. `CONTRACT` adds a testable wire/ABI/resource
boundary, but does not mean that both implementations or runtime gates are
complete. In particular, the trade-session record remains `TRACED`, while
trade-check and Cargo gold acknowledgement records are `CONTRACT`.
The indexed/parameterized-message record describes a coordinated extension,
not an unlocated native equivalent.

`CLIENT_TESTED` requires the real affected flow in the built candidate;
static/source checks and builds do not substitute for execution. The
`STATICALLY_EVIDENCED` inventory note above is not a flow-schema promotion.

## Native corpus and access

The historical full decompilation corpus and its metadata are local-only
research inputs. Do not assume that a dated path in a handoff exists now.
The currently reused recovered project is `WYD748`, program `WYD.exe`,
under the ignored `tmproject/build/native-research/` directory. That is a
local research cache, not a product dependency or a versioned binary.

The historical corpus listed 4,146 functions; that snapshot count is not a
current completeness claim. Text exports lose data xrefs, indirect calls,
types, and receiver ownership. A missing textual function or caller never
proves absence from the native binary. Resolve only the needed gap in the
project and retain a focused, identity-bearing excerpt.

When the auxiliary corpus is configured, these queries can locate a new
seed; they are not mandatory resume steps:

```powershell
python .agents/skills/wyd-client748-research/scripts/query_corpus.py stats --repo .
python .agents/skills/wyd-client748-research/scripts/query_corpus.py flow 0055890a
python .agents/skills/wyd-client748-research/scripts/query_corpus.py search "FieldScene2.bin"
```

After a changed record batch:

```powershell
python .agents/skills/wyd-client748-research/scripts/validate_research.py --repo .
```

Do not store binaries, full decompilations, broad xref scans, dumps,
credentials, or bulk pseudocode here. `references/client748/` is read-only
historical evidence; active implementation belongs in client source/assets.
