"""Build text-only language overlays without importing legacy gameplay data.

Run with Python -B. The historical client is an explicit read-only input.
Generated runtime resources use the client's Windows-1252 encoding.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

ROOT = Path(__file__).resolve().parents[2]
RUNTIME = ROOT / "tmproject/client748"
POLICY = Path(__file__).with_name("client_locale_overrides.json")
PROTECTED_ITEMS = {4151, 4153, 4154, 4156}
TEXT_FILES = [
    "UI/chardescbeast.txt", "UI/chardescfoema.txt", "UI/chardeschunter.txt", "UI/chardesctrans.txt",
    "UI/command.txt", "UI/descskillm.txt", "UI/etc.txt", "UI/hellStoredesc.txt",
    "UI/interface.txt", "UI/interface1.txt", "UI/interface2.txt", "UI/interface3.txt",
    "UI/mix3desc.txt", "UI/mix4desc.txt", "UI/mix6desc.txt", "UI/PotalPos.txt",
    "UI/QuestSubjects.txt", "UI/QuestSubjects2.txt", "UI/QuestSubjects3.txt",
    "UI/QuestContents.txt", "UI/QuestContents2.txt", "UI/QuestContents3.txt",
    "UI/EffectString.txt", "UI/EffectSubString.txt", "UI/UIString.txt", "UI/strdef.bin",
]


def checksum(data):
    result = 0
    for i, unsigned in enumerate(data):
        value = unsigned if unsigned < 128 else unsigned - 256
        mod = i % 7
        if mod == 0: result += int(value / 2)
        if mod == 1: result += value ^ 255
        if mod == 2: result += 3 * value
        if mod == 3: result += 2 * value
        if mod == 4: result -= int(value / 7)
        if mod == 5: result -= value
        else: result += int(value / 3)
    return result & 0xffffffff


def message_overrides(data, overrides, language):
    """Apply explicit project messages without changing existing row indices."""
    if len(data) < 4 or (len(data) - 4) % 128:
        raise ValueError('Unexpected message table layout')
    payload = bytearray(data[:-4])
    if struct.unpack('<I', data[-4:])[0] != checksum(payload):
        raise ValueError('Invalid message table checksum')
    for key, translations in sorted(overrides.items(), key=lambda pair: int(pair[0])):
        index = int(key)
        text = translations.get(language, translations['EN']).encode('cp1252')
        if not 0 <= index < 2000 or len(text) >= 128 or any(c in text for c in (b'\0', b'\r', b'\n', b'\t')):
            raise ValueError(f'Invalid message override: {index}/{language}')
        if format_tokens(translations['EN'].encode('cp1252')) != format_tokens(text):
            raise ValueError(f'Incompatible message placeholders: {index}/{language}')
        if index * 128 > len(payload):
            raise ValueError(f'Message overrides must append contiguous rows: {index}')
        encoded = bytes(value ^ 0x5a for value in text.ljust(128, b'\0'))
        payload[index * 128:(index + 1) * 128] = encoded
    return bytes(payload) + struct.pack('<I', checksum(payload))


def item_rows(data):
    if len(data) != 6500 * 140 + 4:
        raise ValueError("Unexpected ItemList layout")
    decoded = bytes(x ^ 0x5a for x in data[:-4])
    return [decoded[i:i + 140] for i in range(0, len(decoded), 140)]


def identity(row):
    return row[64:70] + row[132:140]


def names(data):
    if len(data) % 68:
        raise ValueError("Unexpected Itemname layout")
    result = {}
    for offset in range(0, len(data), 68):
        index, = struct.unpack_from("<i", data, offset)
        if index == -1: continue
        if index in result or not 0 <= index < 6500:
            raise ValueError("Invalid or duplicate item name ID")
        raw = data[offset + 4:offset + 68]
        decoded = bytes((x - j) % 256 if j < 62 else x for j, x in enumerate(raw))
        result[index] = decoded.split(b"\0", 1)[0]
    return result


def encode_names(records):
    result = bytearray()
    for index, text in sorted(records.items()):
        if len(text) >= 62 or b"\0" in text:
            raise ValueError(f"Invalid item name {index}")
        raw = text.ljust(64, b"\0")
        result.extend(struct.pack("<i", index))
        result.extend((x + j) % 256 if j < 62 else x for j, x in enumerate(raw))
    return bytes(result)


def help_records(data):
    records = {}; key = None; rows = []
    def publish():
        if key is None: return
        # Historical files contain repeated IDs; the native loader keeps the last.
        normalized = [row[:8] + b" " + row[8:].lstrip(b" ") for row in rows[:9]]
        records[key] = normalized + [b"FFFFFFFF "] * (9 - len(normalized))
    for line in data.splitlines():
        if line.strip().isdigit():
            publish(); key = int(line); rows = []
        elif re.match(rb"^[0-9A-Fa-f]{8}", line):
            rows.append(line)
        elif line.strip():
            raise ValueError(f"Invalid help row for {key}")
    publish()
    return records


def encode_help(records):
    return b"\r\n".join(line for key, rows in records.items() for line in [str(key).encode(), *rows]) + b"\r\n"


def format_tokens(raw):
    return re.findall(rb"%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLIw])?[diuoxXfFeEgGaAcCsSpn%])", raw)


def inline_captions(data):
    sizes = {1: 40, 2: 164, 3: 32, 6: 52, 10: 48, 12: 176, 13: 184, 15: 28, 16: 40}
    result = {}; offset = 0
    while offset < len(data):
        if len(data) - offset < 4: return {}
        kind, = struct.unpack_from('<i', data, offset); offset += 4
        size = sizes.get(kind, 0)
        if not size or offset + size > len(data): return {}
        if kind in (2, 12):
            control, parent, texture = struct.unpack_from('<iii', data, offset)
            start = offset + (36 if kind == 2 else 48)
            text = data[start:start + 128].split(b'\0', 1)[0]
            if b'\n' in text or b'\r' in text or b'\t' in text: return {}
            result[f'{kind}:{control}:{parent}:{texture}'] = text
        offset += size
    return result


def build(runtime, source, overrides):
    output = {}; gaps = []; imported = []
    for relative, expected in overrides['canonical_text_sha256'].items():
        if hashlib.sha256((runtime / relative).read_bytes()).hexdigest() != expected:
            raise ValueError(f'Canonical text changed: {relative}; review translations before regenerating')
    active_data = (runtime / "ItemList.bin").read_bytes()
    if hashlib.sha256(active_data).hexdigest() != overrides['canonical_itemlist_sha256']:
        raise ValueError('Canonical item catalog changed; review translations before regenerating')
    current_name_data = (runtime / "Itemname.bin").read_bytes()
    if hashlib.sha256(current_name_data).hexdigest() != overrides['canonical_itemname_sha256']:
        raise ValueError('Canonical item names changed; review translations before regenerating')
    active = item_rows(active_data); old = item_rows((source / "ItemList.bin").read_bytes())
    current_names = names(current_name_data)
    translated_names = names((source / "Itemname.bin").read_bytes())
    english_names = {i: current_names.get(i, row[:64].split(b"\0", 1)[0]) for i, row in enumerate(active)}
    for key, value in overrides['english_item_names'].items():
        english_names[int(key)] = value.encode('cp1252')
    merged = dict(english_names)
    for i, text in translated_names.items():
        if i not in PROTECTED_ITEMS and identity(active[i]) == identity(old[i]) and text and not text.isdigit():
            merged[i] = text; imported.append(i)
    for key, value in overrides["item_names"].items():
        merged[int(key)] = value.encode("cp1252")
    for language, records in [("EN", english_names), ("PT", merged)]:
        output[f"{language}/Itemname.bin"] = encode_names(records)
        rows = [records[i].ljust(64, b"\0") + row[64:] for i, row in enumerate(active)]
        encoded = bytes(x ^ 0x5a for x in b"".join(rows))
        output[f"{language}/ItemList.bin"] = encoded + struct.pack("<I", checksum(encoded))
    current_help_data = (runtime / "itemhelp.dat").read_bytes()
    if hashlib.sha256(current_help_data).hexdigest() != overrides['canonical_itemhelp_sha256']:
        raise ValueError('Canonical item help changed; review localized descriptions before regenerating')
    current_help = help_records(current_help_data)
    old_help = help_records((source / "itemhelp.dat").read_bytes())
    current_skills = help_records((runtime / "skillhelp.dat").read_bytes())
    for language in ["EN", "PT"]:
        records = dict(current_help)
        if language == "PT":
            for key in records:
                if key in old_help and key != 3022 and (key >= 5000 or identity(active[key]) == identity(old[key])):
                    records[key] = old_help[key]
            for key, rows in overrides["item_help"].items():
                records[int(key)] = [row.encode("cp1252") for row in rows]
        for key in range(4016, 4026):
            grade = chr(ord('A') + (key - 4016) % 5)
            maximum = 6 if key < 4021 else 9
            prose = [text.format(grade=grade, maximum=maximum) for text in overrides['repliction'][language]]
            records[key] = [(('FFFF00FF' if j == 0 else 'FFFF0000' if 4 <= j <= 6 else 'FFFFFFFF') + ' ' + text).encode('cp1252') for j, text in enumerate(prose)]
        output[f"{language}/itemhelp.dat"] = encode_help(records)
        skill_records = current_skills if language == "EN" else {key: old_help[key] for key in current_skills}
        if language == 'PT':
            for key, rows in overrides.get('skill_help', {}).items():
                skill_records[int(key)] = [row.encode('cp1252') for row in rows]
        output[f"{language}/skillhelp.dat"] = encode_help(skill_records)
    for relative in TEXT_FILES:
        base = runtime / relative; historical = source / relative
        if not base.is_file(): continue
        original = base.read_bytes(); translated = original
        if relative in overrides["files"]:
            translated = (overrides["files"][relative].rstrip('\n') + '\n').replace('\n', '\r\n').encode('cp1252')
        elif relative == "UI/strdef.bin":
            old_messages = historical.read_bytes()
            if len(old_messages) > len(original) or (len(old_messages) - 4) % 128:
                raise ValueError("Historical message table layout changed")
            payload = bytearray(original[:-4])
            for offset in range(0, len(old_messages) - 4, 128):
                before = bytes(x ^ 0x5a for x in original[offset:offset + 128]).split(b'\0', 1)[0]
                after = bytes(x ^ 0x5a for x in old_messages[offset:offset + 128]).split(b'\0', 1)[0]
                if format_tokens(before) == format_tokens(after):
                    payload[offset:offset + 128] = old_messages[offset:offset + 128]
                else: gaps.append(f"strdef index {offset // 128}: incompatible format placeholders; English retained")
            translated = bytes(payload) + struct.pack('<I', checksum(payload))
            original = message_overrides(original, overrides.get('messages', {}), 'EN')
            translated = message_overrides(translated, overrides.get('messages', {}), 'PT')
        elif "Quest" in relative:
            # Blocks and subject indices differ across distributions. Remap
            # explicitly by semantic identity, preserving current level keys.
            suffix = re.search(r'(\d*)\.txt$', relative)[1]
            mapping = overrides['quest_mapping'][suffix]
            old_lines = historical.read_bytes().splitlines(); lines = original.splitlines()
            if 'Subjects' in relative:
                for target, origin in enumerate(mapping):
                    if origin is not None:
                        label = old_lines[origin][10:].strip()
                        label = re.sub(rb'\s*-\s*LV\s*\d+\s*$', b'', label)
                        label = re.sub(rb'^LV\s*\d+\s*', b'', label)
                        level = re.search(rb'\bLV\s*(\d+)', lines[target])
                        lines[target] = lines[target][:11] + b'LV ' + level[1] + b' ' + label
                    elif str(target) in overrides.get('quest_subject_labels', {}).get(suffix, {}):
                        label = overrides['quest_subject_labels'][suffix][str(target)].encode('cp1252')
                        level = re.search(rb'\bLV\s*(\d+)', lines[target])
                        lines[target] = lines[target][:11] + b'LV ' + level[1] + b' ' + label
                    else: gaps.append(f'{relative} index {target}: no equivalent historical quest; English retained')
            else:
                for target, origin in enumerate(mapping):
                    if origin is None: continue
                    block = old_lines[origin * 20:(origin + 1) * 20]
                    # Some historical pages have a blank row before their title.
                    if len(block) > 1 and b'<<' not in block[0] and b'<<' in block[1]:
                        block = block[1:] + [b'FFFFFF ']
                    lines[target * 20:(target + 1) * 20] = block + [b'FFFFFF '] * (20 - len(block))
                for target, block in overrides.get('quest_pages', {}).get(suffix, {}).items():
                    if len(block) != 20: raise ValueError('Quest pages must contain exactly 20 rows')
                    start = int(target) * 20
                    lines[start:start + 20] = [row.encode('cp1252') for row in block]
            translated = b'\r\n'.join(lines) + b'\r\n'
        elif historical.is_file():
            translated = historical.read_bytes()
            if relative == 'UI/PotalPos.txt':
                a = original.splitlines(); b = translated.splitlines()
                if len(a) != len(b): raise ValueError('Portal row count differs')
                for i, (x, y) in enumerate(zip(a, b)):
                    if re.findall(rb'\d+', x)[-2:] != re.findall(rb'\d+', y)[-2:]:
                        b[i] = x
                        gaps.append(f'{relative} row {i}: coordinates differ; English retained')
                translated = b'\r\n'.join(b) + b'\r\n'
            if relative == 'UI/hellStoredesc.txt' and len(translated.splitlines()) != len(original.splitlines()):
                raise ValueError('Store page boundaries differ')
        else: gaps.append(f'{relative}: no historical translation; English retained')
        output['EN/' + relative] = original
        output['PT/' + relative] = translated
    english_captions = {}; portuguese_captions = {}
    for path in sorted((runtime / 'UI').glob('*.bin')):
        historical = source / 'UI' / path.name
        if not historical.is_file(): continue
        base = inline_captions(path.read_bytes()); old = inline_captions(historical.read_bytes())
        for key, text in base.items():
            if not text.strip(): continue
            full_key = path.name.lower() + ':' + key
            english_captions[full_key] = text
            if key in old and old[key].strip():
                portuguese_captions[full_key] = old[key]
    for key, value in overrides.get('captions', {}).items():
        encoded = value.encode('cp1252')
        if key not in english_captions or len(encoded) >= 128 or any(x in encoded for x in (b'\r', b'\n', b'\t', b'\0')):
            raise ValueError(f'Invalid caption override: {key}')
        portuguese_captions[key] = encoded
    for language, records in [('EN', english_captions), ('PT', portuguese_captions)]:
        output[f'{language}/UI/captions.txt'] = b'\r\n'.join(key.encode() + b'\t' + value for key, value in sorted(records.items())) + b'\r\n'
    report = {
        'classification': 'EXTENSAO_COORDENADA',
        'source': source.name,
        'canonical_itemlist_sha256': hashlib.sha256(active_data).hexdigest(),
        'historical_itemlist_sha256': hashlib.sha256((source / 'ItemList.bin').read_bytes()).hexdigest(),
        'historical_itemname_sha256': hashlib.sha256((source / 'Itemname.bin').read_bytes()).hexdigest(),
        'translated_item_name_ids': imported,
        'manual_item_name_ids': sorted(map(int, overrides['item_names'])),
        'english_name_override_ids': sorted(map(int, overrides['english_item_names'])),
        'retained_display_name_ids': [i for i in merged if merged[i] and not merged[i].isdigit() and merged[i] == english_names[i]],
        'protected_item_ids': sorted(PROTECTED_ITEMS),
        'portuguese_inline_captions': len(portuguese_captions),
        'gaps': gaps,
        'files': {name: {'sha256': hashlib.sha256(data).hexdigest(), 'size': len(data)} for name, data in output.items()},
    }
    return output, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    output, report = build(RUNTIME, args.source, json.loads(POLICY.read_text(encoding='utf-8')))
    for name, data in output.items():
        path = RUNTIME / name
        if args.check:
            if not path.is_file() or path.read_bytes() != data: raise SystemExit(f'Stale translation resource: {name}')
        else:
            path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(data)
    audit = ROOT / 'build/client-locales/resource-audit.json'
    audit.parent.mkdir(parents=True, exist_ok=True)
    audit.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(f'{len(output)} text resources; {len(report["translated_item_name_ids"])} item names; {len(report["gaps"])} documented fallbacks. Audit: {audit}')


if __name__ == '__main__':
    main()
