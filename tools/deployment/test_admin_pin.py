"""Credential-generation regression tests; fixtures never touch a live host."""

import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("admin_pin", Path(__file__).parent / "vps/admin-pin.py")
admin_pin = importlib.util.module_from_spec(spec)
spec.loader.exec_module(admin_pin)


class AdminPINTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / "game.env"

    def test_generation_preserves_other_credentials_and_leading_zeroes(self):
        self.path.write_text("WYD_DATABASE_URL=fixture", encoding="utf-8")
        with patch.object(admin_pin.secrets, "randbelow", return_value=1234):
            self.assertEqual(admin_pin.configure(self.path), "created")
        self.assertEqual(self.path.read_text(encoding="utf-8"), "WYD_DATABASE_URL=fixture\nWYD_ADMIN_PIN=001234\n")

    def test_valid_pin_is_not_rotated(self):
        original = "WYD_DATABASE_URL=fixture\nWYD_ADMIN_PIN=001234\n"
        self.path.write_text(original, encoding="utf-8")
        self.assertEqual(admin_pin.configure(self.path, migrate=True), "unchanged")
        self.assertEqual(self.path.read_text(encoding="utf-8"), original)

    def test_invalid_pin_requires_explicit_migration(self):
        for value in ("12345678", "12345", "abcdef", "１２３４５６", ""):
            with self.subTest(value=value):
                original = f"WYD_DATABASE_URL=fixture\nWYD_ADMIN_PIN={value}\n"
                self.path.write_text(original, encoding="utf-8")
                with self.assertRaisesRegex(ValueError, "explicit --migrate"):
                    admin_pin.configure(self.path)
                self.assertEqual(self.path.read_text(encoding="utf-8"), original)
                with patch.object(admin_pin.secrets, "randbelow", return_value=999999):
                    self.assertEqual(admin_pin.configure(self.path, migrate=True), "migrated")
                self.assertEqual(self.path.read_text(encoding="utf-8"), "WYD_DATABASE_URL=fixture\nWYD_ADMIN_PIN=999999\n")

    def test_duplicates_are_rejected_without_changing_file(self):
        original = "WYD_ADMIN_PIN=123456\nWYD_ADMIN_PIN=654321\n"
        self.path.write_text(original, encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "Duplicate"):
            admin_pin.configure(self.path, migrate=True)
        self.assertEqual(self.path.read_text(encoding="utf-8"), original)


if __name__ == "__main__":
    unittest.main()
