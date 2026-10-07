"""Focused positive and rejection tests for the TMHuman relocation gate."""
import importlib.util
import subprocess
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

spec = importlib.util.spec_from_file_location('relocation', Path(__file__).with_name('tmhuman-separation.py'))
relocation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(relocation)


class RelocationTests(unittest.TestCase):
    def test_overloads_nested_braces_and_literals(self):
        source = '''// int TMHuman::Attack(int n) { }
int TMHuman::Attack(int n);
int TMHuman::Attack(int n) {
    if (n) { const char* a = "}"; char b = '{'; }
    const char* r = R"tag({ " // not a comment /* })tag";
    return 1; /* } */
}
int TMHuman::Attack(float n) { return 2; }
TMHuman::TMHuman(TMScene* pParentScene) { }
TMHuman::~TMHuman() { }
'''
        definitions = relocation.methods(source)
        self.assertEqual([m['signature'] for m in definitions], [
            'int TMHuman::Attack(int n)', 'int TMHuman::Attack(float n)',
            'TMHuman::TMHuman(TMScene* pParentScene)', 'TMHuman::~TMHuman()'])
        self.assertIn('return 1;', definitions[0]['text'])
        self.assertNotIn('return 2;', definitions[0]['text'])

    def test_unterminated_body(self):
        with self.assertRaisesRegex(ValueError, 'Unterminated definition'):
            relocation.methods('int TMHuman::Attack(int n) { if (n) { }')

    def test_comments_and_whitespace_are_not_code_changes(self):
        self.assertEqual(relocation.digest('int a = 1; // comment'),
                         relocation.digest('int /* another comment */ a=1;'))
        self.assertNotEqual(relocation.digest('int a'), relocation.digest('inta'))

    def test_literal_contents_are_never_normalized(self):
        for first, second in [('"two words"', '"twowords"'),
                              ('R"tag(" // a /* })tag"', 'R"tag(" // b /* })tag"'),
                              ("'}'", "'{'"), ('1', '2')]:
            self.assertNotEqual(relocation.digest(first), relocation.digest(second))

    def test_static_data_is_outside_method_inventory(self):
        source = 'int TMHuman::m_value = 1;\nint TMHuman::InitObject() { return 0; }'
        self.assertEqual(relocation.digest(relocation.outside_methods(source)),
                         relocation.digest('int TMHuman::m_value = 1;'))

    def verify_fixture(self, source, expected_message):
        entities = Mock()
        entities.glob.return_value = [Path('TMHuman.cpp')]
        with patch.object(relocation, 'ENTITIES', entities), patch.object(relocation, 'read', return_value=source):
            with self.assertRaisesRegex(ValueError, expected_message):
                relocation.verify('int TMHuman::InitObject() { return 1; }')

    def test_missing_definition(self):
        self.verify_fixture('', 'missing')

    def test_duplicate_definition(self):
        self.verify_fixture('int TMHuman::InitObject() { return 1; }\n' * 2, 'Duplicate:')

    def test_changed_body(self):
        self.verify_fixture('int TMHuman::InitObject() { return 2; }', 'changed')

    def test_extra_definition(self):
        self.verify_fixture('int TMHuman::InitObject() { return 1; }\nvoid TMHuman::Init() {}', 'extra')

    def test_duplicate_baseline(self):
        with self.assertRaisesRegex(ValueError, 'Duplicate baseline'):
            relocation.verify('int TMHuman::InitObject() {}\n' * 2)

    def test_pinned_policy_and_static_rejections(self):
        baseline = subprocess.check_output(['git', 'show', f'{relocation.REVISION}:{relocation.BASE}'],
                                           cwd=relocation.ROOT).decode('utf-8-sig')
        real_read = relocation.read
        mutations = [
            ('HumanCostumeRefinement.h', lambda text: text.replace('4375', '4374'), 'Unreviewed'),
            ('TMHumanAppearance.cpp', lambda text: text.replace(
                'human_costume::UsesFixedBodyRefinement(m_sCostume)',
                'human_costume::UsesFixedBodyRefinement(m_sCostume + 1)'), 'Unreviewed'),
            ('TMHuman.cpp', lambda text: text + '\nstatic int unreviewed = 1;\n', 'Core static data'),
            # The reviewed namespace-scope change removed one overload only; also
            # changing the remaining one is an unexpected namespace change.
            ('TMHuman.cpp', lambda text: text.replace('bool _locationCheck(TMVector2 vec2, int mapX, int mapY)',
                                                      'bool _locationCheckGone(TMVector2 vec2, int mapX, int mapY)'),
             'Core static data'),
            # A body changed after its review no longer matches the recorded digest.
            ('TMHumanEffects.cpp', lambda text: text.replace('beriel_aura::InRange(', 'beriel_aura::InRange(1.0f + '),
             'changed'),
            ('TMHuman.h', lambda text: text + '\nint unreviewedDeclaration;\n', 'class declaration changed'),
        ]
        for filename, mutate, message in mutations:
            with self.subTest(filename=filename):
                def altered_read(path):
                    text = real_read(path)
                    return mutate(text) if path.name == filename else text
                with patch.object(relocation, 'read', side_effect=altered_read):
                    with self.assertRaisesRegex(ValueError, message):
                        relocation.verify(baseline, complete=True, with_policies=True)

    def test_method_counts_reconcile(self):
        spec = importlib.util.spec_from_file_location('method_counts', Path(__file__).with_name('method_counts.py'))
        counts_module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(counts_module)
        baseline = {'same': 'a', 'pinned': 'b', 'reviewed': 'c', 'loose': 'd', 'removed': 'e'}
        actual = {'same': 'a', 'pinned': 'B', 'reviewed': 'C', 'loose': 'D', 'added': 'f'}
        counts = counts_module.categories(baseline, actual, pinned={'pinned', 'added'}, reviewed={'reviewed'})
        self.assertEqual((counts['identical'], counts['pinned'], counts['reviewed'], counts['added'],
                          counts['removed']), (1, 1, 1, 1, 1))
        self.assertEqual(counts['unexplained'], ['loose'])
        both = counts['identical'] + counts['pinned'] + counts['reviewed'] + len(counts['unexplained'])
        self.assertEqual(both + counts['removed'], counts['baseline'])
        self.assertEqual(both + counts['added'], counts['current'])


if __name__ == '__main__':
    unittest.main()
