"""UI control matrix: 7.48 resources, native bindings and TMProject bindings.

Read-only. Sources:
  resource  every RC file `tmproject/client748/UI/*.bin` walked with the record
            sizes the client compiles (BinPanel, BinButton, ...; files that store
            captions inline use LegacyBinButton/LegacyBinText, the same rule as
            WYD748_IsLegacyRCFile); one row per control ID and file
  native    IDs passed to the native FindControl: virtual slot 0x48 of the
            control container at scene + 0x28 (decompiled corpus)
  client    IDs passed to FindControl (or a local find*/Find* helper) in
            tmproject/TMProject748, constants
            resolved, both arms of `cond ? A : B`
  aliases   the WYD748_TranslateControlID table (internal/core/WYD748Assets.cpp):
            FindControl resolves a newer ID to its native control, so an aliased
            ID is present whenever its native control is

Gaps per control ID:
  N  bound by native code, never bound by the TMProject (missing feature)
  X  bound by the TMProject but absent from every 7.48 resource (null at run time)
  T  bound by the TMProject only (present in a resource, unused by native code)

Usage:
  python .agents/research/ui-matrix.py --tsv <out.tsv> --markdown <out.md>
"""
import argparse
import importlib.util
import re
import struct
import subprocess
from collections import defaultdict
from pathlib import Path
import importlib.util as _ilu
_wt_spec = _ilu.spec_from_file_location('worktree', Path(__file__).with_name('worktree.py'))
worktree = _ilu.module_from_spec(_wt_spec)
_wt_spec.loader.exec_module(worktree)

ROOT = Path(__file__).resolve().parents[2]
RUNTIME_UI = ROOT / 'tmproject/client748/UI'
CORPUS = ROOT / 'tmproject/build/native-research/decompiled'
MARKER = '<!-- Decisions below are maintained by hand and kept on regeneration. -->'

spec = importlib.util.spec_from_file_location('opcode_matrix', Path(__file__).with_name('opcode-matrix.py'))
opcode_matrix = importlib.util.module_from_spec(spec)
spec.loader.exec_module(opcode_matrix)

# CONTROL_TYPE values serialized in the RC stream -> record structs.
MODERN = {1: 'BinPanel', 2: 'BinButton', 3: 'BinCheckBox', 6: 'BinListBox', 10: 'BinProgress',
          12: 'BinText', 13: 'BinEdit', 15: 'Bin3DObj', 16: 'BinGrid'}
LEGACY = {**MODERN, 2: 'LegacyBinButton', 12: 'LegacyBinText'}
TYPE_NAMES = {1: 'panel', 2: 'button', 3: 'checkbox', 6: 'listbox', 10: 'progress', 12: 'text',
              13: 'edit', 15: '3dobj', 16: 'grid'}


def walk(data, sizes, table):
    """Records (type, id, parent) if the whole file walks exactly, else None."""
    records, offset = [], 0
    while offset < len(data):
        if offset + 4 > len(data):
            return None
        kind = struct.unpack_from('<i', data, offset)[0]
        name = table.get(kind)
        size = sizes.get(name)
        if not size or offset + 4 + size > len(data):
            return None
        control_id, parent = struct.unpack_from('<ii', data, offset + 4)
        records.append((kind, control_id, parent))
        offset += 4 + size
    return records


def resource_controls():
    sizes = opcode_matrix.compiled_sizes(set(LEGACY.values()) | set(MODERN.values()), ('UIBinary.h',))
    controls = defaultdict(set)
    parsed = {}
    for path in sorted(RUNTIME_UI.glob('*.bin')):
        data = path.read_bytes()
        modern, legacy = walk(data, sizes, MODERN), walk(data, sizes, LEGACY)
        records = legacy if legacy and not modern else modern
        if not records:
            continue
        parsed[path.name] = ('legacy' if records is legacy and legacy and not modern else 'modern', len(records))
        for kind, control_id, parent in records:
            controls[control_id].add(f'{path.stem}:{TYPE_NAMES.get(kind, kind)}')
    return controls, parsed, sizes


def native_bindings(corpus=CORPUS):
    bindings = defaultdict(set)
    number = r'(0x[0-9a-f]+|\d+)'
    for path in sorted(Path(corpus).glob('*.c')):
        text = path.read_text(encoding='utf-8', errors='replace')
        for value in re.findall(r'\*\*\(int \*\*\)\(\w+ \+ 0x28\) \+ 0x48\)\)\(' + number + r'\)', text):
            bindings[int(value, 0)].add(path.stem)
        for alias in set(re.findall(r'(\w+) = \*\(int \*\*\)\(\w+ \+ 0x28\);', text)):
            for value in re.findall(r'\(\*\*\(code \*\*\)\(\*' + re.escape(alias) + r' \+ 0x48\)\)\(' + number + r'\)', text):
                bindings[int(value, 0)].add(path.stem)
    return bindings


def client_constants():
    constants = {}
    files = worktree.relative('tmproject/TMProject748', None, ROOT)
    for rel in files:
        if rel.endswith('.h') and '/Dependencies/' not in rel:
            for name, value in re.findall(r'constexpr\s+(?:auto|int|unsigned int)\s+(\w+)\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*;',
                                          (ROOT / rel).read_text(encoding='utf-8-sig', errors='replace')):
                constants[name] = int(value, 0)
    return constants, files


def client_bindings():
    constants, files = client_constants()
    bindings = defaultdict(set)
    for rel in files:
        if not rel.endswith('.cpp') or '/Dependencies/' in rel or '/tests/' in rel:
            continue
        text = re.sub(r'//[^\n]*|/\*.*?\*/', ' ', (ROOT / rel).read_text(encoding='utf-8-sig', errors='replace'), flags=re.S)
        for argument in re.findall(r'\b(?:FindControl|[fF]ind[A-Z]\w*)\(\s*([^()]*?(?:\([^()]*\)[^()]*?)*)\s*\)', text):
            for token in re.findall(r'\b(0x[0-9A-Fa-f]+|\d{2,}|[A-Za-z_]\w*)(?:[uU]\b|\b)', argument):
                value = int(token, 0) if re.match(r'0x|\d', token) else constants.get(token)
                if value is not None:
                    bindings[value].add(rel.split('/')[-1])
    return bindings


def control_aliases():
    """Newer ID -> native ID from the WYD748_TranslateControlID table."""
    text = (ROOT / 'tmproject/TMProject748/internal/core/WYD748Assets.cpp').read_text(encoding='utf-8-sig')
    table = text[text.index('ControlIDTranslation translations[]'):]
    table = table[:table.index('};')]
    return {int(modern): int(native) for native, modern in re.findall(r'\{\s*(\d+)\s*,\s*(\d+)\s*\}', table)}


def build():
    resources, parsed, sizes = resource_controls()
    native, client = native_bindings(), client_bindings()
    aliases = control_aliases()
    rows = []
    for control_id in sorted(set(native) | set(client)):
        target = aliases.get(control_id)
        present = control_id in resources or target in resources
        gaps = ''
        if control_id in native and control_id not in client:
            gaps += 'N'
        if control_id in client and not present:
            gaps += 'X'
        if control_id in client and control_id not in native and present:
            gaps += 'T'
        located = ' '.join(sorted(resources.get(control_id, ())))
        if not located and target in resources:
            located = f'alias of {target}: ' + ' '.join(sorted(resources[target]))
        rows.append({'id': control_id, 'hex': f'0x{control_id:X}',
                     'resources': located[:120] or '-',
                     'native': ' '.join(sorted(native.get(control_id, ())))[:80] or '-',
                     'client': ' '.join(sorted(client.get(control_id, ())))[:80] or '-', 'gaps': gaps})
    return rows, parsed, resources


def write(rows, parsed, resources, tsv, markdown):
    columns = list(rows[0].keys())
    Path(tsv).write_text('\n'.join(['\t'.join(columns)] + ['\t'.join(str(r[c]) for c in columns) for r in rows]) + '\n',
                         encoding='utf-8')
    counts = defaultdict(int)
    for r in rows:
        for gap in r['gaps']:
            counts[gap] += 1
    out = ['# UI control compatibility matrix', '',
           'Generated by `.agents/research/ui-matrix.py`; do not edit the tables by hand.', '',
           '| Gap | Meaning | IDs |', '| --- | --- | --- |',
           f"| N | bound by native code, never bound by the TMProject | {counts['N']} |",
           f"| X | bound by the TMProject, absent from every 7.48 resource | {counts['X']} |",
           f"| T | bound by the TMProject only (in a resource, unused natively) | {counts['T']} |", '',
           '## RC files walked', '', '| File | Layout | Controls |', '| --- | --- | --- |']
    out += [f'| `UI/{name}` | {layout} | {count} |' for name, (layout, count) in sorted(parsed.items())]
    out += ['', '## Controls bound by either client', '',
            '| ID | Hex | 7.48 resources | Native functions | TMProject files | Gaps |',
            '| --- | --- | --- | --- | --- | --- |']
    out += [f"| {r['id']} | {r['hex']} | {r['resources']} | {r['native']} | {r['client']} | {r['gaps'] or '-'} |"
            for r in rows]
    kept = MARKER + '\n'
    if Path(markdown).is_file():
        previous = Path(markdown).read_text(encoding='utf-8')
        if MARKER in previous:
            kept = previous[previous.index(MARKER):]
    Path(markdown).write_text('\n'.join(out) + '\n\n' + kept, encoding='utf-8')
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--tsv', required=True)
    parser.add_argument('--markdown', required=True)
    args = parser.parse_args()
    rows, parsed, resources = build()
    counts = write(rows, parsed, resources, args.tsv, args.markdown)
    print(f'{len(rows)} bound IDs; {len(parsed)} RC files, {len(resources)} resource IDs; gaps: '
          + ', '.join(f'{k}={counts[k]}' for k in 'NXT'))


if __name__ == '__main__':
    main()
