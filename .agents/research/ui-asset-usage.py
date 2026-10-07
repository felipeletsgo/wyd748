r"""Which UI textures does the 7.48 runtime use now that the modern UI is forced?

Read-only. The client always loads the modern scene resources (`*2.bin`) and
g_UIVer is 2. A control picks its texture through a texture set
(`UI/UITextureSetList.txt`: set -> texture indexes) and the texture index picks
a file (`UI/UITextureList.bin`: 264-byte records, file name first).

A texture file is kept when any of these reaches it:
  - a texture set of a control in a modern RC file (*2.bin, LoginScene2.bin);
  - a texture set (absolute value) named by a number on a client source line
    that handles texture sets or constructs a control (conservative);
  - a direct path or a printf pattern (`UI\m%02d%02d.wyt`) in the client
    sources or in the native path strings;
  - its name inside another runtime list (Effect, Mesh, Env texture lists).
It is classic-only when its sets appear only in the classic RC files
(FieldScene.bin, SelCharScene.bin, SelServerScene.bin, LoginScene.bin).
Files in UI/ that no texture list entry and no path names are reported apart.

Usage:
  python .agents/research/ui-asset-usage.py [--markdown out.md]
"""
import argparse
import importlib.util
import re
import struct
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
UI = ROOT / 'tmproject/client748/UI'
SOURCES = ROOT / 'tmproject/TMProject748/internal'
NATIVE_PATHS = ROOT / '.agents/research/client748/exports/native-path-strings.tsv'
CLASSIC_RC = ('FieldScene.bin', 'SelCharScene.bin', 'SelServerScene.bin', 'LoginScene.bin')
MODERN_RC = ('FieldScene2.bin', 'SelCharScene2.bin', 'SelServerScene2.bin', 'LoginScene2.bin')

spec = importlib.util.spec_from_file_location('ui_matrix', Path(__file__).with_name('ui-matrix.py'))
ui_matrix = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ui_matrix)


def texture_files():
    data = (UI / 'UITextureList.bin').read_bytes()
    files = {}
    for index in range(len(data) // 264):
        name = data[index * 264:index * 264 + 255].split(b'\0')[0].decode('latin-1')
        if name:
            files[index] = re.sub(r'\\+', '/', name).lower()
    return files


def texture_sets():
    text = (UI / 'UITextureSetList.txt').read_text(encoding='latin-1')
    sets = defaultdict(set)
    current = None
    for line in text.splitlines():
        m = re.match(r'\s*SetIndex:\s*(-?\d+)', line)
        if m:
            current = int(m.group(1))
            continue
        m = re.match(r'\s*(\d+),\d+,\d+,\d+,\d+,\d+,\d+\s*$', line)
        if m and current is not None:
            sets[current].add(int(m.group(1)))
    return sets


def rc_sets(names, sizes):
    used = defaultdict(set)
    for name in names:
        path = UI / name
        if not path.is_file():
            continue
        data = path.read_bytes()
        for table in (ui_matrix.MODERN, ui_matrix.LEGACY):
            records = ui_matrix.walk(data, sizes, table)
            if records:
                break
        else:
            continue
        offset = 0
        while offset < len(data):
            kind = struct.unpack_from('<i', data, offset)[0]
            size = sizes[table[kind]]
            texture_set = struct.unpack_from('<i', data, offset + 4 + 8)[0]
            used[abs(texture_set)].add(name)
            offset += 4 + size
    return used


def code_sets_and_paths():
    sets, paths = set(), set()
    for path in SOURCES.rglob('*.*'):
        if path.suffix not in ('.cpp', '.h') or 'Dependencies' in path.parts:
            continue
        text = path.read_text(encoding='utf-8-sig', errors='replace')
        for line in text.splitlines():
            code = line.split('//')[0]
            if re.search(r'TextureSet|new S[A-Z]\w*\(', code):
                sets.update(abs(int(n)) for n in re.findall(r'-?\b\d+\b', code))
            for p in re.findall(r'"([^"]+\.wyt)"', code):
                paths.add(re.sub(r'\\+', '/', p).lower())
    if NATIVE_PATHS.is_file():
        for line in NATIVE_PATHS.read_text(encoding='utf-8').splitlines():
            parts = line.split('\t')
            if len(parts) > 1 and parts[1].lower().endswith('.wyt'):
                paths.add(re.sub(r'\\+', '/', parts[1]).lower())
    return sets, paths


def pattern_regex(pattern):
    out = ''
    for token in re.split(r'(%0?\d*[ds])', pattern):
        if re.fullmatch(r'%0?(\d*)d', token):
            width = re.fullmatch(r'%0?(\d*)d', token).group(1)
            out += r'\d{%s}' % width if width else r'\d+'
        elif token == '%s':
            out += '.*'
        else:
            out += re.escape(token)
    return re.compile(out + '$')


def other_list_mentions():
    """Lowercased bytes of every runtime list except the UI texture list."""
    blobs = []
    for folder in ('Effect', 'Mesh', 'Env', 'UI'):
        for path in (UI.parent / folder).glob('*'):
            if path.suffix.lower() in ('.txt', '.bin') and path.name.lower() != 'uitexturelist.bin':
                blobs.append(path.read_bytes().lower())
    return blobs


def analyze():
    sizes = ui_matrix.opcode_matrix.compiled_sizes(
        set(ui_matrix.LEGACY.values()) | set(ui_matrix.MODERN.values()), ('UIBinary.h',))
    files, sets = texture_files(), texture_sets()
    modern, classic = rc_sets(MODERN_RC, sizes), rc_sets(CLASSIC_RC, sizes)
    code, paths = code_sets_and_paths()
    keep_sets = set(modern) | code
    reached = defaultdict(set)   # file -> sets reaching it
    for set_index, textures in sets.items():
        for texture in textures:
            if texture in files:
                reached[files[texture]].add(set_index)
    patterns = [pattern_regex(p) for p in paths if '%' in p]
    blobs = other_list_mentions()

    def named(name):
        if name in paths or any(r.match(name) for r in patterns):
            return True
        stem = name.split('/')[-1].encode('latin-1')
        return any(stem in blob for blob in blobs)

    on_disk = {('ui/' + p.name).lower(): p for p in UI.glob('*.wyt')}
    classic_only, kept, unreferenced = [], [], []
    for name, path in sorted(on_disk.items()):
        by_sets = reached.get(name, set())
        if named(name) or by_sets & keep_sets:
            kept.append(name)
        elif by_sets and by_sets <= set(classic):
            classic_only.append((name, path.stat().st_size, sorted(by_sets)))
        elif not by_sets:
            unreferenced.append((name, path.stat().st_size))
        else:
            kept.append(name)  # reached only by sets no RC or code names: keep, unknown
    classic_rc = [(n, (UI / n).stat().st_size) for n in CLASSIC_RC if (UI / n).is_file()]
    return classic_only, unreferenced, classic_rc, kept


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--markdown')
    args = parser.parse_args()
    classic_only, unreferenced, classic_rc, kept = analyze()
    mb = lambda n: f'{n / 1048576:.2f} MB'
    lines = ['| File | Size | Texture sets |', '| --- | --- | --- |']
    lines += [f'| `{n}` | {mb(s)} | {", ".join(map(str, sets))} |' for n, s, sets in classic_only]
    unref = ['| File | Size |', '| --- | --- |'] + [f'| `{n}` | {mb(s)} |' for n, s in unreferenced]
    rc = ['| File | Size |', '| --- | --- |'] + [f'| `ui/{n.lower()}` | {mb(s)} |' for n, s in classic_rc]
    total = sum(s for _, s, _ in classic_only) + sum(s for _, s in classic_rc)
    print(f'classic-only textures: {len(classic_only)} ({mb(sum(s for _, s, _ in classic_only))}); '
          f'classic RC files: {len(classic_rc)}; unreferenced UI textures: {len(unreferenced)} '
          f'({mb(sum(s for _, s in unreferenced))}); kept: {len(kept)}; classic total {mb(total)}')
    if args.markdown:
        Path(args.markdown).write_text('\n'.join(
            ['## Classic-only textures', ''] + lines + ['', '## Classic scene resources', ''] + rc +
            ['', '## UI textures no list or path references', ''] + unref) + '\n', encoding='utf-8')
    else:
        print('\n'.join(lines + [''] + rc + [''] + unref))


if __name__ == '__main__':
    main()
