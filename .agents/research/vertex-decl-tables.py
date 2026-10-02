"""Convert field-by-field D3DVERTEXELEMENT9 assignments to aggregate tables.

Rewrites each 'D3DVERTEXELEMENT9 Name[N];' declaration that is followed by
assignments of all six fields (Stream, Offset, Type, Method, Usage,
UsageIndex) for every element into one braced initializer at the declaration.
--check compares every element value at a Git revision with the current
initializers and fails on any difference, missing or extra element.

Usage (from the repository root):
  python .agents/research/vertex-decl-tables.py --file <path> [--write]
  python .agents/research/vertex-decl-tables.py --file <path> --check --revision <commit> [--baseline <path>]
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FIELDS = ('Stream', 'Offset', 'Type', 'Method', 'Usage', 'UsageIndex')
DECL = re.compile(r'^(?P<indent>[ \t]*)D3DVERTEXELEMENT9 (?P<name>\w+)\[(?P<count>\d+)\];[ \t]*\n', re.M)
ASSIGN = re.compile(r'^[ \t]*(?P<name>\w+)\[(?P<index>\d+)\]\.(?P<field>\w+) = (?P<value>-?\d+);[ \t]*\n', re.M)
TABLE = re.compile(r'^[ \t]*D3DVERTEXELEMENT9 (?P<name>\w+)\[(?P<count>\d+)\] =\s*\{(?P<body>[\s\S]*?)\n[ \t]*\};', re.M)


def assignments(text):
    """Return {name: [[6 values], ...]} from field-by-field assignments."""
    tables = {}
    for decl in DECL.finditer(text):
        name, count = decl.group('name'), int(decl.group('count'))
        values = {}
        for m in ASSIGN.finditer(text, decl.end()):
            if m.group('name') != name:
                continue
            key = (int(m.group('index')), m.group('field'))
            if key in values:
                sys.exit(f'{name}{list(key)} assigned twice')
            values[key] = int(m.group('value'))
        rows = []
        for i in range(count):
            missing = [f for f in FIELDS if (i, f) not in values]
            if missing:
                sys.exit(f'{name}[{i}] leaves {missing} unassigned; not a pure table')
            rows.append([values[(i, f)] for f in FIELDS])
        extra = [k for k in values if k[0] >= count or k[1] not in FIELDS]
        if extra:
            sys.exit(f'{name} has out-of-range assignments: {extra}')
        tables[name] = rows
    return tables


def initializers(text):
    tables = {}
    for m in TABLE.finditer(text):
        rows = [[int(v) for v in re.findall(r'-?\d+', row)] for row in re.findall(r'\{([^{}]*)\}', m.group('body'))]
        if len(rows) != int(m.group('count')) or any(len(r) != 6 for r in rows):
            sys.exit(f'Malformed table {m.group("name")}')
        tables[m.group('name')] = rows
    return tables


def rewrite(text):
    tables = assignments(text)
    # Remove assignment lines of converted tables.
    text = ASSIGN.sub(lambda m: '' if m.group('name') in tables else m.group(0), text)

    def table(decl):
        name, indent = decl.group('name'), decl.group('indent')
        rows = ',\n'.join(f'{indent}\t{{ ' + ', '.join(str(v) for v in row) + ' }' for row in tables[name])
        return (f'{indent}// {{ Stream, Offset, Type, Method, Usage, UsageIndex }}\n'
                f'{indent}D3DVERTEXELEMENT9 {name}[{decl.group("count")}] =\n{indent}{{\n{rows}\n{indent}}};\n')
    return DECL.sub(table, text), len(tables)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--file', required=True)
    parser.add_argument('--write', action='store_true')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--revision')
    parser.add_argument('--baseline', help='baseline path at the revision (defaults to --file)')
    args = parser.parse_args()
    path = ROOT / args.file
    text = path.read_text(encoding='utf-8-sig').replace('\r\n', '\n')
    if args.check:
        base = subprocess.check_output(['git', 'show', f'{args.revision}:{args.baseline or args.file}'], cwd=ROOT)
        expected = assignments(base.decode('utf-8-sig').replace('\r\n', '\n'))
        actual = initializers(text)
        if expected != actual:
            for name in sorted(set(expected) | set(actual)):
                if expected.get(name) != actual.get(name):
                    print(f'MISMATCH {name}: expected {expected.get(name)} actual {actual.get(name)}')
            sys.exit(1)
        print(f'PASS: {len(actual)} vertex declarations, {sum(len(r) for r in actual.values())} elements match {args.revision}')
        return
    new_text, count = rewrite(text)
    if args.write:
        raw = path.read_bytes()
        bom = raw.startswith(b'\xef\xbb\xbf')
        eol = '\r\n' if b'\r\n' in raw else '\n'
        path.write_bytes((b'\xef\xbb\xbf' if bom else b'') + new_text.replace('\n', eol).encode('utf-8'))
    print(f'{count} declarations converted' + ('' if args.write else ' (dry run)'))


if __name__ == '__main__':
    main()
