"""Regression gates for changed projects, asset lists and audit scope.

These are static/format gates, never client-execution or native-parity proof.
"""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('audit', Path(__file__).with_name('repository-change-audit.py'))
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class AuditParserTests(unittest.TestCase):
    def test_artifacts_are_not_counted_as_product(self):
        self.assertEqual(audit.domain('build/fixture/data/base/123'), 'local-build-or-database-artifact')
        self.assertEqual(audit.domain('tmproject/client748/mesh/a.msh'), 'client-runtime-assets')
        self.assertEqual(audit.domain('wydgo748/data/npcs/a.json'), 'server-data')

    def test_runtime_paths_cannot_escape(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.assertEqual(audit.runtime_path(root, 'sound\\\\ambient\\one.wav'), root / 'sound/ambient/one.wav')
            for value in ('../outside.wav', 'C:\\outside.wav', '/outside.wav', 'sound/../../outside.wav'):
                with self.subTest(value=value), self.assertRaises(ValueError):
                    audit.runtime_path(root, value)

    def test_project_parser_excludes_msbuild_macros(self):
        with tempfile.TemporaryDirectory() as directory:
            project = Path(directory) / 'test.vcxproj'
            project.write_text('<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003"><ItemGroup><ClCompile Include="internal\\a.cpp"/><ClCompile Include="$(Generated)\\b.cpp"/><Filter Include="internal"/></ItemGroup></Project>', encoding='utf-8')
            self.assertEqual(audit.project_references(project), [('ClCompile', 'internal/a.cpp')])

    def test_sound_parser_rejects_malformed_rows(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'sound.txt'
            path.write_text('1 sound/a.wav 2\n\n', encoding='utf-8')
            self.assertEqual(audit.sound_rows(path), [(1, 'sound/a.wav', 2)])
            for row in ('1 path', '1 path 2 extra', '-1 path 1', '1 path 0', 'x path 1'):
                path.write_text(row, encoding='utf-8')
                with self.subTest(row=row), self.assertRaises(ValueError):
                    audit.sound_rows(path)

    def test_music_parser_keeps_the_fixed_column(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'Music.txt'
            path.write_text('Thema            music\\login.mp3\t\t\n', encoding='utf-8')
            self.assertEqual(audit.music_rows(path), ['music\\login.mp3'])
            path.write_text('short\n', encoding='utf-8')
            with self.assertRaises(ValueError):
                audit.music_rows(path)


class ChangedRepositoryTests(unittest.TestCase):
    def test_changed_project_entries_resolve_and_do_not_duplicate(self):
        projects = [p for p in audit.changed_paths() if p.endswith(('.vcxproj', '.vcxproj.filters'))]
        self.assertTrue(projects, 'no project changes were audited')
        for relative in projects:
            project = audit.ROOT / relative
            if not project.is_file():
                continue
            seen = set()
            for kind, value in audit.project_references(project):
                with self.subTest(project=relative, reference=value):
                    self.assertNotIn((kind, value.lower()), seen)
                    seen.add((kind, value.lower()))
                    self.assertTrue((project.parent / value).is_file(), 'project references an absent file')

    def test_new_client_translation_units_are_registered(self):
        root = audit.ROOT / 'tmproject/TMProject748'
        projects = [root / 'TMProject748.vcxproj', root / 'tests/ArchitectureTests.vcxproj']
        registered = {str((project.parent / value).resolve()).lower()
                      for project in projects for kind, value in audit.project_references(project)
                      if kind == 'ClCompile'}
        for relative in audit.changed_paths():
            if relative.startswith('tmproject/TMProject748/internal/') and relative.endswith('.cpp'):
                path = audit.ROOT / relative
                if path.is_file():
                    with self.subTest(path=relative):
                        self.assertIn(str(path.resolve()).lower(), registered)

    def test_all_changed_json_documents_parse(self):
        count = 0
        for relative in audit.changed_paths():
            if relative.endswith('.json') and not relative.startswith('build/'):
                path = audit.ROOT / relative
                if path.is_file():
                    with self.subTest(path=relative):
                        json.loads(path.read_text(encoding='utf-8-sig'))
                        count += 1
        self.assertGreater(count, 0)

    def test_packaged_music_and_sound_references_exist(self):
        runtime = audit.ROOT / 'tmproject/client748'
        sounds = audit.sound_rows(runtime / 'sound/soundlist.txt')
        self.assertGreater(len(sounds), 300)
        for _, value, _ in sounds:
            with self.subTest(sound=value):
                self.assertTrue(audit.runtime_path(runtime, value).is_file(), 'dangling sound reference')
        for value in audit.music_rows(runtime / 'Music.txt'):
            with self.subTest(music=value):
                self.assertTrue(audit.runtime_path(runtime, value).is_file(), 'dangling music reference')

    def test_changed_wave_files_have_complete_riff_envelopes(self):
        count = 0
        for relative in audit.changed_paths():
            if not relative.startswith('tmproject/client748/') or not relative.lower().endswith('.wav'):
                continue
            path = audit.ROOT / relative
            if not path.is_file():
                continue
            with self.subTest(wave=relative):
                with path.open('rb') as handle:
                    header = handle.read(12)
                self.assertEqual(header[:4], b'RIFF')
                self.assertEqual(header[8:12], b'WAVE')
                self.assertLessEqual(int.from_bytes(header[4:8], 'little') + 8, path.stat().st_size)
                count += 1
        self.assertGreater(count, 0)


if __name__ == '__main__':
    unittest.main()
