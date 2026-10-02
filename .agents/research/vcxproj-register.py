"""Add or remove source registrations in a .vcxproj and its .filters file.

Entries are inserted after a reference entry and take its filter, so related
files stay grouped. Duplicate additions and missing removals are errors, and
the result is checked for exactly one registration per path.

Usage (paths relative to the project directory, backslashes or slashes):
  python .agents/research/vcxproj-register.py --project tmproject/TMProject748/TMProject748.vcxproj \
      --like internal/ui/SGrid.cpp --add internal/ui/SGridTooltip.cpp --remove internal/ui/Old.cpp
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def kind(path):
    return 'ClCompile' if path.lower().endswith(('.cpp', '.c')) else 'ClInclude'


def element(tag, path, text):
    escaped = re.escape(path)
    return re.compile(r'[ \t]*<' + tag + r' Include="' + escaped + r'"\s*(?:/>|>[\s\S]*?</' + tag + r'>)\r?\n')


def load(path):
    raw = path.read_bytes()
    bom = raw.startswith(b'\xef\xbb\xbf')
    text = raw.decode('utf-8-sig')
    eol = '\r\n' if '\r\n' in text else '\n'
    return text.replace('\r\n', '\n'), bom, eol


def save(path, text, bom, eol):
    path.write_bytes((b'\xef\xbb\xbf' if bom else b'') + text.replace('\n', eol).encode('utf-8'))


def edit(text, is_filters, adds, removes, like):
    for path in removes:
        pattern = element(kind(path), path, text)
        matches = pattern.findall(text)
        if len(matches) != 1:
            sys.exit(f'Expected one registration to remove for {path}, found {len(matches)}')
        text = pattern.sub('', text, count=1)
    by_kind = {}
    for path in adds:
        by_kind.setdefault(kind(path), []).append(path)
    for tag, paths in by_kind.items():
        anchor_path = like.get(tag)
        if anchor_path is None:
            sys.exit(f'--like needs a {tag} reference for {paths}')
        anchor = element(tag, anchor_path, text).search(text)
        if not anchor:
            sys.exit(f'Reference {anchor_path} is not registered')
        filt = re.search(r'<Filter>([^<]*)</Filter>', anchor.group()) if is_filters else None
        block = ''
        for path in paths:
            if element(tag, path, text).search(text):
                sys.exit(f'Already registered: {path}')
            if is_filters and filt:
                block += f'    <{tag} Include="{path}">\n      <Filter>{filt.group(1)}</Filter>\n    </{tag}>\n'
            else:
                block += f'    <{tag} Include="{path}" />\n'
        text = text[:anchor.end()] + block + text[anchor.end():]
    for path in adds:
        if len(element(kind(path), path, text).findall(text)) != 1:
            sys.exit(f'Registration check failed for {path}')
    return text


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--project', required=True, type=Path)
    parser.add_argument('--add', nargs='*', default=[])
    parser.add_argument('--remove', nargs='*', default=[])
    parser.add_argument('--like', nargs='*', default=[], help='reference .cpp and/or .h already registered')
    args = parser.parse_args()
    norm = lambda p: p.replace('/', '\\')
    adds = [norm(p) for p in args.add]
    removes = [norm(p) for p in args.remove]
    like = {kind(p): norm(p) for p in args.like}
    project = ROOT / args.project
    for path, is_filters in ((project, False), (project.with_suffix('.vcxproj.filters'), True)):
        text, bom, eol = load(path)
        save(path, edit(text, is_filters, adds, removes, like), bom, eol)
    print(f'{project.name}: +{len(adds)} -{len(removes)} registrations')


if __name__ == '__main__':
    main()
