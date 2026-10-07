"""Inventory the complete HEAD-to-worktree delta without hiding deletions.

This records scope, not behavioral approval. Local database/build artifacts
are kept in the inventory, explicitly separated from product files. Only the
requested JSON evidence output is written; repository content is read-only.
"""
import argparse
import json
import subprocess
from collections import Counter
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]


def git_bytes(root, *args):
    return subprocess.run(['git', '-c', 'core.quotepath=false', *args], cwd=root,
                          capture_output=True, check=True).stdout


def changed_paths(root=ROOT):
    tracked = git_bytes(root, 'diff', '--name-only', '-z', 'HEAD').split(b'\0')
    untracked = git_bytes(root, 'ls-files', '--others', '--exclude-standard', '-z').split(b'\0')
    return sorted({p.decode('utf-8') for p in tracked + untracked if p})


def domain(path):
    if path.startswith('build/'):
        return 'local-build-or-database-artifact'
    if path.startswith('tmproject/client748/'):
        return 'client-runtime-assets'
    if path.startswith('tmproject/'):
        return 'client-source-and-build'
    if path.startswith('wydgo748/data/'):
        return 'server-data'
    if path.startswith('wydgo748/'):
        return 'server-source-and-tests'
    if path.startswith('DOCS/'):
        return 'documentation'
    if path.startswith('.agents/'):
        return 'research-and-skills'
    return 'repository-tooling'


def inventory(root=ROOT):
    rows = []
    for relative in changed_paths(root):
        path = root / relative
        rows.append({'path': relative, 'domain': domain(relative),
                     'exists': path.is_file(),
                     'bytes': path.stat().st_size if path.is_file() else None,
                     'approval': 'not-implied-by-inventory'})
    return rows


def project_references(path):
    references = []
    for node in ET.parse(path).getroot().iter():
        kind = node.tag.rsplit('}', 1)[-1]
        if kind not in ('ClCompile', 'ClInclude', 'ResourceCompile', 'None'):
            continue
        include = node.get('Include')
        if include and '$(' not in include:
            references.append((kind, include.replace('\\', '/')))
    return references


def runtime_path(runtime, value):
    # Both spellings occur in the legacy lists. Refuse escape/absolute paths.
    value = value.replace('\\', '/')
    if ':' in value or value.startswith('/'):
        raise ValueError('absolute runtime path')
    resolved = (runtime / value).resolve()
    if not resolved.is_relative_to(runtime.resolve()):
        raise ValueError('runtime path escapes its root')
    return resolved


def sound_rows(path):
    rows = []
    for number, line in enumerate(path.read_text(encoding='utf-8-sig').splitlines(), 1):
        if not line.strip():
            continue
        fields = line.split()
        if len(fields) != 3:
            raise ValueError(f'sound row {number}: expected index, path and channels')
        index, channels = int(fields[0]), int(fields[2])
        if index < 0 or channels < 1:
            raise ValueError(f'sound row {number}: invalid index or channels')
        rows.append((index, fields[1], channels))
    return rows


def music_rows(path):
    rows = []
    for number, line in enumerate(path.read_text(encoding='utf-8-sig').splitlines(), 1):
        if not line.strip():
            continue
        value = line[17:].rstrip(' \t\r\n')
        if not value:
            raise ValueError(f'music row {number}: missing column-17 path')
        rows.append(value)
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    rows = inventory()
    report = {'head': git_bytes(ROOT, 'rev-parse', 'HEAD').decode().strip(),
              'scope': 'all tracked HEAD deltas plus nonignored untracked paths',
              'counts': dict(sorted(Counter(r['domain'] for r in rows).items())),
              'files': rows}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({'head': report['head'], 'counts': report['counts']}, indent=2))


if __name__ == '__main__':
    main()
