"""Regression tests for extract-method.py safety rules and verification."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('extract', Path(__file__).with_name('extract-method.py'))
extract = importlib.util.module_from_spec(spec)
spec.loader.exec_module(extract)

SOURCE = '''#include "pch.h"

int Owner::Run(int value, int other)
{
\tint total = value;
\tchar text[8]{};
\tif (value > 3)
\t{
\t\ttotal += other;
\t\ttext[0] = 'a';
\t\tm_count = total;
\t}
\telse
\t{
\t\tm_count = 0;
\t}
\treturn total + text[0];
}
'''
HEADER = '''#pragma once
class Owner
{
public:
\tint Run(int value, int other);
private:
\tint m_count;
};
'''


HANDLER_TAIL = '\t\tm_count = total;\n\t}\n\telse'
HANDLER_TAIL_RETURNING = '\t\tif (other)\n\t\t\treturn 1;\n\t\tm_count = total;\n\t\treturn total;\n\t}\n\telse'
HANDLER_TAIL_FALLING_THROUGH = '\t\tif (other)\n\t\t\treturn 1;\n\t\tm_count = total;\n\t}\n\telse'


LOOP_SOURCE = '''
int Owner::Loop(int value, int other)
{
\tint total = 0;
\tfor (int i = 0; i < value; ++i)
\t{
\t\tswitch (i)
\t\t{
\t\tcase 1:
\t\t{
\t\t\tif (other)
\t\t\t\treturn total;
\t\t\tfor (int j = 0; j < 3; ++j)
\t\t\t{
\t\t\t\tif (j == other)
\t\t\t\t\tbreak;
\t\t\t\ttotal += j;
\t\t\t}
\t\t\tif (total > 9)
\t\t\t\tcontinue;
\t\t\tm_count = total;
\t\t}
\t\tbreak;
\t\tdefault:
\t\t\tbreak;
\t\t}
\t}
\treturn total;
}
'''


class ExtractMethodTests(unittest.TestCase):
    def setUp(self):
        self.dir = Path(tempfile.mkdtemp())
        (self.dir / 'Owner.cpp').write_text(SOURCE, encoding='utf-8', newline='\n')
        (self.dir / 'Owner.h').write_text(HEADER, encoding='utf-8', newline='\n')

    def spec(self, extractions):
        return {'source': str(self.dir / 'Owner.cpp'), 'header': str(self.dir / 'Owner.h'), 'class': 'Owner',
                'method': 'int Owner::Run(int value, int other)', 'extractions': extractions}

    def test_valid_extraction_is_verified_and_written(self):
        extract.run(self.spec([{'helper': 'Accumulate', 'after': 'if (value > 3)',
                                'params': [['int&', 'total'], ['int&', 'other'], ['char (&)[8]', 'text']]}]), True)
        source = (self.dir / 'Owner.cpp').read_text(encoding='utf-8')
        header = (self.dir / 'Owner.h').read_text(encoding='utf-8')
        self.assertIn('Accumulate(total, other, text);', source)
        self.assertIn('void Owner::Accumulate(int& total, int& other, char (&text)[8])', source)
        self.assertIn('void Accumulate(int& total, int& other, char (&text)[8]);', header)

    def test_rejects_control_escape(self):
        with self.assertRaisesRegex(ValueError, 'return'):
            source = SOURCE.replace("\t\tm_count = total;\n", "\t\treturn total;\n")
            (self.dir / 'Owner.cpp').write_text(source, encoding='utf-8', newline='\n')
            extract.run(self.spec([{'helper': 'Accumulate', 'after': 'if (value > 3)',
                                    'params': [['int&', 'total'], ['int&', 'other'], ['char (&)[8]', 'text']]}]), False)

    def test_rejects_missing_outer_local(self):
        with self.assertRaisesRegex(ValueError, 'not parameters'):
            extract.run(self.spec([{'helper': 'Accumulate', 'after': 'if (value > 3)',
                                    'params': [['int&', 'total'], ['char (&)[8]', 'text']]}]), False)

    def test_rejects_unused_parameter(self):
        with self.assertRaisesRegex(ValueError, 'not used'):
            extract.run(self.spec([{'helper': 'Clear', 'after': 'else', 'params': [['int&', 'total']]}]), False)

    def test_rejects_ambiguous_anchor(self):
        with self.assertRaisesRegex(ValueError, 'exactly one line'):
            extract.run(self.spec([{'helper': 'X', 'after': '{', 'params': []}]), False)

    def test_escape_analysis(self):
        escapes = lambda code: extract.control_escapes(extract.tokens(code))
        self.assertEqual(escapes('for (int i = 0; i < 3; ++i) { if (i) break; continue; }'), [])
        self.assertEqual(escapes('switch (x) { case 1: y = 2; break; }'), [])
        self.assertEqual(escapes('do { if (x) break; } while (x);'), [])
        self.assertEqual(escapes('if (x) { break; }'), ['break'])
        self.assertEqual(escapes('switch (x) { case 1: continue; }'), ['continue'])
        self.assertEqual(escapes('for (;;) { } break;'), ['break'])
        self.assertEqual(escapes('if (x) return;'), ['return'])

    def test_declaration_detection(self):
        names = lambda code: extract.declared_names(extract.tokens(code))
        self.assertEqual(names('{ int a = 1; for (int i = 0; i < N; ++i) { } char b[4]{}; auto* p = q; }'[:-1]),
                         {'a', 'b', 'p'})
        self.assertEqual(names('{ if (x) { int hidden = 1; } x = y < z; }'[:-1]), set())
        self.assertEqual(names('{ std::vector<int> v; const char* s = t; unsigned int u; }'[:-1]), {'v', 's', 'u'})
        self.assertEqual(extract.signature_parameters('int A::F(int a, char (&b)[8], const X* c = nullptr)'),
                         {'a', 'b', 'c'})

    def test_nth_anchor_selects_one_match(self):
        extract.run(self.spec([{'helper': 'Clear', 'after': 'else', 'nth': 0, 'params': []}]), False)
        with self.assertRaisesRegex(ValueError, 'only 1 matches'):
            extract.run(self.spec([{'helper': 'Clear', 'after': 'else', 'nth': 1, 'params': []}]), False)

    def test_return_terminated_handler(self):
        source = SOURCE.replace(HANDLER_TAIL, HANDLER_TAIL_RETURNING)
        (self.dir / 'Owner.cpp').write_text(source, encoding='utf-8', newline='\n')
        extract.run(self.spec([{'helper': 'Handle', 'after': 'if (value > 3)', 'returns': True,
                                'params': [['int&', 'total'], ['int&', 'other'], ['char (&)[8]', 'text']]}]), True)
        written = (self.dir / 'Owner.cpp').read_text(encoding='utf-8')
        self.assertIn('return Handle(total, other, text);', written)
        self.assertIn('int Owner::Handle(int& total, int& other, char (&text)[8])', written)
        self.assertIn('int Handle(int& total, int& other, char (&text)[8]);', (self.dir / 'Owner.h').read_text(encoding='utf-8'))

    def test_multiline_condition_anchor(self):
        source = SOURCE.replace('\tif (value > 3)\n', '\tif (value > 3 &&\n\t\t(other || value))\n')
        (self.dir / 'Owner.cpp').write_text(source, encoding='utf-8', newline='\n')
        extract.run(self.spec([{'helper': 'Accumulate', 'after': 'if (value > 3 &&',
                                'params': [['int&', 'total'], ['int&', 'other'], ['char (&)[8]', 'text']]}]), True)
        written = (self.dir / 'Owner.cpp').read_text(encoding='utf-8')
        self.assertIn('\tif (value > 3 &&\n\t\t(other || value))\n\t{\n\t\tAccumulate(total, other, text);', written)

    def test_returns_requires_final_return(self):
        source = SOURCE.replace(HANDLER_TAIL, HANDLER_TAIL_FALLING_THROUGH)
        (self.dir / 'Owner.cpp').write_text(source, encoding='utf-8', newline='\n')
        with self.assertRaisesRegex(ValueError, 'final top-level return'):
            extract.run(self.spec([{'helper': 'Handle', 'after': 'if (value > 3)', 'returns': True,
                                    'params': [['int&', 'total'], ['int&', 'other'], ['char (&)[8]', 'text']]}]), False)
        self.assertTrue(extract.ends_with_return(extract.tokens('a = 1; if (b) { return 2; } return 3;')))
        self.assertFalse(extract.ends_with_return(extract.tokens('if (b) { return 2; }')))

    def test_statement_range(self):
        source = SOURCE.replace('\tint total = value;\n\tchar text[8]{};\n',
                                '\tint total = value;\n\tchar text[8]{};\n\ttotal += 2;\n\tm_count = total;\n\ttext[1] = 0;\n')
        (self.dir / 'Owner.cpp').write_text(source, encoding='utf-8', newline='\n')
        extract.run(self.spec([{'helper': 'Prepare', 'from': 'total += 2;', 'until': 'text[1] = 0;',
                                'params': [['int&', 'total']]}]), True)
        written = (self.dir / 'Owner.cpp').read_text(encoding='utf-8')
        self.assertIn('\tPrepare(total);\n\ttext[1] = 0;', written)
        self.assertIn('void Owner::Prepare(int& total)\n{\n\ttotal += 2;\n\tm_count = total;\n}', written)

    def test_range_rejects_leaked_declaration(self):
        source = SOURCE.replace('\tchar text[8]{};\n', '\tchar text[8]{};\n\tint step = 2;\n\ttotal += step;\n')
        (self.dir / 'Owner.cpp').write_text(source, encoding='utf-8', newline='\n')
        with self.assertRaisesRegex(ValueError, 'used after it'):
            extract.run(self.spec([{'helper': 'Step', 'from': 'int step = 2;', 'until': 'total += step;',
                                    'params': []}]), False)

    def test_range_rejects_partial_statement(self):
        with self.assertRaisesRegex(ValueError, 'complete statements'):
            extract.run(self.spec([{'helper': 'Half', 'from': 'if (value > 3)', 'until': 'm_count = total;',
                                    'params': [['int&', 'value']]}]), False)

    def flow_spec(self, extraction):
        spec = self.spec([extraction])
        spec['method'] = 'int Owner::Loop(int value, int other)'
        return spec

    def write_loop(self):
        (self.dir / 'Owner.cpp').write_text(SOURCE + LOOP_SOURCE, encoding='utf-8', newline='\n')

    def test_flow_block_rewrites_only_escapes(self):
        self.write_loop()
        extract.run(self.flow_spec({'helper': 'HandleOne', 'after': 'case 1:', 'flow': True,
                                    'params': [['int&', 'other'], ['int&', 'total']]}), True)
        written = (self.dir / 'Owner.cpp').read_text(encoding='utf-8')
        header = (self.dir / 'Owner.h').read_text(encoding='utf-8')
        self.assertIn('const ExtractedFlow extractedFlow = HandleOne(other, total, extractedResult);', written)
        self.assertIn('if (extractedFlow == ExtractedFlow::Return)\n\t\t\t\treturn extractedResult;', written)
        self.assertIn('if (extractedFlow == ExtractedFlow::Continue)\n\t\t\t\tcontinue;', written)
        self.assertNotIn('ExtractedFlow::Break', written)
        self.assertIn('{ extractedResult = total; return ExtractedFlow::Return; }', written)
        self.assertIn('\t\tif (j == other)\n\t\t\tbreak;\n', written)   # the inner loop's break is untouched
        self.assertIn('\tm_count = total;\n\treturn ExtractedFlow::Next;\n}', written)
        self.assertIn('ExtractedFlow HandleOne(int& other, int& total, int& extractedResult);', header)
        self.assertIn('#include "ExtractedFlow.h"', header)

    def test_flow_inverse_rejects_converted_inner_break(self):
        produced = extract.flow_inverse(extract.tokens('for (;;) { return ExtractedFlow::Break; } return ExtractedFlow::Next;'))
        restored, positions = produced
        self.assertEqual(restored, extract.tokens('for (;;) { break; }'))
        self.assertNotEqual(sorted(positions), sorted(extract.escape_positions(restored)))

    def test_flow_requires_escape(self):
        with self.assertRaisesRegex(ValueError, 'no escaping statement'):
            extract.run(self.spec([{'helper': 'Clear', 'after': 'else', 'flow': True, 'params': []}]), False)

    def test_member_only_block_needs_no_parameters(self):
        extract.run(self.spec([{'helper': 'Clear', 'after': 'else', 'params': []}]), True)
        self.assertIn('void Owner::Clear()', (self.dir / 'Owner.cpp').read_text(encoding='utf-8'))


if __name__ == '__main__':
    unittest.main()
