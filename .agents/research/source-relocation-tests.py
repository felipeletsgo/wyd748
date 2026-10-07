"""Positive and rejection tests for source-relocation.py manifests (audit A05)."""
import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('relocation', Path(__file__).with_name('source-relocation.py'))
relocation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(relocation)

BASE = '''int Kept(int a) { return a; }
int Changed(int a) { return a + 1; }
int Removed(int a) { return a - 1; }
'''


class ManifestTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        git = lambda *args: subprocess.run(['git', *args], cwd=self.root, check=True, capture_output=True)
        git('init', '-q')
        git('config', 'user.email', 'test@example.invalid')
        git('config', 'user.name', 'test')
        (self.root / 'a.cpp').write_text(BASE)
        (self.root / 'b.cpp').write_text('int Other() { return 0; }\n')
        git('add', '.')
        git('commit', '-q', '-m', 'base')
        self.revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=self.root, text=True).strip()
        self.saved_root = relocation.ROOT
        relocation.ROOT = self.root

    def tearDown(self):
        relocation.ROOT = self.saved_root
        self.temp.cleanup()

    def digest_of(self, text, name):
        for key, body in relocation.items(relocation.tokens(text)):
            if name in key:
                return relocation.digest(body)
        raise KeyError(name)

    def manifest(self, reviewed=None, current=('a.cpp', 'b.cpp'), removed_files=()):
        group = {'baseline': ['a.cpp', 'b.cpp'], 'current': list(current)}
        if removed_files:
            group['removed_files'] = list(removed_files)
        return {'revision': self.revision, 'groups': [group], 'reviewed': reviewed or {}}

    def write(self, text):
        (self.root / 'a.cpp').write_text(text)

    def test_unchanged_tree_passes(self):
        self.assertEqual(relocation.verify(self.manifest())[1], [])

    def test_unreviewed_change_fails(self):
        self.write(BASE.replace('a + 1', 'a + 2'))
        failures = relocation.verify(self.manifest())[1]
        self.assertTrue(any(f.startswith('missing') for f in failures))

    def test_reviewed_change_and_removal_pass(self):
        current = BASE.replace('a + 1', 'a + 2').replace('int Removed(int a) { return a - 1; }\n', '')
        self.write(current)
        reviewed = {
            'Changed': {'before': self.digest_of(BASE, 'Changed'), 'after': self.digest_of(current, 'Changed')},
            'Removed': {'before': self.digest_of(BASE, 'Removed'), 'after': None},
        }
        self.assertEqual(relocation.verify(self.manifest(reviewed))[1], [])

    def test_reviewed_removal_of_a_present_definition_fails(self):
        reviewed = {'Removed': {'before': self.digest_of(BASE, 'Removed'), 'after': None}}
        failures = relocation.verify(self.manifest(reviewed))[1]
        self.assertTrue(any(f.startswith('extra') for f in failures))

    def test_review_does_not_cover_a_later_edit(self):
        first = BASE.replace('a + 1', 'a + 2')
        reviewed = {'Changed': {'before': self.digest_of(BASE, 'Changed'), 'after': self.digest_of(first, 'Changed')}}
        self.write(BASE.replace('a + 1', 'a + 3'))
        self.assertTrue(relocation.verify(self.manifest(reviewed))[1])

    def test_pin_chain_passes(self):
        first = BASE.replace('a + 1', 'a + 2')
        second = BASE.replace('a + 1', 'a + 3')
        self.write(second)
        reviewed = {
            'first': {'before': self.digest_of(BASE, 'Changed'), 'after': self.digest_of(first, 'Changed')},
            'second': {'before': self.digest_of(first, 'Changed'), 'after': self.digest_of(second, 'Changed')},
        }
        self.assertEqual(relocation.verify(self.manifest(reviewed))[1], [])

    def test_unlisted_missing_file_fails_without_raising(self):
        (self.root / 'b.cpp').unlink()
        failures = relocation.verify(self.manifest())[1]
        self.assertIn('current file missing (not a reviewed removal): b.cpp', failures)

    def test_listed_removed_file_must_be_gone(self):
        failures = relocation.verify(self.manifest(removed_files=['b.cpp']))[1]
        self.assertIn('file listed as removed still exists: b.cpp', failures)

    def test_removed_file_needs_its_definitions_reviewed(self):
        (self.root / 'b.cpp').unlink()
        failures = relocation.verify(self.manifest(removed_files=['b.cpp']))[1]
        self.assertTrue(any(f.startswith('missing') and 'Other' in f for f in failures))
        reviewed = {'Other': {'before': self.digest_of('int Other() { return 0; }\n', 'Other'), 'after': None}}
        self.assertEqual(relocation.verify(self.manifest(reviewed, removed_files=['b.cpp']))[1], [])


if __name__ == '__main__':
    unittest.main()
