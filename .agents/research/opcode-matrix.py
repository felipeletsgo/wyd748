"""Three-way WYD 7.48 opcode matrix: native client, TMProject client, Go server.

Read-only over the three sources. It reports, per opcode, what each side does
and classifies the compatibility gaps:

  A  sent by the native or TMProject client, no Go server handler
  B  present in TMProject or the server, unknown to the native client
  C  native, but not handled or not sent by the TMProject client
  D  sizes disagree between sources
  E  emitted by the server but not handled by the TMProject client

Sources:
  native   size gate FUN_0055890a.c and the decompiled corpus written by
           ExportWydOpcodeSites.java (tmproject/build/native-research/decompiled)
  client   tmproject/TMProject748 constants, receive dispatch, ExpectedSize,
           Header.Type assignments and static_assert sizes
  server   wydgo748/internal/wire opcodes, exactInboundPacketSize, handler
           switch cases and packet builders

Usage:
  python .agents/research/opcode-matrix.py opcodes --out <opcodes.txt>
  python .agents/research/opcode-matrix.py matrix --tsv <out.tsv> --markdown <out.md> --native-export <out.tsv>
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
CLIENT = ROOT / 'tmproject/TMProject748'
SERVER = ROOT / 'wydgo748'
NATIVE_GATE = ROOT / 'tmproject/build/native-research/FUN_0055890a.c'
NATIVE_SEND = 'FUN_0055f2dd'
NATIVE_CORPUS = ROOT / 'tmproject/build/native-research/decompiled'
NATIVE_EXPORTS = ROOT / '.agents/research/client748/exports'
NATIVE_SHA256 = '8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593'


def tracked(prefix, suffixes):
    # Working tree: tracked and untracked (not ignored) files that exist.
    return worktree.files(prefix, suffixes, ROOT)


def read(path):
    return path.read_text(encoding='utf-8-sig', errors='replace')


def strip_comments(text):
    return re.sub(r'//[^\n]*|/\*.*?\*/', ' ', text, flags=re.S)


# ---------------------------------------------------------------- native ---

def native_gate(path=NATIVE_GATE):
    """Opcode -> fixed size from the native size gate (FUN_0055890a)."""
    sizes, current = {}, None
    for line in read(path).splitlines():
        # Ghidra prints some labels in decimal (case 899: is opcode 0x383).
        match = re.search(r'case (0x[0-9a-f]+|\d+):|uVar1 == (0x[0-9a-f]+|\d+)\)', line)
        if match:
            current = int(match.group(1) or match.group(2), 0)
        size = re.search(r'\*param_1 != (0x[0-9a-f]+|\d+)', line)
        if size and current is not None:
            sizes[current] = int(size.group(1), 0)
            current = None
    return sizes


TYPE_FIELD = r'\*\((?:u?short|undefined2) \*\)\(\s*\w+ \+ 4\)'
# Ghidra prints a constant in hexadecimal or decimal (0x383 can appear as 899).
NUMBER = r'(0x[0-9a-f]+|\d+)\b'


def native_corpus(corpus=NATIVE_CORPUS, opcodes=None, exports=NATIVE_EXPORTS):
    """Opcode -> native receive and send sites from the decompiled corpus.

    Receive: comparisons of the packet Type field (*(short *)(p + 4), or a
    local copied from it) and `case` labels of a switch over that field.
    Send: a header built in consecutive locals, `local_N = OP;` with the Size
    word at local_(N+4), passed to a call as `(&local_(N+4), SIZE)`; the
    second argument is the size sent."""
    sites = defaultdict(lambda: {'recv': set(), 'send': set(), 'send_sizes': set(), 'stored': set()})
    corpus = Path(corpus)
    if not corpus.is_dir():
        return sites
    # Functions the decompiler could not process are covered by focused
    # instruction exports: word stores of an opcode into an object field.
    for export in sorted(Path(exports).glob('opcode-census-*-stores.tsv')):
        for line in export.read_text(encoding='utf-8').splitlines():
            store = re.search(r'instruction=MOV word ptr \[\w+ \+ 0x[0-9a-f]+\],(0x[0-9a-f]+);', line)
            if store and (opcodes is None or int(store.group(1), 16) in opcodes):
                sites[int(store.group(1), 16)]['stored'].add(line.split('\t')[2])
    for path in sorted(corpus.glob('*.c')):
        name = path.stem
        text = path.read_text(encoding='utf-8', errors='replace')
        # The size gate validates every opcode and the send routine compares the
        # outgoing type (throttling); neither is a receive handler.
        receives = name not in ('FUN_0055890a', NATIVE_SEND)
        aliases = set(re.findall(r'(\w+) = ' + TYPE_FIELD + r';', text))
        field = '(?:' + TYPE_FIELD + ('|' + '|'.join(map(re.escape, aliases)) if aliases else '') + ')'
        for match in re.finditer(field + r'\s*[!=]= ' + NUMBER, text):
            if receives and int(match.group(1), 0) >= 0x100:
                sites[int(match.group(1), 0)]['recv'].add(name)
        for match in re.finditer(r'switch\s*\(\s*' + field + r'\s*\)\s*\{', text):
            depth, end = 1, match.end()
            while end < len(text) and depth:
                depth += (text[end] == '{') - (text[end] == '}')
                end += 1
            for case in re.findall(r'case (0x[0-9a-f]+|\d+):', text[match.end():end]):
                if receives and int(case, 0) >= 0x100:
                    sites[int(case, 0)]['recv'].add(name)
        # Prebuilt packets: the opcode is stored into an object field and the
        # packet is sent later (native mix panels keep their request this way).
        # A field that receives several values in one function is a packet
        # header only when every value is a known opcode (a mix panel choosing
        # its request); otherwise it is a selector such as an animation index.
        stores = defaultdict(set)
        # A buffer cleared by memset that starts 4 bytes before the stored
        # field is a packet header (Size at +0, Type at +4): its opcode counts
        # even when no other source knows it, and the memset gives its size.
        cleared = {m.group(1).replace(' ', ''): int(m.group(2), 0) for m in re.finditer(
            r'_memset\(\(void \*\)\((\w+ \+ 0x[0-9a-f]+)\),0,(0x[0-9a-f]+|\d+)\)', text)}
        for match in re.finditer(r'\*\((?:u?short|undefined2) \*\)\(([^;]*?\+ 0x[0-9a-f]+)\) = ' + NUMBER + ';', text):
            field, value = match.group(1).replace(' ', ''), int(match.group(2), 0)
            stores[field].add(value)
            base = re.match(r'(\w+)\+0x([0-9a-f]+)$', field)
            header = base and f'{base.group(1)}+0x{int(base.group(2), 16) - 4:x}'
            if header in cleared and 0x100 <= value <= 0xFFFF:
                sites[value]['stored'].add(name)
                sites[value]['send_sizes'].add(cleared[header])
        for values in stores.values():
            if len(values) > 1 and (opcodes is None or not values <= opcodes):
                continue
            for value in values:
                if opcodes is None or value in opcodes:
                    sites[value]['stored'].add(name)
        for match in re.finditer(r'\blocal_([0-9a-f]+) = ' + NUMBER + ';', text):
            offset, value = int(match.group(1), 16), int(match.group(2), 0)
            if opcodes is not None and value not in opcodes:
                continue
            size_local = f'local_{offset + 4:x}'
            call = re.search(r'\w+\(&?' + size_local + r',\s*(0x[0-9a-f]+|\d+)\)', text)
            if not call and NATIVE_SEND + '(' in text:
                # Some send calls decompile without arguments; the header is
                # then sized by the memset that clears it just before.
                call = re.search(r'_memset\(&?' + size_local + r',\s*0,\s*(0x[0-9a-f]+|\d+)\)', text)
            if call:
                sites[value]['send'].add(name)
                sites[value]['send_sizes'].add(int(call.group(1), 0))
    return sites


# ---------------------------------------------------------------- client ---

def client_constants():
    """Name -> value for MSG_*_Opcode and namespaced *Opcode constants."""
    raw, aliases = {}, {}
    for path in tracked('tmproject/TMProject748', ('.h', '.cpp')):
        if 'Dependencies' in path.parts:
            continue
        for match in re.finditer(r'\b(?:constexpr|const|#define)\s+(?:auto|unsigned short|unsigned int|int|short)?\s*'
                                 r'(\w*Opcode\w*)\s*=?\s*(0x[0-9A-Fa-f]+|\d+|\w+)', strip_comments(read(path))):
            name, value = match.groups()
            if re.match(r'0x|\d', value):
                raw[name] = int(value, 0)
            else:
                aliases[name] = value
    for _ in range(5):
        for name, target in aliases.items():
            if target in raw:
                raw[name] = raw[target]
    return raw


def client_sizes():
    """Struct name -> size from static_assert(sizeof(X) == N)."""
    sizes = {}
    for path in tracked('tmproject/TMProject748', ('.h',)):
        for match in re.finditer(r'static_assert\(\s*sizeof\(\s*([\w:]+)\s*\)\s*==\s*(0x[0-9A-Fa-f]+|\d+)', read(path)):
            sizes[match.group(1).split('::')[-1]] = int(match.group(2), 0)
    return sizes


def compiled_sizes(struct_names, extra_headers=()):
    """Exact sizeof() of client structs, asked from the compiler.

    A probe including Basedef.h and the wire headers is compiled and run with
    the Debug|Win32 options MSBuild recorded (see include-prune.py), so the
    sizes are the ones the client really uses, packing included."""
    import importlib.util
    spec = importlib.util.spec_from_file_location('prune', Path(__file__).with_name('include-prune.py'))
    prune = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(prune)
    env = prune.compiler_environment()
    commands = prune.recorded_commands()
    options = commands[next(s for s in commands if s.endswith(prune.os.path.normcase(r'\pch.cpp')))]
    options = [o for o in options if not re.match(r'/(?:Fo|Fd|Fp|Yc|Yu|ZI|Z7|Zi|Gm|JMC|c$)', o, re.I)]
    work = prune.WORK
    work.mkdir(parents=True, exist_ok=True)
    headers = sorted({p.name for p in (CLIENT / 'internal/wire').glob('*.h')}) + list(extra_headers)
    probe = ['#include "pch.h"', '#include "Basedef.h"'] + [f'#include "{h}"' for h in headers] + \
            ['#include <cstdio>', 'int main()', '{']
    probe += [f'    std::printf("%s %u\\n", "{name}", static_cast<unsigned>(sizeof({name})));'
              for name in sorted(struct_names)]
    probe += ['    return 0;', '}']
    source = work / 'size_probe.cpp'
    source.write_text('\n'.join(probe) + '\n', encoding='utf-8')
    exe = work / 'size_probe.exe'
    # Unknown or ambiguous names are dropped one at a time until the probe builds.
    names = set(struct_names)
    for _ in range(len(names) + 1):
        response = work / 'probe.rsp'
        response.write_text(' '.join(options + [f'/Fe"{exe}"', f'/Fo"{work / "size_probe.obj"}"', f'"{source}"']),
                            encoding='utf-8')
        compiler = prune.shutil.which('cl.exe', path=env.get('Path') or env.get('PATH'))
        result = prune.subprocess.run([compiler, f'@{response}'], env=env, capture_output=True, cwd=work)
        if result.returncode == 0:
            break
        log = (result.stdout + result.stderr).decode('mbcs', 'replace')
        bad = {n for n in names if re.search(r"'" + re.escape(n) + r"'", log)}
        if not bad:
            raise SystemExit('Size probe failed:\n' + log[-3000:])
        names -= bad
        source.write_text('\n'.join(l for l in source.read_text(encoding='utf-8').split('\n')
                                    if not any(f'"{n}"' in l for n in bad)) + '\n', encoding='utf-8')
    output = prune.subprocess.run([str(exe)], capture_output=True, text=True).stdout
    return {name: int(size) for name, size in (line.split() for line in output.splitlines() if line.strip())}


def client_usage(constants, struct_sizes):
    """Opcode -> receive/send sites and struct types in the TMProject client."""
    usage = defaultdict(lambda: {'recv': set(), 'send': set(), 'gate_size': None, 'send_size': None,
                                 'gate_type': None, 'send_type': None})
    names = sorted(constants, key=len, reverse=True)
    name_pattern = re.compile(r'(?:\w+::)?\b(' + '|'.join(map(re.escape, names)) + r')\b')
    for path in tracked('tmproject/TMProject748/internal', ('.cpp', '.h')):
        text = strip_comments(read(path))
        lines = text.split('\n')
        rel = path.relative_to(CLIENT).as_posix()
        for line in lines:
            # Some handlers compare or assign the Type field with a literal
            # opcode, in hexadecimal or decimal.
            for literal in re.findall(r'\bType\s*[!=]=\s*(0x[0-9A-Fa-f]+|\d{3,5})\b', line):
                usage[int(literal, 0)]['recv'].add(rel)
            for literal in re.findall(r'\.\s*Type\s*=\s*(0x[0-9A-Fa-f]+|\d{3,5})\s*;', line):
                usage[int(literal, 0)]['send'].add(rel)
        # Literal case labels count only inside a switch over a Type field.
        for match in re.finditer(r'switch\s*\([^()]*\bType\s*\)\s*\{', text):
            depth, end = 1, match.end()
            while end < len(text) and depth:
                depth += (text[end] == '{') - (text[end] == '}')
                end += 1
            for literal in re.findall(r'\bcase\s+(0x[0-9A-Fa-f]+|\d{3,5})\s*:', text[match.end():end]):
                usage[int(literal, 0)]['recv'].add(rel)
        for index, line in enumerate(lines):
            for match in name_pattern.finditer(line):
                name = match.group(1)
                value = constants[name]
                before = line[:match.start()].rstrip()
                if before.endswith('case') or before.endswith('==') or before.endswith('!='):
                    usage[value]['recv'].add(rel)
                    returned = re.search(r'return\s+sizeof\(\s*([\w:]+)\s*\)', line)
                    if returned and 'ReceivedPacketDispatch' in rel:
                        usage[value]['gate_type'] = returned.group(1)
                elif re.search(r'\.\s*Type\s*=$', before):    # x.Type / x.Header.Type, not pStd->Type
                    usage[value]['send'].add(rel)
                    variable = re.search(r'(\w+)\s*(?:\.|->)\s*(?:Header\s*\.\s*)?Type\s*=$', before)
                    if variable and usage[value]['send_type'] is None:
                        declaration = re.compile(r'\b([A-Za-z_][\w:]*)\s*\*?\s*' + re.escape(variable.group(1))
                                                 + r'\s*(?:\{|=|;|\[)')
                        for back in range(index, max(index - 40, -1), -1):
                            found = declaration.search(lines[back])
                            if found and found.group(1) not in ('auto', 'return', 'const', 'struct') and \
                                    re.match(r'(MSG_|STRUCT_|\w+::)', found.group(1)):
                                usage[value]['send_type'] = found.group(1)
                                break
    return usage


# ---------------------------------------------------------------- server ---

def server_constants():
    """Go name -> value for wire opcodes and integer constants used as sizes."""
    constants, comments = {}, {}
    for path in tracked('wydgo748/internal', ('.go',)):
        if path.name.endswith('_test.go'):
            continue
        for match in re.finditer(r'^\s*(\w+)\s*(?:uint16|int|uint32)?\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*(?://\s*(.*))?$',
                                 read(path), re.M):
            name, value, comment = match.groups()
            constants[name] = int(value, 0)
            if name.startswith('Op'):
                comments[name] = (comment or '').strip()
    return constants, comments


def server_usage(constants):
    usage = defaultdict(lambda: {'handled': False, 'size': None, 'send': set(), 'name': None})
    ops = {name: value for name, value in constants.items() if name.startswith('Op')}
    for name, value in ops.items():
        usage[value]['name'] = name
    security = read(SERVER / 'internal/game/security.go')
    body = re.search(r'func exactInboundPacketSize\(.*?\n}\n', security, re.S).group(0)
    for case, result in re.findall(r'case ([^:]+):\s*(?://[^\n]*\n\s*)*return ([^,]+), true', body):
        size = result.strip()
        size = int(size, 0) if re.match(r'0x|\d', size) else constants.get(size.split('.')[-1])
        for op in re.findall(r'wire\.(\w+)', case):
            if op in ops:
                usage[ops[op]]['size'] = size
    for path in tracked('wydgo748/internal', ('.go',)):
        if path.name.endswith('_test.go') or path.name == 'opcodes.go' or 'loadtest' in path.parts:
            continue
        rel = path.relative_to(SERVER).as_posix()
        for line in strip_comments(read(path)).split('\n'):
            refs = [r for r in re.findall(r'\b(?:wire\.)?(Op\w+)\b', line) if r in ops]
            if not refs:
                continue
            if line.strip().startswith('case ') and path.name == 'world.go':
                for ref in refs:
                    usage[ops[ref]]['handled'] = True
            elif re.search(r'\b(Build|StandardParm2?|PutHeader)\s*\(|\bType:\s*', line):
                for ref in refs:
                    usage[ops[ref]]['send'].add(rel)
    return usage


# ---------------------------------------------------------------- matrix ---

def size_gaps(native_size, client_gate_size, server_size, client_recv, server_recv):
    """D when two known receive sizes differ; U when a receiving side's size is unknown.

    An unknown size leaves that contract uncompared, so it is reported, never
    treated as a match (and never inferred to clear the marker).
    """
    gaps = []
    known = {v for v in (native_size, client_gate_size, server_size) if v}
    if len(known) > 1:
        gaps.append('D')
    if (client_recv and not client_gate_size) or (server_recv and not server_size):
        gaps.append('U')
    return gaps



def build():
    gate = native_gate()
    constants = client_constants()
    server_consts, comments = server_constants()
    known = set(gate) | set(constants.values()) | {v for k, v in server_consts.items() if k.startswith('Op')}
    sites = native_corpus(opcodes=known)
    client = client_usage(constants, client_sizes())
    types = {t for info in client.values() for t in (info['gate_type'], info['send_type']) if t}
    sizes = compiled_sizes(types)
    for info in client.values():
        info['gate_size'] = sizes.get(info['gate_type'])
        info['send_size'] = sizes.get(info['send_type'])
    server = server_usage(server_consts)
    names = defaultdict(set)
    for name, value in constants.items():
        names[value].add(name)
    for value, info in server.items():
        if info['name']:
            names[value].add(info['name'])
    opcodes = sorted(set(gate) | {v for v in sites if sites[v]['recv'] or sites[v]['send'] or sites[v]['stored']}
                     | {v for v in client if client[v]['recv'] or client[v]['send']}
                     | {v for v in server if server[v]['handled'] or server[v]['size'] or server[v]['send']})
    rows = []
    build.native_sites = sites
    for op in opcodes:
        n, c, s = sites[op], client[op], server[op]
        # Native: validated by the size gate or received/sent by native code.
        native = op in gate or bool(n['recv']) or bool(n['send']) or bool(n['stored'])
        native_send = bool(n['send']) or bool(n['stored'])
        client_send, client_recv = bool(c['send']), bool(c['recv'])
        server_recv = s['handled'] or s['size'] is not None
        server_send = bool(s['send'])
        gaps = []
        if (native_send or client_send) and not server_recv:
            gaps.append('A')
        if not native:
            gaps.append('B')
        if native and ((n['recv'] and not client_recv) or (native_send and not client_send)):
            gaps.append('C')
        native_send_size = min(n['send_sizes']) if len(n['send_sizes']) == 1 else None
        # Only receive contracts are compared. Send sizes are the size of the
        # buffer the sender builds in; variable frames are cut before sending
        # (attack frames are built in one 96-byte buffer and sent as 48/52/96),
        # so they stay informational.
        gaps.extend(size_gaps(gate.get(op), c['gate_size'], s['size'], client_recv, server_recv))
        if server_send and not client_recv:
            gaps.append('E')
        rows.append({
            'opcode': f'0x{op:X}', 'names': ', '.join(sorted(names[op])) or '-',
            'native_size': gate.get(op, ''), 'native_recv': len(n['recv']), 'native_send': len(n['send']),
            'native_send_sizes': '/'.join(map(str, sorted(n['send_sizes']))),
            'native_recv_functions': ' '.join(sorted(n['recv'])[:4]),
            'native_send_functions': ' '.join(sorted(n['send'] | n['stored'])[:4]),
            'client_recv': client_recv, 'client_send': client_send,
            'client_gate_size': c['gate_size'] or '', 'client_send_size': c['send_size'] or '',
            'server_recv': server_recv, 'server_size': s['size'] or '', 'server_send': server_send,
            'gaps': ''.join(gaps),
        })
    return rows


def write_tsv(rows, path):
    columns = list(rows[0].keys())
    lines = ['\t'.join(columns)] + ['\t'.join(str(row[c]) for c in columns) for row in rows]
    Path(path).write_text('\n'.join(lines) + '\n', encoding='utf-8')


DECISIONS_MARKER = '<!-- Decisions below are maintained by hand and kept on regeneration. -->'


def write_native_export(sites, path):
    """Versioned native evidence: one line per opcode, form and function."""
    lines = ['opcode\tform\tfunction\tsize', f'#native_sha256\t{NATIVE_SHA256}\t\t']
    for op in sorted(sites):
        info = sites[op]
        size = '/'.join(map(str, sorted(info['send_sizes'])))
        for form, functions in (('recv', info['recv']), ('send', info['send']), ('stored', info['stored'])):
            for function in sorted(functions):
                lines.append(f'0x{op:X}\t{form}\t{function}\t{size if form == "send" else ""}')
    Path(path).write_text('\n'.join(lines) + '\n', encoding='utf-8')


def write_markdown(rows, path):
    counts = defaultdict(int)
    for row in rows:
        for gap in row['gaps']:
            counts[gap] += 1
    yes = lambda v: 'yes' if v else '-'
    out = ['# Opcode compatibility matrix', '',
           'Generated by `.agents/research/opcode-matrix.py`; do not edit the table by hand.',
           f'Native client `references/client748/WYD.exe` (SHA-256 `{NATIVE_SHA256[:12]}...`); native '
           'evidence per function is in `.agents/research/client748/exports/opcode-census-native.tsv`. '
           'Sizes are bytes. "Native" means validated by the size gate `FUN_0055890a` or received, '
           'sent or stored by native code.', '',
           '| Gap | Meaning | Rows |', '| --- | --- | --- |',
           f"| A | sent by a client, no server handler | {counts['A']} |",
           f"| B | unknown to the native client | {counts['B']} |",
           f"| C | native, missing in TMProject | {counts['C']} |",
           f"| D | size mismatch | {counts['D']} |",
           f"| E | server emits, TMProject does not handle | {counts['E']} |",
           f"| U | received with an unknown size (size not compared) | {counts['U']} |", '',
           '| Opcode | Names | Native size | Native recv/send sites | TMProject recv | TMProject send | TMProject sizes (gate/send) | Server recv (size) | Server send | Gaps |',
           '| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |']
    for r in rows:
        out.append(f"| {r['opcode']} | {r['names']} | {r['native_size'] or '-'} | {r['native_recv']}/{r['native_send']} | "
                   f"{yes(r['client_recv'])} | {yes(r['client_send'])} | {r['client_gate_size'] or ('?' if r['client_recv'] else '-')}/{r['client_send_size'] or '-'} | "
                   f"{yes(r['server_recv'])}{' (' + str(r['server_size'] or '?') + ')' if r['server_recv'] else ''} | "
                   f"{yes(r['server_send'])} | {r['gaps'] or '-'} |")
    kept = ''
    if Path(path).is_file():
        previous = Path(path).read_text(encoding='utf-8')
        if DECISIONS_MARKER in previous:
            kept = previous[previous.index(DECISIONS_MARKER):]
    Path(path).write_text('\n'.join(out) + '\n\n' + (kept or DECISIONS_MARKER + '\n'), encoding='utf-8')
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest='command', required=True)
    ops = sub.add_parser('opcodes', help='Write every known opcode (for the Ghidra census)')
    ops.add_argument('--out', required=True)
    mat = sub.add_parser('matrix')
    mat.add_argument('--tsv', required=True)
    mat.add_argument('--markdown', required=True)
    mat.add_argument('--native-export', required=True)
    args = parser.parse_args()
    if args.command == 'opcodes':
        values = set(native_gate()) | set(client_constants().values()) | \
            {v for k, v in server_constants()[0].items() if k.startswith('Op')}
        values = sorted(v for v in values if 0x100 <= v <= 0xFFFF)
        Path(args.out).write_text('\n'.join(f'0x{v:x}' for v in values) + '\n', encoding='utf-8')
        print(f'{len(values)} opcodes')
        return
    rows = build()
    write_tsv(rows, args.tsv)
    write_native_export(build.native_sites, args.native_export)
    counts = write_markdown(rows, args.markdown)
    print(f'{len(rows)} opcodes; gaps: ' + ', '.join(f'{k}={counts[k]}' for k in 'ABCDEU'))


if __name__ == '__main__':
    main()
