"""Exercise resource/data matrix parsers without historical binaries or writes.

Synthetic fixtures distinguish consistency checks from native-parity evidence.
"""
import importlib.util
import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(filename))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


resource = load('resource_audit', 'resource-matrix.py')
data = load('data_audit', 'data-consistency.py')


class ResourceMatrixTests(unittest.TestCase):
    def test_normalization_and_development_exclusions(self):
        self.assertEqual(resource.normalize('./Mesh\\\\A.MSH'), 'mesh/a.msh')
        self.assertTrue(resource.is_path('mesh\\%02d.msh'))
        for value in ('C:\\development\\a.msh', '../../tmsrv/run/a.dat', 'game.exe', 'd3d9.dll', 'see this file.txt'):
            with self.subTest(value=value):
                self.assertFalse(resource.is_path(value))

    def test_formatted_paths_are_anchored_and_escape_punctuation(self):
        regex = resource.pattern_of('mesh/a.%02d.msh')
        self.assertIsNotNone(regex)
        self.assertTrue(regex.fullmatch('mesh/a.12.msh'))
        for value in ('mesh/aX12.msh', 'mesh/a.xx.msh', 'xmesh/a.12.msh', 'mesh/a.12.msh.bak'):
            self.assertIsNone(regex.fullmatch(value))
        self.assertIsNone(resource.pattern_of('mesh/a.msh'))

    def test_compiled_scripts_formats_and_authoritative_tables(self):
        with patch.object(resource, 'native_paths', return_value={'ui/dialog.txt': {'native'}, 'mesh/%d.msh': {'native'}}), \
             patch.object(resource, 'client_paths', return_value={'ui/dialog.txt': {'client'}, 'missing.dat': {'client'}}), \
             patch.object(resource, 'runtime_files', return_value={'ui/dialog.bin': Path('dialog.bin'), 'mesh/1.msh': Path('1.msh'), 'mesh/2.msh': Path('2.msh')}), \
             patch.object(resource, 'reference_files', return_value={}), \
             patch.object(resource, 'server_files', return_value={'missing': 'wydgo748/data/missing.dat'}):
            rows, _ = resource.build()
        by_path = {row['path']: row for row in rows}
        self.assertEqual(by_path['ui/dialog.txt']['runtime'], 'as .bin')
        self.assertEqual(by_path['ui/dialog.txt']['gaps'], '')
        self.assertEqual(by_path['mesh/%d.msh']['runtime'], 2)
        self.assertEqual(by_path['mesh/%d.msh']['gaps'], 'N')
        self.assertEqual(by_path['missing.dat']['gaps'], 'TMS')

    def test_regeneration_preserves_manual_decisions(self):
        with tempfile.TemporaryDirectory() as directory:
            tsv, markdown = Path(directory) / 'out.tsv', Path(directory) / 'out.md'
            marker = '<!-- Decisions below are maintained by hand and kept on regeneration. -->'
            markdown.write_text('old generated content\n' + marker + '\nReviewed decision.\n', encoding='utf-8')
            row = {'path': 'a.dat', 'family': '(root)', 'format': '', 'native': '-', 'client': 'client', 'runtime': 0, 'reference_kr': 0, 'server': '', 'gaps': 'TM'}
            counts = resource.write([row], {}, tsv, markdown)
            self.assertEqual(counts['M'], 1)
            text = markdown.read_text(encoding='utf-8')
            self.assertEqual(text.count(marker), 1)
            self.assertTrue(text.endswith(marker + '\nReviewed decision.\n'))
            self.assertNotIn('old generated content', text)


class DataMatrixTests(unittest.TestCase):
    def test_item_record_decodes_signed_fields_and_ignores_trailer(self):
        record = bytearray(140)
        record[:4] = b'Test'
        struct.pack_into('<8h', record, 64, 12, 34, 0, 100, 20, 30, 40, 50)
        struct.pack_into('<Hh', record, 80, 99, -5)
        struct.pack_into('<ihhhh', record, 128, 123456, 2, 3, 4, 5)
        encoded = bytes(value ^ 0x5A for value in record) + b'\x01\x02\x03\x04'
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'ItemList.bin'
            path.write_bytes(encoded)
            with patch.object(data, 'CLIENT_ITEMS', path), patch.object(data, 'ITEM_COUNT', 1):
                row = data.client_items()[0]
            self.assertEqual(row['name'], 'Test')
            self.assertEqual((row['mesh'], row['texture']), (12, 34))
            self.assertEqual(row['req'], (100, 20, 30, 40, 50))
            self.assertEqual(row['effects'], {99: -5})
            self.assertEqual((row['price'], row['unique'], row['pos'], row['extra'], row['grade']), (123456, 2, 3, 4, 5))

    def test_truncated_and_oversized_item_tables_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'ItemList.bin'
            for size in (0, 139, 140, 143, 145):
                path.write_bytes(bytes(size))
                with self.subTest(size=size), patch.object(data, 'CLIENT_ITEMS', path), patch.object(data, 'ITEM_COUNT', 1), self.assertRaises(SystemExit):
                    data.client_items()

    def test_server_numeric_effects_do_not_need_a_name_alias(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'items.csv'
            path.write_text('1,Test,12.34,100.20.30.40.50,2,123456,3,4,5,99,-5,EF_KNOWN,8,EF_UNKNOWN,9\n', encoding='utf-8')
            with patch.object(data, 'SERVER_ITEMS', path):
                rows, unknown = data.server_items({'EF_KNOWN': 100})
            self.assertEqual(rows[1]['effects'], {99: -5, 100: 8})
            self.assertEqual(unknown, {'EF_UNKNOWN'})

    def test_effect_comparison_uses_each_sides_numeric_ids(self):
        row = {'name': 'Test', 'mesh': 1, 'texture': 2, 'req': (0, 0, 0, 0, 0), 'unique': 0, 'price': 1, 'pos': 0, 'extra': 0, 'grade': 0, 'effects': {36: 5}}
        with patch.object(data, 'effect_ids', side_effect=[{'EF_CLIENT': 36}, {'EF_SERVER': 36}]), \
             patch.object(data, 'client_items', return_value={1: row}), \
             patch.object(data, 'server_items', return_value=({1: dict(row)}, set())) as server:
            rows, _, count, _, unknown = data.compare_items()
        server.assert_called_once_with({'EF_SERVER': 36})
        self.assertEqual(rows, [])
        self.assertEqual(count, 1)
        self.assertEqual(unknown, set())

    def test_missing_client_rows_are_not_silently_dropped(self):
        row = {'name': 'Server only'}
        with patch.object(data, 'effect_ids', return_value={}), \
             patch.object(data, 'client_items', return_value={}), \
             patch.object(data, 'server_items', return_value=({3990: row}, set())):
            rows, counts, *_ = data.compare_items()
        self.assertEqual(rows, [(3990, 'Server only', 'missing in client', '', '')])
        self.assertEqual(counts['missing in client'], 1)

    def test_invalid_skill_binary_size_is_not_a_consistency_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'SkillData.bin'
            path.write_bytes(b'bad')
            with patch.object(data, 'CLIENT_SKILL_BIN', path):
                self.assertIsNone(data.compare_skill_bin())


if __name__ == '__main__':
    unittest.main()
