"""Client/server data consistency: the tables both sides load must agree.

Read-only. Compares, field by field:
  items   tmproject/client748/ItemList.bin (6500 x 140-byte records, XOR 0x5A,
          then a 4-byte trailer) with wydgo748/data/itemlist.csv
  skills  tmproject/client748/SkillData.csv with wydgo748/data/SkillData.csv

Effect IDs are translated with `#define EF_*` from the client's
internal/ui/ItemEffect.h. A difference is a lead: the authoritative side and the
fix are decided per field.

Usage:
  python .agents/research/data-consistency.py --tsv <out.tsv> --markdown <out.md>
"""
import argparse
import csv
import re
import struct
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CLIENT_ITEMS = ROOT / 'tmproject/client748/ItemList.bin'
SERVER_ITEMS = ROOT / 'wydgo748/data/itemlist.csv'
CLIENT_SKILLS = ROOT / 'tmproject/client748/SkillData.csv'
SERVER_SKILLS = ROOT / 'wydgo748/data/SkillData.csv'
EFFECTS = ROOT / 'tmproject/TMProject748/internal/ui/ItemEffect.h'
SERVER_EFFECTS = ROOT / 'wydgo748/data/ItemEffect.h'
ITEM_COUNT, RECORD = 6500, 140
MARKER = '<!-- Decisions below are maintained by hand and kept on regeneration. -->'


def effect_ids(path=EFFECTS):
    text = Path(path).read_text(encoding='utf-8-sig', errors='replace')
    return {name: int(value) for name, value in re.findall(r'^\s*#define\s+(EF_\w+)\s+(\d+)', text, re.M)}


def client_items():
    data = CLIENT_ITEMS.read_bytes()
    if len(data) != ITEM_COUNT * RECORD + 4:
        raise SystemExit(f'{CLIENT_ITEMS.name}: {len(data)} bytes, expected {ITEM_COUNT * RECORD + 4}')
    items = {}
    for index in range(ITEM_COUNT):
        row = bytes(b ^ 0x5A for b in data[index * RECORD:(index + 1) * RECORD])
        name = row[:64].split(b'\0')[0].decode('cp949', errors='replace')
        mesh, texture, visual, lvl, str_, int_, dex, con = struct.unpack_from('<8h', row, 64)
        effects = {}
        for slot in range(12):
            effect, value = struct.unpack_from('<Hh', row, 80 + slot * 4)
            if effect:
                effects[effect] = value
        # File layout (proven on items 1, 1101 and 2000): price int, then
        # unique, pos, extra and grade as shorts; not the in-memory struct.
        price, unique, pos, extra, grade = struct.unpack_from('<ihhhh', row, 128)
        items[index] = {'name': name, 'mesh': mesh, 'texture': texture, 'req': (lvl, str_, int_, dex, con),
                        'unique': unique, 'price': price, 'pos': pos, 'extra': extra, 'grade': grade,
                        'effects': effects}
    return items


def server_items(ids):
    items, unknown = {}, set()
    with SERVER_ITEMS.open(encoding='utf-8-sig', errors='replace', newline='') as handle:
        for row in csv.reader(handle):
            if not row or not row[0].strip().isdigit():
                continue
            mesh = [int(x) for x in row[2].split('.')]
            req = tuple(int(x) for x in row[3].split('.'))
            effects = {}
            for i in range(9, len(row) - 1, 2):
                name = row[i].strip()
                if not name:
                    continue
                if name.isdigit():
                    effects[int(name)] = int(row[i + 1])   # numeric effect ID, as the server loads it
                    continue
                if name not in ids:
                    unknown.add(name)
                    continue
                effects[ids[name]] = int(row[i + 1])
            items[int(row[0])] = {'name': row[1].strip(), 'mesh': mesh[0], 'texture': mesh[1], 'req': req,
                                  'unique': int(row[4]), 'price': int(row[5]), 'pos': int(row[6]),
                                  'extra': int(row[7]), 'grade': int(row[8]), 'effects': effects}
    return items, unknown


def compare_items():
    # Each side names effects with its own ItemEffect.h; compare the numbers.
    names = {v: k for k, v in effect_ids().items()}
    client, (server, unknown) = client_items(), server_items(effect_ids(SERVER_EFFECTS))
    rows, fields = [], Counter()
    for index in sorted(server):
        s, c = server[index], client.get(index)
        if c is None:
            rows.append((index, s['name'], 'missing in client', '', ''))
            fields['missing in client'] += 1
            continue
        for field in ('mesh', 'texture', 'req', 'unique', 'price', 'pos', 'extra', 'grade'):
            if s[field] != c[field]:
                rows.append((index, s['name'], field, c[field], s[field]))
                fields[field] += 1
        for effect in sorted(set(s['effects']) | set(c['effects'])):
            cv, sv = c['effects'].get(effect), s['effects'].get(effect)
            if cv != sv:
                rows.append((index, s['name'], names.get(effect, f'EF_{effect}'), cv, sv))
                fields['effects'] += 1
    client_only = [i for i, c in client.items() if i not in server and (c['name'] or c['mesh'])]
    return rows, fields, len(server), client_only, unknown


CLIENT_SKILL_BIN = ROOT / 'tmproject/client748/SkillData.bin'
SKILL_FIELDS = ('SkillPoint', 'TargetType', 'ManaSpent', 'Delay', 'Range', 'InstanceType', 'InstanceValue',
                'TickType', 'TickValue', 'AffectType', 'AffectValue', 'AffectTime')
SKILL_TAIL = ('InstanceAttribute', 'TickAttribute', 'Aggressive', 'MaxTarget', 'bParty', 'AffectResist',
              'Passive', 'ForceDamage')


def compare_skill_bin():
    """SkillData.bin (104 x 96 bytes, XOR 0x5A; native FUN_0054f692) vs server CSV.

    The server CSV has no ForceDamage column; it is reported only when the
    client value is not zero."""
    data = CLIENT_SKILL_BIN.read_bytes()
    count, size = 104, 96
    if len(data) != count * size:
        return None
    with SERVER_SKILLS.open(encoding='utf-8-sig', errors='replace', newline='') as handle:
        server = {int(r[0]): r for r in csv.reader(handle) if r and r[0].strip().isdigit()}
    rows = []
    for index in range(count):
        record = bytes(b ^ 0x5A for b in data[index * size:(index + 1) * size])
        head = struct.unpack_from('<12i', record, 0)
        act1, act2 = list(record[48:56]), list(record[56:64])
        tail = struct.unpack_from('<8i', record, 64)
        row = server.get(index)
        if row is None:
            if any(head) or any(tail):
                rows.append((index, '', 'missing on server', '', ''))
            continue
        name = row[22].strip()
        for field, client_value, server_value in zip(SKILL_FIELDS, head, (int(x) for x in row[1:13])):
            if client_value != server_value:
                rows.append((index, name, field, client_value, server_value))
        for field, client_value, server_value in (('Act1', act1, [int(x) for x in row[13].split('.')]),
                                                  ('Act2', act2, [int(x) for x in row[14].split('.')])):
            if client_value != server_value:
                rows.append((index, name, field, client_value, server_value))
        for field, client_value, server_value in zip(SKILL_TAIL[:7], tail[:7], (int(x) for x in row[15:22])):
            if client_value != server_value:
                rows.append((index, name, field, client_value, server_value))
        if tail[7]:
            rows.append((index, name, 'ForceDamage (no server column)', tail[7], ''))
    return rows


CLIENT_NAMES = ROOT / 'tmproject/client748/Itemname.bin'
SERVER_NAMES = ROOT / 'wydgo748/data/Itemname.csv'


def compare_item_names():
    """Itemname.bin (int index + 64-byte name, byte i stored as name[i] + i for
    i < 62; native FUN_004b1d64) vs the server Itemname.csv."""
    data = CLIENT_NAMES.read_bytes()
    client = {}
    for offset in range(0, len(data) - 67, 68):
        index = struct.unpack_from('<i', data, offset)[0]
        raw = bytearray(data[offset + 4:offset + 68])
        for i in range(62):
            raw[i] = (raw[i] - i) & 0xFF
        client[index] = bytes(raw).split(b'\0')[0].decode('cp949', errors='replace')
    with SERVER_NAMES.open(encoding='utf-8-sig', errors='replace', newline='') as handle:
        server = {int(r[0]): r[1].strip() for r in csv.reader(handle) if len(r) > 1 and r[0].strip().isdigit()}
    differing = [(i, client.get(i), server[i]) for i in sorted(server) if client.get(i) != server[i]]
    client_only = [i for i in client if i not in server]
    return len(client), len(server), differing, client_only


def compare_skills():
    def load(path):
        with path.open(encoding='utf-8-sig', errors='replace', newline='') as handle:
            return [row for row in csv.reader(handle)]
    if not CLIENT_SKILLS.is_file() or not SERVER_SKILLS.is_file():
        return None
    client, server = load(CLIENT_SKILLS), load(SERVER_SKILLS)
    differences = []
    for i in range(max(len(client), len(server))):
        c = client[i] if i < len(client) else None
        s = server[i] if i < len(server) else None
        if c != s:
            differences.append((i + 1, c, s))
    return len(client), len(server), differences


def write(items, skills, skill_bin, names, tsv, markdown):
    rows, fields, server_count, client_only, unknown = items
    lines = ['index\tname\tfield\tclient\tserver'] + ['\t'.join(str(x) for x in r) for r in rows]
    Path(tsv).write_text('\n'.join(lines) + '\n', encoding='utf-8')
    out = ['# Client and server data consistency', '',
           'Generated by `.agents/research/data-consistency.py`; do not edit the tables by hand. '
           'Full item differences: `.agents/research/client748/inventory/item-data-differences.tsv`.', '',
           '## Items: `ItemList.bin` (client) vs `itemlist.csv` (server)', '',
           f'Server items compared: {server_count}. Differences by field:', '',
           '| Field | Differences |', '| --- | --- |']
    out += [f'| {field} | {count} |' for field, count in fields.most_common()]
    out += ['', f'Client records with data but no server row: {len(client_only)}.']
    if unknown:
        out += [f'Server effect names unknown to the server `ItemEffect.h`: {", ".join(sorted(unknown))}.']
    out += ['', 'First differences:', '', '| Index | Name | Field | Client | Server |', '| --- | --- | --- | --- | --- |']
    out += [f'| {r[0]} | {r[1]} | {r[2]} | {r[3]} | {r[4]} |' for r in rows[:40]]
    out += ['', '## Skills: `SkillData.bin` (client, loaded by the native client) vs `SkillData.csv` (server)', '']
    if skill_bin is None:
        out.append('`SkillData.bin` does not have the native size (104 x 96 bytes).')
    else:
        kinds = Counter(r[2] for r in skill_bin)
        out.append(f'Differences: {len(skill_bin)} ' + (str(dict(kinds)) if kinds else '') + '.')
        # Skill names stay in the server data; the document lists indexes only.
        out += ['', '| Skill index | Field | Client | Server |', '| --- | --- | --- | --- |']
        out += [f'| {r[0]} | {r[2]} | {r[3]} | {r[4] if r[4] != "" else "-"} |' for r in skill_bin[:60]]
    client_names, server_names, differing, client_only = names
    missing = sum(1 for _, c, _ in differing if c is None)
    out += ['', '## Item names: `Itemname.bin` (client) vs `Itemname.csv` (server)', '',
            f'Client names: {client_names}; server names: {server_names}; server names absent from the client: '
            f'{missing}; names that differ: {len(differing) - missing}; client-only: {len(client_only)}.', '',
            '| Index | Client | Server |', '| --- | --- | --- |']
    out += [f'| {i} | {c if c is not None else "-"} | {s} |' for i, c, s in differing if c is not None][:40]
    out += ['', '## Skills: `SkillData.csv` (client runtime audit copy) vs `SkillData.csv` (server)', '']
    if skills is None:
        out.append('One of the files is missing.')
    else:
        client_rows, server_rows, differences = skills
        out.append(f'Client rows: {client_rows}; server rows: {server_rows}; differing rows: {len(differences)}.')
    kept = MARKER + '\n'
    if Path(markdown).is_file():
        previous = Path(markdown).read_text(encoding='utf-8')
        if MARKER in previous:
            kept = previous[previous.index(MARKER):]
    Path(markdown).write_text('\n'.join(out) + '\n\n' + kept, encoding='utf-8')
    return fields


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--tsv', required=True)
    parser.add_argument('--markdown', required=True)
    args = parser.parse_args()
    items = compare_items()
    skills = compare_skills()
    skill_bin = compare_skill_bin()
    fields = write(items, skills, skill_bin, compare_item_names(), args.tsv, args.markdown)
    print(f'items: {items[2]} compared, {sum(fields.values())} field differences {dict(fields)}; '
          f'client-only records {len(items[3])}; skills: '
          + ('missing' if skills is None else f'{len(skills[2])} differing rows'))


if __name__ == '__main__':
    main()
