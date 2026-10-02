"""Reproducible TMHuman relocation manifest and semantic-preservation gate.

Reads the committed baseline, not historical client binaries. --move performs
only a bulk mechanical rewrite of complete definitions and project entries.
Run from the repository root. Generated reports belong in ignored build output.
"""
import argparse
import hashlib
import json
import re
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

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
    pattern = r'^(?:(?:int|void|bool|float)[ \t]+)?TMHuman::(~?\w+)\([^;{}]*\)\s*\{'
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
    if with_policies:
        policy = read(ENTITIES / 'HumanCostumeRefinement.h')
        if (expected.get(POLICY_SIGNATURE) != POLICY_BASELINE or
                actual.get(POLICY_SIGNATURE) != POLICY_METHOD or digest(policy) != POLICY_HEADER):
            raise ValueError('Unreviewed costume-policy edit')
        expected[POLICY_SIGNATURE] = POLICY_METHOD
    if expected != actual:
        raise ValueError(json.dumps(dict(missing=sorted(expected.keys() - actual.keys()),
            extra=sorted(actual.keys() - expected.keys()),
            changed=[s for s in expected.keys() & actual.keys() if expected[s] != actual[s]]), indent=2))
    header = 'tmproject/TMProject748/internal/game/entities/TMHuman.h'
    original_header = subprocess.check_output(['git', 'show', f'{REVISION}:{header}'], cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')
    if original_header != read(ROOT / header):
        raise ValueError('TMHuman class declaration changed')
    if complete:
        if digest(outside_methods(baseline)) != digest(outside_methods(read(ENTITIES / 'TMHuman.cpp'))):
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
    suffix = '; 127 unchanged bodies and one pinned, tested policy extraction' if with_policies else ''
    print(f'PASS: {len(actual)} definitions, overloads and TMHuman.h preserved{suffix}')
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

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--move', nargs='+', choices=[g for g in GROUPS if g] + ['Packets'])
    parser.add_argument('--manifest', action='store_true')
    parser.add_argument('--complete', action='store_true', help='Require final ownership and registration')
    parser.add_argument('--refresh-includes', action='store_true', help='Mechanically prune/infer unit dependencies')
    parser.add_argument('--with-policies', action='store_true', help='Verify the exact reviewed second-phase extraction')
    args = parser.parse_args()
    if args.with_policies and (args.move or args.refresh_includes):
        parser.error('Policy verification is read-only; do not combine with relocation')
    baseline = subprocess.check_output(['git', 'show', f'{REVISION}:{BASE}'], cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')
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
