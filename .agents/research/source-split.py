"""Mechanical source splitter for TMProject748 relocation batches.

Moves complete top-level definitions (with their leading comments) from one
source file into new translation units, in original order. Nothing inside a
definition is edited. Each new file starts with '#include "pch.h"' followed by
the source's own include block, so it compiles with the same declarations.
Run source-relocation.py afterwards; this tool does not prove equivalence.

Spec (JSON):
  {"source": "tmproject/.../File.cpp",
   "targets": {"tmproject/.../FileSuffix.cpp": ["regex on definition head", ...]},
   "header_note": "optional first-line comment for new files"}
A definition moves to the first target whose pattern matches its head (the
text before its first '{' or ';'). Unmatched definitions stay in the source.
Use --dry-run to list the assignment without writing.
"""
import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def scan(text):
    """Yield (start, end) spans of top-level items, including leading trivia."""
    i, n = 0, len(text)
    item_start = 0
    depth = 0
    head_end = None
    spans = []

    def skip_string(j):
        quote = text[j]
        j += 1
        while j < n and text[j] != quote:
            j += 2 if text[j] == '\\' else 1
        return j + 1

    while i < n:
        c = text[i]
        if text.startswith('//', i):
            i = text.find('\n', i)
            i = n if i < 0 else i
            continue
        if text.startswith('/*', i):
            i = text.index('*/', i) + 2
            continue
        if c == 'R' and text.startswith('R"', i) and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == '_')):
            m = re.match(r'R"([^(\s]*)\(', text[i:])
            if m:
                close = ')' + m.group(1) + '"'
                i = text.index(close, i + m.end()) + len(close)
                continue
        if c in '"\'':
            i = skip_string(i)
            continue
        if c == '#' and depth == 0 and text[text.rfind('\n', 0, i) + 1:i].strip() == '':
            end = i
            while True:
                line_end = text.find('\n', end)
                line_end = n if line_end < 0 else line_end
                if text[end:line_end].rstrip().endswith('\\'):
                    end = line_end + 1
                    continue
                break
            spans.append(('directive', item_start, line_end + 1 if line_end < n else n, None))
            item_start = i = line_end + 1
            continue
        if c in '({[':
            if c == '{' and depth == 0 and head_end is None:
                head_end = i
            depth += 1
        elif c in ')}]':
            depth -= 1
            if c == '}' and depth == 0:
                rest = text[i + 1:]
                following = re.match(r'\s*(\S)', rest)
                nxt = following.group(1) if following else ''
                head = text[item_start:head_end] if head_end is not None else ''
                code_head = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/', ' ', head).strip()
                if nxt == ';':
                    end = i + 1 + following.end()
                elif nxt in (',', '{'):
                    i += 1
                    continue
                elif re.match(r'(class|struct|enum|union|typedef|namespace)\b', code_head) and \
                        not code_head.startswith('namespace') or re.search(r'=\s*$', code_head):
                    i += 1
                    continue
                else:
                    end = i + 1
                line_end = text.find('\n', end)
                end = n if line_end < 0 else line_end + 1
                spans.append(('item', item_start, end, head_end))
                item_start, head_end = end, None
                i = end
                continue
        elif c == ';' and depth == 0:
            line_end = text.find('\n', i)
            end = n if line_end < 0 else line_end + 1
            spans.append(('item', item_start, end, None))
            item_start, head_end = end, None
            i = end
            continue
        i += 1
    if item_start < n:
        spans.append(('trailing', item_start, n, None))
    return spans


def head_of(text, start, end, head_end):
    raw = text[start:head_end if head_end is not None else end]
    return ' '.join(re.sub(r'//[^\n]*|/\*[\s\S]*?\*/', ' ', raw).split())


def split_header(spec, text, spans, bom, dry_run):
    """Split a header into per-owner headers and turn it into an umbrella.

    The first target receives the source's own include directives. Every
    target includes the earlier targets whose declared names it references,
    so each header keeps the declarations that preceded it in the original.
    """
    source_path = ROOT / spec['source']
    targets = {path: [re.compile(p) for p in patterns] for path, patterns in spec['targets'].items()}
    order = list(targets)
    moved = {path: [] for path in order}
    kept = []
    includes = []
    for kind, s, e, h in spans:
        piece = text[s:e]
        if kind == 'directive':
            if re.match(r'\s*#\s*include\b', piece.lstrip()):
                includes.append(piece.strip() + '\n')
            elif not re.match(r'\s*#\s*pragma\s+once', piece.strip()):
                kept.append(piece)
            continue
        head = head_of(text, s, e, h)
        destination = next((p for p, pats in targets.items() if any(x.search(head) for x in pats)), None)
        if destination:
            moved[destination].append(piece)
            if dry_run:
                print(f'{Path(destination).name:28} <- {head[:100]}')
        elif piece.strip():
            kept.append(piece)
    if [p for p in kept if p.strip()]:
        sys.exit('Unassigned header content:\n' + ''.join(kept)[:2000])
    defined = {}
    for path in order:
        names = set()
        for piece in moved[path]:
            for m in re.finditer(r'^\s*(?:class|struct|enum\s+class|enum|union)\s+(\w+)\s*(?:final\s*)?[:{]', piece, re.M):
                names.add(m.group(1))
        defined[path] = names
    if dry_run:
        return
    note = spec.get('header_note')
    for index, path in enumerate(order):
        body_text = ''.join(moved[path])
        deps = []
        for earlier in order[:index]:
            if any(re.search(r'\b' + re.escape(n) + r'\b', body_text) for n in defined[earlier]):
                deps.append(earlier)
        lines = ['#pragma once\n'] + (['// ' + note + '\n'] if note else [])
        if index == 0:
            lines += includes
        target = ROOT / path
        for dep in deps:
            rel = Path((ROOT / dep).resolve()).relative_to(target.parent.resolve()) if \
                (ROOT / dep).resolve().is_relative_to(target.parent.resolve()) else None
            lines.append(f'#include "{(rel or Path(dep).name).as_posix()}"\n')
        content = ''.join(lines) + '\n' + body_text.lstrip('\n')
        if target.exists():
            sys.exit(f'Refusing to overwrite existing file: {path}')
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes((b'\xef\xbb\xbf' if bom else b'') + content.encode('utf-8'))
        print(f'{path}: {len(moved[path])} items, includes {len(deps)} sibling header(s)')
    umbrella = ['#pragma once\n']
    if spec.get('umbrella_note'):
        umbrella.append('\n// ' + spec['umbrella_note'] + '\n')
    for path in order:
        umbrella.append(f'#include "{Path(path).relative_to(Path(spec["source"]).parent).as_posix()}"\n')
    source_path.write_bytes((b'\xef\xbb\xbf' if bom else b'') + ''.join(umbrella).encode('utf-8'))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--spec', required=True, type=Path)
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    spec = json.loads(args.spec.read_text(encoding='utf-8'))
    source_path = ROOT / spec['source']
    if spec.get('mode') == 'header':
        raw = source_path.read_bytes()
        text = raw.decode('utf-8-sig').replace('\r\n', '\n')
        spans = scan(text)
        if ''.join(text[s:e] for _, s, e, _ in spans) != text:
            sys.exit('Scanner did not cover the source exactly')
        split_header(spec, text, spans, raw.startswith(b'\xef\xbb\xbf'), args.dry_run)
        return
    raw = source_path.read_bytes()
    bom = raw.startswith(b'\xef\xbb\xbf')
    text = raw.decode('utf-8-sig').replace('\r\n', '\n')
    spans = scan(text)
    if ''.join(text[s:e] for _, s, e, _ in spans) != text:
        sys.exit('Scanner did not cover the source exactly')

    # A directive span may start with comments; copy only its #include line.
    includes = [m.group(0) + '\n' for kind, s, e, _ in spans if kind == 'directive'
                for m in [re.search(r'^[ \t]*#\s*include\b[^\n]*', text[s:e], re.M)]
                if m and 'pch.h' not in m.group(0)]
    targets = {path: [re.compile(p) for p in patterns] for path, patterns in spec['targets'].items()}
    moved = {path: [] for path in targets}
    kept = []
    for kind, s, e, h in spans:
        piece = text[s:e]
        destination = None
        if kind == 'item':
            head = head_of(text, s, e, h)
            for path, patterns in targets.items():
                if any(p.search(head) for p in patterns):
                    destination = path
                    break
        if destination:
            moved[destination].append(piece)
            if args.dry_run:
                print(f'{Path(destination).name:32} <- {head_of(text, s, e, h)[:110]}')
        else:
            kept.append(piece)
    empty = [p for p, items in moved.items() if not items]
    if empty:
        sys.exit('No definitions matched for: ' + ', '.join(empty))
    if args.dry_run:
        return
    note = spec.get('header_note')
    for path, pieces in moved.items():
        target = ROOT / path
        if target.exists():
            sys.exit(f'Refusing to overwrite existing file: {path}')
        body = '#include "pch.h"\n' + ('// ' + note + '\n' if note else '') + ''.join(includes)
        body += '\n' + ''.join(pieces).lstrip('\n')
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes((b'\xef\xbb\xbf' if bom else b'') + body.encode('utf-8'))
    source_path.write_bytes((b'\xef\xbb\xbf' if bom else b'') + ''.join(kept).encode('utf-8'))
    for path, pieces in moved.items():
        print(f'{path}: {len(pieces)} definitions')


if __name__ == '__main__':
    main()
