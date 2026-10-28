"""Validate shipped language resources against the active gameplay catalog.

Run from any directory with python -B tools/client-assets/test_client_locales.py.
Historical packages are needed only for explicit regeneration, not these tests.
"""
from collections import Counter
import copy
import json
from pathlib import Path
import re
import struct
import unittest

import build_client_locales as locales


class ClientLocalesTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.runtime = locales.RUNTIME
        cls.policy = json.loads(locales.POLICY.read_text(encoding='utf-8'))
        cls.base_items = locales.item_rows((cls.runtime / 'ItemList.bin').read_bytes())

    def read(self, language, relative):
        return (self.runtime / language / relative).read_bytes()

    def test_only_translation_resources_are_distributed(self):
        expected = {*locales.TEXT_FILES, 'ItemList.bin', 'Itemname.bin',
                    'itemhelp.dat', 'skillhelp.dat', 'UI/captions.txt'}
        expected = {name for name in expected if name == 'UI/captions.txt' or (self.runtime / name).is_file()}
        for language in ('EN', 'PT'):
            actual = {path.relative_to(self.runtime / language).as_posix()
                      for path in (self.runtime / language).rglob('*') if path.is_file()}
            self.assertEqual(actual, expected)
            self.assertNotIn('SkillData.bin', actual)

    def test_global_chat_help_matches_server_visibility(self):
        for language in ('EN', 'PT'):
            rows = self.read(language, 'UI/command.txt').decode('cp1252').splitlines()
            index = next(i for i, row in enumerate(rows) if row.startswith('33FFFF -- '))
            description = rows[index + 1].lower()
            self.assertIn('global', description)
            self.assertNotIn('allies', description)
            self.assertNotIn('aliados', description)

    def test_item_gameplay_bytes_and_wide_values_remain_canonical(self):
        for language in ('EN', 'PT'):
            data = self.read(language, 'ItemList.bin')
            self.assertEqual(struct.unpack('<I', data[-4:])[0], locales.checksum(data[:-4]))
            items = locales.item_rows(data)
            self.assertEqual(len(items), 6500)
            for index, (base, translated) in enumerate(zip(self.base_items, items)):
                self.assertEqual(base[64:], translated[64:], (language, index))
            self.assertFalse((self.runtime / language / 'ItemListWide.csv').exists())

    def test_itemname_ids_and_names_match_each_localized_catalog(self):
        for language in ('EN', 'PT'):
            records = locales.names(self.read(language, 'Itemname.bin'))
            self.assertEqual(set(records), set(range(6500)))
            for index, item in enumerate(locales.item_rows(self.read(language, 'ItemList.bin'))):
                self.assertEqual(item[:64].split(b'\0', 1)[0], records[index], (language, index))

    def test_reassigned_items_use_explicit_current_names(self):
        names = locales.names(self.read('PT', 'Itemname.bin'))
        for index in locales.PROTECTED_ITEMS:
            self.assertEqual(names[index], self.policy['item_names'][str(index)].encode('cp1252'))

    def test_new_and_imported_item_names_use_reviewed_aliases(self):
        for language, field in (('EN', 'english_item_names'), ('PT', 'item_names')):
            records = locales.names(self.read(language, 'Itemname.bin'))
            for index, text in self.policy[field].items():
                self.assertEqual(records[int(index)], text.encode('cp1252'), (language, index))

    def test_custom_help_and_repliction_rules_survive_translation(self):
        for language in ('EN', 'PT'):
            records = locales.help_records(self.read(language, 'itemhelp.dat'))
            canonical = locales.help_records((self.runtime / 'itemhelp.dat').read_bytes())
            self.assertEqual(set(records), set(canonical))
            custom = b' '.join(line[9:] for line in records[3022])
            self.assertIn(b'+9', custom)
            self.assertIn(b'+10', custom)
            self.assertIn(b'+11', custom)
            for index in range(4016, 4026):
                text = b' '.join(line[9:] for line in records[index])
                self.assertIn(str(6 if index < 4021 else 9).encode(), text)
                self.assertNotIn(b'30%', text)

    def test_help_rows_fit_native_buffers(self):
        for language in ('EN', 'PT'):
            for name in ('itemhelp.dat', 'skillhelp.dat'):
                for index, rows in locales.help_records(self.read(language, name)).items():
                    self.assertEqual(len(rows), 9, (language, name, index))
                    for row in rows:
                        self.assertRegex(row, rb'^[0-9A-Fa-f]{8} ')
                        self.assertLess(len(row[9:]), 128, (language, name, index))

    def test_skill_ids_and_formula_numbers_match_current_help(self):
        english = locales.help_records(self.read('EN', 'skillhelp.dat'))
        portuguese = locales.help_records(self.read('PT', 'skillhelp.dat'))
        self.assertEqual(len(english), 102)
        self.assertEqual(set(english), set(portuguese))
        for index in english:
            numbers = lambda rows: Counter(re.findall(rb'\d+(?:[.,]\d+)*', b' '.join(row[9:] for row in rows)))
            self.assertEqual(numbers(english[index]), numbers(portuguese[index]), index)

    def test_quest_keys_levels_and_page_boundaries_are_unchanged(self):
        for suffix in ('', '2', '3'):
            en = self.read('EN', f'UI/QuestSubjects{suffix}.txt').splitlines()
            pt = self.read('PT', f'UI/QuestSubjects{suffix}.txt').splitlines()
            self.assertEqual(len(en), len(pt))
            for before, after in zip(en, pt):
                self.assertEqual(before[:11], after[:11])
                self.assertEqual(re.search(rb'\bLV\s*\d+', before)[0], re.search(rb'\bLV\s*\d+', after)[0])
            for language in ('EN', 'PT'):
                pages = self.read(language, f'UI/QuestContents{suffix}.txt').splitlines()
                self.assertEqual(len(pages), len(en) * 20)
                for row in pages:
                    self.assertRegex(row, rb'^[0-9A-Fa-f]{6}( |$)')
                    self.assertLess(len(row[7:]), 128)

    def test_quest_numeric_rules_preserve_current_values(self):
        for suffix, indices in (('', (1, 3, 5, 6, 8, 9, 10, 11, 12)), ('3', (1, 2, 3, 4))):
            en = self.read('EN', f'UI/QuestContents{suffix}.txt').splitlines()
            pt = self.read('PT', f'UI/QuestContents{suffix}.txt').splitlines()
            for index in indices:
                numbers = lambda rows: Counter(re.findall(rb'\d+', b' '.join(row[7:] for row in rows)))
                self.assertEqual(numbers(en[index * 20:(index + 1) * 20]),
                                 numbers(pt[index * 20:(index + 1) * 20]), (suffix, index))

    def test_server_messages_keep_printf_contracts_and_valid_checksum(self):
        en = self.read('EN', 'UI/strdef.bin'); pt = self.read('PT', 'UI/strdef.bin')
        self.assertEqual(len(en), len(pt))
        self.assertEqual(struct.unpack('<I', pt[-4:])[0], locales.checksum(pt[:-4]))
        for offset in range(0, len(en) - 4, 128):
            decode = lambda data: bytes(value ^ 0x5a for value in data[offset:offset + 128]).split(b'\0', 1)[0]
            self.assertEqual(locales.format_tokens(decode(en)), locales.format_tokens(decode(pt)), offset // 128)

    def test_mystic_land_messages_are_localized_with_english_fallback(self):
        for language in ('EN', 'PT'):
            data = self.read(language, 'UI/strdef.bin')
            self.assertEqual(struct.unpack('<I', data[-4:])[0], locales.checksum(data[:-4]))
            for index in range(440, 445):
                raw = data[index * 128:(index + 1) * 128]
                text = bytes(value ^ 0x5a for value in raw).split(b'\0', 1)[0].decode('cp1252')
                self.assertEqual(text, self.policy['messages'][str(index)][language])
        base = (self.runtime / 'UI/strdef.bin').read_bytes()
        self.assertEqual(base, self.read('EN', 'UI/strdef.bin'))
        self.assertEqual(locales.message_overrides(base, self.policy['messages'], 'UNKNOWN'), base)
        gap = str((len(base) - 4) // 128 + 1)
        for invalid in ({gap: {'EN': 'Gap'}}, {'445': {'EN': '%d', 'PT': '%s'}},
                        {'445': {'EN': 'x' * 128}}, {'445': {'EN': 'Bad\nrow'}}):
            with self.assertRaises(ValueError):
                locales.message_overrides(base, invalid, 'PT')

    def test_official_npc_messages_preserve_locale_and_parameter_contract(self):
        base = (self.runtime / 'UI/strdef.bin').read_bytes()
        for language in ('EN', 'PT'):
            data = self.read(language, 'UI/strdef.bin')
            self.assertEqual(struct.unpack('<I', data[-4:])[0], locales.checksum(data[:-4]))
            for index in range(445, 452):
                raw = data[index * 128:(index + 1) * 128]
                text = bytes(value ^ 0x5a for value in raw).split(b'\0', 1)[0]
                self.assertEqual(text.decode('cp1252'), self.policy['messages'][str(index)][language])
                self.assertLess(len(text), 128)
                self.assertEqual(locales.format_tokens(text), [b'%s', b'%s'] if index == 451 else [])
        self.assertEqual(base, self.read('EN', 'UI/strdef.bin'))
        self.assertEqual(locales.message_overrides(base, self.policy['messages'], 'UNKNOWN'), base)

    def test_ui_string_indices_preserve_current_contract(self):
        def parse(data):
            result = {}
            for row in data.splitlines():
                if not row.strip(): continue
                index, text = row.split(None, 1)
                self.assertNotIn(int(index), result)
                self.assertLess(len(text), 64)
                result[int(index)] = text
            return result
        en = parse(self.read('EN', 'UI/UIString.txt')); pt = parse(self.read('PT', 'UI/UIString.txt'))
        self.assertEqual(set(en), set(pt))
        self.assertEqual(len(pt), 228)

    def test_custom_help_pagination_and_command_tokens_are_preserved(self):
        for name in ('interface.txt', 'interface1.txt', 'interface2.txt', 'interface3.txt',
                     'etc.txt', 'command.txt', 'hellStoredesc.txt'):
            self.assertEqual(len(self.read('EN', 'UI/' + name).splitlines()),
                             len(self.read('PT', 'UI/' + name).splitlines()), name)
        for language in ('EN', 'PT'):
            commands = self.read(language, 'UI/command.txt')
            for token in (b'/summonguildmembers', b'/Endwar', b'/Endallies', b'/Kingdom'):
                self.assertIn(token, commands)
            interface = self.read(language, 'UI/interface.txt')
            for token in (b'Shift', b'Ctrl', b'C.C.'):
                self.assertIn(token, interface)

    def test_caption_keys_cover_all_shared_layout_captions(self):
        def parse(language):
            result = {}
            for row in self.read(language, 'UI/captions.txt').splitlines():
                key, value = row.split(b'\t', 1)
                self.assertNotIn(key, result)
                self.assertLess(len(value), 128)
                result[key] = value
            return result
        en = parse('EN'); pt = parse('PT')
        self.assertEqual(set(en), set(pt))
        self.assertEqual(len(pt), 259)
        for key in en:
            self.assertTrue((self.runtime / 'UI' / key.decode().split(':')[0]).is_file())

    def test_changed_canonical_help_requires_translation_review(self):
        policy = copy.deepcopy(self.policy)
        policy['canonical_text_sha256']['skillhelp.dat'] = '0' * 64
        with self.assertRaisesRegex(ValueError, 'Canonical text changed: skillhelp.dat'):
            locales.build(self.runtime, Path('unused-historical-input'), policy)

    def test_changed_catalog_or_names_requires_translation_review(self):
        for field, message in (('canonical_itemlist_sha256', 'Canonical item catalog changed'),
                               ('canonical_itemname_sha256', 'Canonical item names changed')):
            policy = copy.deepcopy(self.policy)
            policy[field] = '0' * 64
            with self.assertRaisesRegex(ValueError, message):
                locales.build(self.runtime, Path('unused-historical-input'), policy)


if __name__ == '__main__':
    unittest.main(verbosity=2)
