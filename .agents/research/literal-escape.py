"""Make client sources codepage-independent without changing compiled output.

MSVC compiles TMProject748 without /utf-8, so a source file is read in the
build machine's ANSI code page unless it starts with a UTF-8 BOM. Non-ASCII
bytes inside string and character literals therefore compile to different
values depending on that code page and on the file encoding. This tool:

  * rewrites every non-ASCII byte inside a string/char literal as an octal
    escape holding the byte the compiler produces today (raw byte for files
    without BOM; Windows-1252 byte of the character for UTF-8 BOM files);
  * collapses a multi-byte character literal assigned to a char into the
    single byte MSVC keeps (the last one), which removes C4305/C4309;
  * converts legacy CP949 comments of the listed headers to UTF-8 with BOM.

Each affected translation unit is compiled before and after with the recorded
Debug|Win32 options (see include-prune.py). Any object difference restores
every file and fails.

Usage: python .agents/research/literal-escape.py [--write]
"""
import argparse
import importlib.util
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prune', Path(__file__).with_name('include-prune.py'))
prune = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prune)

PROJECT = ROOT / 'tmproject/TMProject748'
LITERAL_FILES = [
    'internal/app/scenes/NewAppWindow.cpp',
    'internal/app/scenes/TMScene.cpp',
    'internal/core/Basedef.cpp',
    'internal/platform/windows/DirShow.cpp',
    'internal/ui/controls/SEditableText.cpp',
]
# Multi-byte character literals that are assigned to a single char.
CHAR_ASSIGNMENTS = {'internal/ui/controls/SEditableText.cpp'}
CP949_HEADERS = ['internal/ui/ItemEffect.h']
TOKEN = re.compile(rb'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)


def compiled_bytes(text, bom):
    """Bytes the compiler emits for the non-ASCII part of a literal body."""
    return text.decode('utf-8').encode('cp1252') if bom else text


def escape_literal(token, bom, collapse_char):
    quote, body = token[:1], token[1:-1]
    out = bytearray()
    i = 0
    while i < len(body):
        if body[i] == 0x5C:                      # keep existing escapes untouched
            out += body[i:i + 2]
            i += 2
            continue
        j = i
        while j < len(body) and body[j] > 127:
            j += 1
        if j == i:
            out.append(body[i])
            i += 1
            continue
        chunk = compiled_bytes(body[i:j], bom)
        out += b''.join(b'\\%03o' % b for b in chunk)
        i = j
    if quote == b"'" and collapse_char and out.count(b'\\') > 1 and b'\\' + out.split(b'\\')[-1] == out[-4:]:
        out = out[-4:]                           # MSVC keeps the last byte of a multichar assigned to char
    return quote + bytes(out) + quote


def rewrite(raw):
    bom = raw.startswith(b'\xef\xbb\xbf')
    return bom, lambda collapse: (raw[:3] if bom else b'') + TOKEN.sub(
        lambda m: escape_literal(m.group(), bom, collapse)
        if m.group()[:1] in (b'"', b"'") and any(b > 127 for b in m.group()) else m.group(),
        raw[3:] if bom else raw)


def restore(originals):
    """Write every original back; report every path that could not be restored."""
    failed = []
    for path, data in originals.items():
        try:
            path.write_bytes(data)
        except OSError as error:
            failed.append(f'{path}: {error}')
    if failed:
        raise RuntimeError('Could not restore original sources: ' + '; '.join(failed))


def verify(args, new):
    """Compile the affected units before and after; reject any object difference."""
    if args.against:
        # Compile the revision's files first, then the current ones; restore afterwards.
        for path in new:
            rel = path.relative_to(ROOT).as_posix()
            path.write_bytes(prune.subprocess.run(['git', 'show', f'{args.against}:{rel}'], cwd=ROOT,
                                                  capture_output=True, check=True).stdout)
        args.write = True

    # Units to compile: the rewritten .cpp files plus every unit including a header.
    units = [p for p in new if p.suffix == '.cpp']
    for header in (p for p in new if p.suffix == '.h'):
        for cpp in PROJECT.rglob('*.cpp'):
            if 'Dependencies' not in cpp.parts and f'"{header.name}"' in cpp.read_text(encoding='utf-8', errors='ignore'):
                units.append(cpp)
    env = prune.compiler_environment()
    commands = prune.recorded_commands()
    prune.WORK.mkdir(parents=True, exist_ok=True)
    pch = prune.WORK / 'pch.pch'
    pch_source = next(s for s in commands if s.endswith(prune.os.path.normcase(r'\pch.cpp')))
    if prune.run_cl(env, prune.adapt(commands[pch_source], prune.WORK / 'pch.obj', pch, create_pch=True), pch_source).returncode:
        sys.exit('Private precompiled header failed to build')
    obj = prune.WORK / 'unit.obj'

    def digests():
        result = {}
        for unit in units:
            ok, log = prune.compile_unit(env, commands[prune.os.path.normcase(str(unit.resolve()))], unit, obj, pch)
            if not ok:
                raise RuntimeError(f'{unit.name} failed to compile\n{log}')
            result[unit] = prune.digest(obj)
        return result

    before = digests()
    originals = dict(new) if args.against else {p: p.read_bytes() for p in new}
    # Candidates stay on disk only after every compilation and digest
    # comparison passed in write mode; any other outcome restores the originals.
    verified = False
    changed = []
    try:
        for p, data in new.items():
            p.write_bytes(data)
        after = digests()
        changed = [u.name for u in units if before[u] != after[u]]
        verified = not changed
    finally:
        if not (verified and args.write):
            restore(originals)
    if changed:
        sys.exit(f'REJECTED: objects changed for {changed}; all files restored')
    print(f"{'VERIFIED' if args.against else 'WROTE' if args.write else 'OK (dry run)'}: {len(new)} files, "
          f"{len(units)} translation units compiled to identical objects")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--write', action='store_true')
    parser.add_argument('--against', metavar='REV',
                        help='verify the current files against REV (e.g. HEAD) instead of rewriting them')
    args = parser.parse_args()

    new = {}
    for rel in LITERAL_FILES + CP949_HEADERS:
        path = PROJECT / rel
        if args.against:
            new[path] = path.read_bytes()
        elif rel in CP949_HEADERS:
            new[path] = b'\xef\xbb\xbf' + path.read_bytes().decode('cp949').encode('utf-8')
        else:
            _, apply = rewrite(path.read_bytes())
            new[path] = apply(rel in CHAR_ASSIGNMENTS)
    # With --against the working files are the ones to keep, whatever happens.
    keep = dict(new) if args.against else None
    try:
        verify(args, new)
    finally:
        if keep:
            for p, data in keep.items():
                p.write_bytes(data)


if __name__ == '__main__':
    main()
