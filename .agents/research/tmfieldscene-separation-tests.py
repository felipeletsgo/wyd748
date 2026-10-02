"""Positive and fail-closed tests for the field scene relocation gate."""
import importlib.util
import contextlib
import copy
import io
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

spec = importlib.util.spec_from_file_location('field_relocation', Path(__file__).with_name('tmfieldscene-separation.py'))
relocation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(relocation)


class FieldRelocationTests(unittest.TestCase):
    def test_complete_signatures_and_literal_braces(self):
        source = '''// int TMFieldScene::Ignored() { }
TMFieldScene::TMFieldScene() : TMScene() { }
TMFieldScene::~TMFieldScene() { }
unsigned int TMFieldScene::GetLascDescParamId() { return 1; }
SGridControl* TMFieldScene::GetCarryGridForSlot(int slot) const {
    if (slot) { const char* a = "}"; char b = '{'; }
    const char* r = R"tag({ " // not a comment /* })tag";
    return nullptr;
}
int TMFieldScene::TryStageNativeMixItem(SGridControl* grid,
    int x, int y) { return 0; }
'''
        definitions = relocation.methods(source)
        self.assertEqual(len(definitions), 5)
        self.assertEqual(definitions[2]['signature'], 'unsigned int TMFieldScene::GetLascDescParamId()')
        self.assertTrue(definitions[3]['signature'].endswith('const'))
        self.assertIn('return nullptr;', definitions[3]['text'])
        self.assertEqual(definitions[4]['signature'],
            'int TMFieldScene::TryStageNativeMixItem(SGridControl* grid, int x, int y)')

    def test_unterminated_body(self):
        with self.assertRaisesRegex(ValueError, 'Unterminated'):
            relocation.methods('int TMFieldScene::FrameMove(int n) { if (n) { }')

    def test_static_data_is_not_a_method(self):
        source = 'unsigned int TMFieldScene::m_dwCargoID = 0;\nint TMFieldScene::FrameMove(int n) {}'
        self.assertEqual(relocation.digest(relocation.outside_methods(source)),
            relocation.digest('unsigned int TMFieldScene::m_dwCargoID = 0;'))

    def test_tokens_preserve_literals_and_ignore_only_comments_and_spacing(self):
        self.assertEqual(relocation.digest('int a = 1; // comment'), relocation.digest('int /* comment */ a=1;'))
        for first, second in [('"two words"', '"twowords"'), ('int a', 'inta'),
            ('R"tag(" // a /* })tag"', 'R"tag(" // b /* })tag"'), ("'}'", "'{'"), ('1', '2')]:
            self.assertNotEqual(relocation.digest(first), relocation.digest(second))

    def reject_fixture(self, source, message):
        scenes = Mock()
        scenes.glob.return_value = [Path('TMFieldScene.cpp')]
        with patch.object(relocation, 'SCENES', scenes), patch.object(relocation, 'read', return_value=source):
            with self.assertRaisesRegex(ValueError, message):
                relocation.verify('int TMFieldScene::FrameMove(int n) { return 1; }')

    def test_missing(self):
        self.reject_fixture('', 'missing')

    def test_duplicate(self):
        self.reject_fixture('int TMFieldScene::FrameMove(int n) { return 1; }\n' * 2, 'Duplicate definition')

    def test_changed(self):
        self.reject_fixture('int TMFieldScene::FrameMove(int n) { return 2; }', 'changed')

    def test_extra(self):
        self.reject_fixture('int TMFieldScene::FrameMove(int n) { return 1; }\nvoid TMFieldScene::Unknown() {}', 'extra')

    def test_duplicate_baseline(self):
        with self.assertRaisesRegex(ValueError, 'Duplicate baseline'):
            relocation.verify('int TMFieldScene::FrameMove(int n) {}\n' * 2)


class CompleteTreeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.original = relocation.baseline()
        cls.sources = {p: relocation.read(p) for p in relocation.SCENES.glob('TMFieldScene*.cpp')}
        cls.sources[relocation.ROOT / relocation.HEADER] = relocation.read(relocation.ROOT / relocation.HEADER)
        cls.sources.update({relocation.SCENES / name: relocation.read(relocation.SCENES / name)
                           for name in relocation.SUPPORT_HEADERS})
        cls.sources.update({relocation.ROOT / 'tmproject/TMProject748' / name:
                           relocation.read(relocation.ROOT / 'tmproject/TMProject748' / name)
                           for name in relocation.POLICIES})

    def verify_tree(self, changes=None, with_policies=True):
        sources = self.sources | (changes or {})
        with patch.object(relocation, 'read', side_effect=sources.__getitem__), contextlib.redirect_stdout(io.StringIO()):
            return relocation.verify(self.original, complete=True, with_policies=with_policies)

    def test_complete_reviewed_tree(self):
        entries = self.verify_tree()
        self.assertEqual(len(entries), 293)
        self.assertEqual(len({entry['file'] for entry in entries}), 18)

    def test_strict_mode_rejects_policy_change(self):
        with self.assertRaisesRegex(ValueError, 'changed'):
            self.verify_tree(with_policies=False)

    def test_skill_requests_preserve_send_prediction_and_timing_order(self):
        source = self.sources[relocation.SCENES / 'TMFieldSceneSkills.cpp']
        bodies = {method['name']: method['text'] for method in relocation.methods(source)}
        for name in ('SkillUse', 'AutoSkillUse'):
            body = bodies[name]
            with self.subTest(method=name):
                self.assertEqual(body.count('skill_attack::Area<MSG_Attack>'), 1)
                self.assertEqual(body.count('skill_attack::Direct<MSG_Attack>'), 1)
                self.assertEqual(body.count('skill_attack::SelectEnvelope('), 2)
                area = body.index('const int nSize = skill_attack::SelectEnvelope(')
                iterator = 'l' if name == 'SkillUse' else 'i'
                clear = body.index(f'for (int {iterator} = g_pSpell[(unsigned char)cSkillIndex].MaxTarget; {iterator} < 13; ++{iterator})')
                self.assertLess(clear, area)
                tokens = ['memcpy(&stAttackLocal, &stAttack, nSize);',
                          'OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);',
                          'SendOneMessage((char*)&stAttack, nSize);', 'IncSkillSel();']
                if name == 'AutoSkillUse':
                    tokens = tokens[2:] + tokens[:2]
                tokens += ['m_dwOldAttackTime = dwServerTime;',
                           'm_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;']
                position = area
                for token in tokens:
                    next_position = body.find(token, position)
                    self.assertGreaterEqual(next_position, position, token)
                    position = next_position + len(token)
                position = body.index('const int Size = skill_attack::SelectEnvelope(')
                for token in ['SendOneMessage((char*)&stAttack, Size);', 'IncSkillSel();',
                              'memcpy(&stLocalAttack, &stAttack, sizeof(stLocalAttack));',
                              'OnPacketEvent(stAttack.Header.Type, (char*)&stLocalAttack);',
                              'm_dwOldAttackTime = dwServerTime;',
                              'm_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;',
                              'm_pMyHuman->m_pMoveSkillTargetHuman = 0;']:
                    next_position = body.find(token, position)
                    self.assertGreaterEqual(next_position, position, token)
                    position = next_position + len(token)
                direct = body.index('skill_attack::Direct<MSG_Attack>')
                guard = body.index('if (!pTarget)', direct)
                return_value = 'return 1;' if name == 'SkillUse' else 'return 0;'
                self.assertIn(return_value, body[guard:guard + 45])

    def test_rejects_dispatcher_and_policy_mutations(self):
        mutations = []
        for signature in relocation.REVIEWED_METHODS:
            path, method = next((path, method) for path, source in self.sources.items()
                if path.suffix == '.cpp' for method in relocation.methods(source)
                if method['signature'] == signature)
            mutations.append((path, self.sources[path][:method['end'] - 1] + 'return 23;\n' +
                              self.sources[path][method['end'] - 1:], 'dispatcher/request'))
        for relative in relocation.POLICIES:
            path = relocation.ROOT / 'tmproject/TMProject748' / relative
            mutations.append((path, self.sources[path] + '\nconstexpr int unexpected = 23;\n', 'policy'))
        for name in relocation.REVIEWED_HELPERS:
            path, helper = next((path, helper) for path, source in self.sources.items()
                if path.suffix == '.cpp' for helper in relocation.auxiliary(source)
                if helper['name'] == name)
            mutations.append((path, self.sources[path][:helper['end'] - 1] + 'return 23;\n' +
                              self.sources[path][helper['end'] - 1:], 'auxiliary definition'))
        for path, source, message in mutations:
            with self.subTest(path=path.name), self.assertRaisesRegex(ValueError, message):
                self.verify_tree({path: source})

    def test_rejects_header_static_and_shared_declaration_mutations(self):
        mutations = [(relocation.ROOT / relocation.HEADER, 'Scene declaration/layout'),
                     (relocation.SCENES / 'TMFieldScene.cpp', 'Core static data')]
        mutations += [(relocation.SCENES / name, 'shared declaration') for name in relocation.SUPPORT_HEADERS]
        for path, message in mutations:
            with self.subTest(path=path.name), self.assertRaisesRegex(ValueError, message):
                self.verify_tree({path: self.sources[path] + '\nint unexpected = 23;\n'})

    def test_rejects_changed_helper(self):
        path = relocation.SCENES / 'TMFieldSceneTrade.cpp'
        helper = next(h for h in relocation.auxiliary(self.sources[path]) if h['name'] == 'WYD748_ResetTradeOffer')
        source = self.sources[path][:helper['end'] - 1] + 'int unexpected = 23;\n' + self.sources[path][helper['end'] - 1:]
        with self.assertRaisesRegex(ValueError, 'auxiliary definition'):
            self.verify_tree({path: source})

    def test_rejects_pch_and_unreviewed_scope_code(self):
        path = relocation.SCENES / 'TMFieldSceneChat.cpp'
        for source, message in [(self.sources[path].replace('#include "pch.h"\n', '', 1), 'independent translation'),
                                (self.sources[path] + '\nint unexpected = 23;\n', 'namespace-scope code')]:
            with self.subTest(message=message), self.assertRaisesRegex(ValueError, message):
                self.verify_tree({path: source})

    def test_rejects_wrong_member_and_helper_owners(self):
        with patch.dict(relocation.OWNERS, {'FrameMove': 'Chat'}), self.assertRaisesRegex(ValueError, 'Wrong owner'):
            self.verify_tree()
        with patch.dict(relocation.AUXILIARY, {'WYD748_ResetTradeOffer': 'Chat'}), self.assertRaisesRegex(ValueError, 'Wrong helper owner'):
            self.verify_tree()

    def test_rejects_missing_and_duplicate_registrations(self):
        original_parse = relocation.ET.parse
        for kind, entry, message in [
            ('ClCompile', 'internal\\app\\scenes\\TMFieldSceneChat.cpp', 'build registration'),
            ('ClInclude', 'internal\\app\\scenes\\FieldSceneTradeSupport.h', 'shared declaration registration'),
            ('ClInclude', 'internal\\ui\\FieldChatControl.h', 'policy registration'),
        ]:
            for filters in (False, True):
                for duplicate in (False, True):
                    def mutated_parse(path):
                        tree = original_parse(path)
                        if str(path).endswith('.filters') == filters:
                            for parent in tree.iter():
                                node = next((n for n in parent if n.tag.endswith('}' + kind)
                                             and n.attrib.get('Include') == entry), None)
                                if node is not None:
                                    parent.append(copy.deepcopy(node)) if duplicate else parent.remove(node)
                                    break
                        return tree
                    with self.subTest(kind=kind, filters=filters, duplicate=duplicate), \
                            patch.object(relocation.ET, 'parse', side_effect=mutated_parse), \
                            self.assertRaisesRegex(ValueError, message):
                        self.verify_tree()


if __name__ == '__main__':
    unittest.main()
