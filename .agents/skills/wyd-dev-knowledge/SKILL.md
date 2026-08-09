---
name: wyd-dev-knowledge
description: >-
  Documented knowledge base of WYD (With Your Destiny) server/client development,
  learned from the WebCheats forum (the top WYD dev community) and our own reverse
  engineering. Consult when working on WYD packet protocol/encryption, stat/score
  formulas, client versions (Cliver), GM commands, dev tooling, or the native
  7.5x function inventory. The FACTS are written here directly — you do not need
  to re-open the forum to use them. Complements `wyd-researcher` (local source
  sweep) and the `wyd-forum-scout` agent (fresh web lookups for gaps).
---

# WYD development — documented knowledge base

Self-contained notes on how WYD servers/clients work, distilled from the WebCheats WYD community
and our own work on this project. Use the facts directly; forum URLs appear only as provenance.

## 0. The knowledge landscape (so you know where truth lives)

- **Our project stack**: closed-source **7.54** `TMSRV.exe` extended by a hook plugin
  (`Sourcer Star Micronics/`), and the full-source **7.59-based SERVER W2PP**. Target client: **7.48**.
- **Most reliable sources, in order**: (1) our own code + live OllyDbg/Ghidra on the 7.54 exe;
  (2) local reference trees — `SOURCE(secrets) para referência/Descompilação/` (hand-decompiled 7.54
  natives, closest to our exe) and `source 7.59/Code/` (full readable 7.59 server); (3) the WebCheats
  forum.
- **WebCheats reality**: thread titles/discussion are public, but **verbatim code/source is behind a
  registration wall** (*"É necessário se cadastrar para acessar o conteúdo"*). We do **not** use the
  user's password. So the forum is best for *leads and public facts*, not code dumps — for actual code
  our LOCAL trees are both richer and fully accessible.
- **Era map** (learned by sweeping the archive back to 2008): 2008–~2012 WYD content is almost entirely
  **player-side** (CheatEngine speedhacks, gold/arch/celestial tricks, wall-jumping, file editors). The
  serious **server-source, protocol, and decompilation** work is **recent (2017–2025)**, concentrated in
  the *Descompilação* and *Desenvolvimento* subforums. Don't expect foundational engine internals in the
  old pages.

## 1. Packet protocol & encryption

**Header** (`_MSG`, 12 bytes, little-endian) — confirmed from our W2PP `CPSock`/`Basedef.h`:

| off | field | type |
|---|---|---|
| 0 | `Size` | int16 (whole packet size) |
| 2 | `KeyWord` | int8 (index into key table) |
| 3 | `CheckSum` | int8 |
| 4 | `Type` | int16 (opcode) |
| 6 | `ID` | int16 (client/conn id) |
| 8 | `ClientTick` | uint32 |

**Opcode/FLAG scheme** (stable across 7.48→7.59; verified both directions):
`FLAG_GAME2CLIENT 0x100`, `FLAG_CLIENT2GAME 0x200`, `FLAG_DB2GAME 0x400`, `FLAG_GAME2DB 0x800`.
Opcode = `base | flags`. Known anchors: `AccountLogin 0x20D` (13|C2G), `CNFAccountLogin 0x10A`
(10|G2C, the char-list/login-confirm to client), `CreateChar 0x20F`, `DeleteChar 0x211`,
`CNFCharacterLogin/enter-world 0x114` (20|G2C), `UpdateScore 0x336`, `CreateMob 0x364`,
`DBAccountLogin 0x803` (3|G2D). Char-list *refresh* after create/delete rides `0x110`/`0x112`.

**Encryption**: symmetric, **byte-by-byte** transform over the payload starting at **byte 4** (Type
onward), driven by a **512-byte key table** (`pKeyWord[512]`; pairs — `[i*2]` selects the keyword,
`[i*2+1]` is the transform byte). `KeyWord` field (offset 2) is a random index chosen per packet;
`CheckSum` (offset 3) is a running sum validated on receive. Sibling game **Aika** uses the *same
logic but in 4-byte chunks* — useful mental model. `INITCODE` handshake for our stack = `0x1F11F311`.

**Login-confirm (`0x10A`) body — the layout the 7.48 client actually reads** (learned the hard way
porting W2PP; the client casts the buffer and reads a 16-byte preamble BEFORE the char list):
`header(12)` · `char SecretCode[16]` (client copies this to its send-queue = crypto seed for next
packets) · `STRUCT_SELCHAR SelChar` **@offset 28** · `STRUCT_ITEM Cargo[128]` · `int Coin` ·
`char AccountName[16]` · `int SSN1,SSN2`. The 7.48 `STRUCT_SELCHAR`/`STRUCT_SCORE` are the **28-byte**
(WORD) variants, not 759's 44-byte (int) variant — see §2.

**Tooling — WYDStudio_PacketSniffer** (client-side, better than fopen dumps; captures BOTH directions):
- `WYDHook.dll` hooks the client Send/Recv, decrypts with the in-memory key table, writes plaintext to
  `PacketLog/Send` + `PacketLog/Recv`, shows headers live.
- `WYDSigGen.exe` scans `WYD.exe` → `WYD_Signatures.ini` (3-level fallback byte patterns; survives client
  updates). `WYDInjector.exe` injects (run as admin).
- Config: `WYD_KeyTable_custom.txt` (override keys), `WYD_PacketFilter.txt` (opcode filter; empty = all).
- Works only on stock `CPSock` clients without anti-injection.

## 2. Stats / score / combat

- **`STRUCT_SCORE` differs by version** (this is the #1 porting trap): 7.48/7.54 = **28 bytes** with
  `WORD Level/Defense/Attack`, packed `Merchant:4|Direction:4` + `Speed:4|ChaosRate:4` bytes,
  `WORD maxHP/maxMP/curHP/curMP`, `WORD STR/INT/DEX/CON`, `BYTE Mastery[4]`. 7.59 = **~44 bytes** with
  `int Level/Ac/Damage`, `int` HP/MP, `short Special[4]`. **WYD Global** is a different branch entirely
  (`STRUCT_MOB_GLOBAL` **1368 B**, `STRUCT_ITEM_GLOBAL` **12 B**) — never assume 7.x sizes for Global.
- **`STRUCT_ITEM` is stable at 8 bytes** across 7.48/7.54/7.59 (`short Index` + 3×{`uint8 eff, uint8 val`}).
  Global's item is 12 B (different).
- **Physical damage** (our worked example): 7.54 exe ≈ `Attack += Str/3 + Master[0] + Level` (no DEX
  term); 7.59 `Basedef.cpp` adds a DEX term (`Str/2 + Dex/3 + Special[0] + Level`, only when untransformed
  `face<4`). That gap is why "add DEX to damage" is a real feature, not a bug.
- **`bStatus` vs `Status`**: `bStatus` = base/raw score; `Status` = recomputed score (by
  `BASE_GetCurrentScore`) that gets sent to the client and used in combat. Patch the right one.
- **Rule for all ported formulas**: algorithms port across versions; **struct layouts / field offsets /
  absolute addresses do NOT** — re-express against our field names and re-verify offsets on our exe.

The community has hand-decompiled these 7.56/7.57 natives (leads if our local `Descompilação/` lacks a
version): `BASE_GetCurrentScore`, `CPSock::AddMessage`, `CUser`, `GetBonusHp/GetBonusMP`, `GetMountBonus`,
`BASE_GetRoute`, `BASE_ReadItemListFile`, CMOB `BattleProcessor`/`StandingByProcessor`,
`ProcessHardCoreDeath`, `DoWar/DoAlly/SetCityTower`, `DoRanking`, `CreateGuild/CreateSubGuild`,
`GetMobID(const char*)`. (Verbatim code login-walled; prefer our local trees.)

## 3. Client versions (Cliver)

- **7.48** — older client; the target of this project (needs the 7.54 server to speak 7.48 wire format).
- **7.54–7.56** — mid; sequential update patches. **7.58** — OnGame. **7.59–7.60** — newer; several
  reports of crashing on Windows 10. **6.80** — Supreme Destiny (legacy).
- **Version detection**: client config file carries the version in its **first two bytes** (hex).
- **Patch naming**: sequential `<from><to>.zip` (e.g. `10001001.zip` = 1000→1001).
- **Our Cliver bypass** (already live): the server reads the live accepted `min` version at each `0x20D`
  and rewrites the packet's Cliver to `min+1`, so a downgraded 7.48 client connects to the 7.54 exe
  without patching the client. Don't hardcode a fixed Cliver — a stock protocol DLL rewrites `min`.

## 4. GM commands (this W2PP/Thyra-lineage server supports them)

Sent as a **whisper to the NPC named `gm`** with `+command args` (standard GM level = **2000**). The
active `+set` group maps to our config constants (`BILLING`, `FREEEXP`, `CHARSELBILL`, `POTIONCOUNT`,
event vars). Legacy `+set` (level ≠ 2000) is inert on a standard GM.

- **Reload/maint**: `+timer <ms≥5000>` `+saveall` `+decay` `+reboot` `+delayreboot` `+log` `+reloadnpc`
  `+reloaddroplist` `+makedroplist` `+reloadevento` `+reloadpesa` `+reloadbalanceamento` `+reloadperga`
  `+reloadcash` `+updatebalanceio` `+reloadexptable` `+reloadserver` `+reloadguild` `+readguildname`
  `+billconnect`.
- **Config `+set`**: `billmode|billfree|charselbill|potioncount|potiondelay|partybonus|guildboard <v>`,
  `evstart|evend|evitem|evrate|evindex|evdelete|evon|evnotice`, `double|deadpoint|dungeonevent <0|1>`,
  `champ|chall <zone> <guild>`, `drop <pos> <rate>`.
- **Moderation**: `+derrubar`(0) `+desligar`(1) `+banfirewall`(2) `+mute` `+muteall` `+desmuteall`
  `+kick <nick>` `+bann <account> <days>` (<30 temp, ≥30 perm).
- **Char (current target)**: `+name` `+cp` `+frag` `+celestial <0..3>` `+class` `+learn` `+buff <type>`
  `+nobuff` `+citizen` `+soul` `+snoop` `+divinetime` `+isdivine`.
- **Mob/item/spawn**: `+generate <genIdx>` `+create <mobFile>`(./npc/) `+read` `+save` `+kill <name>`
  `+gift <target> <item> <e1> <v1> <e2> <v2>` `+createkefra` `+killkefra` `+setjoias` `+nojoias`
  `+rebuild` `+trtarget|+trsource|+trrate <treasure1-8> ...`.
- **Map/teleport/notice**: `+weather` `+attmap` `+trn` `+teleport <x> <y>` (<100 → grid*128+64)
  `+emotion <type> <v>` `+gridon`/`+gridoff` `+notice` `+bcnot` `+chiefnotice`.
- **Exp/dungeon/quiz**: `+waterexp` `+svexp <baseLvl> <xp>` `+nigexp` `+quiz` `+bigquiz` `+cubo`.
- **Guild/war/event**: `+guildday <0..6>` `+guildhour` `+newbiehour` `+weekmode <0..5>` `+gfame`
  `+iniciarguerra` `+finalizarguerra` `+chiefsummon` `+event` `+partydif` `+rvrhour` `+rvrbonus`
  `+gtorrehour` `+hardcore <0|1>` `+dropitem <0|1>` `+maxnightmare` `+statsapphire` `+battleroyal <item>`
  `+maxbpoints`.
- **No-`+` direct whispers**: `reloadrop` `limpinventario` `infoplay` `not <text>` `onlineip`
  `onlineiplist`.
- **Broken**: `+npko` (IPPos=-1), `+gerar` (empty parser buffer).

## 5. Common build/DB/crash gotchas (community-confirmed, matches ours)

- **MySQL "Client does not support authentication protocol"** — MySQL 8 `caching_sha2_password` vs old
  client; use `mysql_native_password` / MySQL 5.7. We hit the sibling issue (`skip_ssl` + connector 6.1
  vs 5.7). **DB import encoding**: cp1252/utf8 mismatch corrupts Portuguese — import with the matching
  charset.
- **Crash triage**: build the server with VS warning level 3/4 and use the VS debugger; WYD's own "debug
  mode" surfaces crash causes. (Our own crashes came from mis-aligned inline-asm trampolines and wrong
  packet struct sizes — both class-of-bug the community documents.)
- **Grid/teleport math**: map coords are grid-based; a value <100 in teleport means `grid*128+64`. Getting
  grid math wrong = teleport bugs. (This is the same `SPX/SPY`↔`Position` space we convert in the char
  list.)

## 6. Running a 7.48 client against a 759/754 server (wire-format deltas)

Hard-won facts from porting **SERVER W2PP** (759) to the **7.48 client**. Method that works: hook the
7.54 Micronics plugin's `CPSock::AddMessage` to **capture the real 7.48 wire bytes**, then replicate
byte-for-byte — never guess struct sizes. Full detail lives in the auto-memory `project_w2pp_748_protocol.md`;
the deltas that generalize:

- **Enter-world (0x114) Size field = 788** (`0x0314`), even though the send buffer is 1244. The client
  frames packets by the Size field; sending 1244 leaves 456 stray bytes → **stream desync → a phantom
  duplicate character + an uncontrollable player**. Layout: header(12)+PosX/PosY(4)+STRUCT_MOB(756)+trailing(16).
- **STRUCT_MOB in the enter-world = 756 bytes**: Equip[16]@108(packet), **Inventory[64]@236** — the
  inventory slot count is the SAME (64) as 759; don't "fix" it. Exp is uint32 (not long long); scores are
  the 28-byte WORD form.
- **Movement opcodes are SWAPPED between 759 and 7.48.** The 7.48 client **walks by sending `0x366`
  continuously** (its layout: TargetX/Y@24/@26, Route = ASCII direction-digit string@28). 759 walks with
  `0x36C` (`_MSG_Action`=108) and treats `0x366` (`_MSG_Action2`=102) as *Stop* (and rejects repeats +
  a 900ms anti-speed on the 0x36C path). So: inbound, remap the 0x366 layout to `MSG_Action` and treat as
  walk (disable the 900ms check); outbound, convert 0x36C→0x366.
- **Char-list is `0x10E`** (not 0x10A), **1800 bytes**, positions as separate `HomeTownX[4]`+`HomeTownY[4]`,
  no SecretCode preamble (SelChar@offset 12).
- **Second/numeric password ("senha numerica")** is a 759 step the 7.48 client doesn't do; it gates
  USER_SELCHAR→USER_CHARWAIT. Open both gates (TMSrv char-login + DBSrv SecurePass) to let 7.48 through.
- **Appearance:** the 7.48 client builds a humanoid from **body-part item meshes**, so a "naked" char
  (Mortal form, Equip[1-15] empty) renders broken/"dead" — inject the default human body set
  `{1103,1115,1127,1139,1151}` into the empty body slots. And `BASE_VisualItemCode` refine **10-16**
  (0xA000+ in the ItemEff word's top nibble) exists only in 759 client meshes → cap to `& 0x0FFF` for 7.48.
- **Standing/alive pose** comes from an **Action_Stop (0x366)** the server sends for the player *after*
  its own CreateMob; without it the char lies "dead" even with HP>0.
- **A mob's own CreateMob is for OTHERS** — the player's own body comes from the enter-world, so exclude
  the entering player from its CreateMob multicast (else another duplicate).

## 7. NPC interaction & merchant shop (7.48 protocol, verified in-game)

The **client decides** a lot here — when the local NPC source is silent, read the **client source**
`SERVER W2PP/SOURCE GAME/Projects/TMProject/` (759 client, same logic as 7.48): `Basedef.h` for the
`MSG_*` structs + opcode `constexpr`s, `TMFieldScene.cpp` for the `OnPacket*` handlers and the NPC
click handler. This is how the facts below were nailed (each cost hours of guessing before reading it).

- **NPC click:** the client recognizes an NPC's *type* from **`m_stScore.Reserved & 0xF`** — and
  `Reserved` is the **`Merchant` byte** of the score (offset 6 in the 28-byte score = **@106 in the
  CreateMob Status@100**). So `Merchant` = **low nibble = type, high nibble = facing direction**
  (`nDir = Reserved>>4`). Type map (client click handler ~`TMFieldScene.cpp:14518`): **3 = gold shop**
  (client sends `0x27B` REQShopList + sets its merchant id), 1/2 = other NPC actions, others = quests.
  A generic click the client doesn't map to a type sends **`0x28B`** (UseNPC: `npcid@12`, `ClickOk@16`).
- **Open shop:** client → `0x27B` REQShopList (`TargetID@12`). Server → **`0x17C` ShopList (532B)**:
  `ShopType int@12`, `STRUCT_ITEM List[64]@16`, `Tax int@528`, header `ID=ESCENE_FIELD 0x7530`. The
  window only opens when **`ShopType == 1`** (client `OnPacketShopList`: 1=normal gold, 3=skill master,
  4=donate; any other value is **silently ignored**). Client shows only the first 27 in a 5-col grid and
  reads the **price from its OWN itemlist** (server doesn't send price).
- **Shop stock = the mob's `Inventory[]`** (`Npc->Inventory[sellSlot]`, Secrets `BuyItem.cpp:23`).
- **Buy:** client → `0x379` `MSG_Buy` (24B): `TargetID u16@12`, `TargetCarryPos(=shop slot) i16@14`,
  `MyCarryPos i16@16`, `Coin int@18`. **CRITICAL: the client sends `TargetID=0`** (buying is a drag into
  the inventory grid, whose merchant-id field is 0) — even the native Secrets server would reject it
  (`mobID<1000`). So buy MUST be **server-authoritative**: track which shop the player opened server-side
  and use that + `sellSlot@14`, never the packet's TargetID. Native charges exactly
  `ItemList[index].Price` and deducts gold (Secrets `BuyItem.cpp`).
- **Sell** = `0x37A` (`MSG_Sell`: `npcId`, `type`, `sellSlot`); **close shop** = `0x196` (`150|0x100`).
- **`itemlist.csv` price column** (parser `BASE_ReadItemListFile`, W2PP `Basedef.cpp:6077`): comma cols
  `0=Index 1=Name 2=Mesh.Texture 3=Req 4=unique` **`5=Price`** `6=nPos 7=Extra 8=Grade 9+=effects`.
  Lines starting `#` are comments. The Micronics `itemlist.csv` is the one compatible with the 7.48 client.

**Method that keeps paying off:** when a client action does nothing, add a server log of the raw packet
(opcode + bytes), have the user perform it, then read the matching `OnPacket*`/click handler in the
client source to see the exact precondition/field the client requires. Beats static guessing every time.

## 8. Forum archive sweep (2026-07-15)

This section records the broad public crawl performed on 2026-07-15. It is an
index and triage layer: a thread title proves that a routine/topic was studied,
but it does **not** prove the hidden code or formula is correct. Promote a fact
to the sections above only after reading the body and comparing it with local
source/decompilation.

### Coverage

- Fully indexed: Development (17 pages), Decompilation (7), Tutorials (50),
  Website (3): **77 pages / 1,861 unique topics**.
- Every Decompilation topic was opened and the authenticated account reacted to
  all **163 topics**. The unlocked snapshot contains code artifacts in 159
  topics. The complete source/symbol index lives in
  [`references/webcheats-decompilation-catalog.md`](references/webcheats-decompilation-catalog.md).
- Every Development topic was opened: **422 topics / 506 substantive public
  posts**.
- Questions was indexed through all **186 pages / 4,602 unique topics**. Title
  distribution (overlapping): server/source/build 480; stats/skills 341; items
  283; world/NPC/map 252; client/UI 265; social/party/guild/trade 163; protocol 21.
- Authenticated verification found a second access layer: most Decompilation
  code blocks require the logged-in member to react/LIKE to each individual
  topic. Login alone reveals comments but not the main code. The account owner
  explicitly approved the 163 visible reactions used for this archive snapshot.
- The archive confirms the era split: old pages are mostly player guides and
  cheats; engine-quality material is concentrated in 2019+ Development and
  especially the 2023+ Decompilation section.

### Publicly confirmed facts added by this sweep

- The initial `0x1F11F311` hello and the 116-byte `0x20D` login may arrive in a
  **single 120-byte TCP read**. Framing must consume the first four bytes then
  decrypt/frame the remaining packet; one TCP read is not one WYD packet.
- Login scene reply is `0x10E` on clients **below 7.55**, while newer families
  use `0x10A`. This matches our 7.48 char-list choice.
- `0x182` is a generic **grid item update** (inventory/cargo/equip), not the
  packet that creates a ground drop. Do not infer semantics from the name alone.
- Client opcode `0x1C1` is named `REQArray` in a client source but its handler
  immediately returns; public analysis treats it as legacy/no-op.
- Old TMSrv outbound packet capture is centered on `CPSock::AddMessage`.
- W2PP resist order is often copied incorrectly. Correct semantic order is
  `Resist[0]=Fire`, `[1]=Ice`, `[2]=Sacred`, `[3]=Thunder`.
- In 7.57+, summons/evocations are held in a second list (community name
  `MobList`) rather than `PartyList`; the reported native address is version
  specific and must not be reused. Keeping summons outside the party is native
  behavior, not a custom UX choice.
- Implementing out-of-party summons touches more than party UI: attack handler,
  empty-grid selection, StandingBy/BattleProcessor, attack-area selection,
  ProcessAffect, MobAttackOne, SendSummonParty, GenerateSummon/GenerateMob and
  mount processing all need to agree on ownership and targeting.
- Packet `0x3EA` (`SendSummonParty` in 7.57+) displays the request/icon asking
  another character to be summoned; it is not the summon creature list.
- `CheckMobHpDamage`/the auxiliary mob-HP counting mechanism exists because
  older clients use narrow HP fields. Community reports 7.59 clients moved to
  4-byte HP and no longer need the same boss-HP workaround. Verify per wire.
- `NPCGener_new` is generated by the server at boot with normalized/valid
  numbering; administrators should edit `NPCGener.txt`, not maintain both files.
- `GridMulticast mobidx,pos` commonly means a mob lost valid map/grid state,
  usually by walking into nonexistent/blocked terrain. Suppressing the log is
  not a fix; validate/reposition the mob and correct route/grid handling.
- Public RouteType observations: 0 stationary; 1 follows its route then leaves/
  waits for regeneration; 2 cycles configured points; 5 is suspected leader
  following for mounts/summons. Values 3/4/6 remain unconfirmed leads.
- Buff lifetime is server-owned. A robust design keeps an active-affect list,
  ticks/removes it centrally, broadcasts add/remove, and triggers score
  recalculation only for affects that change derived stats. This agrees with
  our actor/tick design.
- Client/server `SkillData` mismatch or extreme edits can break automatic rebuff;
  keep both files structurally compatible and vary one field at a time.
- A skill can deal correct high damage while the UI shows a wrapped/smaller magic
  value; treat displayed magic attack as a client field-width/layout problem
  before changing authoritative damage.
- There is no need to reserve a globally unique volatile for each custom item:
  a shared volatile can dispatch server-side by item ID. This is the safer model
  already used by `volatiles.json` overrides.
- WYDStudio_PacketSniffer discovers the live key table, can dump/reload a custom
  table, hooks send/recv, logs decrypted packets separately, filters opcodes,
  rotates at a configurable 1..200 dumps, and generates signatures with three
  fallback patterns. It only targets stock CPSock clients without anti-injection.
- Current ItemList/SkillData families reported by the public converter use
  140-byte, 168-byte and newer 172-byte records. The tool recomputes checksum;
  do not assume the 7.48 record size for Loki/Global files.
- `UITextureListN` tools report two layouts, 264 and 528 bytes. FieldScene
  `HUD_UNK_16` is an icon grid used by inventory/equipment/bags, not a generic
  unknown panel.

### Authenticated decompilation findings

These are semantic findings from the unlocked bodies. Function addresses and
posted struct widths were deliberately not promoted because they belong to the
author's 7.56/7.57 binaries, not our 7.54 executable.

**Character points and base HP/MP (7.56 family):**

- `BASE_GetHpMp` uses the class table `(Str, Int, Dex, Con, baseHP, baseMP)`:
  TK `(8,4,7,6,80,45)`, Foema `(5,8,5,5,60,65)`, BM
  `(6,6,9,5,70,55)`, HT `(8,9,13,6,75,60)`. The posted formula is
  `MaxHP = baseHP + 2*Con - baseCon + hpPerLevel*Level` and
  `MaxMP = baseMP + 2*Int - baseInt + mpPerLevel*Level`, with mortal
  per-level HP `[3,1,1,2]` and MP `[1,3,2,1]` for TK/FM/BM/HT.
- Mortal score points are not simply `5*level` at all levels: the 7.56 routine
  adds another `5*(level-254)` from 254 and `10*(level-299)` from 299, then
  switches to a fixed base plus `12` per level after 354. Arch starts at
  `6*level`, adds quest/item-derived `EF_PREVBONUS`, and also uses `12` per
  level after 354. Celestial has a separate staircase, crystal/fury quest
  bonuses, and a fixed `+1000`; do not reuse mortal caps for it.
- Mortal skill-point budget is `3*level` plus one extra point for levels
  200..354; Arch uses `4*level + 3*min(EF_PREVBONUS, cap)` with a reduction
  after 354; Celestial uses a fixed base of 1600. Learned skills consume the
  `SkillPoint` stored in `SkillData`. The posted decompilation appears to assign
  `SkillBonus` incorrectly in one branch, so use the formula, not that final
  assignment, and compare with the 7.59 source.
- Special-point budget starts at `2*level` for Mortal. Arch adds levels above
  199 and `2*EF_PREVBONUS`, then removes the extra progression after 354;
  Celestial uses 855. Available points are budget minus the four allocated
  `Special[]` values.
- Packet `0x277` applies score, special and skill bonuses server-side. Score
  allocation spends 100 at once when at least 300 points remain, otherwise 1;
  INT and CON also add two base MP/HP per spent point. Special mastery is capped
  at 200, 255 or 320 depending on learned eighth skills. Skill learning verifies
  class, target, price, prerequisites and the learned bit before mutation.

**Item aggregation and refinement:**

- Static effects come from `ItemList`; dynamic effects come from the item's
  three effect slots. `EF_RESISTALL` contributes to each elemental resistance
  in both paths. Mounts and temporary mounts are special-cased rather than
  passed through ordinary equipment refinement.
- For ordinary effects, sanctification 1..10 scales approximately by
  `(sanc+10)/10`. The posted 11..15 multipliers are 220%, 250%, 280%, 320%
  and 370%. Identity/placement/requirements/volatile/incubation/previous-bonus
  fields are excluded; regen is also treated specially. Never sanctify metadata.
- `EF_RUNSPEED` is clamped before the `+9` refinement bonus. `EF_REGENHP/MP`
  is multiplied by sanctification in the static path. These exceptions explain
  why a generic “sum every effect then multiply” implementation diverges.
- Dual-wield damage uses the stronger hand plus 50% of the weaker hand when
  weapon unique types match, otherwise plus 30%; TK and HT learned skills add
  another 15%/10%. A complete matching armor set adds 5% AC. Recompute score
  from equipped server-side items after every equip/unequip/drop mutation.

**Affects, buffs and regeneration:**

- `SetAffect` refuses a weaker replacement and refuses a shorter replacement
  of the same type. For ordinary durations it stores `(time+8)/8`; special
  calendar/item affects use encoded absolute values. It calls score recompute
  immediately after changing the slot.
- `SetTick` derives `AffectType`, `TickType`, `AffectTime`, values and aggression
  from `SkillData`, clamps level to 255, reserves the last affect slot for Soul
  Limit, and applies learned-eighth duration modifiers. This is strong evidence
  that duration, stacking and refresh policy belong in one server-owned affect
  service, not per-skill timers.
- `ProcessAffect` is the central periodic executor. It handles Aura of Life,
  poison, spectral-link MP drain, thunder chains, pet attacks and other ticks,
  then decides whether HP, equipment, affects and/or score packets must be
  broadcast. A buff icon without this lifecycle is cosmetic; a lifecycle
  without `SendAffect`/score broadcast desynchronizes observers.
- `ReqHp/ReqMp` are authoritative requested values and `ApplyHp/ApplyMp` move
  current values toward them in bounded steps (posted cap 2000). Any direct HP
  or MP mutation must keep requested and current values coherent or observers
  will see phantom damage/death.

**Grid, height, movement and visibility:**

- `GetEmptyMobGridValid` first validates bounds and stale grid occupants, rejects
  height `127`, then searches rings of radius 1, 2 and 3. A candidate must be
  empty, valid terrain and differ from the requested tile's height by less than
  50. This is the native model for spawn collision, respawn displacement and
  “do not float above terrain.”
- HeightMap is a native signed `char` grid: raw byte 250 means height -6. Spawn
  relocation and walking must not share a threshold. `GetEmptyMobGridValid`
  uses signed delta `<50`, while `BASE_GetRoute` and the client use strict
  `abs(int8(next)-int8(current)) < 8` (`MH=8`) for every step.
- Native mob walking sends route segments rather than one Action per tile.
  Segment length is up to `2*Speed` (8 tiles at speed 4); `GetAction` sends the
  segment endpoint with an empty route and the client rebuilds/interpolates it.
  Every new Action resets the client's movement clock, so per-tile Actions
  visibly restart the walk animation.
- `DoTeleport` resolves an empty grid, builds a `MSG_Action` with teleport
  effect, multicasts it, recomputes/sends score, and clears movement-stop state.
  Updating only coordinates is insufficient.
- `BASE_GetRoute` encodes eight directions as numeric-keypad characters:
  NW=`1`, N=`2`, NE=`3`, W=`4`, E=`6`, SW=`7`, S=`8`, SE=`9`. Each step checks
  height continuity; height `127` and excessive deltas terminate/redirect the
  route. Movement should transmit route/speed/tick state, not periodic absolute
  coordinate corrections.
- The moving `GridMulticast` updates occupancy, broadcasts the action to the old
  and new view rectangles, removes entities leaving view, creates mobs/items
  entering view, and stores `LastTime/LastSpeed/LastX/LastY`. This explains
  stuck corpses and edge-of-visibility ghosts when only one half is implemented.
- The stationary multicast rewrites `CNFMobKill.Exp/Hold` per receiving player.
  A single shared serialized kill packet therefore cannot be reused blindly
  when recipients have different XP/hold state.

**Combat and damage presentation:**

- `ProcessAttackPacket` validates play/death state, attacker/target IDs, client
  tick, anti-double-attack timing (posted threshold 900 ms), position drift,
  wall/quest regions and delayed logout/recall/restart states before damage.
  `SKIPCHECKTICK` is server-internal and must not be trusted from clients.
- The posted parry formula clamps equipment evasion to 0..100, uses DEX in
  tiers (`DEX<=1000` at half weight, 1000..3000 at quarter weight, above 3000
  at eighth weight), subtracts attacker accuracy, adds affect bonuses and clamps
  the final value to 1..650.
- Double/critical flags are sampled from a 1024-entry hit-rate progression.
  Attack speed contributes `((AttackRun>>4)-5)*100`; critical contributes
  `4*Critical`. Preserve the rolling progression instead of rolling two unrelated
  percentages per hit.
- `CheckMobHpDamage` contains the old-client HP-multiplier workaround for special
  pet IDs and clamps ordinary displayed HP (64k in the older branch). Do not let
  this presentation compatibility layer replace authoritative wide HP.
- Client 7.71 decompilation maps floating-number types 3/5 to normal/critical
  physical textures and types 7/8 to normal/critical magic textures (texture
  sets 143/144 for magic). The numeric type, not the server damage formula,
  selects the color.

**Progression, party and summons:**

- `CheckGetLevel` uses the correct Mortal/Arch or Celestial XP table, exposes
  quarter-level segments, and on level-up recalculates score points, HP/MP,
  skill points, special points and defense before refilling and broadcasting.
  The post contains an obvious `MP = MaxHP` decompilation error; correct it to
  MaxMP rather than copying literally.
- XP is a pipeline: global/zone bonus, evolution/level reduction, quest-region
  modifiers and XP-hold consumption are separate stages. 7.56 and 7.57 tables
  differ substantially, especially Celestial, so choose one target ruleset and
  keep the stages independently testable.
- Party request/accept validates connected state, existing leadership/membership,
  duplicate membership, capacity and roughly ±200 effective levels across
  evolved classes. `SendAddParty` carries an explicit `PartyIndex`; retaining a
  stable leader-owned slot order prevents the client party window from shuffling.
- 7.56 summons are stored in the leader's party list, use `RouteType=5`, inherit
  the leader, and scale attack/defense/HP from INT and Special[2]. The posted
  7.57 `GenerateSummon` body still uses high mob IDs in `PartyList`, while other
  7.57 material describes a separate summon/MobList. Treat this as a revision
  conflict: follow the complete target source's ownership model consistently
  across AI, visibility, party packets and deletion; never mix the two layouts.
- `0x3EA` carries `PartyLeader` plus `short PartyList[MAX_PARTY]` for a summon-
  party request/display. It is not sufficient to drive summon creature AI.

**Protocol layouts exposed by the archive:**

- Login `0x20D` checks packet size and `CLIVER <= version <= CLIVER+3`, rejects
  password prefixes `@`, `*`, `_`, uppercases account name, moves the connection
  from accept to login state, forwards the request to DBSrv and copies 16 MAC
  bytes only when the packet is large enough.
- `CNFMobKill` differs by family: the 7.56 post uses 16-bit killed/killer IDs and
  32-bit EXP (24 bytes total after its header assumptions); 7.57 uses 32-bit IDs
  and 64-bit EXP (32 bytes). Select by client wire contract, never by struct cast
  convenience.
- `0x374` is an item/world-state update path for gates and obstacles. It validates
  the item-derived gate ID, consumes a matching server-side key, changes height/
  state and multicasts the result. This reinforces that opcode names are not a
  substitute for inspecting the handler.

### Complete decompilation inventory

**Core score/items/combat:** `BASE_GetCurrentScore`, `CMob::GetCurrentScore`,
`BASE_GetSkillDamage`, `BASE_GetMobAbility`, `BASE_GetItemAbility`,
`BASE_GetStaticItemAbility`, `BASE_GetBonusItemAbility`, `BASE_GetItemSanc`,
`BASE_SetItemSanc`, `BASE_CanEquip`, `BASE_GetHpMp`, `GetBonusHp/GetBonusMP`,
`GetAttack`, `GetParryRate`, `BASE_GetDoubleCritical`, `BASE_GetHitPosition`,
`BASE_GetBonusScorePoint`, `BASE_GetBonusSkillPoint`,
`BASE_GetBonusSpecialPoint`, `BASE_GetBonusAcPoint`, `SetItemBonus`,
`SetSpecialItemAdd`, `CompareGod2Item`, `UpdateGodItem`, `UpdateMantle`,
`BASE_GetMandleLev`, `LinkMountHp`, `GetMountBonus`, `CheckMobHpDamage`.

**Networking/packets:** `CPSock::ReadMessage`, `CPSock::AddMessage`, login `0x20D`,
`ProcessAttackPacket 0x367/0x39D/0x39E`, `_MSG_ApplyBonus 0x277`,
`_MSG_UpdateItem 0x374`, `Exec_MSG_ReqRanking 0x39F`, `SendScore`, `SendEtc`,
`SendDamage`, `SendEnvEffect`, `SendWarInfo`, `SendShopList`, `SendUpdateCoin`,
`SendItemCarry`, `SendPutItemCarry`, `SendPushUser`, `GridMulticast` (two forms),
`GetCreateMob`, `GetCreateMobTrade`, `CreateMob`, `CreateItem`, `GetDropItem`,
`PutItem`, `CloseUser`, `CUser::AcceptUser`, `CUser`, `InitMob`.

**Affects/AI/summons:** `SetAffect`, `SetUseItemAffect`, `SetTick`,
`ProcessAffect`, `RegenMob`, `ReqHP/MP`, `ApplyHP/MP`, `BattleProcessor`,
`StandingByProcessor`, `MobProcessor`, `SelectTargetFromEnemyList`,
`BASE_GetRoute`, `SetSegment`, `GenerateMob`, `GenerateSummon` (7.56/7.57+),
`RequestSummon`, `DoReqSummon`, `SummonedMobControl`, `SendSummonParty 0x3EA`.

**World/grid/teleport:** `GetEmptyMobGrid`, `GetEmptyMobGridValid`,
`GetEmptyLowMobGrid`, `GetEmptyItemGrid`, `DoTeleport`, `BASE_InitializeMobname`,
`BASE_ReadItemListFile`, `GetMobID(const char*)`, `Draw/ReloadDraw`,
`MainWndProc`, `TrainingCamp`.

**Party/guild/war:** `Exec_MSG_REQParty`, `AcceptParty`, `CreateGuild`,
`CreateSubGuild`, clan/mantle helpers, `SetGuildFame`, `GuildZoneReport`,
`GuildWarSummon`, `ProcessGuildRaidEnd`, `DoWar`, `DoAlly`, `SetCityTower`,
`FinishCastleWar`, `SetTownTax`, `GetTownTax`, `DecideWinner`, `ClearCrown`,
`kingdomDragon`, `DoRanking`, `ProcessRanking`.

**Progression/quests/events:** `CMob::CheckGetLevel`, `ApplyExpBonus`,
`ApplyExpReduction`, `ApplyExpHold`, `CheckLevelBlock`, `CreateHardCoreCharacter`,
`ProcessHardCoreDeath`, `CreateSubGod`, `SetSubClass`, `CQuest::InitCQuest`,
`CQuest::ReadCQuestFile`, `CQuest::ProcessNpcClicFunction`, Ice Queen,
Khepra, Nightmare, SecretRoom, RuneQuestZone, GoldenDragon, King, Shaman,
Armia Dragon, Quiz/StartQuiz/SendRandomQuiz, ranking and hardcore PK drop.

**UseItem/crafting catalog:** extraction crystals 3021..3026, Mythril bars
3027..3031, birth accelerator 3438, exile orders 3479/5602, warrior seal 4146,
mysterious stone 4148, fortune cube 4905, ideal stone 5338, skill books
5500..5547, volatile 15 rations, 16 amagos, 19 fireworks, 20 SecretRoom,
21..30 / 131..140 / 161..170 families, 173..175 Nightmare and 190 Replation;
`GetMatchCombine` and `GetMatchCombineAylin` also have dedicated threads.

### Research warnings learned from the archive

- Several posted decompilations are explicitly marked “review” or later corrected.
  `BASE_GetCurrentScore`, empty-grid loops and mixed 7.56/7.57 work are common
  examples. Compare with local 7.59 source and live behavior before porting.
- Hidden tags have historically stripped operators/characters from code during
  forum migrations. Even authenticated code should be treated as a lead, not a
  byte-perfect source file.
- Never copy public absolute addresses (for example the 7.57 MobList address)
  into 7.54 hooks. Only semantics are portable.
- Download threads frequently contain dead links or unreviewed binaries. Prefer
  source repositories; validate hashes and run unknown tools isolated.

## 9. WYD-Go emulator findings (confirmed in code and client tests)

Use `$wyd-go-feature` for the complete implementation workflow and detailed
contracts. The following facts are portable knowledge learned while matching the
7.48 client:

- Craft windows are not one uniform NPC flow. Aylin (head 55) and Agatha (56)
  open anywhere; Tiny (68) has a global fallback. Lindy (67), Compositor (54),
  Odin (67 + Merchant 8) and the correct Ehre mode (68) use map-chunk checks in
  `MouseClick_MixNPC`.
- A locally-opened craft window may send no `0x28B`. The server must resolve the
  expected visible/nearby artisan when the combine opcode arrives instead of
  requiring a prior click-owned `CraftNPC` value.
- Native craft feedback is two packets in order: `SendClientMessage`, then
  `_MSG_CombineComplete` (`0x3A7`) with 0 invalid, 1 success, 2 failure. The
  signal updates the panel but does not replace textual feedback.
- The W2PP wording is “Processing complete” on success and “Combine failed” on
  failure; Micronics uses equivalent “Success in composition” / “There was a
  failure in composition” messages. WYD-Go uses ASCII Portuguese to avoid code
  page corruption in the legacy client.
- `Merchant != 0` is a protocol-level NPC function and must be a hard noncombat
  invariant. Validate this not only during target acquisition but again in the
  active attack ticker, clearing stale targets and removing the entity from the
  active-mob index.
- Spatial lookup is part of correctness as well as performance: NPC interaction,
  craft fallback, wake/sleep and visibility should query local cells rather than
  scan thousands of generated mobs.
- A regression fix is incomplete without a test for the original transition
  (for example, craft opened without 0x28B or a merchant already present in the
  active AI map). Run tests, vet, server build and diff validation together.

## Provenance (citations only — do not re-fetch to *use* the facts above)

WebCheats WYD: `/forum/135-`. Subforums by age (low id = older): Downloads `136`, Dúvidas `137`,
Tutoriais `138` (back to Jun 2008), Bate-Papo `139`, Servidores Privados `215`, Desenvolvimento `362`
(2017+), Descompilação `732`. Sort a subforum oldest-first with
`?sortby=start_date&sortdirection=asc`. For a *new* gap not covered above, use the `wyd-forum-scout`
agent (it won't log in) and then **write the newly-learned fact back into this file**.

High-value public provenance from the 2026-07-15 sweep:

- Packet hello/login framing: `/topic/2250752-packets-do-wyd/`
- Packet sniffer: `/topic/2307132-wydstudio_packetsniffer/`
- W2PP packet tracing: `/topic/2307499-w2pp-dump-de-packets-para-analise/`
- Correct elemental order: `/topic/2283810-erro-currentscore-w2pp/`
- Attack/MobList correction: `/topic/2291386-descompilacao-processattackpacket-0x367-0x39d-0x39e-tmsrv-756-tmsrv-757/`
- Out-of-party summon dependency list: `/topic/2291269-descompilacao-sendsummonparty-tmsrv-757-packet-0x3ea/`
- NPCGener_new behavior: `/topic/2295497-toolsource-npc-gener-for-wyd/`
- Grid loss diagnosis: `/topic/2256811-systemerrgridmulticast-mobidxpos/`
- Buff lifecycle architecture: `/topic/2301224-controle-de-tempobau-xp-buffs-no-geral-itens-trajes-e-etc/`
- Volatile dispatch by item ID: `/topic/2265783-como-achar-volatile-livre/`
- ItemList/SkillData record families: `/topic/2307552-wydconverter-itemlistskilldata-todas-as-versoes/`
