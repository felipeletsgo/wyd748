"""Validate game-only upgrade compatibility without touching installed services."""
import importlib.util
import io
from pathlib import Path
import tarfile
import tempfile
import subprocess
import sys
import unittest

spec = importlib.util.spec_from_file_location('game_only_layout', Path(__file__).parent / 'vps/check-game-only-layout.py')
layout = importlib.util.module_from_spec(spec)
spec.loader.exec_module(layout)


class GameOnlyLayoutTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.game = self.root / 'game'
        self.game.mkdir()
        self.version = Path(layout.__file__).with_name('data-layout.version').read_text().strip()
        self.marker = self.game / 'data-layout.version'
        self.marker.write_text(self.version)
        self.config = self.root / 'server.txt'
        self.config.write_text('items=data/catalog/itemlist.csv\nnpcs=data/npcs\nweb_admin_staff=/etc/wyd-go/staff.json\n')
        self.archive = self.root / 'release.tar.gz'
        self.package()

    def package(self, marker=True, version=None):
        entries = {'game/data/catalog/itemlist.csv': b'fixture', 'game/data/npcs/Aki.json': b'{}'}
        if marker:
            entries['game/data-layout.version'] = (version or self.version).encode()
        with tarfile.open(self.archive, 'w:gz') as archive:
            for name, content in entries.items():
                entry = tarfile.TarInfo(name)
                entry.size = len(content)
                archive.addfile(entry, io.BytesIO(content))

    def check(self):
        return layout.check(self.game, self.config, self.archive)

    def test_current_layout_accepts_files_directories_and_external_staff(self):
        before = self.config.read_bytes(), self.archive.read_bytes(), self.marker.read_bytes()
        self.check()
        self.assertEqual(before, (self.config.read_bytes(), self.archive.read_bytes(), self.marker.read_bytes()))

    def test_pre_migration_installation_requires_full_release(self):
        self.marker.unlink()
        with self.assertRaisesRegex(ValueError, 'full release'):
            self.check()

    def test_old_marker_requires_full_release(self):
        self.marker.write_text('1')
        with self.assertRaisesRegex(ValueError, 'full release'):
            self.check()

    def test_old_profile_is_rejected_even_with_current_marker(self):
        self.config.write_text('items=data/itemlist.csv\n')
        before = self.config.read_bytes()
        with self.assertRaisesRegex(ValueError, 'Configured input items'):
            self.check()
        self.assertEqual(before, self.config.read_bytes())

    def test_missing_new_profile_input_is_rejected(self):
        self.config.write_text('npcgener=data/spawns/missing.txt\n')
        with self.assertRaisesRegex(ValueError, 'Configured input npcgener'):
            self.check()

    def test_missing_or_incompatible_archive_marker_is_rejected(self):
        for marker, version in ((False, None), (True, '1')):
            with self.subTest(marker=marker, version=version):
                self.package(marker, version)
                with self.assertRaises((ValueError, KeyError)):
                    self.check()

    def test_absolute_current_release_paths_are_validated(self):
        self.config.write_text('items=/srv/wyd-go/current/game/data/catalog/itemlist.csv\n')
        self.check()
        self.config.write_text('items=/srv/wyd-go/current/game/data/itemlist.csv\n')
        with self.assertRaisesRegex(ValueError, 'Configured input items'):
            self.check()

    def test_obsolete_configuration_keys_are_rejected(self):
        self.config.write_text('npcgener2=data/spawns/NPCGener.txt\n')
        with self.assertRaisesRegex(ValueError, 'Obsolete configuration'):
            self.check()

    def test_equivalent_relative_legacy_paths_are_rejected(self):
        for path in ('./data/itemlist.csv', 'data/../data/itemlist.csv', 'data//itemlist.csv'):
            with self.subTest(path=path):
                self.config.write_text(f'items={path}\n')
                with self.assertRaisesRegex(ValueError, 'full release'):
                    self.check()

    def test_case_insensitive_obsolete_keys_are_rejected(self):
        for key in ('NPCGENER2', 'Accounts', 'Guilds', 'CHARSTATE', 'Instance_State'):
            with self.subTest(key=key):
                self.config.write_text(f'{key}=data/npcs\n')
                with self.assertRaisesRegex(ValueError, 'Obsolete configuration'):
                    self.check()

    def test_equivalent_current_paths_are_accepted(self):
        for path in ('./data/catalog/itemlist.csv', 'data//catalog/itemlist.csv',
                     'data/./catalog/itemlist.csv', '././data/npcs/',
                     '/srv/wyd-go/current//game/./data//catalog/itemlist.csv'):
            with self.subTest(path=path):
                self.config.write_text(f' Items = "{path}"\n')
                self.check()

    def test_data_path_traversal_and_backslashes_remain_rejected(self):
        for path in ('./data/../data/catalog/itemlist.csv',
                     'data/catalog/../catalog/itemlist.csv',
                     '/srv/wyd-go/current/game/./data/../data/catalog/itemlist.csv',
                     './data/catalog\\itemlist.csv'):
            with self.subTest(path=path):
                self.config.write_text(f'items={path}\n')
                with self.assertRaisesRegex(ValueError, 'Unsupported data path'):
                    self.check()

    def test_cli_rejects_normalized_legacy_profile_without_mutation(self):
        for setting in ('items=./data/itemlist.csv', 'NPCGENER2=data/npcs'):
            with self.subTest(setting=setting):
                self.config.write_text(setting + '\n')
                before = self.config.read_bytes(), self.archive.read_bytes(), self.marker.read_bytes()
                result = subprocess.run([sys.executable, '-B', str(Path(layout.__file__)),
                                         '--installed-game', str(self.game), '--config', str(self.config),
                                         '--archive', str(self.archive)], capture_output=True, text=True, timeout=5)
                self.assertEqual(result.returncode, 1)
                self.assertIn('full release', result.stderr)
                self.assertNotIn('Traceback', result.stderr)
                self.assertEqual(before, (self.config.read_bytes(), self.archive.read_bytes(), self.marker.read_bytes()))

    def test_cli_rejects_corrupt_archive_without_traceback_or_mutation(self):
        self.archive.write_bytes(b'not a tar archive')
        before = self.config.read_bytes(), self.marker.read_bytes()
        result = subprocess.run([sys.executable, '-B', str(Path(layout.__file__)),
                                 '--installed-game', str(self.game), '--config', str(self.config),
                                 '--archive', str(self.archive)], capture_output=True, text=True, timeout=5)
        self.assertEqual(result.returncode, 1)
        self.assertIn('preflight rejected', result.stderr)
        self.assertNotIn('Traceback', result.stderr)
        self.assertEqual(before, (self.config.read_bytes(), self.marker.read_bytes()))

    def test_comments_blank_lines_and_quoted_paths(self):
        self.config.write_text('# profile\n\n; comment\n items = "data/catalog/itemlist.csv" \nnpcs=data/npcs\n')
        self.check()

    def test_directory_name_prefix_does_not_satisfy_an_input(self):
        self.config.write_text('npcs=data/npc\n')
        with self.assertRaisesRegex(ValueError, 'Configured input npcs'):
            self.check()


if __name__ == '__main__':
    unittest.main()
