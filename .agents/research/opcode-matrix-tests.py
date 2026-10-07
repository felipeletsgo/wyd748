"""Regression tests for opcode-matrix.py parsers."""
import importlib.util
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('matrix', Path(__file__).with_name('opcode-matrix.py'))
matrix = importlib.util.module_from_spec(spec)
spec.loader.exec_module(matrix)

GATE = """uint FUN_0055890a(short *param_1)
{
    uVar1 = param_1[2];
    if (uVar1 < 0x36c) {
      if (uVar1 == 0x36b) {
        local_8 = (uint)(*param_1 != 0x3c);
      }
      else {
        switch(uVar1) {
        case 0x101:
          local_8 = (uint)(*param_1 != 0x6c);
          break;
        case 899:
          local_8 = (uint)(*param_1 != 0x9c);
          break;
        case 0x3ca:
          if (*param_1 != 0x24) {
            local_8 = 1;
          }
          break;
        }
      }
    }
    else if (uVar1 == 0x3c0f) {
      local_8 = (uint)(*param_1 != 0x754);
    }
}
"""

CORPUS = {
    'FUN_00400000': """void FUN_00400000(int param_1)
{
  if (*(short *)(param_1 + 4) == 0x36b) {
    return;
  }
  switch(*(short *)(param_1 + 4)) {
  case 899:
    break;
  }
}
""",
    'FUN_00400100': """void FUN_00400100(void)
{
  local_38 = 0x366;
  FUN_0055f2dd(local_3c,0x34);
}
""",
    'FUN_00400200': """void FUN_00400200(void)
{
  local_d28 = 0xed7;
  _memset(local_d2c,0,0x10);
  FUN_0055f2dd();
  *(undefined2 *)(DAT_013b71e8 + 0xd00) = 0x3a6;
}
""",
    'FUN_00400300': """void FUN_00400300(int param_1)
{
  *(undefined2 *)(param_1 + 0x162) = 0x118;
  *(undefined2 *)(param_1 + 0x162) = 0x119;
  *(undefined2 *)(param_1 + 0x162) = 0x125;
}
""",
    'FUN_00400400': """void FUN_00400400(void)
{
  _memset((void *)(local_1c + 0x26e54),0,0x20);
  *(undefined2 *)(local_1c + 0x26e58) = 0x3b8;
}
""",
    'FUN_0055f2dd': """void FUN_0055f2dd(int param_1)
{
  if (*(short *)(param_1 + 4) != 0x366) {
    return;
  }
}
""",
}


class OpcodeMatrixTests(unittest.TestCase):
    def setUp(self):
        self.dir = Path(tempfile.mkdtemp())

    def test_gate_reads_equalities_cases_and_inequality_form(self):
        gate = self.dir / 'gate.c'
        gate.write_text(GATE, encoding='utf-8')
        self.assertEqual(matrix.native_gate(gate), {0x36b: 0x3c, 0x101: 0x6c, 0x383: 0x9c, 0x3ca: 0x24, 0x3c0f: 0x754})

    def test_corpus_receive_send_and_stored_forms(self):
        corpus = self.dir / 'decompiled'
        corpus.mkdir()
        for name, code in CORPUS.items():
            (corpus / f'{name}.c').write_text(code, encoding='utf-8')
        sites = matrix.native_corpus(corpus, opcodes={0x36b, 0x383, 0x366, 0xed7, 0x3a6, 0x119}, exports=self.dir)
        self.assertEqual(sites[0x36b]['recv'], {'FUN_00400000'})
        self.assertEqual(sites[0x383]['recv'], {'FUN_00400000'})         # decimal switch case
        self.assertEqual(sites[0x366]['send'], {'FUN_00400100'})         # header + send call
        self.assertEqual(sites[0x366]['send_sizes'], {0x34})
        self.assertEqual(sites[0x366]['recv'], set())                    # the send routine is no receiver
        self.assertEqual(sites[0xed7]['send_sizes'], {0x10})             # sized by the memset
        self.assertEqual(sites[0x3a6]['stored'], {'FUN_00400200'})       # prebuilt packet
        self.assertEqual(sites[0x119]['stored'], set())                 # selector field, not a packet
        self.assertEqual(sites[0x3b8]['stored'], {'FUN_00400400'})       # cleared header, unknown opcode
        self.assertEqual(sites[0x3b8]['send_sizes'], {0x20})
    def test_missing_corpus_is_empty(self):
        self.assertEqual(len(matrix.native_corpus(self.dir / "absent")), 0)


if __name__ == "__main__":
    unittest.main()
