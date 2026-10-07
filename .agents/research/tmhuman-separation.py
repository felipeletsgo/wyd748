"""Reproducible TMHuman relocation manifest and semantic-preservation gate.

Reads the committed baseline, not historical client binaries. --move performs
only a bulk mechanical rewrite of complete definitions and project entries.
Run from the repository root. Generated reports belong in ignored build output.
"""
import argparse
import difflib
import hashlib
import json
import re
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path
import importlib.util as _mc_ilu
_mc_spec = _mc_ilu.spec_from_file_location('method_counts', Path(__file__).with_name('method_counts.py'))
method_counts = _mc_ilu.module_from_spec(_mc_spec)
_mc_spec.loader.exec_module(method_counts)

ROOT = Path(__file__).resolve().parents[2]
ENTITIES = ROOT / 'tmproject/TMProject748/internal/game/entities'
BASE = 'tmproject/TMProject748/internal/game/entities/TMHuman.cpp'
REVISION = '77e8dcc7f1af929be1c6418d08b97c7dd8412713'
GROUPS = {
    'Movement': 'InitPosition InitAngle SetAngle SetPosition MoveTo OnlyMove IsGoMore SetWantAngle GetRoute GenerateRouteTable StraightRouteTable ChangeRouteBuffer SetSpeed',
    'UI': 'IsMouseOver OnCharEvent LabelPosition LabelPosition2 HideLabel SetChatMessage GetChatLen SetInMiniMap UpdateGuildName SetGuildBattleHPColor SetGuildBattleHPBar SetGuildBattleMPBar SetGuildBattleLifeCount CreateControl DestroyControl StrByteCheck _locationCheck',
    'Animation': 'SetAnimation AnimationFrame SetMotion',
    'Combat': 'MoveAttack MoveGet Attack Punched Fire Die Stand PlayAttackSound PlayPunchedSound MAutoAttack',
    'Appearance': 'SetRace SetWeaponType CheckWeapon SetPacketMOBItem SetPacketEquipItem SetColorItem GetLegType GetBloodColor SetCharHeight SetAvatar SetMantua SetCitizenMantle UnSetCitizenMantle SetHumanCostume',
    'Mounts': 'SetImportedMountCostume SetMountCostume UpdateMount GetMyHeight',
    'Render': 'Render SetColorMaterial',
    'Effects': 'RenderEffect FrameMoveEffect FrameMoveEffect_AvatarTrans FrameMoveEffect_AvatarFoema FrameMoveEffect_AvatarBMaster FrameMoveEffect_AvatarHunter SetHandEffect StartKhepraDieEffect RenderEffect_RudolphCostume RenderEffect_Khepra RenderEffect_LegendBerielKeeper RenderEffect_LegendBeriel RenderEffect_Pig_Wolf RenderEffect_DungeonBear RenderEffect_Hydra RenderEffect_DarkNightZombieTroll RenderEffect_DarkElf RenderEffect_Minotauros RenderEffect_EmeraldDragon RenderEffect_BoneDragon RenderEffect_Golem RenderEffect_Skull',
    '': 'TMHuman ~TMHuman InitObject Init FrameMove RestoreDeviceObjects InvalidateDeviceObjects DelayDelete IsMerchant UpdateScore CheckAffect Is2stClass IAmkhepra',
}
OWNERS = {name: group for group, names in GROUPS.items() for name in names.split()}
HEADER = 'tmproject/TMProject748/internal/game/entities/TMHuman.h'
# Reviewed method extractions (Stage 3): written by --pin from extract-method.py specs.
EXTRACTIONS = ROOT / '.agents/research/relocations/tmhuman-extractions.json'


def load_extractions():
    if not EXTRACTIONS.is_file():
        return {'methods': {}, 'helpers': {}, 'declarations': []}
    return json.loads(EXTRACTIONS.read_text(encoding='utf-8'))


# Reviewed changes after the relocation (UI batches): written by --review and
# applied after the extractions. Each method records the digest it replaces
# (null when new) and its reviewed digest (null when removed).
REVIEWS = ROOT / '.agents/research/relocations/tmhuman-reviews.json'


def load_reviews():
    if not REVIEWS.is_file():
        return {'methods': {}, 'header': {'removed': [], 'added': [], 'removed_declarations': []}}
    return json.loads(REVIEWS.read_text(encoding='utf-8'))


for _signature, _pin in load_extractions()['helpers'].items():
    OWNERS[re.search(r'TMHuman::(~?\w+)\(', _signature).group(1)] = _pin['owner']
LITERAL = r'R"(?P<tag>[^ ()\\\t\r\n]{0,16})\([\s\S]*?\)(?P=tag)"|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\''
# Explicit, reviewed second-phase edit, never a blanket exclusion from comparison.
POLICY_SIGNATURE = 'int TMHuman::SetHumanCostume()'
POLICY_BASELINE = '2f0ae9a234b5b160121cb8bf97ecf59cff13876da5b4870e3c02a7d5c3ac7459'
POLICY_METHOD = 'b4acd1fec71ff8bc3a2569c97118e0504199a972bd98e218454d12de92b9bec9'
POLICY_HEADER = 'f6bc15291c692b88e583221858e7803eb9c156dd3ee36e6ef8c04aa08f995768'

def masked(source):
    """Mask comments/literals without changing offsets (including raw strings)."""
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL
    return re.sub(pattern, lambda m: re.sub(r'[^\n]', ' ', m.group()), source)

def methods(source):
    code = masked(source)
    pattern = r'^(?:(?:int|void|bool|float|ExtractedFlow)[ \t]+)?TMHuman::(~?\w+)\([^;{}]*\)\s*\{'
    result = []
    for match in re.finditer(pattern, code, re.M):
        opening = match.end() - 1
        depth = 1
        end = opening + 1
        while end < len(code) and depth:
            depth += (code[end] == '{') - (code[end] == '}')
            end += 1
        if depth:
            raise ValueError('Unterminated definition: ' + match.group(1))
        signature = re.sub(r'\s+', ' ', source[match.start():opening].strip())
        result.append(dict(name=match.group(1), signature=signature,
                           start=match.start(), end=end, text=source[match.start():end]))
    return result

def owner(method):
    name = method['name']
    return 'Packets' if name.startswith('OnPacket') else OWNERS[name]

def read(path):
    return path.read_text(encoding='utf-8-sig')

def digest(text):
    # Preserve all code and literal bytes; only comments and whitespace may differ.
    without_comments = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL,
        lambda m: m.group() if m.group().startswith(('"', "'", 'R"')) else ' ', text)
    tokens = [m.group() for m in re.finditer(LITERAL + r'|\w+|[^\s]', without_comments)]
    return hashlib.sha256(json.dumps(tokens, ensure_ascii=False).encode()).hexdigest()

def outside_methods(source):
    for method in reversed(methods(source)):
        source = source[:method['start']] + source[method['end']:]
    return re.sub(r'^#include[^\n]*\n', '', source, flags=re.M)

def includes(body, baseline):
    dependencies = ['pch.h', 'TMHuman.h', 'TMGlobal.h']
    declaration = masked(read(ENTITIES / 'TMHuman.h'))
    aliases = {
        'TMEffectSkinMesh.h': r'\bTMSkinMesh\b|m_pSkinMesh|m_pMount',
        'ObjectManager.h': r'g_pObjectManager',
        'TMCamera.h': r'g_pCamera|m_pCamera|\bpCamera\b',
        'TMScene.h': r'g_pCurrentScene|m_pParentScene',
        'TMFieldScene.h': r'\bTMFieldScene\b',
        'TMGround.h': r'm_pGround',
        'TMObjectContainer.h': r'm_p\w*Container|m_pHumanList',
        'SControl.h': r'm_p\w*(?:Label|Bar)|\bSControl\b',
        'SControlContainer.h': r'm_pControlContainer',
        'TMFont3.h': r'g_pFont|\bTMFont\w*\b',
        'TMUtil.h': r'\b(?:SendPacket|SendOneMessage|GetSoundAndPlay|GetSoundAndPlayIfNot)\b',
        'TMEffectSWSwing.h': r'm_pSwingEffect|\bTMEffectSWSwing\b',
        'ItemEffect.h': r'\bEF_\w+\b',
        'SGrid.h': r'g_pItemGridXY|m_pGrid\w*|\bSGridControl\b',
        'TMShade.h': r'm_pShade|\bTMShade\b',
        'TMLog.h': r'\bLOG_WRITELOG\w*\b',
        'TMEffectMeshRotate.h': r'm_pMantua|m_pHelm|m_pRotateBone|\bTMEffectMeshRotate\b',
        'TMButterFly.h': r'm_pButterFly|m_pButterfly|\bTMButterFly\b',
        'TMSkillMagicShield.h': r'm_pMagicShield|\bTMSkillMagicShield\b',
        'TMEffectCharge.h': r'm_pCharge|\bTMEffectCharge\b',
        'TMEffectFirework.h': r'\bTMEffectFireWork\b',
        'TMLight.h': r'm_pLight|\bTMLight\b',
        'DeathMotionPolicy.h': r'death_motion::',
        'SkinMotionPolicy.h': r'skin_motion::',
        'HumanAnglePolicy.h': r'human_angle::',
        'BaseCostumeLook.h': r'base_costume748::',
        'AirMoveMotion.h': r'\b(?:Consume|Cancel|Discard|Restore|HasNext)AirMove\w*\b',
        '../../render/mesh/CostumeSelection.h': r'costume748::',
        '../../ui/ResourceBarProjection.h': r'resource_ui::',
        '../../ui/ObservedAffectProjection.h': r'affect_ui::',
    }
    for header in re.findall(r'^#include "([^"]+)"', baseline, re.M):
        stem = Path(header).stem
        fields = re.findall(r'\b' + re.escape(stem) + r'\s*\*\s*(\w+)', declaration)
        field_pattern = '|'.join(r'\b' + re.escape(field) + r'\b' for field in fields)
        pattern = aliases.get(header, r'\b' + re.escape(stem) + r'\b')
        if re.search(pattern + ('|' + field_pattern if field_pattern else ''), body):
            dependencies.append(header)
    return ''.join('#include "' + header + '"\n' for header in dict.fromkeys(dependencies)) + '\n'

def verify(baseline, complete=False, with_policies=False):
    expected = {m['signature']: digest(m['text']) for m in methods(baseline)}
    baseline_digests = dict(expected)
    if len(expected) != len(methods(baseline)):
        raise ValueError('Duplicate baseline signature')
    actual = {}
    entries = []
    for path in sorted(ENTITIES.glob('TMHuman*.cpp')):
        for method in methods(read(path)):
            signature = method['signature']
            if signature in actual:
                raise ValueError('Duplicate: ' + signature)
            actual[signature] = digest(method['text'])
            entries.append(dict(signature=signature, owner=owner(method), file=path.name,
                                lines=method['text'].count('\n') + 1,
                                sha256=actual[signature]))
    extractions = load_extractions()
    if with_policies:
        policy = read(ENTITIES / 'HumanCostumeRefinement.h')
        if expected.get(POLICY_SIGNATURE) != POLICY_BASELINE or digest(policy) != POLICY_HEADER:
            raise ValueError('Unreviewed costume-policy edit')
        expected[POLICY_SIGNATURE] = POLICY_METHOD
        # Stage 3 extractions chain after the review above; the final comparison
        # still requires every body, helper included, to match its pin exactly.
        for signature, pin in extractions['methods'].items():
            if expected.get(signature) != pin['before']:
                raise ValueError('Unreviewed method extraction: ' + signature)
            expected[signature] = pin['after']
        for signature, pin in extractions['helpers'].items():
            if signature in expected:
                raise ValueError('Extracted helper shadows a baseline definition: ' + signature)
            expected[signature] = pin['digest']
        for signature, change in load_reviews()['methods'].items():
            if expected.get(signature) != change['before']:
                raise ValueError('Unreviewed change chain: ' + signature)
            if change['after'] is None:
                expected.pop(signature)
            else:
                expected[signature] = change['after']
        if actual.get(POLICY_SIGNATURE) != expected.get(POLICY_SIGNATURE):
            raise ValueError('Unreviewed costume-policy edit')
    if expected != actual:
        raise ValueError(json.dumps(dict(missing=sorted(expected.keys() - actual.keys()),
            extra=sorted(actual.keys() - expected.keys()),
            changed=[s for s in expected.keys() & actual.keys() if expected[s] != actual[s]]), indent=2))
    original_header = subprocess.check_output(['git', 'show', f'{REVISION}:{HEADER}'], cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')
    current_header = read(ROOT / HEADER)
    if with_policies:
        # Only the pinned private helper declarations may be added to the header.
        lines = current_header.split('\n')
        for declaration in extractions['declarations']:
            matches = [i for i, line in enumerate(lines) if line.strip() == declaration]
            if not matches:
                raise ValueError('Pinned helper declaration missing: ' + declaration)
            del lines[matches[0]]
        # Reviewed header edits (lines removed from the baseline or added to the
        # current declaration) are recorded by --review with the method changes.
        reviewed_header = load_reviews()['header']
        for line in reviewed_header['added']:
            if line not in lines:
                raise ValueError('Reviewed header line missing: ' + line)
            lines.remove(line)
        current_header = '\n'.join(lines)
        original_lines = original_header.split('\n')
        for line in reviewed_header['removed']:
            if line not in original_lines:
                raise ValueError('Reviewed header removal is not in the baseline: ' + line)
            original_lines.remove(line)
        original_header = '\n'.join(original_lines)
    if original_header != current_header:
        raise ValueError('TMHuman class declaration changed')
    if complete:
        static_before = digest(outside_methods(baseline))
        static_after = digest(outside_methods(read(ENTITIES / 'TMHuman.cpp')))
        reviewed_static = load_reviews().get('static_data') if with_policies else None
        if static_before != static_after and reviewed_static != {'before': static_before, 'after': static_after}:
            raise ValueError('Core static data or namespace-scope code changed')
        project = ROOT / 'tmproject/TMProject748/TMProject748.vcxproj'
        registrations = []
        for path in (project, project.with_suffix('.vcxproj.filters')):
            registrations.append([node.attrib['Include'] for node in ET.parse(path).iter()
                if node.tag.endswith('}ClCompile') and 'Include' in node.attrib])
        for path in ENTITIES.glob('TMHuman*.cpp'):
            source = read(path)
            if not source.startswith('#include "pch.h"\n') or re.search(r'#include\s+["<][^">]*\.cpp', source):
                raise ValueError('Invalid independent translation unit: ' + path.name)
            if path.name != 'TMHuman.cpp' and digest(outside_methods(source)) != digest(''):
                raise ValueError('Unexpected namespace-scope code: ' + path.name)
            entry = 'internal\\game\\entities\\' + path.name
            if any(items.count(entry) != 1 for items in registrations):
                raise ValueError('Missing/duplicate build registration: ' + path.name)
        for entry in entries:
            if entry['file'] != f"TMHuman{entry['owner']}.cpp":
                raise ValueError('Wrong method owner: ' + entry['signature'])
        print('PASS: complete ownership, static data, PCH and independent build/filter registration')
    print(f'PASS: {len(actual)} definitions, overloads and TMHuman.h preserved')
    if with_policies:
        pinned = {POLICY_SIGNATURE} | set(extractions['methods']) | set(extractions['helpers'])
        counts = method_counts.categories(baseline_digests, actual, pinned, load_reviews()['methods'])
        if counts['unexplained']:
            raise ValueError('Changed bodies without a pin or review: ' + ', '.join(counts['unexplained']))
        print('PASS: ' + method_counts.summary(counts))
    return entries

def move(groups, baseline):
    sources = {path: read(path) for path in ENTITIES.glob('TMHuman*.cpp')}
    selected = {group: [] for group in groups}
    for path, source in sources.items():
        definitions = methods(source)
        for method in definitions:
            if owner(method) in selected:
                selected[owner(method)].append(method)
        for method in reversed(definitions):
            if owner(method) in selected:
                source = source[:method['start']] + source[method['end']:]
        sources[path] = source
    order = {m['signature']: n for n, m in enumerate(methods(baseline))}
    for group, definitions in selected.items():
        destination = ENTITIES / f'TMHuman{group}.cpp'
        definitions += methods(sources.get(destination, ''))
        body = '\n\n'.join(m['text'] for m in sorted(definitions, key=lambda m: order[m['signature']])) + '\n'
        sources[destination] = includes(body, baseline) + body
    for path, source in sources.items():
        source = re.sub(r'\n{3,}', '\n\n', source)
        if not path.exists() or read(path) != source:
            path.write_text(source, encoding='utf-8', newline='\n')
    project = ROOT / 'tmproject/TMProject748/TMProject748.vcxproj'
    filters = project.with_suffix('.vcxproj.filters')
    for group in groups:
        entry = f'internal\\game\\entities\\TMHuman{group}.cpp'
        for path in (project, filters):
            source = read(path)
            if f'Include="{entry}"' in source:
                continue
            anchor = '    <ClCompile Include="internal\\game\\entities\\TMHuman.cpp"'
            start = source.index(anchor)
            end = source.index('\n', start) if path == project else source.index('</ClCompile>', start) + len('</ClCompile>')
            addition = f'\n    <ClCompile Include="{entry}" />' if path == project else f'\n    <ClCompile Include="{entry}">\n      <Filter>Source Files\\Engine</Filter>\n    </ClCompile>'
            source = source[:end] + addition + source[end:]
            path.write_text(source, encoding='utf-8', newline='\n')

def pin(spec_path, baseline):
    """Record a verified extract-method.py extraction of a TMHuman method."""
    spec = json.loads(Path(spec_path).read_text(encoding='utf-8'))
    pins = load_extractions()
    method_signature = re.sub(r'\s+', ' ', spec['method'].strip())
    current = {}
    for path in sorted(ENTITIES.glob('TMHuman*.cpp')):
        for method in methods(read(path)):
            current[method['signature']] = (digest(method['text']), path.name)
    if method_signature not in current:
        raise SystemExit('Method not found: ' + method_signature)
    after, owner_file = current[method_signature]
    group = owner_file[len('TMHuman'):-len('.cpp')]
    if method_signature in pins['helpers']:
        pins['helpers'][method_signature]['digest'] = after      # a helper decomposed again
    else:
        reviewed = {POLICY_SIGNATURE: POLICY_METHOD}
        baseline_digests = {m['signature']: digest(m['text']) for m in methods(baseline)}
        before = (pins['methods'].get(method_signature, {}).get('before')
                  or reviewed.get(method_signature) or baseline_digests[method_signature])
        pins['methods'][method_signature] = {'before': before, 'after': after}
    header = read(ROOT / HEADER)
    original_header = subprocess.check_output(['git', 'show', f'{REVISION}:{HEADER}'], cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')
    added = []
    matcher = difflib.SequenceMatcher(a=original_header.split('\n'), b=header.split('\n'), autojunk=False)
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if tag in ('replace', 'delete'):
            raise SystemExit('The header changed baseline lines; only insertions can be pinned')
        if tag == 'insert':
            added += [line.strip() for line in header.split('\n')[j1:j2]]
    if any(not line for line in added):
        raise SystemExit('Blank lines were added to the header; remove them before pinning')
    pins['declarations'] = added
    for extraction in spec['extractions']:
        signatures = [s for s in current if re.search(r'TMHuman::' + extraction['helper'] + r'\(', s)]
        if len(signatures) != 1:
            raise SystemExit(f"Expected one helper {extraction['helper']}, found {len(signatures)}")
        helper_digest, helper_file = current[signatures[0]]
        if helper_file != owner_file:
            raise SystemExit(f"Helper {extraction['helper']} is not beside its method")
        pins['helpers'][signatures[0]] = {'digest': helper_digest, 'owner': group}
        declarations = [l.strip() for l in header.split('\n') if re.search(r'\b' + extraction['helper'] + r'\(', l)]
        if len(declarations) != 1 or declarations[0] not in added:
            raise SystemExit(f"Expected one added declaration of {extraction['helper']}")
    EXTRACTIONS.write_text(json.dumps(pins, indent=1) + '\n', encoding='utf-8')
    print(f"pinned {method_signature} and {len(spec['extractions'])} helper(s) in {EXTRACTIONS.name}")


def review(baseline):
    """Record the current method differences from the pinned chain for review.

    Header lines removed from or added to the baseline (beyond the pinned
    helper declarations) are recorded too, for --with-policies to accept.
    """
    expected = {m['signature']: digest(m['text']) for m in methods(baseline)}
    expected[POLICY_SIGNATURE] = POLICY_METHOD
    extractions = load_extractions()
    for signature, pin in extractions['methods'].items():
        expected[signature] = pin['after']
    for signature, pin in extractions['helpers'].items():
        expected[signature] = pin['digest']
    actual = {}
    for path in sorted(ENTITIES.glob('TMHuman*.cpp')):
        for method in methods(read(path)):
            actual[method['signature']] = digest(method['text'])
    changes = {s: {'before': expected.get(s), 'after': actual.get(s)}
               for s in sorted(expected.keys() | actual.keys()) if expected.get(s) != actual.get(s)}
    original_lines = subprocess.check_output(['git', 'show', f'{REVISION}:{HEADER}'], cwd=ROOT).decode(
        'utf-8-sig').replace('\r\n', '\n').split('\n')
    lines = read(ROOT / HEADER).split('\n')
    for declaration in extractions['declarations']:
        matches = [i for i, line in enumerate(lines) if line.strip() == declaration]
        if matches:
            del lines[matches[0]]
    removed, added = [], []
    for tag, a1, a2, b1, b2 in difflib.SequenceMatcher(None, original_lines, lines, autojunk=False).get_opcodes():
        if tag in ('delete', 'replace'):
            removed.extend(original_lines[a1:a2])
        if tag in ('insert', 'replace'):
            added.extend(lines[b1:b2])
    record = {'methods': changes, 'header': {'removed': removed, 'added': added, 'removed_declarations': []}}
    static_before = digest(outside_methods(baseline))
    static_after = digest(outside_methods(read(ENTITIES / 'TMHuman.cpp')))
    if static_before != static_after:
        record['static_data'] = {'before': static_before, 'after': static_after}
    REVIEWS.write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
    print(f'Recorded {len(changes)} method changes, {len(removed)} removed and {len(added)} added header '
          f'lines in {REVIEWS.relative_to(ROOT)}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--move', nargs='+', choices=[g for g in GROUPS if g] + ['Packets'])
    parser.add_argument('--manifest', action='store_true')
    parser.add_argument('--complete', action='store_true', help='Require final ownership and registration')
    parser.add_argument('--refresh-includes', action='store_true', help='Mechanically prune/infer unit dependencies')
    parser.add_argument('--with-policies', action='store_true', help='Verify the exact reviewed second-phase extraction')
    parser.add_argument('--pin', metavar='SPEC', help='Pin a verified extract-method.py extraction')
    parser.add_argument('--review', action='store_true', help='Record the current reviewed method changes')
    args = parser.parse_args()
    if args.with_policies and (args.move or args.refresh_includes):
        parser.error('Policy verification is read-only; do not combine with relocation')
    baseline = subprocess.check_output(['git', 'show', f'{REVISION}:{BASE}'], cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')
    if args.review:
        review(baseline)
        return
    if args.pin:
        pin(args.pin, baseline)
        return
    verify(baseline, with_policies=args.with_policies)
    if args.move:
        move(args.move, baseline)
    if args.refresh_includes:
        for path in ENTITIES.glob('TMHuman*.cpp'):
            source = read(path)
            body = re.sub(r'^#include[^\n]*\n', '', source, flags=re.M).lstrip('\n')
            updated = includes(body, baseline) + body
            if source != updated:
                path.write_text(updated, encoding='utf-8', newline='\n')
    entries = verify(baseline, args.complete, args.with_policies)
    if args.manifest:
        for entry in entries:
            print(f"| `{entry['signature']}` | `TMHuman{entry['owner']}.cpp` |")

if __name__ == '__main__':
    main()
