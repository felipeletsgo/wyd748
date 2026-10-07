"""Per-skill visual effects: native 7.48 client vs TMProject.

Read-only. For each visual dispatch, the branches on the skill index and the
objects allocated inside each branch are compared:

  attack  native FUN_00489a3e, key *(short *)(param_1 + 0x18) (MSG_Attack.SkillIndex)
          TMProject OnPacketAttack and its helpers, key pAttack->SkillIndex
  event   native FUN_00506f9d, key *(short *)(param_1 + 0x4d0) (deferred effect)
          TMProject TMHuman::PlayEffectEvent, key m_stEffectEvent.sEffectIndex

Native allocations are `operator_new(SIZE)`; TMProject allocations are
`new Class(...)`, sized by the compiler. An allocation belongs to the innermost
branch whose condition compares the key with constants. Equal size multisets
per skill are a strong sign of the same effects; a difference is a lead.

Usage:
  python .agents/research/effects-matrix.py --tsv <out.tsv> --markdown <out.md>
"""
import argparse
import importlib.util
import re
import subprocess
from collections import Counter, defaultdict
from pathlib import Path
import importlib.util as _ilu
_wt_spec = _ilu.spec_from_file_location('worktree', Path(__file__).with_name('worktree.py'))
worktree = _ilu.module_from_spec(_wt_spec)
_wt_spec.loader.exec_module(worktree)

ROOT = Path(__file__).resolve().parents[2]
CORPUS = ROOT / 'tmproject/build/native-research/decompiled'
CLIENT = ROOT / 'tmproject/TMProject748'
MARKER = '<!-- Decisions below are maintained by hand and kept on regeneration. -->'

spec = importlib.util.spec_from_file_location('opcode_matrix', Path(__file__).with_name('opcode-matrix.py'))
opcode_matrix = importlib.util.module_from_spec(spec)
spec.loader.exec_module(opcode_matrix)

DISPATCHES = [
    {'name': 'attack', 'native': 'FUN_00489a3e', 'native_key': r'\*\(short \*\)\(param_1 \+ 0x18\)',
     'client_file': 'internal/app/scenes/TMFieldSceneCombatPackets.cpp',
     'client_methods': ['int TMFieldScene::OnPacketAttack(', 'TMFieldScene::DispatchAttackSkillVisuals(',
                        'TMFieldScene::ApplyTargetedAttackHits(', 'TMFieldScene::ApplyGroundAttackHits('],
     'client_key': r'pAttack->SkillIndex'},
    {'name': 'event', 'native': 'FUN_00506f9d', 'native_key': r'\*\(short \*\)\(param_1 \+ 0x4d0\)',
     'client_file': 'internal/game/entities/TMHumanEffects.cpp',
     'client_methods': ['TMHuman::PlayEffectEvent('],
     'client_key': r'm_stEffectEvent\.sEffectIndex'},
]


def masked(text):
    return re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'',
                  lambda m: ' ' * len(m.group()), text, flags=re.S)


def matching(code, open_index, opening='(', closing=')'):
    depth = 0
    for i in range(open_index, len(code)):
        if code[i] == opening:
            depth += 1
        elif code[i] == closing:
            depth -= 1
            if depth == 0:
                return i
    return len(code) - 1


def branches(text, key):
    """(start, end, skills) for every if-branch whose condition tests key == N."""
    code = masked(text)
    result = []
    for match in re.finditer(r'\bif\s*\(', code):
        open_paren = match.end() - 1
        close_paren = matching(code, open_paren)
        condition = text[open_paren:close_paren + 1]
        skills = {int(v, 0) for v in re.findall(key + r'\s*==\s*(0x[0-9a-fA-F]+|\d+)', condition)}
        if not skills:
            continue
        rest = close_paren + 1
        while rest < len(code) and code[rest].isspace():
            rest += 1
        end = matching(code, rest, '{', '}') if rest < len(code) and code[rest] == '{' else code.find(';', rest)
        result.append((rest, end, skills))
    return result


def attribute(text, key, allocation):
    """Skill -> list of allocation labels, using the innermost enclosing branch."""
    spans = branches(text, key)
    per_skill = defaultdict(list)
    for match in re.finditer(allocation, masked(text)):
        position = match.start()
        enclosing = [s for s in spans if s[0] <= position <= s[1]]
        if not enclosing:
            continue
        start, end, skills = max(enclosing, key=lambda s: s[0])
        for skill in skills:
            per_skill[skill].append(text[match.start():match.end()])
    covered = set().union(*(s[2] for s in spans)) if spans else set()
    return per_skill, covered


def client_text(dispatch):
    source = (CLIENT / dispatch['client_file']).read_text(encoding='utf-8-sig', errors='replace')
    parts = []
    for signature in dispatch['client_methods']:
        start = source.find(signature)
        if start < 0:
            continue
        line = source.rfind('\n', 0, start) + 1
        opening = source.index('{', start)
        parts.append(source[line:matching(masked(source), opening, '{', '}') + 1])
    return '\n'.join(parts)


def class_headers(names):
    files = worktree.relative('tmproject/TMProject748/internal', ('.h',), ROOT)
    headers = set()
    for rel in files:
        if rel.endswith('.h'):
            text = (ROOT / rel).read_text(encoding='utf-8-sig', errors='replace')
            if any(re.search(r'\bclass\s+' + re.escape(n) + r'\b[^;]*\{', text) for n in names):
                headers.add(rel.split('/')[-1])
    return sorted(headers)


def build():
    rows = []
    for dispatch in DISPATCHES:
        native_text = (CORPUS / f"{dispatch['native']}.c").read_text(encoding='utf-8', errors='replace')
        native, native_covered = attribute(native_text, dispatch['native_key'], r'operator_new\((0x[0-9a-f]+|\d+)\)')
        client, client_covered = attribute(client_text(dispatch), dispatch['client_key'], r'\bnew\s+(\w+)\s*\(')
        classes = {re.match(r'new\s+(\w+)', label).group(1) for labels in client.values() for label in labels}
        sizes = opcode_matrix.compiled_sizes(classes, tuple(class_headers(classes)))
        native_of = lambda skill: [int(re.search(r'\((.*)\)', l).group(1), 0) for l in native.get(skill, [])]
        classes_of = lambda skill: [re.match(r'new\s+(\w+)', l).group(1) for l in client.get(skill, [])]
        # Modern classes may have grown (TMEffectBillBoard is 388 bytes, native
        # 0x180): learn each class's native size from single-allocation pairs.
        votes = defaultdict(Counter)
        for skill in native_covered & client_covered:
            if len(native_of(skill)) == 1 and len(set(classes_of(skill))) == 1:
                votes[classes_of(skill)[0]][native_of(skill)[0]] += 1
        native_size_of = {c: v.most_common(1)[0][0] for c, v in votes.items()}
        for skill in sorted(native_covered | client_covered):
            native_sizes = Counter(native_of(skill))
            client_classes = classes_of(skill)
            client_sizes = Counter(native_size_of.get(c, sizes.get(c)) for c in client_classes)
            if skill not in client_covered:
                gap = 'missing in TMProject'
            elif skill not in native_covered:
                gap = 'TMProject only'
            elif native_sizes == client_sizes:
                gap = ''
            elif None in client_sizes:
                gap = 'unsized class'
            else:
                gap = 'different allocations'
            rows.append({'dispatch': dispatch['name'], 'skill': skill,
                         'native_sizes': ' '.join(f'0x{s:x}x{n}' for s, n in sorted(native_sizes.items())),
                         'client_classes': ' '.join(f'{c}({sizes.get(c, "?")})' for c in client_classes),
                         'gap': gap})
    return rows


def write(rows, tsv, markdown):
    columns = list(rows[0].keys())
    Path(tsv).write_text('\n'.join(['\t'.join(columns)] + ['\t'.join(str(r[c]) for c in columns) for r in rows]) + '\n',
                         encoding='utf-8')
    counts = Counter((r['dispatch'], r['gap'] or 'same allocations') for r in rows)
    out = ['# Skill visual effects matrix', '',
           'Generated by `.agents/research/effects-matrix.py`; do not edit the tables by hand. '
           'Native sizes are `operator_new` arguments (`0xSIZExCOUNT`); TMProject classes show their compiled size.', '',
           '| Dispatch | Result | Skills |', '| --- | --- | --- |']
    out += [f'| {d} | {g} | {n} |' for (d, g), n in sorted(counts.items())]
    out += ['', '| Dispatch | Skill | Native allocations | TMProject allocations | Gap |', '| --- | --- | --- | --- | --- |']
    out += [f"| {r['dispatch']} | {r['skill']} | {r['native_sizes'] or '-'} | {r['client_classes'] or '-'} | {r['gap'] or '-'} |"
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
    rows = build()
    counts = write(rows, args.tsv, args.markdown)
    print(f'{len(rows)} skill rows; ' + ', '.join(f'{d}/{g}={n}' for (d, g), n in sorted(counts.items())))


if __name__ == '__main__':
    main()
