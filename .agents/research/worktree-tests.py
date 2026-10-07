"""Tests for the working-tree inventory and the opcode size coverage (audit A03/A04)."""
import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(filename))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


worktree = load('worktree', 'worktree.py')
matrix = load('matrix', 'opcode-matrix.py')


class InventoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        git = lambda *args: subprocess.run(['git', *args], cwd=self.root, check=True, capture_output=True)
        git('init', '-q')
        git('config', 'user.email', 'test@example.invalid')
        git('config', 'user.name', 'test')
        src = self.root / 'src'
        src.mkdir()
        (src / 'kept.go').write_text('package a\n')
        (src / 'deleted.go').write_text('package a\n')
        (self.root / '.gitignore').write_text('*.log\n')
        git('add', '.')
        git('commit', '-q', '-m', 'base')
        (src / 'deleted.go').unlink()                 # deleted in the working tree only
        (src / 'untracked.go').write_text('package a\n')  # new handler, not committed
        (src / 'ignored.log').write_text('x\n')

    def tearDown(self):
        self.temp.cleanup()

    def test_untracked_and_existing_files_only(self):
        names = [p.name for p in worktree.files('src', ('.go',), self.root)]
        self.assertEqual(names, ['kept.go', 'untracked.go'])

    def test_ignored_and_other_suffixes_are_left_out(self):
        self.assertEqual(worktree.relative('src', None, self.root), ['src/kept.go', 'src/untracked.go'])

    def test_unreadable_file_is_not_hidden(self):
        # A listed path that turns into a directory is skipped by is_file(); a
        # real read failure still surfaces at the caller's read.
        path = worktree.files('src', ('.go',), self.root)[0]
        path.unlink()
        path.mkdir()
        with self.assertRaises(OSError):
            path.read_text()


class SizeCoverageTests(unittest.TestCase):
    def test_matching_sizes_have_no_gap(self):
        self.assertEqual(matrix.size_gaps(24, 24, 24, True, True), [])

    def test_real_mismatch_is_d(self):
        self.assertEqual(matrix.size_gaps(24, 24, 16, True, True), ['D'])

    def test_unknown_server_size_is_u_not_a_match(self):
        self.assertEqual(matrix.size_gaps(24, 24, None, True, True), ['U'])

    def test_unknown_client_gate_size_is_u(self):
        self.assertEqual(matrix.size_gaps(108, None, 108, True, False), ['U'])

    def test_sides_that_do_not_receive_need_no_size(self):
        self.assertEqual(matrix.size_gaps(16, None, None, False, False), [])


if __name__ == '__main__':
    unittest.main()
