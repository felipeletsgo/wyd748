"""Remove #include lines that provably do not affect the compiled object.

For each translation unit, the exact compiler options recorded by MSBuild for
the Debug|Win32 build are replayed with a private precompiled header and
deterministic output (/Brepro) into a scratch directory. Debug information and
/JMC are left out because they record every included header even when nothing
from it is used. An include is removed only when the object file compiled
without it is byte-for-byte identical to the baseline object, so generated
code, data, and linker directives (#pragma comment) are unchanged. During a
trial the include line is replaced by an empty line; the final file with the
lines deleted must compile to the same object again (this also rejects
removals that would shift a __LINE__ value). The source file is always
restored on failure.

Usage (repository root, after a Debug|Win32 build):
  python .agents/research/include-prune.py --list <file-with-unit-paths> [--write]
"""
import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TLOG = ROOT / 'tmproject/build/obj/TMProject748/Debug/TMProject748.tlog/CL.command.1.tlog'
WORK = ROOT / 'tmproject/build/include-prune'
VCVARS = Path(r'C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat')
INSTALLER = r'C:\Program Files (x86)\Microsoft Visual Studio\Installer'
DROPPED = re.compile(r'/(?:Fo|Fd|Fp|Yc|Yu|ZI|Z7|Zi|Gm|JMC)', re.IGNORECASE)


def compiler_environment():
    base = dict(os.environ)
    base['PATH'] = INSTALLER + os.pathsep + base.get('PATH', '')
    out = subprocess.check_output(f'cmd /c ""{VCVARS}" >nul && set"', shell=True, env=base)
    env = {}
    for line in out.decode('mbcs', 'replace').splitlines():
        if '=' in line:
            key, value = line.split('=', 1)
            env[key] = value
    return env


def recorded_commands():
    """Map current-checkout sources to their recorded option tokens (no sources)."""
    text = TLOG.read_bytes().decode('utf-16')
    root = os.path.normcase(str(ROOT))
    commands, current = {}, None
    for line in text.lstrip('\ufeff').splitlines():
        if line.startswith('^'):
            current = [os.path.normcase(p) for p in line[1:].split('|')]
        elif current:
            options = [t for t in re.findall(r'(?:[^\s"]|"[^"]*")+', line)
                       if os.path.normcase(t.strip('"')) not in current]
            for source in current:
                if source.startswith(root):
                    commands[source] = options
            current = None
    return commands


def adapt(options, obj, pch, create_pch=False):
    """Replace output, PCH and debug options; keep everything else identical."""
    kept = [o for o in options if not DROPPED.match(o)]
    kept += ['/Brepro', '/Yc"pch.h"' if create_pch else '/Yu"pch.h"', f'/Fp"{pch}"', f'/Fo"{obj}"']
    return kept


def run_cl(env, options, source):
    response = WORK / 'command.rsp'
    response.write_text(' '.join(options + [f'"{source}"']), encoding='utf-8')
    compiler = shutil.which('cl.exe', path=env.get('Path') or env.get('PATH'))
    return subprocess.run([compiler, f'@{response}'], env=env, capture_output=True)


def compile_unit(env, options, source, obj, pch):
    result = run_cl(env, adapt(options, obj, pch), source)
    return result.returncode == 0, result.stdout.decode('mbcs', 'replace')[-2000:]


HEADERS = {}


def owns(line, stem, text):
    """Keep the header that declares what the unit defines, even when another
    include happens to bring it in: <Owner>.h for <Owner><Suffix>.cpp, or a
    header (or umbrella one level deep) declaring a class defined here."""
    match = re.search(r'#\s*include\s*"([^"]+)"', line)
    if not match:
        return False
    name = Path(match.group(1)).name
    if stem.startswith(Path(name).stem):
        return True
    if not HEADERS:
        for header in (ROOT / 'tmproject/TMProject748').rglob('*.h'):
            HEADERS.setdefault(header.name, header)
    defined = set(re.findall(r'^\s*(?:[\w:<>*&\s]+\s)?(\w+)::~?\w+\s*\(', text, re.MULTILINE))
    header = HEADERS.get(name)
    if not header or not defined:
        return False
    body = header.read_text(encoding='utf-8-sig', errors='replace')
    nested = [HEADERS.get(Path(n).name) for n in re.findall(r'#\s*include\s*"([^"]+)"', body)]
    texts = [body] + [h.read_text(encoding='utf-8-sig', errors='replace') for h in nested if h]
    return any(re.search(rf'\b(?:class|struct)\s+{re.escape(c)}\b[^;]*\{{', t)
               for c in defined for t in texts)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def encode(lines, eol, bom):
    return (b'\xef\xbb\xbf' if bom else b'') + eol.join(lines).encode('utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--list', required=True, type=Path)
    parser.add_argument('--write', action='store_true')
    args = parser.parse_args()
    env = compiler_environment()
    commands = recorded_commands()
    WORK.mkdir(parents=True, exist_ok=True)
    pch = WORK / 'pch.pch'
    pch_source = next(s for s in commands if s.endswith(os.path.normcase(r'\pch.cpp')))
    result = run_cl(env, adapt(commands[pch_source], WORK / 'pch.obj', pch, create_pch=True), pch_source)
    if result.returncode:
        sys.exit('Private precompiled header failed to build\n'
                 + (result.stdout + result.stderr).decode('mbcs', 'replace')[-3000:])

    units = [ROOT / l.strip() for l in args.list.read_text().splitlines() if l.strip()]
    report = []
    for unit in units:
        key = os.path.normcase(str(unit.resolve()))
        if key not in commands:
            report.append(f'{unit.name}: no recorded command (build Debug|Win32 first)')
            continue
        original = unit.read_bytes()
        bom = original.startswith(b'\xef\xbb\xbf')
        text = original.decode('utf-8-sig')
        eol = '\r\n' if '\r\n' in text else '\n'
        lines = text.replace('\r\n', '\n').split('\n')
        # The object path is part of the debug information, so every compile
        # of a unit writes the same file and is hashed immediately.
        obj = WORK / 'unit.obj'
        ok, log = compile_unit(env, commands[key], unit, obj, pch)
        if not ok:
            report.append(f'{unit.name}: baseline compile failed\n{log}')
            continue
        baseline = digest(obj)
        ok, _ = compile_unit(env, commands[key], unit, obj, pch)
        if not ok or digest(obj) != baseline:
            report.append(f'{unit.name}: output is not deterministic; skipped')
            continue
        candidates = [i for i, l in enumerate(lines)
                      if re.match(r'\s*#\s*include\b', l) and 'pch.h' not in l
                      and not owns(l, unit.stem, text)]
        removed = set()
        try:
            for index in candidates:
                trial = ['' if i == index or i in removed else l for i, l in enumerate(lines)]
                unit.write_bytes(encode(trial, eol, bom))
                ok, _ = compile_unit(env, commands[key], unit, obj, pch)
                if ok and digest(obj) == baseline:
                    removed.add(index)
        finally:
            unit.write_bytes(original)
        if removed:
            final = encode([l for i, l in enumerate(lines) if i not in removed], eol, bom)
            try:
                unit.write_bytes(final)
                ok, _ = compile_unit(env, commands[key], unit, obj, pch)
                verified = ok and digest(obj) == baseline
            finally:
                unit.write_bytes(final if verified and args.write else original)
            if not verified:
                report.append(f'{unit.relative_to(ROOT).as_posix()}: deleting the lines changes the object; kept')
                continue
        report.append(f'{unit.relative_to(ROOT).as_posix()}: {len(removed)} of {len(candidates)} includes removable'
                      + ''.join(f'\n    - {lines[i].strip()}' for i in sorted(removed)))
    print('\n'.join(report))


if __name__ == '__main__':
    main()
