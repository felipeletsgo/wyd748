"""Extract statement blocks of a large member function into private helpers.

Each extraction names a block by its first line (exact text, after leading
whitespace) inside one method. The block must be a single statement: an
'if'/'else if'/'else' body or a braced compound statement. The tool:

  1. moves the block's statements into 'void Owner::Helper(params)' placed
     right after the method, and replaces the block body with 'Helper(args);';
  2. declares the helper as a private member in the class header;
  3. verifies by re-inlining that the method is token-identical to the
     original once each call is replaced by the helper body.

Safety rules enforced before writing:
  * no 'return', 'break', 'continue' or 'goto' inside the block (outside
    lambdas), so control flow cannot change;
  * every identifier the block uses that is declared earlier in the method
    (local or parameter) must be a helper parameter with the same name; it is
    passed by reference, preserving aliasing and mutation;
  * parameter types are given explicitly in the spec and checked by the build.

Spec (JSON):
 {"source": "tmproject/.../File.cpp", "header": "tmproject/.../File.h",
  "class": "SGridControl", "method": "int SGridControl::MouseOver(int nCellX, int nCellY, int bPtInRect)",
  "anchor": "private:"   # header line after which declarations are added
  "extractions": [{"helper": "DescribeSkillItem", "after": "if (IsSkill(pItem->m_pItem->sIndex) == 1)",
                   "params": [["SGridControlItem*&", "pItem"], ["char (&)[128]", "szDesc"]]}]}

Usage: python .agents/research/extract-method.py --spec <spec.json> [--write]
"""
import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LITERAL = r'R"(?P<d>[^(\s]*)\([\s\S]*?\)(?P=d)"|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\''
TOKEN = re.compile(LITERAL + r'|\w+|::|->|<<=|>>=|[-+*/%&|^!=<>]=|&&|\|\||<<|>>|\+\+|--|\S')
KEYWORDS = set('''alignas alignof asm auto bool break case catch char char16_t char32_t class const constexpr
const_cast continue decltype default delete do double dynamic_cast else enum explicit export extern false float
for friend goto if inline int long mutable namespace new noexcept nullptr operator private protected public
register reinterpret_cast return short signed sizeof static static_assert static_cast struct switch template this
throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while'''.split())


def strip(text):
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL,
                  lambda m: m.group() if m.group()[0] in '"\'R' else ' ', text)


def tokens(text):
    return [m.group() for m in TOKEN.finditer(strip(text).replace('\\\n', ' '))]


def matching_brace(text, open_index):
    """Index of the '}' matching text[open_index] == '{' (comments/literals aware)."""
    masked = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL, lambda m: ' ' * len(m.group()), text)
    depth = 0
    for i in range(open_index, len(masked)):
        if masked[i] == '{':
            depth += 1
        elif masked[i] == '}':
            depth -= 1
            if depth == 0:
                return i
    raise ValueError('Unbalanced braces')


def matching_paren(text, open_index):
    """Index of the ')' matching text[open_index] == '(' (comments/literals aware)."""
    masked = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL, lambda m: ' ' * len(m.group()), text)
    depth = 0
    for i in range(open_index, len(masked)):
        if masked[i] == '(':
            depth += 1
        elif masked[i] == ')':
            depth -= 1
            if depth == 0:
                return i
    raise ValueError('Unbalanced parentheses')


def method_span(text, signature):
    start = text.find(signature)
    if start < 0 or text.find(signature, start + 1) >= 0:
        raise ValueError('Method signature must occur exactly once: ' + signature)
    open_brace = text.index('{', start + len(signature))
    return start, open_brace, matching_brace(text, open_brace)


TYPE_WORDS = {'const', 'unsigned', 'signed', 'short', 'long', 'int', 'char', 'float', 'double', 'bool', 'auto',
              'void', 'struct', 'static', 'volatile', 'wchar_t'}
NOT_TYPE_START = {'return', 'else', 'case', 'goto', 'new', 'delete', 'throw', 'if', 'while', 'for', 'switch',
                  'do', 'sizeof', 'default'}


def declarations(toks):
    """Yield (index, name, for_init) for local declarations and parameters.

    A declaration is a statement that starts at a boundary ('{', '}', ';', or
    the opening '(' of a 'for'/parameter list), continues with type tokens only
    (identifiers, type keywords, '::', '<', '>', '*', '&', ','), and names an
    identifier followed by '=', ';', '[', '{', ',', ')' or '('.
    """
    for i in range(1, len(toks) - 1):
        name, nxt = toks[i], toks[i + 1]
        if not re.match(r'^[A-Za-z_]\w*$', name) or name in KEYWORDS:
            continue
        if nxt not in ('=', ';', '[', '{', ',', ')', '(', ':'):
            continue
        j = i - 1
        depth_angle = 0
        while j >= 0:
            t = toks[j]
            if t in ('{', '}', ';'):
                break
            if t == '(':
                break
            if t == '>':
                depth_angle += 1
            elif t == '<':
                depth_angle -= 1
                if depth_angle < 0:      # 'a < b' comparison, not a template type
                    j = -2
                    break
            elif depth_angle == 0 and t == ',':
                break
            elif not (re.match(r'^[A-Za-z_]\w*$', t) or t in ('::', '*', '&', '<', '>')):
                j = -2
                break
            j -= 1
        if j == -2:
            continue
        type_tokens = toks[j + 1:i]
        if not type_tokens or type_tokens[0] in NOT_TYPE_START or any(t in NOT_TYPE_START for t in type_tokens):
            continue
        if not any(re.match(r'^[A-Za-z_]\w*$', t) for t in type_tokens):
            continue
        boundary = toks[j] if j >= 0 else '{'   # start of the token stream is a statement start
        if boundary == '(':
            opener = toks[j - 1] if j >= 1 else ''
            if opener not in ('for',) and not re.match(r'^[A-Za-z_]\w*$', opener):
                continue
            yield i, name, opener == 'for'
        elif boundary == ',' :
            continue  # multi-declarators are reported through all_declared()
        else:
            if nxt == '(' and type_tokens[-1] not in ('auto',) and len(type_tokens) == 1 and \
                    type_tokens[0][0].islower() and type_tokens[0] not in TYPE_WORDS:
                continue  # 'foo bar(' is a call-like statement, not a declaration
            yield i, name, False


def declared_names(tokens_before):
    """Names declared in scopes that are still open at the end of tokens_before."""
    open_scopes = [set()]
    pending_for = set()
    decl_at = {i: (name, for_init) for i, name, for_init in declarations(tokens_before)}
    paren = 0
    for i, t in enumerate(tokens_before):
        if i in decl_at:
            name, for_init = decl_at[i]
            (pending_for if for_init else open_scopes[-1]).add(name)
        if t == '(':
            paren += 1
        elif t == ')':
            paren -= 1
        elif t == '{':
            open_scopes.append(set(pending_for))
            pending_for = set()
        elif t == '}':
            if len(open_scopes) > 1:
                open_scopes.pop()
        elif t == ';' and paren == 0 and pending_for:
            pending_for = set()   # braceless for body ended
    names = set().union(*open_scopes) | pending_for
    return names


def signature_parameters(signature):
    """Names of a method's parameters (last identifier of each top-level part)."""
    inner = signature[signature.index('(') + 1:signature.rindex(')')]
    names, depth, part = set(), 0, ''
    for ch in inner + ',':
        if ch in '(<[':
            depth += 1
        elif ch in ')>]':
            depth -= 1
        if ch == ',' and depth == 0:
            words = re.findall(r'[A-Za-z_]\w*', part.split('=')[0])
            if words and words[-1] not in KEYWORDS:
                names.add(words[-1])
            part = ''
        else:
            part += ch
    return names


def all_declared(method_tokens):
    """Every name declared anywhere in the method, scope-insensitive."""
    names = {name for _, name, _ in declarations(method_tokens)}
    paren = 0
    braces = []          # True for braces that open an initializer list
    for i, t in enumerate(method_tokens[:-1]):
        if t == '(':
            paren += 1
        elif t == ')':
            paren -= 1
        elif t == '{':
            braces.append(i > 0 and method_tokens[i - 1] in ('=', ',', '{', ']'))
        elif t == '}':
            if braces:
                braces.pop()
        elif t == ',' and paren == 0 and not any(braces) and re.match(r'^[A-Za-z_]\w*$', method_tokens[i + 1]) and \
                i + 2 < len(method_tokens) and method_tokens[i + 2] in ('=', ';', '[', ','):
            names.add(method_tokens[i + 1])
    return names - KEYWORDS


def used_names(block_tokens):
    used = set()
    for i, t in enumerate(block_tokens):
        if re.match(r'^[A-Za-z_]\w*$', t) and t not in KEYWORDS:
            if i and block_tokens[i - 1] in ('.', '->', '::'):
                continue
            used.add(t)
    return used


SCALAR_RESULTS = {'int', 'unsigned int', 'HRESULT', 'DWORD', 'BOOL', 'bool', 'char', 'short', 'long',
                  'unsigned short', 'unsigned char', 'LRESULT', 'float', 'UINT'}
FLOW_ENUM = 'ExtractedFlow'
FLOW_NAMES = ('extractedFlow', 'extractedResult')


def escape_positions(block_tokens):
    """(index, kind) of every return/goto and every break/continue that leaves the block."""
    found = []
    scopes = []
    pending = None
    paren = 0
    for i, t in enumerate(block_tokens):
        if t in ('for', 'while', 'switch', 'do'):
            if t == 'while' and i and block_tokens[i - 1] == '}':
                continue
            pending = 'switch' if t == 'switch' else 'loop'
            paren = 0
            continue
        if t == '(':
            paren += 1
        elif t == ')':
            paren -= 1
        elif t == '{':
            scopes.append(pending if pending and paren == 0 else 'block')
            pending = None
        elif t == '}':
            if scopes:
                scopes.pop()
        elif t == ';' and pending and paren == 0:
            pending = None
        if t in ('return', 'goto'):
            found.append((i, t))
        elif t == 'break' and not any(s in ('loop', 'switch') for s in scopes):
            found.append((i, t))
        elif t == 'continue' and 'loop' not in scopes:
            found.append((i, t))
    return found


def has_lambda(block_tokens):
    return any(t == '[' and i + 1 < len(block_tokens) and block_tokens[i + 1] in ('&', '=', ']')
               and (i == 0 or block_tokens[i - 1] in ('(', ',', '=', 'return', '{', ';'))
               for i, t in enumerate(block_tokens))


def spanned_tokens(text):
    """Tokens of text with their (start, end) offsets; comments are skipped."""
    out = []
    masked = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/', lambda m: ' ' * len(m.group()), text)
    for m in TOKEN.finditer(masked):
        out.append((m.group(), m.start(), m.end()))
    return out


def flow_rewrite(inner, valued):
    """Rewrite the escaping statements of a block into ExtractedFlow returns."""
    spans = spanned_tokens(inner)
    toks = [s[0] for s in spans]
    if toks != tokens(inner):
        raise ValueError('flow mode does not support line continuations in the block')
    edits = []
    for index, kind in escape_positions(toks):
        end = index + 1
        depth = 0
        while end < len(toks) and not (toks[end] == ';' and depth == 0):
            depth += toks[end] in '([{'
            depth -= toks[end] in ')]}'
            end += 1
        start_off, end_off = spans[index][1], spans[end][2]
        if kind == 'return' and end > index + 1:
            expression = inner[spans[index + 1][1]:spans[end - 1][2]]
            replacement = f'{{ extractedResult = {expression}; return {FLOW_ENUM}::Return; }}'
        elif kind == 'return':
            replacement = f'return {FLOW_ENUM}::Return;'
        else:
            replacement = f'return {FLOW_ENUM}::{kind.capitalize()};'
        edits.append((start_off, end_off, replacement))
    for start_off, end_off, replacement in sorted(edits, reverse=True):
        inner = inner[:start_off] + replacement + inner[end_off:]
    return inner


def flow_inverse(helper_tokens):
    """Undo flow_rewrite at token level. Returns (tokens, produced) where
    produced lists (index, kind) of each restored escape statement."""
    toks = list(helper_tokens)
    tail = ['return', FLOW_ENUM, '::', 'Next', ';']
    if toks[-5:] == tail:
        toks = toks[:-5]
    out, produced, i = [], [], 0
    closing = [';', 'return', FLOW_ENUM, '::', 'Return', ';', '}']
    while i < len(toks):
        if toks[i:i + 3] == ['{', 'extractedResult', '=']:
            j = i + 3
            while j < len(toks) and toks[j:j + 7] != closing:
                j += 1
            if j >= len(toks):
                raise ValueError('Verification failed: malformed flow return')
            produced.append((len(out), 'return'))
            out += ['return'] + toks[i + 3:j] + [';']
            i = j + 7
            continue
        if toks[i:i + 3] == ['return', FLOW_ENUM, '::'] and i + 4 < len(toks) and toks[i + 4] == ';':
            kind = {'Return': 'return', 'Continue': 'continue', 'Break': 'break'}.get(toks[i + 3])
            if not kind:
                raise ValueError('Verification failed: unexpected flow value ' + toks[i + 3])
            produced.append((len(out), kind))
            out += [kind, ';'] if kind != 'return' else ['return', ';']
            i += 5
            continue
        out.append(toks[i])
        i += 1
    return out, produced


def control_escapes(block_tokens):
    """Statements that could leave the block: any return/goto, and any
    break/continue not enclosed by a loop or switch that starts inside it."""
    bad = []
    scopes = []          # kind of each open brace: 'loop', 'switch' or 'block'
    pending = None       # loop/switch keyword waiting for its body brace
    paren = 0
    for i, t in enumerate(block_tokens):
        if t in ('for', 'while', 'switch', 'do'):
            # 'while' after a 'do' body closes that loop; it opens no new body.
            if t == 'while' and i and block_tokens[i - 1] == '}':
                continue
            pending = 'switch' if t == 'switch' else 'loop'
            paren = 0
            continue
        if t == '(':
            paren += 1
        elif t == ')':
            paren -= 1
        elif t == '{':
            scopes.append(pending if pending and paren == 0 else 'block')
            pending = None
        elif t == '}':
            if scopes:
                scopes.pop()
        elif t == ';' and pending and paren == 0:
            pending = None   # braceless loop body ended; be conservative below
        if t in ('return', 'goto'):
            bad.append(t)
        elif t == 'break' and not any(s in ('loop', 'switch') for s in scopes):
            bad.append(t)
        elif t == 'continue' and 'loop' not in scopes:
            bad.append(t)
    return bad


def run(spec, write):
    source_path, header_path = ROOT / spec['source'], ROOT / spec['header']
    raw = source_path.read_bytes()
    bom = raw.startswith(b'\xef\xbb\xbf')
    text = raw.decode('utf-8-sig')
    crlf = '\r\n' in text
    text = text.replace('\r\n', '\n')
    original_text = text
    cls = spec['class']
    m_start, m_open, m_close = method_span(text, spec['method'])
    original_method = text[m_start:m_close + 1]
    signature_tokens = tokens(spec['method'])
    helpers = []
    replacements = []
    def line_offset(anchor, nth):
        body_lines = text[m_open:m_close + 1].split('\n')
        lines = [i for i, line in enumerate(body_lines) if line.strip() == anchor]
        if nth is not None:
            if nth >= len(lines):
                raise ValueError(f"Anchor {anchor!r} has only {len(lines)} matches")
            lines = [lines[nth]]
        if len(lines) != 1:
            raise ValueError(f"Anchor must match exactly one line in the method: {anchor!r} ({len(lines)})")
        return m_open + sum(len(l) + 1 for l in body_lines[:lines[0]])

    for ex in spec['extractions']:
        if 'from' in ex:
            # Statement range: from the 'from' line up to (not including) 'until'.
            if ex.get('returns'):
                raise ValueError(f"{ex['helper']}: ranges cannot use 'returns'")
            open_brace = line_offset(ex['from'], ex.get('nth_from')) - 1   # the newline before the range
            close_brace = line_offset(ex['until'], ex.get('nth_until'))
            if close_brace <= open_brace + 1:
                raise ValueError(f"{ex['helper']}: 'until' must follow 'from'")
            inner = text[open_brace + 1:close_brace]
            range_tokens = tokens(inner)
            balance = {'{': 0, '(': 0, '[': 0}
            for t in range_tokens:
                if t in '{([':
                    balance[t] += 1
                elif t in '})]':
                    balance[{'}': '{', ')': '(', ']': '['}[t]] -= 1
                if any(v < 0 for v in balance.values()):
                    break
            if any(balance.values()) or not range_tokens or range_tokens[0] == 'else':
                raise ValueError(f"{ex['helper']}: range is not a sequence of complete statements")
            if range_tokens[-1] not in (';', '}'):
                raise ValueError(f"{ex['helper']}: range must end with a complete statement")
            after_tokens = tokens(text[close_brace:m_close + 1])
            leaked = sorted(all_declared(range_tokens) & used_names(after_tokens))
            if leaked:
                raise ValueError(f"{ex['helper']}: names declared in the range are used after it: {leaked}")
        else:
            offset = line_offset(ex['after'], ex.get('nth'))
            line_end = text.index('\n', offset)
            if ex['after'].count('(') > ex['after'].count(')'):
                # The condition continues on later lines: the block starts on
                # the line after the parenthesis that closes it.
                line_end = text.index('\n', matching_paren(text, text.index('(', offset)))
            open_brace = text.index('{', line_end)
            if text[line_end:open_brace].strip():
                raise ValueError(f"Block after {ex['after']!r} must start with a brace on the next line")
            close_brace = matching_brace(text, open_brace)
            inner = text[open_brace + 1:close_brace]
        inner_tokens = tokens(inner)
        escapes = control_escapes(inner_tokens)
        if ex.get('returns'):
            # Return-terminated handler: every path must end in a return, so the
            # block may become 'return Helper(...);' with the method's type.
            if not ends_with_return(inner_tokens):
                raise ValueError(f"{ex['helper']}: 'returns' needs a final top-level return statement")
            escapes = [e for e in escapes if e != 'return']
        if ex.get('flow'):
            if 'from' in ex or ex.get('returns'):
                raise ValueError(f"{ex['helper']}: 'flow' applies to blocks only")
            if 'goto' in escapes or has_lambda(inner_tokens):
                raise ValueError(f"{ex['helper']}: flow mode rejects goto and lambdas")
            if not escapes:
                raise ValueError(f"{ex['helper']}: block has no escaping statement; omit 'flow'")
            method_names = set(tokens(original_method))
            if method_names & set(FLOW_NAMES):
                raise ValueError(f"{ex['helper']}: the method already uses {sorted(method_names & set(FLOW_NAMES))}")
            positions = escape_positions(inner_tokens)
            valued = any(k == 'return' and inner_tokens[i + 1] != ';' for i, k in positions)
            rtype = _method_return_type(spec)
            if valued and rtype not in SCALAR_RESULTS:
                raise ValueError(f"{ex['helper']}: flow returns need a scalar method type, not {rtype!r}")
            ex['_flow'] = {'kinds': {k for _, k in positions}, 'valued': valued, 'rtype': rtype}
            escapes = []
        if escapes:
            raise ValueError(f"{ex['helper']}: block contains {sorted(set(escapes))}")
        before_tokens = signature_tokens + tokens(text[m_open:open_brace])
        locals_before = declared_names(before_tokens) | signature_parameters(spec['method'])
        needed = sorted(used_names(inner_tokens) & locals_before)
        params = [p[1] for p in ex['params']]
        missing = [n for n in needed if n not in params]
        if missing:
            raise ValueError(f"{ex['helper']}: block uses outer locals that are not parameters: {missing}")
        extra = [p for p in params if p not in used_names(inner_tokens)]
        if extra:
            raise ValueError(f"{ex['helper']}: parameters not used by the block: {extra}")
        # Safety net: a name declared anywhere else in the method but used here
        # without being passed might resolve to a member or global inside the
        # helper. Each such name must be confirmed explicitly in 'not_locals'.
        method_tokens = signature_tokens + tokens(text[m_open:open_brace]) + tokens(text[close_brace:m_close + 1])
        shadow = sorted((used_names(inner_tokens) & (all_declared(method_tokens) | signature_parameters(spec['method'])))
                        - set(params) - all_declared(inner_tokens) - set(ex.get('not_locals', [])))
        if shadow:
            raise ValueError(f"{ex['helper']}: names declared elsewhere in the method need review "
                             f"(pass them or list them in not_locals): {shadow}")
        replacements.append((open_brace, close_brace, ex, inner))
    # Apply from the bottom so offsets stay valid.
    for open_brace, close_brace, ex, inner in sorted(replacements, key=lambda r: -r[0]):
        if 'from' in ex:
            # The range's own lines are one level deeper than the block form;
            # record the enclosing indent so helper bodies dedent the same way.
            indent = re.match(r'[ \t]*', text[open_brace + 1:]).group()
            text = text[:open_brace + 1] + f"{indent}{_call(ex)}\n" + text[close_brace:]
            helpers.append((ex, inner, indent[:-1] if indent.endswith('\t') else indent))
            continue
        indent = re.match(r'[ \t]*', text[text.rfind('\n', 0, open_brace) + 1:]).group()
        if ex.get('_flow'):
            ex['_snippet'] = _flow_call(ex, indent)
            text = text[:open_brace + 1] + ex['_snippet'] + text[close_brace:]
            helpers.append((ex, inner, indent))
            continue
        call = f"{indent}\t{_call(ex)}\n{indent}"
        text = text[:open_brace + 1] + '\n' + call + text[close_brace:]
        helpers.append((ex, inner, indent))
    m_start, m_open, m_close = method_span(text, spec['method'])
    helper_text = ''
    for ex, inner, indent in sorted(helpers, key=lambda h: spec['extractions'].index(h[0])):
        params = ', '.join(_param(t, n) for t, n in _helper_params(ex))
        source_body = inner
        if ex.get('_flow'):
            source_body = flow_rewrite(inner, ex['_flow']['valued'])
            if not ends_with_return(tokens(inner)):
                source_body = source_body.rstrip() + f"\n{indent}\treturn {FLOW_ENUM}::Next;\n"
        body_lines = source_body.strip('\n').split('\n')
        shift = len(indent) + 1
        dedented = '\n'.join(l[shift - 1:] if l.startswith(indent + '\t') else l.lstrip() if not l.strip() else l
                             for l in body_lines)
        comment = f"// Extracted from {spec['method'].split('(')[0].split()[-1]}; behavior is unchanged."
        helper_text += f"\n{comment}\n{_return_type(spec, ex)} {cls}::{ex['helper']}({params})\n{{\n{dedented}\n}}\n"
    text = text[:m_close + 1] + '\n' + helper_text.rstrip('\n') + '\n' + text[m_close + 1:]

    # Verification: inline every call back and compare tokens with the original.
    m_start, m_open, m_close = method_span(text, spec['method'])
    rebuilt = text[m_start:m_close + 1]
    for ex, inner, indent in helpers:
        if ex.get('_flow'):
            if rebuilt.count(ex['_snippet']) != 1:
                raise ValueError('Flow call not unique: ' + ex['helper'])
            rebuilt = rebuilt.replace(ex['_snippet'], inner)
            continue
        call = _call(ex)
        if rebuilt.count(call) != 1:
            raise ValueError('Call not unique: ' + call)
        rebuilt = rebuilt.replace(call, inner)
    if tokens(rebuilt) != tokens(original_method):
        raise ValueError('Verification failed: re-inlined method differs from the original')
    for ex, inner, indent in helpers:
        head = f"{_return_type(spec, ex)} {cls}::{ex['helper']}({', '.join(_param(t, n) for t, n in _helper_params(ex))})\n{{"
        if text.count(head) != 1:
            raise ValueError('Helper definition not unique: ' + ex['helper'])
        brace = text.index(head) + len(head) - 1
        body = text[brace:matching_brace(text, brace) + 1]
        if ex.get('_flow'):
            restored, produced = flow_inverse(tokens(body)[1:-1])
            if restored != tokens(inner):
                raise ValueError('Verification failed: flow helper does not invert to the block: ' + ex['helper'])
            if sorted(produced) != sorted(escape_positions(tokens(inner))):
                raise ValueError('Verification failed: a rewritten statement was not an escape: ' + ex['helper'])
            continue
        if tokens(body)[1:-1] != tokens(inner):
            raise ValueError('Verification failed: helper body differs: ' + ex['helper'])

    header_raw = header_path.read_bytes()
    hbom = header_raw.startswith(b'\xef\xbb\xbf')
    header = header_raw.decode('utf-8-sig')
    hcrlf = '\r\n' in header
    header = header.replace('\r\n', '\n')
    cls_start = re.search(r'\b(class|struct)\s+' + cls + r'\b[^;{]*\{', header)
    cls_end = matching_brace(header, cls_start.end() - 1)
    anchor = spec.get('anchor', 'private:')
    pos = header.find('\n' + anchor if not anchor.startswith('\t') else anchor, cls_start.end(), cls_end)
    declarations = ''.join(f"\t{_return_type(spec, ex)} {ex['helper']}({', '.join(_param(t, n) for t, n in _helper_params(ex))});\n"
                           for ex in spec['extractions'])
    note = f"\t// Helpers extracted from {spec['method'].split('(')[0].split()[-1]}.\n"
    if pos < 0:
        insert_at = cls_end
        block = '\nprivate:\n' + note + declarations
    else:
        insert_at = header.index('\n', pos + 1) + 1
        block = note + declarations
    header = header[:insert_at] + block + header[insert_at:]
    if any(ex.get('_flow') for ex in spec['extractions']) and '#include "ExtractedFlow.h"' not in header:
        header = header.replace('#pragma once\n', '#pragma once\n#include "ExtractedFlow.h"\n', 1)
    if write:
        out = text.replace('\n', '\r\n') if crlf else text
        source_path.write_bytes((b'\xef\xbb\xbf' if bom else b'') + out.encode('utf-8'))
        hout = header.replace('\n', '\r\n') if hcrlf else header
        header_path.write_bytes((b'\xef\xbb\xbf' if hbom else b'') + hout.encode('utf-8'))
    print(f"{'WROTE' if write else 'OK (dry run)'}: {len(helpers)} helper(s) from {spec['method'].split('(')[0]}; "
          f"method {original_method.count(chr(10)) + 1} -> {text[m_start:m_close + 1].count(chr(10)) + 1} lines")


def ends_with_return(block_tokens):
    """True when the last top-level statement of the block is a return."""
    depth = 0
    last_start = 0
    for i, t in enumerate(block_tokens):
        if t in ('{', '(', '['):
            depth += 1
        elif t in ('}', ')', ']'):
            depth -= 1
            if t == '}' and depth == 0:
                last_start = i + 1
        elif t == ';' and depth == 0 and i + 1 < len(block_tokens):
            last_start = i + 1
    tail = block_tokens[last_start:]
    return bool(tail) and tail[0] == 'return' and tail[-1] == ';'


def _method_return_type(spec):
    head = spec['method'].split('(')[0]
    return ' '.join(head[:head.rindex(spec['class'] + '::')].split())


def _helper_params(ex):
    params = list(ex['params'])
    if ex.get('_flow') and ex['_flow']['valued']:
        params.append([ex['_flow']['rtype'] + '&', 'extractedResult'])
    return params


def _flow_call(ex, indent):
    """Lines that replace a flow block's body: call, then the same jumps."""
    flow = ex['_flow']
    args = ', '.join(n for _, n in _helper_params(ex))
    lines = []
    if flow['valued']:
        lines.append(f"{indent}\t{flow['rtype']} extractedResult{{}};")
    lines.append(f"{indent}\tconst {FLOW_ENUM} extractedFlow = {ex['helper']}({args});")
    if 'return' in flow['kinds']:
        value = ' extractedResult' if flow['valued'] else ''
        lines.append(f"{indent}\tif (extractedFlow == {FLOW_ENUM}::Return)\n{indent}\t\treturn{value};")
    if 'continue' in flow['kinds']:
        lines.append(f"{indent}\tif (extractedFlow == {FLOW_ENUM}::Continue)\n{indent}\t\tcontinue;")
    if 'break' in flow['kinds']:
        lines.append(f"{indent}\tif (extractedFlow == {FLOW_ENUM}::Break)\n{indent}\t\tbreak;")
    return '\n' + '\n'.join(lines) + '\n' + indent


def _return_type(spec, ex):
    if ex.get('_flow'):
        return FLOW_ENUM
    if not ex.get('returns'):
        return 'void'
    head = spec['method'].split('(')[0]
    return head[:head.rindex(spec['class'] + '::')].strip()


def _call(ex):
    args = ', '.join(p[1] for p in ex['params'])
    return f"return {ex['helper']}({args});" if ex.get('returns') else f"{ex['helper']}({args});"


def _param(type_text, name):
    m = re.match(r'^(.*)\(&\)(\[.*\])$', type_text)
    if m:
        return f'{m.group(1).strip()} (&{name}){m.group(2)}'
    return f'{type_text} {name}'


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--spec', required=True, type=Path)
    parser.add_argument('--write', action='store_true')
    args = parser.parse_args()
    try:
        run(json.loads(args.spec.read_text(encoding='utf-8')), args.write)
    except ValueError as error:
        sys.exit(f'REJECTED: {error}')


if __name__ == '__main__':
    main()
