"""TMFieldScene definition inventory and behavior-preserving relocation gate.

The committed source is the immutable baseline; runtime binaries are untouched.
--move performs only bulk mechanical relocation of complete member definitions.
Run from the repository root. Unresolved owners block final acceptance.
"""
import argparse
import difflib
import hashlib
import json
import re
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCENES = ROOT / 'tmproject/TMProject748/internal/app/scenes'
BASE = 'tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp'
HEADER = BASE.replace('.cpp', '.h')
REVISION = '77e8dcc7f1af929be1c6418d08b97c7dd8412713'
CHAT_DISPATCH = 'int TMFieldScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)'
CHAT_BASELINE = '2652319983dad3511638716101b8b90a6ad80fdffed238c831ec640faede56bb'
CHAT_REVIEWED = 'b823b55387fb0774bc1319987c087fb202bf46a924cdf332d91b366339769829'
REVIEWED_METHODS = {
    CHAT_DISPATCH: (CHAT_BASELINE, CHAT_REVIEWED),
    'int TMFieldScene::SkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)': (
        '791e203d8a55dcad7408a77aae087638762bb4a095c2da4ba7dd53f29d33ccc5',
        '0cede2a318432fdb41ff4877664ccd0e6a20982cb736187e50c2c296a6756282'),
    'int TMFieldScene::AutoSkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)': (
        'a3f0c57393e332aa27bfbaed0bd71eb30f57352680b4c1ab2c3ce6812387911f',
        'cb52fbbc6943b811817a37a5b8a97b73ebb01d112377a3f5cd8f528981fac237'),
}
POLICIES = {
    'internal/application/FieldChatControlPolicy.h': 'd0f7cc6d45f4fd84901ceeb31402f8263c3d152bd8cddb67805b3f3f083279c1',
    'internal/ui/FieldChatControl.h': '28c9cf9c1995b1e2be0fd9d3c864d540e714a02109a56b3a676a5a2907c5064d',
    'internal/application/SkillRequestPolicy.h': '18b58c7be20336bf5f2d1ecdeaa6843ae667aee323aef5d2c597bfa333915ba8',
    'internal/wire/SkillAttackRequest.h': '43b9dd2cc5d1f2ef8e8af94b86aa23341af78c3bc2b9bdf47eaefe7c01cedb07',
    'internal/wire/AttackVisualDamage.h': '251333095d26e0b71b05c752aefbf5afc39567e30ba6781d075280389485844f',
}
REVIEWED_HELPERS = {
    'GetWYD748AttackVisualDamage': (
        '3459e0d6a471df8fa66a111b78111daf3cd2e38676ad35b102f9fe23506c8509',
        '8ff70c6e798a2b66b3967e3142fa1b1dcba45663737afd8a775ab28e490bd162'),
}
SUPPORT_HEADERS = {
    'FieldSceneInventorySupport.h': 'cec90a0caf5025ec9a39f20180ce32b49ca6a4e9c7089441a844ddab9baafe52',
    'FieldSceneTradeSupport.h': '963c67c75a0b287ee43c3a8480985dbb5942bd9a5b341a57b3befb76a39704b1',
    'FieldSceneWorldSupport.h': 'd91283bd09b05759ec280e60c4c8221c56a7993cab009f43f79d640414bc3106',
}
GROUPS = {
    'Chat': 'SetWhisper SetPartyChat SetGuildChat SetKingDomChat OnPacketMessageChat OnPacketMessageChat_Index OnPacketMessageChat_Param OnPacketMessageWhisper SysMsgChat InsertInChatList',
    '': 'TMFieldScene ~TMFieldScene InitializeScene FrameMove OnControlEvent OnCharEvent OnMouseEvent OnPacketEvent OnMsgBoxEvent TimeDelay GetTimeString',
    'Inventory': 'InitializeCompatInventory GetCarryGridForSlot GetCargoGridForSlot GetCarryCellForSlot GetCargoCellForSlot GetCarrySlotForCell GetCargoSlotForCell DropItem GetItem SetVisibleInventory SetVisibleCargo SetVisibleCargo1 SetInventoryGridType SetGridState SetEquipGridState UpdateMyHuman SetSanc GetItemFromGround UseHPotion UseMPotion UsePPotion IsFeedPotion FeedMount UseTicket UseQuickSloat UseItem SendCapsuleItem ClearInventorySelectedItem Bag_View DropListUpdate UpdateGridDropList VisibleInputCharName SetInVisibleInputCoin',
    'InventoryPackets': 'OnPacketUpdateCargoCoin OnPacketCNFDropItem OnPacketCNFGetItem OnPacketUpdateItem OnPacketRemoveItem OnPacketSwapItem OnPacketDeposit OnPacketWithdraw OnPacketCapsuleInfo',
    'Merchant': 'CheckMerchant SetVisibleShop SetVisibleHellGateStore BuyItemNewStore UpdateNewStore OnPacketShopList OnPacketRMBShopList OnPacketBuy OnPacketSell OnPacketCloseShop OnPacketNewCashRev OnPacketNewBuyCash OnPacketNewCashRev2 OnPacketItemPrice MouseClick_PremiumNPC',
    'Trade': 'SetVisibleTrade SetVisibleAutoTrade SendReqBuy VisibleInputTradeName OnPacketItemSold OnPacketAutoTrade',
    'Mix': 'GetNativeMixPanel GetNativeMixGrid GetNativeMixPacket GetNativeMixSlotCount ResetNativeMixPacket ClearNativeMix DoNativeMix SetVisibleNativeMix TryStageNativeMixItem TryRemoveNativeMixItem ClearCombine ClearCombine2 ClearCombine3 ClearCombine4 ClearCombine5 ClearCombine6 DoCombine DoCombine2 DoCombine3 DoCombine4 DoCombine5 DoCombine6 SetVisibleMixItem SetVisibleMixItem2 SetVisibleMixItem3 SetVisibleMixItemTiini SetVisibleMixItem5 SetVisibleMixItem6 OnPacketCombineComplete SetVisibleMixPanel ClearMixPannel SetVisibleMissionPanel ClearMissionPannel MouseClick_MixNPC',
    'UI': 'UpdateCompatScoreUI UpdateCompatLearnedSkillUI InitializeRuntimeCounterTexts InitializeCompatFieldScene PositionCompatFeaturePanels PositionCompatNativeMixPanels PositionCompatShopPanels PositionCompatTradePanels PositionCompatGamblePanel PositionCompatPartyPanel PositionCompatQuestPanel SelectQuestTab SelectHelpTab SetQuestPanelVisible SetVisibleCharInfo UpdateScoreUI InitBoard LoadMsgText SetButtonTextXY VisibleInputPass UpdateCompatObservedAffects Affect_Main GetLascDescParamId StrByteCheck',
    'Input': 'OnMouseEventCompat OnKeyDownEvent OnAccel MouseClick_NPC MouseMove MouseLButtonDown OnESC OnKeyDebug OnKeySkill OnKeyDash OnKeyPlus OnKeyPK OnKeyName OnKeyAutoTarget OnKeyAuto OnKeyHelp OnKeyRun OnKeyFeedMount OnKeyHPotion OnKeyMPotion OnKeyPPotion OnKeySkillPage OnKeyQuestLog OnKeyReverse OnKeyAutoRun OnKeyGuildOnOff OnKeyShortSkill OnKeyVisibleSkill OnKeyCamView OnKeyVisibleInven OnKeyVisibleCharInfo OnKeyVisibleMinimap OnKeyVisibleParty OnKeyReturn OnKeyNumPad OnKeyTotoTab OnKeyTotoEnter',
    'Skills': 'InitializeCompatSkillBelts GetSkillDelay IsSkillCoolingDown UpdateSkillCooldownUI SkillUse AutoSkillUse SetVisibleSkillMaster SetVisibleSkill UpdateSkillBelt IncSkillSel SetShortSkill SetSkillColor SetMyHumanMagic MouseClick_SkillMasterNPC OnPacketSetShortSkill',
    'Combat': 'MobAttack GetWeaponDamage SetPK SetPosPKRun FrameMove_KhepraDieEffect SetMyHumanExp',
    'CombatPackets': 'OnPacketAttack OnPacketCNFMobKill OnPacketSetHpMode OnPacketNuke OnPacketAction',
    'Movement': 'UpdateTeleportPrompt OfferRespawnPrompt MobMove MobMove2 MobStop SetRunMode AirMove_Main AirMove_Start AirMove_End AirMove_ShowUI OnPacketReqSummon OnPacketCancelSummon',
    'World': 'SetWeather SetVisibleKhepraPortal SetVisiblePotal SetVisibleMiniMap SetCameraView InitCameraView SetVisibleNameLabel OnPacketCreateMobCompat OnPacketCreateMob OnPacketWeather OnPacketCreateItem OnPacketEnvEffect OnPacketSoundEffect',
    'Social': 'SetVisibleParty VisibleInputGuildName Guildmark_Create Guildmark_MakeFileName Guildmark_Find_ArrayIndex Guildmark_Find_EmptyArrayIndex Guildmark_DeleteIdleGuildmark Guildmark_IsCorrectBMP Guildmark_Link OnPacketREQParty OnPacketAddParty OnPacketRemoveParty OnPacketGuildDisable OnPacketReqChallange SetVisibleServerWar SetVisibleRefuseServerWar',
    'Events': 'PGTVisible SetVisibleGamble UpdateGambleRequestTimeout InitializeFireWorkControls UpdateFireWorkButton ClearFireWork UseFireWork DrawCustomFireWork TotoSelect TotoBuy TotoClose SetQuestStatus UpdateQuestTime OnPacketLongMessagePanel OnPacketClearMenu OnPacketCastleState OnPacketStartTime OnPacketRemainCount OnPacketWarInfo OnPacketRemainNPCCount OnPacketRESULTGAMBLE OnPacketREQArray InitializeQuizEventControls OnPacketQuizEvent OnPacketRandomQuiz OnPacketSendExpMsg OnPacketBattle OnPacketInforPlay OnPacketRunQuest12Start OnPacketRunQuest12Count MouseClick_QuestNPC',
    'Automation': 'FindAuto FindProcess SetAutoOption SetAutoSkillNum SetAutoTarget GameAuto ToggleNativeCCMode NewCCMode InitializeCompatCCControls OnPacketMacroWater',
    'SessionPackets': 'OnPacketCNFCharacterLogout OnPacketCNFRemoveServer OnPacketCNFAccountLogin OnPacketCNFCharacterLogin OnPacketAutoKick OnPacketDelayQuit',
}
OWNERS = {name: group for group, names in GROUPS.items() for name in names.split()}
AUXILIARY = {
    'ObservedAffectPanel': 'UI',
    'WYD748_ResetTradeOffer': 'Trade',
    'WYD748_LogTradeSend': 'Trade',
    'WYD748_ParseDecimal': 'Events',
    'WYD748_CancelAutoTradePurchase': 'Trade',
    'WYD748_ReleaseAutoTradeItem': 'Trade',
    'WYD748_AddOwnedGridItem': '',
    'WYD748_IsUnsupportedCompatEquipSlot': 'InventoryPackets',
    'GetWYD748AttackVisualDamage': 'CombatPackets',
    'Guildmark_Download': 'Social',
    'SetMinimapPos': 'World',
}
LITERAL = r'R"(?P<tag>[^ ()\\\t\r\n]{0,16})\([\s\S]*?\)(?P=tag)"|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\''


def read(path):
    return path.read_text(encoding='utf-8-sig')


def masked(source):
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL,
                  lambda m: re.sub(r'[^\n]', ' ', m.group()), source)


def methods(source):
    code = masked(source)
    pattern = (r'^(?:[\w:*&<>]+[ \t]+)*TMFieldScene::(~?\w+)'
               r'\([^;{}]*\)(?:\s+const)?\s*(?::\s*TMScene\(\)\s*)?\{')
    result = []
    for match in re.finditer(pattern, code, re.M):
        opening = match.end() - 1
        end, depth = opening + 1, 1
        while end < len(code) and depth:
            depth += (code[end] == '{') - (code[end] == '}')
            end += 1
        if depth:
            raise ValueError('Unterminated definition: ' + match.group(1))
        result.append(dict(name=match.group(1),
            signature=re.sub(r'\s+', ' ', source[match.start():opening].strip()),
            start=match.start(), end=end, text=source[match.start():end]))
    # Reject a missed qualified definition rather than silently dropping coverage.
    covered = {m['start'] for m in result}
    for candidate in re.finditer(r'^(?:[\w:*&<>]+[ \t]+)*TMFieldScene::~?\w+\s*\(', code, re.M):
        if candidate.start() not in covered:
            raise ValueError('Unparsed definition at offset ' + str(candidate.start()))
    return result


def digest(source):
    code = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|' + LITERAL,
        lambda m: m.group() if m.group().startswith(('"', "'", 'R"')) else ' ', source)
    tokens = [m.group() for m in re.finditer(LITERAL + r'|\w+|[^\s]', code)]
    return hashlib.sha256(json.dumps(tokens, ensure_ascii=False).encode()).hexdigest()


def outside_methods(source):
    for method in reversed(methods(source)):
        source = source[:method['start']] + source[method['end']:]
    return re.sub(r'^#include[^\n]*\n', '', source, flags=re.M)


def auxiliary(source):
    code = masked(source)
    result = []
    for name in AUXILIARY:
        pattern = (r'^[ \t]*(?:[\w:*&<>]+[ \t]+)+' + name +
                   (r'\s+final\s*:\s*public\s+SPanel\s*\{' if name == 'ObservedAffectPanel'
                    else r'\([^;{}]*\)\s*\{'))
        for match in re.finditer(pattern, code, re.M):
            end, depth = match.end(), 1
            while end < len(code) and depth:
                depth += (code[end] == '{') - (code[end] == '}')
                end += 1
            if depth:
                raise ValueError('Unterminated helper: ' + name)
            if name == 'ObservedAffectPanel':
                end += 1  # Preserve the class terminator.
            result.append(dict(name=name, start=match.start(), end=end, text=source[match.start():end]))
    return sorted(result, key=lambda item: item['start'])


def outside_definitions(source):
    code = masked(source)
    wrappers = []
    for match in re.finditer(r'\bnamespace\s*\{', code):
        end, depth = match.end(), 1
        while end < len(code) and depth:
            depth += (code[end] == '{') - (code[end] == '}')
            end += 1
        wrappers += [(match.start(), match.end()), (end - 1, end)]
    spans = wrappers + [(d['start'], d['end']) for d in methods(source) + auxiliary(source)]
    for start, end in sorted(spans, reverse=True):
        source = source[:start] + source[end:]
    return re.sub(r'^#include[^\n]*\n', '', source, flags=re.M)


def baseline():
    return subprocess.check_output(['git', 'show', f'{REVISION}:{BASE}'], cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')


def verify(original, complete=False, with_policies=False):
    definitions = methods(original)
    expected = {m['signature']: digest(m['text']) for m in definitions}
    if len(expected) != len(definitions):
        raise ValueError('Duplicate baseline definition')
    actual, entries = {}, []
    for path in sorted(SCENES.glob('TMFieldScene*.cpp')):
        for method in methods(read(path)):
            signature = method['signature']
            if signature in actual:
                raise ValueError('Duplicate definition: ' + signature)
            actual[signature] = digest(method['text'])
            entries.append(dict(signature=signature, name=method['name'], file=path.name,
                owner=OWNERS.get(method['name'], 'UNRESOLVED'), lines=method['text'].count('\n') + 1))
    if with_policies:
        # Pin both sides of every reviewed transformation. No body is excluded.
        for signature, (before, after) in REVIEWED_METHODS.items():
            if expected.get(signature) != before or actual.get(signature) != after:
                raise ValueError('Unreviewed dispatcher/request transformation: ' + signature)
            expected[signature] = after
        for relative, fingerprint in POLICIES.items():
            if digest(read(ROOT / 'tmproject/TMProject748' / relative)) != fingerprint:
                raise ValueError('Unreviewed policy: ' + relative)
    if expected != actual:
        raise ValueError(json.dumps(dict(missing=sorted(expected.keys() - actual.keys()),
            extra=sorted(actual.keys() - expected.keys()),
            changed=sorted(s for s in expected.keys() & actual.keys() if expected[s] != actual[s])), indent=2))
    original_header = subprocess.check_output(['git', 'show', f'{REVISION}:{HEADER}'], cwd=ROOT).decode('utf-8-sig').replace('\r\n', '\n')
    if original_header != read(ROOT / HEADER):
        raise ValueError('Scene declaration/layout changed')
    if digest(outside_definitions(original)) != digest(outside_definitions(read(SCENES / 'TMFieldScene.cpp'))):
        raise ValueError('Core static data changed without explicit review')
    expected_helpers = {d['name']: digest(d['text'].replace('invalidatedSlot = -1', 'invalidatedSlot'))
                        for d in auxiliary(original)}
    actual_helpers = {}
    for path in SCENES.glob('TMFieldScene*.cpp'):
        for helper in auxiliary(read(path)):
            name = helper['name']
            if name in actual_helpers:
                raise ValueError('Duplicate helper: ' + name)
            actual_helpers[name] = digest(helper['text'].replace('invalidatedSlot = -1', 'invalidatedSlot'))
            if path.name != 'TMFieldScene.cpp' or complete:
                if path.name != f'TMFieldScene{AUXILIARY[name]}.cpp':
                    raise ValueError('Wrong helper owner: ' + name)
    if with_policies:
        for name, (before, after) in REVIEWED_HELPERS.items():
            if expected_helpers.get(name) != before or actual_helpers.get(name) != after:
                raise ValueError('Unreviewed auxiliary definition: ' + name)
            expected_helpers[name] = after
    if expected_helpers != actual_helpers or len(expected_helpers) != len(AUXILIARY):
        raise ValueError('Missing, extra or changed auxiliary definition')
    project = ROOT / 'tmproject/TMProject748/TMProject748.vcxproj'
    trees = [ET.parse(path) for path in (project, project.with_suffix('.vcxproj.filters'))]
    registrations = [[node.attrib['Include'] for node in tree.iter()
        if node.tag.endswith('}ClCompile') and 'Include' in node.attrib] for tree in trees]
    header_registrations = [[node.attrib['Include'] for node in tree.iter()
        if node.tag.endswith('}ClInclude') and 'Include' in node.attrib] for tree in trees]
    for filename, fingerprint in SUPPORT_HEADERS.items():
        entry = 'internal\\app\\scenes\\' + filename
        if not (SCENES / filename).is_file() or any(items.count(entry) != 1 for items in header_registrations):
            raise ValueError('Missing/duplicate shared declaration registration: ' + filename)
        if digest(read(SCENES / filename)) != fingerprint:
            raise ValueError('Unreviewed shared declaration: ' + filename)
    if with_policies:
        for relative in POLICIES:
            entry = relative.replace('/', '\\')
            if any(items.count(entry) != 1 for items in header_registrations):
                raise ValueError('Missing/duplicate policy registration: ' + relative)
    for path in SCENES.glob('TMFieldScene*.cpp'):
        source = read(path)
        if not source.startswith('#include "pch.h"\n') or re.search(r'#include\s+["<][^">]*\.cpp', source):
            raise ValueError('Invalid independent translation unit: ' + path.name)
        if path.name != 'TMFieldScene.cpp' and digest(outside_definitions(source)) != digest(''):
            raise ValueError('Unreviewed namespace-scope code: ' + path.name)
        entry = 'internal\\app\\scenes\\' + path.name
        if any(items.count(entry) != 1 for items in registrations):
            raise ValueError('Missing/duplicate build registration: ' + path.name)
    for entry in entries:
        if complete and entry['owner'] == 'UNRESOLVED':
            raise ValueError('Unresolved owner: ' + entry['signature'])
        if entry['file'] != 'TMFieldScene.cpp' or complete:
            if entry['file'] != f"TMFieldScene{entry['owner']}.cpp":
                raise ValueError('Wrong owner: ' + entry['signature'])
    print(f'PASS: {len(actual)} complete definitions; header, static data and helpers preserved')
    print(f'PASS: {len(actual) - len(REVIEWED_METHODS)} bodies unchanged and {len(REVIEWED_METHODS)} pinned policy/request extractions' if with_policies
          else 'PASS: all member bodies, signatures and literals preserved')
    if with_policies:
        print(f'PASS: {len(AUXILIARY) - len(REVIEWED_HELPERS)} helpers unchanged and {len(REVIEWED_HELPERS)} pinned visual projection extraction')
    print('PASS: unique ownership, PCH and independent build/filter registration')
    return entries


def includes(group, body='', original=''):
    if group == 'Chat':
        return ''.join(f'#include "{name}"\n' for name in (
            'pch.h', 'TMFieldScene.h', 'TMGlobal.h', 'ObjectManager.h',
            'SControl.h', 'SControlContainer.h', 'TMUtil.h')) + '\n'
    dependencies = {
        '': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h ItemEffect.h WYD748Assets.h DirShow.h FieldSceneTradeSupport.h FieldSceneInventorySupport.h FieldSceneWorldSupport.h ../../application/FieldInteractionPolicy.h ../../core/NativeSalePrice.h ServerStatus.h ServerChannelLabel.h ServerEndpoint.h',
        'Inventory': 'SGrid.h SControlContainer.h TMGlobal.h TMItem.h TMObjectContainer.h TMUtil.h ItemEffect.h TMSkinMesh.h WYD748Assets.h ../../application/FieldInteractionPolicy.h ../../game/entities/AppearanceRefinementRefresh.h',
        'InventoryPackets': 'SGrid.h SControlContainer.h TMGlobal.h TMItem.h TMObjectContainer.h TMUtil.h WYD748Assets.h FieldSceneInventorySupport.h ../../application/FieldInteractionPolicy.h',
        'Merchant': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h ItemEffect.h TMItem.h ClientDiagnostics.h ../../core/NativeSalePrice.h FieldSceneInventorySupport.h',
        'Trade': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h TMItem.h ClientDiagnostics.h WYD748Assets.h ../../application/FieldInteractionPolicy.h FieldSceneTradeSupport.h',
        'Mix': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h TMItem.h WYD748Assets.h',
        'UI': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h TMFont3.h TMSkinMesh.h WYD748Assets.h ItemEffect.h FieldSceneWorldSupport.h ../../ui/ObservedAffectProjection.h ../../ui/ResourceBarProjection.h',
        'Input': 'SGrid.h SControlContainer.h TMGlobal.h TMGround.h TMCamera.h TMUtil.h TMItem.h ClientDiagnostics.h WYD748Assets.h ../../application/FieldInteractionPolicy.h',
        'Skills': 'SGrid.h SControlContainer.h TMGlobal.h TMGround.h TMUtil.h TMItem.h ItemEffect.h TMSkillHolyTouch.h TMSkillTownPortal.h TMSkillMagicArrow.h TMSkillJudgement.h TMSkillPoison.h TMSkillMeteorStorm.h TMSkillThunderBolt.h TMSkillSlowSlash.h TMSkillMagicShield.h TMSkillFreezeBlade.h TMEffectMesh.h TMEffectBillBoard.h TMEffectBillBoard2.h TMEffectBillBoard4.h TMEffectSkinMesh.h TMEffectStart.h TMEffectSWSwing.h TMEffectSpark.h TMEffectParticle.h TMShade.h TMArrow.h TMSkillHeavenDust.h TMSkillFlash.h TMEffectCharge.h TMSkillExplosion2.h TMCannon.h TMSkinMesh.h ../../application/SkillCooldownPolicy.h ../../application/FieldInteractionPolicy.h',
        'Combat': 'SGrid.h SControlContainer.h TMGlobal.h TMGround.h TMUtil.h TMItem.h ItemEffect.h TMEffectMesh.h TMEffectBillBoard.h TMEffectBillBoard2.h TMEffectSkinMesh.h TMEffectLevelUp.h TMEffectParticle.h TMEffectDust.h ../../application/FieldInteractionPolicy.h',
        'CombatPackets': 'SGrid.h SControlContainer.h TMGlobal.h TMGround.h TMObjectContainer.h TMUtil.h TMSkinMesh.h TMItem.h TMSkillMagicArrow.h TMSkillJudgement.h TMSkillPoison.h TMSkillMeteorStorm.h TMSkillThunderBolt.h TMSkillSlowSlash.h TMSkillMagicShield.h TMSkillFreezeBlade.h TMEffectMesh.h TMEffectBillBoard.h TMEffectBillBoard2.h TMEffectBillBoard4.h TMEffectSkinMesh.h TMEffectStart.h TMEffectSWSwing.h TMEffectSpark.h TMEffectParticle.h TMShade.h TMArrow.h TMSkillHeavenDust.h TMSkillFlash.h TMEffectCharge.h TMSkillExplosion2.h TMCannon.h TMSkillHolyTouch.h ../../game/entities/DeathMotionPolicy.h',
        'Movement': 'SGrid.h SControlContainer.h TMGlobal.h TMGround.h TMUtil.h TMCamera.h TMEffectBillBoard2.h TMSkillTownPortal.h ../../application/FieldInteractionPolicy.h ../../game/entities/AirMoveMotion.h',
        'World': 'SGrid.h SControlContainer.h TMGlobal.h TMGround.h TMObjectContainer.h TMUtil.h TMItem.h TMHouse.h TMCamera.h TMSun.h TMSky.h TMSnow.h TMRain.h TMSkinMesh.h TMEffectBillBoard.h TMEffectBillBoard2.h TMEffectMesh.h TMEffectDust.h TMEffectParticle.h TMShade.h TMGate.h ../../ui/MiniMapLayout.h',
        'Social': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h WYD748Assets.h <WinInet.h>',
        'Events': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h TMEffectBillBoard.h TMEffectBillBoard2.h TMEffectMesh.h TMEffectParticle.h TMFont3.h TMItem.h WYD748Assets.h',
        'Automation': 'SGrid.h SControlContainer.h TMGlobal.h TMGround.h TMUtil.h WYD748Assets.h features/macro/MacroMsg.h ../../application/CCModePolicy.h ../../application/FieldInteractionPolicy.h',
        'SessionPackets': 'SGrid.h SControlContainer.h TMGlobal.h TMUtil.h AdapterIdentity.h ClientDiagnostics.h',
    }
    names = ['pch.h', 'TMFieldScene.h'] + dependencies[group].split()
    if group in ('', 'Trade'):
        names.append('FieldSceneTradeSupport.h')
    # Add referenced types and member dependencies, not the monolith's blanket
    # include block. The compiler remains the gate for hidden transitive types.
    aliases = {
        'TMGround.h': r'm_pGround',
        'TMCamera.h': r'g_pCamera|g_p3rdCamera|m_pCamera',
        'TMSkinMesh.h': r'm_pSkinMesh|m_pMount|m_pMantua',
        'TMObjectContainer.h': r'm_p\w*Container|m_pHumanList',
        'ClientDiagnostics.h': r'WYD748_DiagnosticsLog',
        'TMLog.h': r'LOG_WRITELOG\w*',
        'TMFont3.h': r'g_pFont',
        'TMEffectSWSwing.h': r'm_pSwingEffect',
        'features/macro/MacroMsg.h': r'stWaterScrollMacro',
        'DirShow.h': r'DS_SOUND_MANAGER',
        'dsutil.h': r'g_pSoundManager',
        'ItemEffect.h': r'\bEF_\w+',
        '../../application/FieldInteractionPolicy.h': r'field_interaction::',
        '../../application/CCModePolicy.h': r'cc_mode::',
        '../../application/SkillCooldownPolicy.h': r'skill_cooldown::',
        '../../game/entities/DeathMotionPolicy.h': r'death_motion::',
        '../../game/entities/AirMoveMotion.h': r'air_move::',
        '../../game/entities/AppearanceRefinementRefresh.h': r'appearance_refinement::',
        '../../ui/ObservedAffectProjection.h': r'observed_affect::',
        '../../ui/ResourceBarProjection.h': r'resource_bar::',
        '../../ui/MiniMapLayout.h': r'minimap_layout::',
    }
    for header in re.findall(r'^#include "([^"]+)"', original, re.M):
        pattern = r'\b' + re.escape(Path(header).stem) + r'\b'
        if re.search(pattern + ('|' + aliases[header] if header in aliases else ''), body):
            names.append(header)
    return ''.join('#include ' + (name if name.startswith('<') else f'"{name}"') + '\n'
                   for name in dict.fromkeys(names)) + '\n'


def move(groups, original):
    sources = {path: read(path) for path in SCENES.glob('TMFieldScene*.cpp')}
    selected = {group: [] for group in groups}
    selected_helpers = {group: [] for group in groups}
    for path, source in sources.items():
        definitions = methods(source)
        helpers = auxiliary(source)
        for helper in helpers:
            if AUXILIARY[helper['name']] in selected_helpers:
                selected_helpers[AUXILIARY[helper['name']]].append(helper)
        for method in definitions:
            if OWNERS.get(method['name']) in selected:
                selected[OWNERS[method['name']]].append(method)
        for method in sorted(definitions + helpers, key=lambda d: d['start'], reverse=True):
            if (OWNERS.get(method['name']) if method in definitions else AUXILIARY[method['name']]) in selected:
                source = source[:method['start']] + source[method['end']:]
        sources[path] = source
    order = {m['signature']: n for n, m in enumerate(methods(original))}
    for group, definitions in selected.items():
        if not definitions:
            raise ValueError('Empty domain: ' + group)
        body = '\n\n'.join(m['text'] for m in sorted(definitions, key=lambda m: order[m['signature']])) + '\n'
        # Shared UI/packet helpers have narrow declarations and exactly one implementation.
        shared = {'WYD748_ResetTradeOffer', 'WYD748_LogTradeSend', 'WYD748_CancelAutoTradePurchase',
                  'WYD748_ReleaseAutoTradeItem', 'WYD748_IsUnsupportedCompatEquipSlot', 'Guildmark_Download',
                  'SetMinimapPos'}
        local = '\n\n'.join(h['text'] for h in selected_helpers[group] if h['name'] not in shared)
        external = '\n\n'.join(h['text'] for h in selected_helpers[group] if h['name'] in shared).replace('invalidatedSlot = -1', 'invalidatedSlot')
        prefix = ('namespace\n{\n' + local + '\n}\n\n' if local else '') + external
        sources[SCENES / f'TMFieldScene{group}.cpp'] = includes(group, prefix + body, original) + prefix + '\n\n' + body
    for path, source in sources.items():
        source = re.sub(r'\n{3,}', '\n\n', source)
        source = re.sub(r'[ \t]+$', '', source, flags=re.M)
        if not path.exists() or read(path) != source:
            path.write_text(source, encoding='utf-8', newline='\n')
    project = ROOT / 'tmproject/TMProject748/TMProject748.vcxproj'
    for group in groups:
        entry = f'internal\\app\\scenes\\TMFieldScene{group}.cpp'
        for path in (project, project.with_suffix('.vcxproj.filters')):
            source = read(path)
            if f'Include="{entry}"' in source:
                continue
            start = source.index('    <ClCompile Include="internal\\app\\scenes\\TMFieldScene.cpp"')
            end = source.index('\n', start) if path == project else source.index('</ClCompile>', start) + len('</ClCompile>')
            addition = f'\n    <ClCompile Include="{entry}" />' if path == project else f'\n    <ClCompile Include="{entry}">\n      <Filter>Source Files\\Engine</Filter>\n    </ClCompile>'
            source = source[:end] + addition + source[end:]
            path.write_text(source, encoding='utf-8', newline='\n')


def test_patch(groups, original):
    """Emit an apply_patch edit; never concatenate units or modify assertions."""
    path = ROOT / 'tmproject/TMProject748/tests/SceneDisconnectContractTests.cpp'
    before = read(path)
    after = before
    definitions = {d['name']: d for d in methods(original)}
    bodies = {
        'deathBody': 'OnPacketCNFMobKill', 'shopListHandler': 'OnPacketShopList',
        'rmbShopHandler': 'OnPacketRMBShopList', 'carryGridBody': 'GetCarryGridForSlot',
        'storeBody': 'UpdateNewStore', 'weaponDamageBody': 'GetWeaponDamage',
        'listingSoldHandler': 'OnPacketItemSold', 'listingSnapshot': 'OnPacketAutoTrade',
        'dropHandler': 'OnPacketCNFDropItem', 'saleHandler': 'OnPacketSell',
        'appearanceRefresh': 'UpdateMyHuman', 'sancRefresh': 'SetSanc',
        'swapHandler': 'OnPacketSwapItem', 'closeSplit': 'SetInVisibleInputCoin',
    }
    def method(name):
        owner = OWNERS[name]
        signature = definitions[name]['signature']
        return ('source_contract::Method(LoadSource("TMProject748/internal/app/scenes/'
            f'TMFieldScene{owner}.cpp"),\n        "{signature}")')
    for variable, name in bodies.items():
        if OWNERS[name] not in groups:
            continue
        pattern = (r'^    const auto (\w+) = fieldSource\.find\("[^\n]+\n'
            r'    const auto \w+ = fieldSource\.find\("[^\n]+\n'
            r'    const auto ' + variable + r' = [\s\S]*?: std::string\{\};')
        after, count = re.subn(pattern, f'    const auto {variable} = {method(name)};', after, flags=re.M)
        if count == 0:
            expression = 'source_contract::Method(fieldSource,\n        "' + definitions[name]['signature'] + '")'
            count = after.count('    const auto ' + variable + ' = ' + expression + ';')
            after = after.replace('    const auto ' + variable + ' = ' + expression + ';',
                '    const auto ' + variable + ' = ' + method(name) + ';')
        if count != 1:
            raise ValueError('Unexpected test source boundary: ' + variable)
    if 'Inventory' in groups:
        after = after.replace('    const auto inventoryStart = fieldSource.find(',
            '    const auto inventorySource = ' + method('SetVisibleInventory') + ';\n    const auto inventoryStart = inventorySource.find(')
        for expression in ('find("auto pCargoPanel', 'substr(inventoryStart'):
            after = after.replace('fieldSource.' + expression, 'inventorySource.' + expression)
    if 'Trade' in groups:
        after = after.replace('source_contract::Method(fieldSource,\n        "void TMFieldScene::SendReqBuy(unsigned int dwControlID)")', method('SendReqBuy'))
        after = after.replace('source_contract::Method(fieldSource,\n        "void TMFieldScene::SetVisibleAutoTrade(int bShow, int bCargo)")', method('SetVisibleAutoTrade'))
        after = re.sub(r'^    const auto buyStart = fieldSource.find\([^\n]+\n', '', after, flags=re.M)
        after = re.sub(r'^    const auto buyEnd = fieldSource.find\([^\n]+\n'
            r'    const auto buyHandler =[\s\S]*?: std::string\{\};',
            '    const auto buyHandler = ' + method('SendReqBuy') + ';', after, flags=re.M)
        for prefix, helper, signature in (
            ('listingCleanup', 'listingSoldCleanup', 'void WYD748_ReleaseAutoTradeItem(SGridControlItem*& pItem)'),
            ('cancel', 'cancelBody', 'void WYD748_CancelAutoTradePurchase(SMessageBox* dialog, int invalidatedSlot)')):
            after = re.sub(r'^    const auto ' + prefix + r'Start = fieldSource.find\([^\n]+\n'
                r'    const auto ' + prefix + r'End = fieldSource.find\([^\n]+\n'
                r'    const auto ' + helper + r' =[\s\S]*?: std::string\{\};',
                '    const auto ' + helper + ' = source_contract::Method(LoadSource(\n'
                '        "TMProject748/internal/app/scenes/TMFieldSceneTrade.cpp"),\n'
                '        "' + signature + '");', after, flags=re.M)
    if 'Skills' in groups:
        after = after.replace('    const auto shortcutStart = fieldSource.find(',
            '    const auto shortcutSource = ' + method('SetShortSkill') + ';\n    const auto shortcutStart = shortcutSource.find(')
        for expression in ('find("pGridItem->m_pItem->sIndex >=', 'find("IsPassiveSkill(', 'find("g_pItemList[pGridItem'):
            after = after.replace('fieldSource.' + expression, 'shortcutSource.' + expression)
    if 'UI' in groups:
        after = after.replace('fieldSource.find("capeIndex > 0 && capeIndex < MAX_ITEMLIST")',
            method('UpdateScoreUI') + '.find("capeIndex > 0 && capeIndex < MAX_ITEMLIST")')
    if 'Movement' in groups:
        after = re.sub(r'^    const auto airMoveStart = fieldSource.find\([^\n]+\n'
            r'    const auto airMoveEndStart = fieldSource.find\([^\n]+\n'
            r'    const auto airMoveStartBody =[\s\S]*?: std::string\{\};',
            '    const auto airMoveStartBody = ' + method('AirMove_Start') + ';', after, flags=re.M)
        after = re.sub(r'^    const auto airMoveEndEnd = fieldSource.find\([^\n]+\n'
            r'    const auto airMoveEndBody =[\s\S]*?: std::string\{\};',
            '    const auto airMoveEndBody = ' + method('AirMove_End') + ';', after, flags=re.M)
    if 'CombatPackets' in groups:
        after = re.sub(r'^    const auto attackStart = field.find\([^\n]+\n'
            r'    const auto attackEnd = field.find\([^\n]+\n',
            '    const auto attack = ' + method('OnPacketAttack') + ';\n', after, flags=re.M)
        after = after.replace('attackStart != std::string::npos && attackEnd != std::string::npos', '!attack.empty()')
        after = re.sub(r'^        const std::string attack = field.substr\([^\n]+\n', '', after, flags=re.M)
    if 'Automation' in groups:
        after = after.replace('const auto ccModeScene = LoadSource("TMProject748/internal/app/scenes/TMFieldScene.cpp");',
            'const auto ccModeScene = LoadSource("TMProject748/internal/app/scenes/TMFieldSceneAutomation.cpp");')
    if before == after:
        raise ValueError('No matching test consumers')
    print('*** Begin Patch\n*** Update File: ' + path.as_posix())
    for line in list(difflib.unified_diff(before.splitlines(), after.splitlines(), n=3))[2:]:
        print('@@' if line.startswith('@@') else line)
    print('*** End Patch')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--move', nargs='+', choices=[g for g in GROUPS if g])
    parser.add_argument('--manifest', action='store_true')
    parser.add_argument('--complete', action='store_true')
    parser.add_argument('--with-policies', action='store_true', help='Accept only the pinned chat dispatcher/policy extraction')
    parser.add_argument('--test-patch', nargs='+', choices=[g for g in GROUPS if g])
    args = parser.parse_args()
    original = baseline()
    if args.test_patch:
        test_patch(args.test_patch, original)
        return
    if args.move and args.with_policies:
        parser.error('Mechanical moves and reviewed policy acceptance must run separately')
    entries = verify(original, args.complete, args.with_policies)
    if args.move:
        move(args.move, original)
        entries = verify(original, args.complete)
    if args.manifest:
        for entry in entries:
            print(f"| `{entry['signature']}` | `{entry['owner'] or 'Core'}` | {entry['lines']} |")


if __name__ == '__main__':
    main()
