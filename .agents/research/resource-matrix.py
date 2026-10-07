"""File matrix: what the native 7.48 client and TMProject read, and what exists.

Read-only. Sources:
  native   path strings exported by ExportWydPathStrings.java
           (.agents/research/client748/exports/native-path-strings.tsv), with
           the functions that reference them
  client   string literals and path formats in tmproject/TMProject748 sources
  folder   files present in tmproject/client748 (the runtime)
  server   wydgo748/data files with the same name stem (authoritative tables)

A path format such as `mesh\\%s.msh` is matched against the runtime files.

Gaps per row:
  N  read by the native client, not referenced by TMProject
  T  referenced by TMProject, not by the native client
  M  referenced by a client but missing from the runtime folder
  S  data file that also exists on the server (contents must agree; see R2)

Usage:
  python .agents/research/resource-matrix.py --tsv <out.tsv> --markdown <out.md>
"""
import argparse
import re
import subprocess
from collections import defaultdict
from pathlib import Path
import importlib.util as _ilu
_wt_spec = _ilu.spec_from_file_location('worktree', Path(__file__).with_name('worktree.py'))
worktree = _ilu.module_from_spec(_wt_spec)
_wt_spec.loader.exec_module(worktree)

ROOT = Path(__file__).resolve().parents[2]
RUNTIME = ROOT / 'tmproject/client748'
# Newer KR client in the local-only reference collection: a source of superior
# assets for references the 7.48 runtime cannot satisfy.
REFERENCE_KR = ROOT / 'references/WYD_KR'
SERVER_DATA = ROOT / 'wydgo748/data'
NATIVE_PATHS = ROOT / '.agents/research/client748/exports/native-path-strings.tsv'
EXTENSIONS = ('txt', 'bin', 'dat', 'csv', 'ini', 'msh', 'wys', 'msa', 'wyt', 'trn', 'ani', 'bon', 'bmp',
              'tga', 'jpg', 'dds', 'wav', 'mp3', 'ogg', 'mid', 'fx', 'psh', 'vsh', 'cfg', 'json')
# Development paths of the native build (../../tmsrv/run/...), absolute paths and
# library names are not runtime files.
IGNORED = re.compile(r'(?i)\.(dll|exe|log|bak)$|^[a-z]:[\\/]|^\.\.[\\/]|%[^sd0-9]')
# A stem is required and spaces are not allowed (messages that mention a file).
PATH = re.compile(r'(?i)^[\w.%\\/:\-\[\]]*[\w%]\.(' + '|'.join(EXTENSIONS) + r')$')


def normalize(path):
    path = path.replace('\\\\', '\\').replace('\\', '/').strip()
    path = path.lstrip('/')
    while path.startswith('./'):
        path = path[2:]
    return path.lower()


def is_path(text):
    return bool(PATH.match(text)) and not IGNORED.search(text)


def native_paths(path=NATIVE_PATHS):
    paths = defaultdict(set)
    if not Path(path).is_file():
        return paths
    for line in Path(path).read_text(encoding='utf-8').splitlines()[2:]:
        parts = line.split('\t')
        if len(parts) >= 2 and is_path(parts[1]):
            paths[normalize(parts[1])] |= set(parts[2].split()) if len(parts) > 2 else set()
    return paths


def client_paths():
    files = worktree.relative('tmproject/TMProject748', ('.cpp', '.h'), ROOT)
    paths = defaultdict(set)
    for rel in files:
        if not rel.endswith(('.cpp', '.h')) or '/Dependencies/' in rel or '/tests/' in rel:
            continue
        text = (ROOT / rel).read_text(encoding='utf-8-sig', errors='replace')
        text = re.sub(r'//[^\n]*|/\*.*?\*/', ' ', text, flags=re.S)
        for literal in re.findall(r'"((?:\\.|[^"\\\n])*)"', text):
            value = literal.replace('\\\\', '\\')
            if is_path(value):
                paths[normalize(value)].add(rel.split('/')[-1])
    return paths


def runtime_files():
    return {normalize(str(p.relative_to(RUNTIME))): p for p in RUNTIME.rglob('*') if p.is_file()}


def reference_files():
    if not REFERENCE_KR.is_dir():
        return {}
    return {normalize(str(p.relative_to(REFERENCE_KR))): p for p in REFERENCE_KR.rglob('*') if p.is_file()}


def server_files():
    if not SERVER_DATA.is_dir():
        return {}
    return {p.stem.lower(): p.relative_to(ROOT).as_posix() for p in SERVER_DATA.iterdir() if p.is_file()}


def pattern_of(path):
    """Regex for a path format (%s, %d, %02d ...), or None for a plain path."""
    if '%' not in path:
        return None
    regex = re.escape(path)
    regex = re.sub(r'%0?\d*d', r'\\d+', regex.replace('\\%', '%'))
    regex = regex.replace('%s', '.+')
    return re.compile('^' + regex + '$')


def build():
    native, client, runtime, server = native_paths(), client_paths(), runtime_files(), server_files()
    reference = reference_files()
    rows = []
    for path in sorted(set(native) | set(client)):
        regex = pattern_of(path)
        if regex:
            matches = [f for f in runtime if regex.match(f)]
            present = len(matches)
        elif path in runtime:
            present = 1
        elif path.endswith('.txt') and path[:-4] + '.bin' in runtime:
            # Resource scripts are named .txt and loaded as compiled .bin
            # (the native RC loader formats "%s.bin").
            present = 'as .bin'
        else:
            present = 0
        stem = Path(path).stem
        gaps = ''
        if path in native and path not in client:
            gaps += 'N'
        if path in client and path not in native:
            gaps += 'T'
        if not present:
            gaps += 'M'
        if regex:
            in_reference = sum(1 for f in reference if regex.match(f) and f not in runtime)
        else:
            in_reference = 1 if path in reference or (path.endswith('.txt') and path[:-4] + '.bin' in reference) else 0
        if stem in server and Path(path).suffix in ('.bin', '.csv', '.txt', '.dat'):
            gaps += 'S'
        family = path.split('/')[0] if '/' in path else '(root)'
        rows.append({'path': path, 'family': family, 'format': 'yes' if regex else '',
                     'native': ' '.join(sorted(native.get(path, ())))[:120] or ('-' if path not in native else '?'),
                     'client': ' '.join(sorted(client.get(path, ())))[:120] or '-',
                     'runtime': present, 'reference_kr': in_reference if not present or regex else '',
                     'server': server.get(stem, '') if 'S' in gaps else '', 'gaps': gaps})
    return rows, runtime


def write(rows, runtime, tsv, markdown):
    columns = list(rows[0].keys())
    Path(tsv).write_text('\n'.join(['\t'.join(columns)] + ['\t'.join(str(r[c]) for c in columns) for r in rows]) + '\n',
                         encoding='utf-8')
    counts = defaultdict(int)
    families = defaultdict(lambda: defaultdict(int))
    for r in rows:
        for gap in r['gaps']:
            counts[gap] += 1
            families[r['family']][gap] += 1
    out = ['# Client resource compatibility matrix', '',
           'Generated by `.agents/research/resource-matrix.py`; do not edit the tables by hand.',
           'Native paths come from `.agents/research/client748/exports/native-path-strings.tsv`. '
           'Paths are lower-case with `/` separators; `%s`/`%d` formats are matched against the runtime folder.', '',
           '| Gap | Meaning | Rows |', '| --- | --- | --- |',
           f"| N | read by the native client, not referenced by TMProject | {counts['N']} |",
           f"| T | referenced by TMProject, not by the native client | {counts['T']} |",
           f"| M | referenced, missing from `tmproject/client748` | {counts['M']} |",
           f"| (M in KR) | missing rows available in `references/WYD_KR` | {sum(1 for r in rows if 'M' in r['gaps'] and r['reference_kr'])} |",
           f"| S | data file that also exists on the server | {counts['S']} |", '',
           '## By family', '', '| Family | N | T | M | S |', '| --- | --- | --- | --- | --- |']
    for family in sorted(families):
        f = families[family]
        out.append(f"| {family} | {f['N']} | {f['T']} | {f['M']} | {f['S']} |")
    out += ['', '## Rows other than per-model meshes and textures', '',
            'Individual `mesh/` model files (`.msh`, `.wys`, `.wyt`, `.msa`) are summarized above; the full list '
            'is in `.agents/research/client748/inventory/resource-matrix.tsv`.', '',
            '| Path | Native functions | TMProject files | In runtime | In WYD_KR | Server | Gaps |',
            '| --- | --- | --- | --- | --- | --- | --- |']
    for r in rows:
        if r['family'] == 'mesh' and Path(r['path']).suffix in ('.msh', '.wys', '.wyt', '.msa') and not r['format']:
            continue
        out.append(f"| `{r['path']}` | {r['native']} | {r['client']} | {r['runtime']} | {r['reference_kr'] if r['reference_kr'] != '' else '-'} | {r['server'] or '-'} | {r['gaps'] or '-'} |")
    marker = '<!-- Decisions below are maintained by hand and kept on regeneration. -->'
    kept = marker + '\n'
    if Path(markdown).is_file():
        previous = Path(markdown).read_text(encoding='utf-8')
        if marker in previous:
            kept = previous[previous.index(marker):]
    Path(markdown).write_text('\n'.join(out) + '\n\n' + kept, encoding='utf-8')
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--tsv', required=True)
    parser.add_argument('--markdown', required=True)
    args = parser.parse_args()
    rows, runtime = build()
    counts = write(rows, runtime, args.tsv, args.markdown)
    print(f'{len(rows)} paths; runtime files {len(runtime)}; gaps: ' + ', '.join(f'{k}={counts[k]}' for k in 'NTMS'))


if __name__ == '__main__':
    main()
