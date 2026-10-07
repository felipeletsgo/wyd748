r"""Map TMProject client code that is dead, without function, or outside 7.48.

Read-only over the sources. Evidence, strongest first:

1. Linker reachability. A Debug build with function-level linking and no
   inlining (`_CL_=/Gy /Ob0`) is linked with `/OPT:REF /VERBOSE:REF`; every
   project function the linker discards is unreachable from the program entry
   points, including classes never constructed (their vtables are discarded
   too). `--build` produces that log in `tmproject/build/dead-code/`.
2. Source checks the linker cannot see: header-only functions never named
   elsewhere, member declarations without a definition, project files that
   are not compiled or not included, `#if 0` blocks and commented-out code.
3. Reachable code that serves no 7.48 contract: client handlers and senders of
   opcodes unknown to the native client (`opcode-matrix.tsv`, gap B) and
   control IDs absent from every 7.48 resource (`ui-matrix.tsv`, gap X).

A name used only by `tests/` is reported as test-only: the product does not
need it, but removing it also removes its test.

Usage:
  python .agents/research/dead-code-map.py [--build] [--tsv out.tsv] [--markdown out.md]
"""
import argparse
import os
import re
import subprocess
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CLIENT = ROOT / 'tmproject'
PROJECT = CLIENT / 'TMProject748'
VCXPROJ = PROJECT / 'TMProject748.vcxproj'
TESTS = PROJECT / 'tests'
BUILD = CLIENT / 'build/dead-code'
LOG = BUILD / 'link-verbose.log'
OPCODES = ROOT / '.agents/research/client748/inventory/opcode-matrix.tsv'
CONTROLS = ROOT / '.agents/research/client748/inventory/ui-matrix.tsv'
SOURCE_ROOTS = ('internal', 'features', 'scenes', 'cmd', 'CommonFiles')
VENDORED = {'DXUtil.cpp', 'DXUtil.h', 'DirShow.cpp', 'DirShow.h', 'dsutil.cpp', 'dsutil.h'}
CALLING = r'__(?:thiscall|cdecl|stdcall|fastcall|vectorcall) '


def read(path):
    return path.read_text(encoding='utf-8-sig', errors='replace').replace('\r\n', '\n')


def strip_comments(text):
    """Blank comments and string literals, keeping line numbers."""
    def blank(match):
        return re.sub(r'[^\n]', ' ', match.group(0))
    return re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', blank, text, flags=re.S)


def sources():
    files = []
    for name in SOURCE_ROOTS:
        files += [p for p in (PROJECT / name).rglob('*') if p.suffix in ('.cpp', '.h') and p.is_file()]
    files += [p for p in PROJECT.glob('*.*') if p.suffix in ('.cpp', '.h')]
    return sorted(files)


def registered():
    tree = ET.parse(VCXPROJ)
    compiled, headers = set(), set()
    for node in tree.iter():
        include = node.attrib.get('Include')
        if not include:
            continue
        path = (PROJECT / include.replace('\\', '/')).resolve()
        if node.tag.endswith('}ClCompile'):
            compiled.add(path)
        elif node.tag.endswith('}ClInclude'):
            headers.add(path)
    return compiled, headers


def rel(path):
    return path.resolve().relative_to(ROOT).as_posix()


def build():
    vswhere = Path(os.environ['ProgramFiles(x86)']) / 'Microsoft Visual Studio/Installer/vswhere.exe'
    msbuild = subprocess.run([str(vswhere), '-latest', '-products', '*', '-requires',
                              'Microsoft.Component.MSBuild', '-find', r'MSBuild\**\Bin\MSBuild.exe'],
                             capture_output=True, text=True, check=True).stdout.split('\n')[0].strip()
    BUILD.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, _CL_='/Gy /Ob0', _LINK_='/OPT:REF /OPT:NOICF /INCREMENTAL:NO /VERBOSE:REF')
    with LOG.open('w', encoding='utf-8') as out:
        result = subprocess.run([msbuild, str(VCXPROJ), '/t:Rebuild', '/p:Configuration=Debug', '/p:Platform=Win32',
                                 f'/p:SolutionDir={CLIENT}\\', f'/p:IntDir={BUILD / "obj"}\\',
                                 f'/p:OutDir={BUILD / "bin"}\\', '/m', '/nologo', '/v:normal'],
                                stdout=out, stderr=subprocess.STDOUT, env=env)
    if result.returncode:
        raise SystemExit(f'Build failed; see {LOG}')


def qualified_name(demangled):
    tail = re.split(CALLING, demangled, maxsplit=1)[1]
    depth = 0
    for index, char in enumerate(tail):
        if char == '<':
            depth += 1
        elif char == '>':
            depth -= 1
        elif char == '(' and depth == 0:
            return tail[:index]
    return tail


def linker_dead(files):
    """Project functions and data the linker discarded, by translation unit."""
    by_stem = {p.stem.lower(): p for p in files if p.suffix == '.cpp'}
    functions, data = [], []
    for demangled, obj in re.findall(r'Discarded "(.*)" \(\S+\) from (\S+)$', read(LOG), re.M):
        if '(' in obj or not obj.endswith('.obj'):
            continue  # library member
        source = by_stem.get(obj[:-4].lower())
        if re.search(CALLING, demangled):
            name = qualified_name(demangled)
            if name.startswith(('std::', 'Concurrency::', 'Microsoft::')) or '`' in name or name.startswith('__'):
                continue
            functions.append((source, name))
        elif '`' not in demangled and 'std::' not in demangled and '::_' not in demangled:
            data.append((source, demangled))
    return sorted(set(functions), key=str), sorted(set(data), key=str)


def definitions(text):
    """Out-of-line function definitions in a .cpp: (name, line)."""
    found = []
    code = strip_comments(text)
    for match in re.finditer(r'^(?!\s)(?!(?:if|for|while|switch|return|else)\b)[^\n;#{}()=]*?'
                             r'\b((?:\w+::)*~?\w+)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?::[^{;]*)?\{',
                             code, re.M | re.S):
        found.append((match.group(1), code.count('\n', 0, match.start()) + 1))
    return found


def word_counts(texts):
    counts = defaultdict(int)
    for text in texts:
        for word in re.findall(r'~?\b[A-Za-z_]\w*', text):
            counts[word] += 1
    return counts


def header_only_functions(files, product, tests):
    """Free/inline functions with a body in a header and no other mention."""
    rows = []
    for path in files:
        if path.suffix != '.h' or path.name in VENDORED:
            continue
        code = strip_comments(read(path))
        for match in re.finditer(r'^\s*(?:inline\s+|static\s+|constexpr\s+|template\s*<[^>]*>\s*)*'
                                 r'[\w:<>,\s\*&]+?\b([A-Za-z_]\w*)\s*\([^;{}()]*\)\s*(?:const\s*)?(?:noexcept\s*)?\{',
                                 code, re.M):
            name = match.group(1)
            if name in ('if', 'for', 'while', 'switch', 'return', 'sizeof', 'operator') or name[0] == '~':
                continue
            line = code.count('\n', 0, match.start()) + 1
            if product[name] <= 1:
                rows.append((path, line, name, 'test-only' if tests[name] else 'unreferenced'))
    return rows


def ghost_declarations(files, product):
    """Member functions declared in a class but defined nowhere."""
    defined = set()
    for path in files:
        for name, _ in definitions(read(path)):
            parts = name.split('::')
            # Namespaced definitions (Macro::Water_N::UseItem) also name Water_N::UseItem.
            defined.update('::'.join(parts[i:]) for i in range(len(parts) - 1))
    rows = []
    for path in files:
        if path.suffix != '.h' or path.name in VENDORED:
            continue
        code = strip_comments(read(path))
        for cls in re.finditer(r'^\s*(?:class|struct)\s+(?:__declspec\([^)]*\)\s*)?(\w+)[^;{]*\{', code, re.M):
            depth, start = 0, cls.end() - 1
            for end in range(start, len(code)):
                depth += {'{': 1, '}': -1}.get(code[end], 0)
                if depth == 0:
                    break
            body = code[start + 1:end]
            # Inline bodies become '@;', so a member defined in the class never
            # matches the bare declaration pattern below.
            flat = body
            for _ in range(4):
                flat = re.sub(r'\{[^{}]*\}', '@;', flat)
            for decl in re.finditer(r'(?:^|[;:])\s*((?:virtual\s+|static\s+|inline\s+)*)[\w:<>,\*&\s]+?'
                                    r'\b(~?\w+)\s*\([^;()]*\)\s*(?:const\s*)?(?:override\s*)?(=\s*\w+\s*)?;', flat):
                name = decl.group(2)
                if decl.group(3) or name in ('if', 'for', 'while', 'operator', 'sizeof') or 'operator' in decl.group(0):
                    continue
                if f'{cls.group(1)}::{name}' not in defined:
                    rows.append((path, cls.group(1), name, product[name.lstrip('~')]))
    return rows


def class_bodies(files):
    """(header, class, body) for every class/struct defined in a project header."""
    for path in files:
        if path.suffix != '.h' or path.name in VENDORED:
            continue
        code = strip_comments(read(path))
        for cls in re.finditer(r'^\s*(?:class|struct)\s+(?:__declspec\([^)]*\)\s*)?(\w+)[^;{()]*\{', code, re.M):
            depth, start = 0, cls.end() - 1
            for end in range(start, len(code)):
                depth += {'{': 1, '}': -1}.get(code[end], 0)
                if depth == 0:
                    break
            yield path, cls.group(1), code[start + 1:end]


def unused_members(files, product_texts, product):
    """Data members never named again, and pointers read but never assigned.

    Wire and data-file structures are filled by whole-buffer copies, so only
    their unused members are reported, never their unassigned pointers.
    """
    joined = '\n'.join(product_texts)
    rows = []
    for path, cls, body in class_bodies(files):
        flat = body
        for _ in range(4):
            flat = re.sub(r'\{[^{}]*\}', '@', flat)
        filled_by_copy = any(part in path.parts for part in ('wire', 'core')) or cls.startswith(('MSG_', 'STRUCT_'))
        # Aggregates built with braces assign every pointer without naming it.
        braced = re.search(r'\b' + cls + r'(?:<[^>]*>)?(?:\s+\w+)?\s*(?:=\s*)?\{', joined.replace(f'struct {cls}', ''))
        for decl in re.finditer(r'(?:^|[;:@])\s*(?!return|typedef|using|friend|enum|static|constexpr|public|private|protected)'
                                r'([\w:<>,\s]+?[\s\*&]+)(\w+)\s*((?:\[[^\]]*\]\s*)*)(?:=[^;]*)?;', flat):
            kind, name = decl.group(1), decl.group(2)
            if '(' in kind or name in ('const', 'override') or kind.strip() in ('return', 'delete', 'goto', 'case', 'else'):
                continue
            mentions = product[name] - 1
            if mentions <= 0:
                rows.append((path, cls, name, 'layout field, unused' if filled_by_copy else 'never used'))
            elif '*' in kind and not filled_by_copy and not braced and not re.search(
                    r'\b' + name + r'\b\s*(?:\[[^\]]*\]\s*)*=(?!=)|\b' + name + r'\s*\(|&\s*(?:\w+(?:->|\.))?' + name + r'\b'
                    r'|(?:memcpy|memset|ZeroMemory|std::fill|fill)\s*\(\s*(?:\w+(?:->|\.))?' + name + r'\b', joined):
                rows.append((path, cls, name, 'pointer never assigned'))
    return rows


def empty_functions(files):
    """Reachable-looking definitions whose body does nothing.

    A body that is empty or only returns a literal (0, 1, nullptr, S_OK, ...)
    has no function; callers keep it alive, so the linker cannot report it.
    """
    # A virtual with an empty body is usually a base-class default or a deliberate
    # override; report it apart from plain functions that do nothing.
    virtuals = set()
    for path, cls, body in class_bodies(files):
        virtuals |= {f'{cls}::{n}' for n in re.findall(r'\bvirtual\b[^;{(]*?\b(\w+)\s*\(', body)}
        virtuals |= {f'{cls}::{n}' for n in re.findall(r'\b(\w+)\s*\([^;{]*\)\s*(?:const\s*)?override\b', body)}
    rows = []
    for path in files:
        if path.suffix != '.cpp' or path.name in VENDORED:
            continue
        code = strip_comments(read(path))
        for name, line in definitions(code):
            start = code.index('{', [m.start() for m in re.finditer(r'^', code, re.M)][line - 1])
            depth = 0
            for end in range(start, len(code)):
                depth += {'{': 1, '}': -1}.get(code[end], 0)
                if depth == 0:
                    break
            body = code[start + 1:end].strip()
            if '::' in name and not name.split('::')[-1].startswith('~') and \
                    name.split('::')[-1] != name.split('::')[-2] and \
                    re.fullmatch(r'(?:return\s+(?:-?\d+|nullptr|NULL|true|false|S_OK|E_FAIL|0x[0-9A-Fa-f]+)\s*;)?', body):
                kind = 'virtual' if '::'.join(name.split('::')[-2:]) in virtuals else 'plain'
                rows.append((path, line, name, body or 'empty', kind))
    return rows


def disabled_blocks(files):
    rows = []
    for path in files:
        if path.name in VENDORED:
            continue
        lines = read(path).split('\n')
        run = []
        for number, line in enumerate(lines + [''], 1):
            text = line.strip()
            if re.match(r'#\s*if\s+0\b', text):
                rows.append((path, number, '#if 0'))
            code_comment = text.startswith('//') and re.search(r'[;{}]\s*$|^//\s*(if|for|while|return)\b', text)
            if code_comment:
                run.append(number)
                continue
            if len(run) >= 3:
                rows.append((path, run[0], f'{len(run)} commented-out code lines'))
            run = []
    return rows


def matrix_rows(path, predicate):
    if not path.is_file():
        return []
    lines = path.read_text(encoding='utf-8').splitlines()
    header = lines[0].split('\t')
    return [dict(zip(header, line.split('\t'))) for line in lines[1:] if predicate(dict(zip(header, line.split('\t'))))]


def analyze():
    files = sources()
    product_texts = [strip_comments(read(p)) for p in files]
    test_texts = [strip_comments(read(p)) for p in TESTS.glob('*.cpp')] if TESTS.is_dir() else []
    product, tests = word_counts(product_texts), word_counts(test_texts)
    compiled, headers = registered()

    functions, data = linker_dead(files) if LOG.is_file() else ([], [])
    dead_by_file = defaultdict(list)
    for source, name in functions:
        dead_by_file[source].append(name)
    dead_files = []
    for source, names in dead_by_file.items():
        if source is None:
            continue
        defined = {n for n, _ in definitions(read(source))}
        short = {n.split('::')[-1] for n in names} | set(names)
        if defined and all(d in short or d.split('::')[-1] in short for d in defined):
            dead_files.append(source)

    included = set()
    for text in [read(p) for p in files] + [read(p) for p in TESTS.glob('*.*') if p.suffix in ('.cpp', '.h')]:
        included |= {Path(m).name.lower() for m in re.findall(r'#\s*include\s+["<]([^">]+)[">]', text)}
    unbuilt = [p for p in files if p.suffix == '.cpp' and p.resolve() not in compiled and 'tests' not in p.parts]
    unincluded = [p for p in files if p.suffix == '.h' and p.name.lower() not in included and p.name != 'pch.h']

    opcode_rows = matrix_rows(OPCODES, lambda r: 'B' in r.get('gaps', '') and
                              ('True' in (r.get('client_recv'), r.get('client_send'))))
    control_rows = matrix_rows(CONTROLS, lambda r: r.get('gaps') == 'X')
    return dict(functions=functions, data=data, dead_files=dead_files, tests=tests,
                header_only=header_only_functions(files, product, tests),
                ghosts=ghost_declarations(files, product), disabled=disabled_blocks(files),
                members=unused_members(files, product_texts, product), empty=empty_functions(files),
                unbuilt=unbuilt, unincluded=unincluded, opcodes=opcode_rows, controls=control_rows)


def markdown(result):
    tests = result['tests']
    out = []
    vendored = lambda p: p is not None and p.name in VENDORED
    own = [(s, n) for s, n in result['functions'] if not vendored(s)]
    out += ['## Summary', '', '| Category | Count |', '| --- | --- |',
            f'| Functions unreachable at link time (project code) | {len(own)} |',
            f'| Functions unreachable at link time (vendored DirectX helpers) | {len(result["functions"]) - len(own)} |',
            f'| Translation units with no reachable function | {len(result["dead_files"])} |',
            f'| Global data discarded by the linker | {len(result["data"])} |',
            f'| Header-only functions never used by the product | {len(result["header_only"])} |',
            f'| Functions whose body does nothing (plain / virtual) | '
            f'{sum(r[4] == "plain" for r in result["empty"])} / {sum(r[4] == "virtual" for r in result["empty"])} |',
            f'| Member declarations without a definition | {len(result["ghosts"])} |',
            f'| Data members never used / pointers never assigned | '
            f'{sum(r[3] == "never used" for r in result["members"])} / '
            f'{sum(r[3] == "pointer never assigned" for r in result["members"])} |',
            f'| Unused fields of wire or data-file layouts (kept for layout) | '
            f'{sum(r[3].startswith("layout") for r in result["members"])} |',
            f'| Sources not compiled / headers not included | {len(result["unbuilt"])} / {len(result["unincluded"])} |',
            f'| `#if 0` and commented-out code blocks | {len(result["disabled"])} |',
            f'| Reachable handlers or senders of non-native opcodes | {len(result["opcodes"])} |',
            f'| Control IDs absent from every 7.48 resource | {len(result["controls"])} |', '']
    out += ['## Translation units with no reachable function', '',
            'Every out-of-line function these files define was discarded by the linker.', '']
    out += [f'- `{rel(p)}`' for p in sorted(result['dead_files'], key=str)] or ['None.']
    out += ['', '## Functions unreachable at link time', '',
            '| File | Function | Tests |', '| --- | --- | --- |']
    for source, name in result['functions']:
        where = rel(source) if source else '(header)'
        parts = name.split('::')
        # A method counts as tested only when its class is named in the tests too.
        used = tests.get(parts[-1]) and (len(parts) == 1 or tests.get(parts[-2]))
        out.append(f'| `{where}` | `{name}` | {"uses it" if used else "-"} |')
    out += ['', '## Global data discarded by the linker', '']
    out += [f'- `{rel(s) if s else "(header)"}`: `{d}`' for s, d in result['data']] or ['None.']
    out += ['', '## Header-only functions never used by the product', '', '| File | Line | Function | Status |',
            '| --- | --- | --- | --- |']
    out += [f'| `{rel(p)}` | {line} | `{name}` | {status} |' for p, line, name, status in result['header_only']]
    out += ['', '## Functions whose body does nothing', '',
            'Empty, or only returning a literal. Callers keep them linked; each needs either the native '
            'behavior or removal together with its callers.', '',
            'A virtual one is usually a base-class default or a deliberate override; a plain one is a stub.', '',
            '| File | Line | Function | Body | Kind |', '| --- | --- | --- | --- | --- |']
    out += [f'| `{rel(p)}` | {n} | `{f}` | `{b}` | {k} |'
            for p, n, f, b, k in sorted(result['empty'], key=lambda r: (r[4] != 'plain', str(r[0]), r[1]))]
    out += ['', '## Member declarations without a definition', '',
            'Declared in a class and defined nowhere; "mentions" counts every use of the name in the product.', '',
            '| Header | Class | Member | Mentions |', '| --- | --- | --- | --- |']
    out += [f'| `{rel(p)}` | `{c}` | `{n}` | {m} |' for p, c, n, m in result['ghosts']]
    out += ['', '## Data members without function', '',
            'A member never named outside its declaration, or a pointer the product reads but never '
            'assigns (always null, so every branch behind it is dead).', '',
            '| Header | Class | Member | Finding |', '| --- | --- | --- | --- |']
    out += [f'| `{rel(p)}` | `{c}` | `{n}` | {k} |' for p, c, n, k in result['members']]
    out += ['', '## Files outside the build', '']
    out += [f'- not compiled: `{rel(p)}`' for p in result['unbuilt']]
    out += [f'- never included: `{rel(p)}`' for p in result['unincluded']]
    if not result['unbuilt'] and not result['unincluded']:
        out.append('None.')
    out += ['', '## Disabled code', '', '| File | Line | Kind |', '| --- | --- | --- |']
    out += [f'| `{rel(p)}` | {n} | {k} |' for p, n, k in result['disabled']]
    out += ['', '## Reachable code for opcodes unknown to the native client', '',
            'From `opcode-matrix.tsv` (gap B with a TMProject receiver or sender). Each row needs a recorded '
            'decision: a coordinated extension with the server, or removal.', '',
            '| Opcode | Names | Client recv | Client send | Server recv | Server send |',
            '| --- | --- | --- | --- | --- | --- |']
    out += [f'| {r["opcode"]} | {r["names"] or "-"} | {r["client_recv"]} | {r["client_send"]} | '
            f'{r["server_recv"]} | {r["server_send"]} |' for r in result['opcodes']]
    out += ['', '## Control IDs absent from every 7.48 resource', '',
            'From `ui-matrix.tsv` (gap X); reasons are recorded in `ui-compatibility-matrix.md`.', '']
    out += [f'- {r["id"]} ({r.get("hex", "")}): {r.get("client", "")}' for r in result['controls']] or ['None.']
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--build', action='store_true', help='rebuild the Debug link log first')
    parser.add_argument('--markdown')
    parser.add_argument('--tsv')
    args = parser.parse_args()
    if args.build:
        build()
    if not LOG.is_file():
        raise SystemExit(f'Missing {LOG}; run with --build')
    result = analyze()
    text = markdown(result)
    if args.tsv:
        rows = ['category\tfile\tname'] + [f'linker-function\t{rel(s) if s else ""}\t{n}' for s, n in result['functions']]
        rows += [f'header-only\t{rel(p)}:{l}\t{n}' for p, l, n, _ in result['header_only']]
        rows += [f'ghost-declaration\t{rel(p)}\t{c}::{n}' for p, c, n, _ in result['ghosts']]
        rows += [f'disabled\t{rel(p)}:{n}\t{k}' for p, n, k in result['disabled']]
        rows += [f'empty-{k}\t{rel(p)}:{n}\t{f}' for p, n, f, _, k in result['empty']]
        rows += [f'member-{k.replace(" ", "-").replace(",", "")}\t{rel(p)}\t{c}::{n}' for p, c, n, k in result['members']]
        rows += [f'unincluded-header\t{rel(p)}\t' for p in result['unincluded']]
        Path(args.tsv).write_text('\n'.join(rows) + '\n', encoding='utf-8')
    if args.markdown:
        path = Path(args.markdown)
        head = path.read_text(encoding='utf-8').split('## Summary')[0] if path.is_file() else ''
        path.write_text(head + text, encoding='utf-8', newline='\n')
    print(text.split('\n## Translation')[0])


if __name__ == '__main__':
    main()
