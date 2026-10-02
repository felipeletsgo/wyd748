"""Behavior-preserving relocation gate for TMProject748 source splits.

Compares the top-level definitions of baseline files at a Git revision with the
definitions found in their replacement files in the working tree. Comments,
whitespace, #include lines and '#pragma once' are ignored; every other token,
including literals, must be preserved. Each definition (function, variable,
class, enum, preprocessor line, ...) must appear the same number of times.
Namespace blocks are opened so their members may move independently.

Usage (from the repository root):
  python .agents/research/source-relocation.py --manifest .agents/research/relocations/<batch>.json

Manifest format:
  {"revision": "<commit>",
   "groups": [{"baseline": ["path/at/revision", ...],
               "current": ["path/in/worktree", ...]}],
   "reviewed": {"<item key>": {"before": "<sha256>", "after": "<sha256>"}}}
'reviewed' pins an intentional, reviewed change of a single definition.
"""
import argparse
import collections
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LITERAL = r'R"(?P<d>[^(\s]*)\([\s\S]*?\)(?P=d)"|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\''
TOKEN = re.compile(LITERAL + r'|#[^\n]*|\w+|::|->|<<=|>>=|[-+*/%&|^!=<>]=|&&|\|\||<<|>>|\+\+|--|\S')


def strip_comments(source):
    pattern = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL)
    return pattern.sub(lambda m: m.group() if m.group()[0] in '"\'R' else ' ', source)


def join_continuations(source):
    return re.sub(r'\\\r?\n', ' ', source)


def tokens(source):
    source = join_continuations(strip_comments(source.replace('\r\n', '\n')))
    result = []
    for match in TOKEN.finditer(source):
        token = match.group()
        if token.startswith('#'):
            directive = ' '.join(token.split())
            if re.match(r'#\s*include\b|#\s*pragma\s+once\b', directive):
                continue
            result.append(directive)
        else:
            result.append(token)
    return result


def items(token_list, prefix=''):
    """Split a token stream into top-level items; open namespace blocks."""
    out, i, n = [], 0, len(token_list)
    while i < n:
        token = token_list[i]
        if token.startswith('#'):
            out.append((prefix + token, [token]))
            i += 1
            continue
        if token == 'namespace':
            j = i + 1
            name = ''
            while j < n and token_list[j] != '{':
                name += token_list[j]
                j += 1
            depth, k = 1, j + 1
            while k < n and depth:
                depth += (token_list[k] == '{') - (token_list[k] == '}')
                k += 1
            out += items(token_list[j + 1:k - 1], prefix + 'namespace ' + (name or '<anon>') + '::')
            i = k
            continue
        start, depth, j, first_brace = i, 0, i, None
        while j < n:
            t = token_list[j]
            if t.startswith('#') and depth == 0:
                break
            if t in '({[':
                if t == '{' and depth == 0 and first_brace is None:
                    first_brace = j
                depth += 1
            elif t in ')}]':
                depth -= 1
                if t == '}' and depth == 0:
                    following = token_list[j + 1] if j + 1 < n else ''
                    # '};' ends a type or initializer.
                    if following == ';':
                        j += 2
                        break
                    # Brace-initialized constructor members continue the item.
                    if following in (',', '{'):
                        j += 1
                        continue
                    # A type followed by a declarator, or a braced initializer,
                    # continues to its ';'; any other closing brace ends a body.
                    head = token_list[start:first_brace]
                    if head and (head[0] in ('class', 'struct', 'enum', 'union', 'typedef') or '=' in head):
                        j += 1
                        continue
                    j += 1
                    break
            elif t == ';' and depth == 0:
                j += 1
                break
            j += 1
        body = token_list[start:j]
        # Forward declarations carry no definition and may repeat after a split.
        if len(body) == 3 and body[0] in ('class', 'struct') and body[2] == ';':
            body = []
        if body:
            out.append((prefix + key(body), body))
        i = max(j, i + 1)
    return out


def key(body):
    head = []
    for t in body:
        if t in ('{', ';', '='):
            break
        head.append(t)
    return ' '.join(head)[:240]


def digest(body):
    return hashlib.sha256(json.dumps(body, ensure_ascii=False).encode()).hexdigest()


def revision_source(revision, path):
    return subprocess.check_output(['git', 'show', f'{revision}:{path}'], cwd=ROOT).decode('utf-8-sig', 'replace')


def collect(sources):
    counter, keys = collections.Counter(), {}
    for source in sources:
        for item_key, body in items(tokens(source)):
            d = digest(body)
            counter[d] += 1
            keys[d] = item_key
    return counter, keys


def verify(manifest):
    revision = manifest['revision']
    reviewed = manifest.get('reviewed', {})
    failures = []
    total = 0
    for group in manifest['groups']:
        before, before_keys = collect(revision_source(revision, p) for p in group['baseline'])
        after, after_keys = collect((ROOT / p).read_text(encoding='utf-8-sig', errors='replace')
                                    for p in group['current'])
        for name, pin in reviewed.items():
            # The reviewed version replaces the baseline definition; both sides
            # must be present, otherwise the pin is stale and the gate fails.
            if before.get(pin['before']) and after.get(pin['after']):
                before[pin['before']] -= 1
                before[pin['after']] += 1
            elif before.get(pin['before']) or after.get(pin['after']):
                failures.append(f'reviewed change not matched: {name}')
        missing = before - after
        extra = after - before
        # Declarations of relocated helpers (no body) may be added to a support
        # header; every listed declaration must exist and nothing else may.
        allowed = set(group.get('new_declarations', []))
        for d in list(extra):
            if after_keys.get(d, '').replace(' ', '') in {a.replace(' ', '') for a in allowed}:
                del extra[d]
                allowed = {a for a in allowed if a.replace(' ', '') != after_keys[d].replace(' ', '')}
        for a in allowed:
            failures.append(f'declared helper not found: {a}')
        # Reviewed new definitions (for example a data table that replaced a
        # branch chain) are pinned by digest, so any later edit fails the gate.
        for label, pinned in group.get('new_items', {}).items():
            if extra.get(pinned):
                extra[pinned] -= 1
                if extra[pinned] <= 0:
                    del extra[pinned]
            else:
                failures.append(f'reviewed new item not found: {label}')
        total += sum(after.values())
        for d, count in missing.items():
            failures.append(f"missing x{count}: {before_keys.get(d, d)}  [{d[:12]}]")
        for d, count in extra.items():
            failures.append(f"extra   x{count}: {after_keys.get(d, d)}  [{d[:12]}]")
    return total, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--manifest', required=True, type=Path)
    args = parser.parse_args()
    manifest = json.loads(args.manifest.read_text(encoding='utf-8'))
    total, failures = verify(manifest)
    if failures:
        print('FAIL: relocation changed definitions')
        print('\n'.join(failures[:80]))
        sys.exit(1)
    print(f'PASS: {total} top-level definitions preserved across {len(manifest["groups"])} group(s); '
          f'{len(manifest.get("reviewed", {}))} reviewed change(s)')


if __name__ == '__main__':
    main()
