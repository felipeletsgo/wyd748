# Server messages by channel language

Status: activated in release `20261031T201414Z` on 2026-10-31. Both language
catalogs and live admission responses were verified. In-game acceptance is pending.

Classification: `MODERNIZACAO_COMPATIVEL`. The server changes only the text
that it puts in existing packets. Packet types, sizes, offsets and the client
are unchanged.

## Rule

- Channel 1 sends every server message in Brazilian Portuguese.
- Channel 2 sends every server message in English.
- Logs, diagnostics, identifiers and documentation stay in English.
- Player-authored text (chat, whispers, names, staff notices) is not changed.

The language belongs to the channel, not to the client language or the
player's citizenship. `lang.txt` still selects the client interface text.

## Message files

The server reads `wydgo748/data/content/language/en.txt` and `pt.txt` at
startup. A missing or invalid file stops the server, as with other data files.

The format follows the native server message table. The 7.59 `TMSRVIN.exe`
loads `Language_wo.txt` at startup (`TMSRVIN.c` near line 73872). W2PP loads
`Language.txt` with `BASE_InitializeMessage` and names the rows in
`TMSrv/Language.h` (secondary evidence). The native text file is not in
`references/`, so its text is not imported.

```text
# comment
<key><TAB><text>
```

- `_NN_`, `_DN_` and `_SN_` keys reuse the native message names from
  `Language.h` where the message has the same meaning.
- `_WG_` keys are WYD-Go messages without a native equivalent.
- Text uses Go format verbs (`%d`, `%s`, `%q`, `%c`). Every language keeps the
  same verbs in the same order.
- Files are UTF-8. The loader converts text to Windows-1252, the code page the
  7.48 client draws. Text that Windows-1252 cannot encode is rejected.
- A row holds at most 127 bytes. Portuguese rows must fit the 94-byte top
  notice and whisper field; a test checks this.
- English is complete. A key absent from `pt.txt` uses the English text.

To change a message, edit the text in both files and restart the server.
Do not change a key without changing its users in the source.

## Configuration

| Key | Default | Meaning |
| --- | --- | --- |
| `language_dir` | `data/content/language` | Directory of `en.txt` and `pt.txt` |
| `channel_language` | `pt` | Language of channel 1 (`en` or `pt`) |
| `channel2_language` | `en` | Language of channel 2 (`en` or `pt`) |

The defaults are in `internal/data/server_config.go`. Profiles need these keys
only to override them. The VPS release inventory requires both files.

## Source contract

- `w.msg(key, args...)` formats a key in the world's language.
- `w.panel(s, key, args...)` sends that text as the top notice.
- `w.tr(text)` localizes English text that reaches a send point through a
  variable: rejections, error text and helper results. It matches a fixed
  English row exactly, or a formatted English row by pattern, and fills the
  values into the Portuguese row. Other text is unchanged.
- `w.msgFor(p, ...)` and `w.trFor(p, ...)` use the language of the
  recipient's channel. Every notice sent to `<player>.Session` uses them, and
  so do guild notices and staff maintenance notices. A guild action, a staff
  command or a shared persistence result can start in one channel and reach a
  player in the other.
- A world built without `WithLanguage` sends English, so package tests keep
  their English assertions.

Most panel messages are keys in the source. Shared helpers that do not own a
`World` still build English text; the send point translates it through the
catalog. Their English text is in `en.txt` too.

## Content data text

Content files keep their canonical English text. The catalog holds the
translation of each text, and the send point translates it:

- `quests.json`: `messages` (offer, success, denied) and NPC `dialogue`.
- `content/boss/*.lua`: `spawn_message`, `death_message` and the `message` of
  skills, summons and phases.
- `instances.json`: instance and stage names, and Cube/OX questions.

A test reads these files and fails when a text has no catalog row. After you
change content text, add or update its row in `en.txt` and `pt.txt`.
A text without a row is sent in English.

## Indexed client rows

The WYD-Go rows 440–495 of the client `UI/strdef.bin` are imported unchanged
into the catalog. The server now sends them as top-notice text in the channel
language (`projectMessage`), not as `0x105`/`0x106` indexes. Cube and OX
questions use version 1 of `0x7F12` with server text; the server no longer
sends version 2 `TextIndex`. Native rows below 440, such as the party
invitation `-938`, still use the client index: the client owns their text and
some of them start client behavior.

## Final audit — 2026-10-31

The audit found and translated the last English player text: a mount save
failure, 15 instance and Nightmare configuration errors, the `/citywar` status
lines, the character information line (guild role, alliance, citizenship) and
refinement roll results (`Success`/`Fail`/`Penalty`).

## Not covered by this change

- NPC, mount and summon display names stay as configured.
- Item names inserted into a sentence come from the server item catalog in
  English. The PT client resources hold Portuguese item names; importing them
  is a separate decision.
- Whisper sender labels such as `[WAR]`, `[Colosseum]` and `[Staff]` stay as
  they are.
- Operation labels inserted into a sentence, such as
  `%s is unavailable. Please try again later.`, stay in English inside the
  Portuguese sentence.
- Native client message rows follow the client language.

## Validation

- `internal/lang`: production catalog completeness, 94-byte Portuguese rows,
  native names, formatting, fallback, exact and pattern translation, invalid
  files and language codes.
- `internal/game/messages_test.go`: every key in game source exists in
  `en.txt`; a Portuguese world localizes keyed and built text; a staff
  maintenance notice follows the channel language.
- `TestContentTextHasCatalogTranslations`: every quest, boss, instance and
  question text has a catalog row.
- `TestProjectMessageRowsMatchTheClientLocaleRows`: the imported rows match
  the EN/PT client `strdef.bin` rows byte for byte.
- `TestCrossChannelGuildPromotionUsesTheRecipientLanguage`: a channel 1
  leader promotes a channel 2 member; the member reads English and the
  leader reads Portuguese. Before the correction the member read Portuguese.
- `TestKeyedMessageCallsPassEveryFormatValue`: each keyed call passes one
  value per format verb of its message.
- `TestEverySourceSentenceHasACatalogRow`: every capitalized English sentence
  in game source, including error text, has a catalog row. Go error strings
  for logs stay lowercase without final punctuation, so they are not matched.
- Existing game tests pass with English worlds. Tests that expected `0x105`
  indexes for WYD-Go rows now expect the top-notice text.

`CLIENT-TESTED` is pending: check accented text in the top notice, chat
whispers and NPC speech in the real client on both channels.
