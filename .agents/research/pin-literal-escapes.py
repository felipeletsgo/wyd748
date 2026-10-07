"""Pin definitions changed only by literal-escape.py in a relocation manifest.

literal-escape.py proves its rewrite by compiling every affected unit before and
after (`--against <rev>` must report identical objects). This script then
records the changed definitions of one manifest: an existing reviewed pin or a
pinned helper gets its new digest; any other changed baseline definition gets a
reviewed before/after entry. Run literal-escape.py --against first.

Usage: python .agents/research/pin-literal-escapes.py --manifest <relocation.json> --names <definition>...
"""
import argparse
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('relocation', Path(__file__).with_name('source-relocation.py'))
relocation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(relocation)
LABEL = 'literal escapes (literal-escape.py --against HEAD: identical objects)'


def items_by_key(text):
    return {key: relocation.digest(body) for key, body in relocation.items(relocation.tokens(text))}


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--manifest', required=True, type=Path)
    parser.add_argument('--names', nargs='+', required=True, help='Qualified names of the changed definitions')
    args = parser.parse_args()
    manifest = json.loads(args.manifest.read_text(encoding='utf-8'))
    current = {}
    for group in manifest['groups']:
        for path in group['current']:
            current.update(items_by_key((ROOT / path).read_text(encoding='utf-8-sig')))
    baseline = {}
    for group in manifest['groups']:
        for path in group['baseline']:
            baseline.update(items_by_key(relocation.revision_source(manifest['revision'], path)))
    reviewed = manifest.setdefault('reviewed', {})
    for name in args.names:
        marker = ' '.join(relocation.tokens(name)) + ' ('
        keys = [k for k in current if marker in f' {k}' or k.startswith(marker.strip())]
        keys = [k for k in keys if f' {marker}' in f' {k}']
        if len(keys) != 1:
            raise SystemExit(f'Expected one current definition of {name}, found {len(keys)}')
        key = keys[0]
        updated = False
        for group in manifest['groups']:
            for helper, _ in list(group.get('new_items', {}).items()):
                if helper.replace('::', ' :: ') in key or helper in key.replace(' ', ''):
                    group['new_items'][helper] = current[key]
                    updated = True
        for pin in reviewed.values():
            if pin.get('method') == key:
                pin['after'] = current[key]
                updated = True
        if not updated:
            if key not in baseline:
                raise SystemExit(f'{name} is neither pinned nor in the baseline')
            reviewed[f'{name}: {LABEL}'] = {'method': key, 'before': baseline[key], 'after': current[key]}
        print(f'pinned {name}')
    args.manifest.write_text(json.dumps(manifest, indent=1), encoding='utf-8')


if __name__ == '__main__':
    main()
