# Native 7.48 subsystem research index

This is a search index, not a current whole-client parity audit. The rows retain
the historical coverage snapshot; consult the actual versioned record for the
current state and inputs. Broad `UNMAPPED` rows do not negate later narrow
records. Update coverage only after validating the relevant record; do not
promote a whole subsystem because one transition is proven.

| Subsystem | Transitions | Recorded coverage | Records and limits |
| --- | --- | --- | --- |
| Bootstrap/configuration | WinMain, config, device, managers, first scene, boot failure | `UNMAPPED` | No complete subsystem claim |
| Transport/cryptography | Connect, seed/queues, framing, decrypt, checksum, disconnect | Narrow `CONTRACT` | `transport/socket-disconnect-return-selectserver.md`: connection, callback, ownership and disconnect only; framing/decrypt/checksum remain separate gaps |
| Opcode size gate | Received header, opcode/size, rejection/logging, actual caller | `LOCATED` | `transport/packet-size-gate.md` |
| Server selection/login | List, credentials, response, key exchange, failures | `UNMAPPED` | Search newer flow records before opening research |
| Character selection/creation | List, create/delete, slot, enter world, teardown | Narrow `CONTRACT` | `lifecycle/character-logout-selectchar-relogin.md`; `ui/select-character-exp-threshold.md` covers selection EXP; creation/deletion are separate |
| Scene/field dispatcher | Socket -> current scene -> controls -> handler -> effects | `UNMAPPED` | No complete dispatcher claim |
| World tick/input | Message loop, FrameMove, mouse/keyboard, focus, auto-run | Narrow `CONTRACT` | `transport/motion-emote-roundtrip.md`: `0x36A` emotes; `ui/screenshot-capture.md`: Print Screen; not general tick/auto-run |
| Entity visibility/recovery | Reference before CreateMob, request, spatial validation, recreation | Narrow `CONTRACT` | `transport/missing-entity-request.md`: C->S `0x369`, not the general visibility lifecycle |
| Death/rebirth | Death state, cooldown, recall, request, persistence | Narrow `CONTRACT` | `transport/restart-recall-request.md`: C->S `0x289`, not the full lifecycle |
| Session keepalive | Scene tick, header, phase, absent response | Narrow `CONTRACT` | `transport/keepalive-ping.md`: C->S `0x3A0`; general timeouts belong to transport |
| City change | Town calculation, frame, HomeTown update | Narrow `CONTRACT` | `transport/change-city-request.md`: C->S `0x291`; position/town rules remain server-authoritative |
| Portal request | Cell confirmation, frame, server resolution | Narrow `CONTRACT` | `transport/req-teleport.md`: C->S `0x290`; route, charge and persistence remain authoritative |
| NPC interaction | Click, confirmation, frame, server allowlist | Narrow `CONTRACT` | `transport/use-npc-request.md`: C->S `0x28B`; context/rules remain authoritative |
| Guild member removal | Confirmation, target, server authorization | Narrow `CONTRACT` | `transport/guild-deprivate-request.md`: C->S `0x28C`; role/target/persistence remain authoritative |
| Guild relations | Confirmation, local/target guild, authorization | Narrow `CONTRACT` | `transport/guild-relations-request.md`: C->S `0xE0E/0xE12`; leadership/war/alliance/persistence remain authoritative |
| Zone challenge confirmation | Selection, frame, server response | Narrow `CONTRACT` | `transport/challenge-confirm-request.md`: C->S `0x28F`, not challenge rules |
| Movement | Intention, route, local prediction, stop, correction | `UNMAPPED` | No complete subsystem claim |
| Combat/skills | Selection, range, intention, confirmation, effects, rejection | `UNMAPPED` | Search transition records |
| Character/Score/equipment | Create/update, score, equipment, owner/observer, HUD | `UNMAPPED` | Search transition records |
| Affects/visual effects | Update, visual creation, tick, removal, relogin | `UNMAPPED` | Search transition records |
| Inventory/cargo | Open, grid, drag, use, move, rollback, close | `UNMAPPED` | Search transition records |
| NPC/shop/auto-trade | Creation, interaction, buy/sell, merchant, teardown | `UNMAPPED` | Search transition records |
| Trade/party/guild | Request, confirmation, windows, sync, cancellation | `UNMAPPED` | Search transition records |
| Mix/quest/special systems | Recipes, slots, intentions, response, failure | `UNMAPPED` | Search transition records |
| UI lifecycle | Construction, binding, root, input, focus, destruction, relogin | `LOCATED` | `ui/control-focus-ime-lifecycle.md` |
| Render/assets | Loaders, meshes, textures, scale, pipeline, device reset | `UNMAPPED` | No complete rendering claim |
| Shutdown/reconnection | Socket close, scene teardown, globals, retry, exit | Narrow `CONTRACT` flows | `lifecycle/application-close-global-shutdown.md`, `transport/socket-disconnect-return-selectserver.md`, `field-scene-rebuild-after-server-move.md`, `character-logout-selectchar-relogin.md`; explicit account switching remains separate |

Record paths above resolve under `.agents/research/client748/flows/`.
[Porting notes](../../../../DOCS/client/port-748.md),
[UI windows](../../../../DOCS/client/ui-windows.md) and source comments are
search aids. Move stable knowledge into the affected transition record as it
is traced. A cited function alone never promotes coverage to `TRACED`.
