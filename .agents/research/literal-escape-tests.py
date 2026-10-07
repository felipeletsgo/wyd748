"""Failure-path tests for literal-escape.py verify() (audit A07).

The compiler is replaced by a fake; sources live in a temporary directory, so
no project file is touched. Every rejected operation must leave the exact
original bytes on disk.
"""
import importlib.util
import tempfile
import types
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('literal_escape', Path(__file__).with_name('literal-escape.py'))
literal_escape = importlib.util.module_from_spec(spec)
spec.loader.exec_module(literal_escape)


class FakeCompiler:
    """Digest of a unit = its current bytes, unless a failure is scripted."""

    def __init__(self, work, fail_on_call=None, constant_digest=False):
        self.calls = 0
        self.fail_on_call = fail_on_call
        self.constant_digest = constant_digest
        self.os = __import__('os')
        self.subprocess = __import__('subprocess')
        self.WORK = work

    def compiler_environment(self):
        return {}

    def recorded_commands(self):
        return Commands()

    def run_cl(self, *args):
        return types.SimpleNamespace(returncode=0)

    def adapt(self, *args, **kwargs):
        return []

    def compile_unit(self, env, command, unit, obj, pch):
        self.calls += 1
        if self.fail_on_call == self.calls:
            return False, 'scripted compiler failure'
        obj.write_bytes(b'same' if self.constant_digest else unit.read_bytes())
        return True, ''

    def digest(self, path):
        return path.read_bytes()


class Commands(dict):
    """Every unit (and pch.cpp) has a recorded command."""

    def __getitem__(self, key):
        return []

    def __iter__(self):
        return iter([literal_escape.prune.os.path.normcase(r'C:\x\pch.cpp')])


class VerifyRollbackTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        root = Path(self.temp.name)
        self.source = root / 'unit.cpp'
        self.original = b'int value = 1;\n'
        self.source.write_bytes(self.original)
        self.candidate = {self.source: b'int value = 2;\n'}
        self.saved_prune = literal_escape.prune
        self.saved_project = literal_escape.PROJECT
        literal_escape.PROJECT = root

    def tearDown(self):
        literal_escape.prune = self.saved_prune
        literal_escape.PROJECT = self.saved_project
        self.temp.cleanup()

    def run_verify(self, compiler, write):
        literal_escape.prune = compiler
        args = types.SimpleNamespace(write=write, against=None)
        literal_escape.verify(args, dict(self.candidate))

    def test_baseline_failure_writes_nothing(self):
        compiler = FakeCompiler(Path(self.temp.name), fail_on_call=1)
        with self.assertRaises(RuntimeError):
            self.run_verify(compiler, write=True)
        self.assertEqual(self.source.read_bytes(), self.original)

    def test_candidate_failure_after_write_restores(self):
        compiler = FakeCompiler(Path(self.temp.name), fail_on_call=2)
        with self.assertRaises(RuntimeError):
            self.run_verify(compiler, write=True)
        self.assertEqual(self.source.read_bytes(), self.original)

    def test_digest_mismatch_restores(self):
        compiler = FakeCompiler(Path(self.temp.name))
        with self.assertRaises(SystemExit):
            self.run_verify(compiler, write=True)
        self.assertEqual(self.source.read_bytes(), self.original)

    def test_successful_dry_run_restores(self):
        compiler = FakeCompiler(Path(self.temp.name), constant_digest=True)
        self.run_verify(compiler, write=False)
        self.assertEqual(self.source.read_bytes(), self.original)

    def test_successful_write_keeps_candidate(self):
        compiler = FakeCompiler(Path(self.temp.name), constant_digest=True)
        self.run_verify(compiler, write=True)
        self.assertEqual(self.source.read_bytes(), self.candidate[self.source])

    def test_restoration_error_is_reported(self):
        missing = Path(self.temp.name) / 'missing-dir' / 'gone.cpp'
        with self.assertRaisesRegex(RuntimeError, 'Could not restore'):
            literal_escape.restore({missing: b'x'})


if __name__ == '__main__':
    unittest.main()
