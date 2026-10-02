"""Pin a verified method extraction in a relocation manifest.

After extract-method.py has verified and written an extraction, record the
reviewed change of the method (baseline digest -> current digest) and the new
helper definitions (by digest) in the relocation manifest that owns the file,
so source-relocation.py keeps passing and any later edit is detected.

Usage:
  python .agents/research/pin-extraction.py --manifest <relocation.json> --spec <extraction spec.json>
"""
import argparse
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('relocation', Path(__file__).with_name('source-relocation.py'))
relocation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(relocation)


def items_by_key(text):
    return {key: relocation.digest(body) for key, body in relocation.items(relocation.tokens(text))}


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--manifest', required=True, type=Path)
    parser.add_argument('--spec', required=True, type=Path)
    args = parser.parse_args()
    manifest = json.loads(args.manifest.read_text(encoding='utf-8'))
    extraction = json.loads(args.spec.read_text(encoding='utf-8'))
    group = next(g for g in manifest['groups'] if extraction['source'] in g['current'])
    method_key = ' '.join(relocation.tokens(extraction['method']))

    def digest_for(sources, key):
        found = [d for src in sources for k, d in items_by_key(src).items() if k == key]
        if len(found) != 1:
            raise SystemExit(f'Expected one definition of {key}, found {len(found)}')
        return found[0]

    current = (ROOT / extraction['source']).read_text(encoding='utf-8-sig')
    new_items = group.setdefault('new_items', {})
    label = f"{extraction['method'].split('(')[0].split()[-1]}: helpers extracted by extract-method.py ({args.spec.name})"
    method_name = extraction['method'].split('(')[0].split()[-1]
    if method_name in new_items:
        # The method is itself a helper from an earlier extraction: it is not in
        # the baseline, so its pinned digest is replaced by the verified result.
        new_items[method_name] = digest_for([current], method_key)
    else:
        baseline = [relocation.revision_source(manifest['revision'], p) for p in group['baseline']]
        # A method already pinned by an earlier reviewed change keeps its original baseline digest.
        before = next((pin['before'] for pin in manifest.get('reviewed', {}).values()
                       if pin.get('method') == method_key), None) or digest_for(baseline, method_key)
        manifest.setdefault('reviewed', {})
        manifest['reviewed'] = {k: v for k, v in manifest['reviewed'].items() if v.get('method') != method_key}
        manifest['reviewed'][label] = {'method': method_key, 'before': before, 'after': digest_for([current], method_key)}
    for ex in extraction['extractions']:
        helper_marker = f" {extraction['class']} :: {ex['helper']} ("
        keys = [k for k in items_by_key(current) if helper_marker in f' {k}']
        if len(keys) != 1:
            raise SystemExit(f"Expected one helper {ex['helper']}, found {len(keys)}")
        new_items[f"{extraction['class']}::{ex['helper']}"] = items_by_key(current)[keys[0]]
    args.manifest.write_text(json.dumps(manifest, indent=1), encoding='utf-8')
    print(f"pinned {label} and {len(extraction['extractions'])} helper(s) in {args.manifest.name}")


if __name__ == '__main__':
    main()
