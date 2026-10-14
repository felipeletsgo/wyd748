"""Exercise the root-only Turnstile environment installer without real keys."""
import importlib.util
import io
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("turnstile_secret", Path(__file__).parent / "vps" / "set-turnstile-secret.py")
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


class DeploymentContractTests(unittest.TestCase):
    def test_initial_install_configures_verification_before_starting_api(self):
        script = (Path(__file__).parent / "vps" / "first-install.sh").read_text(encoding="utf-8")
        self.assertLess(script.index('python3 "$stage/config/set-turnstile-secret.py"'), script.index("systemctl enable --now wyd-player"))


@unittest.skipUnless(os.name == "posix", "Permission checks require a POSIX host")
class SecretInstallerTests(unittest.TestCase):
    def test_rotation_updates_both_authentication_services(self):
        with tempfile.TemporaryDirectory() as directory:
            game, player = Path(directory) / 'game.env', Path(directory) / 'player.env'
            game.write_text('WYD_DATABASE_URL=unchanged\n')
            player.write_text('WYD_TURNSTILE_SECRET=test-value-with-no-real-secret\nOTHER=unchanged\n')
            output = io.StringIO()
            with patch.object(installer, 'Path', side_effect=[game, player]), patch.object(installer.os, 'geteuid', return_value=0), patch.object(installer.sys, 'argv', ['installer', '--from-player-env']), patch.object(installer.sys, 'stdout', output):
                installer.main()
            for path in (game, player):
                self.assertIn('WYD_TURNSTILE_SECRET=test-value-with-no-real-secret\n', path.read_text())
                self.assertEqual(path.stat().st_mode & 0o777, 0o600)
            self.assertNotIn('test-value-with-no-real-secret', output.getvalue())

    def test_rotation_preflight_preserves_configuration(self):
        with tempfile.TemporaryDirectory() as directory:
            game, player = Path(directory) / 'missing.env', Path(directory) / 'player.env'
            original = 'WYD_TURNSTILE_SECRET=test-value-with-no-real-secret\n'
            player.write_text(original)
            with patch.object(installer, 'Path', side_effect=[game, player]), patch.object(installer.os, 'geteuid', return_value=0), patch.object(installer.sys, 'argv', ['installer', '--from-player-env']):
                with self.assertRaises(ValueError):
                    installer.main()
            self.assertEqual(player.read_text(), original)

    def test_atomic_replacement_preserves_other_configuration(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "player.env"
            path.write_text("# Keep configuration\nOTHER=value\nWYD_TURNSTILE_SECRET=old\nWYD_TURNSTILE_SECRET=duplicate\n", encoding="utf-8")
            installer.configure(path, "test-value-with-no-real-secret")
            self.assertEqual(path.stat().st_mode & 0o777, 0o600)
            self.assertEqual(path.read_text(), "# Keep configuration\nOTHER=value\nWYD_TURNSTILE_SECRET=test-value-with-no-real-secret\n")
            self.assertEqual(list(Path(directory).iterdir()), [path])

    def test_invalid_input_and_symlink_leave_original_untouched(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "player.env"
            path.write_text("OTHER=value\n", encoding="utf-8")
            for value in ["", "short", "x" * 257, "x" * 20 + "\n", "x" * 20 + "'", "1x0000000000000000000000000000000AA"]:
                with self.assertRaises(ValueError):
                    installer.configure(path, value)
                self.assertEqual(path.read_text(), "OTHER=value\n")
            alias = Path(directory) / "alias"
            alias.symlink_to(path)
            with self.assertRaises(ValueError):
                installer.configure(alias, "test-value-with-no-real-secret")
            with self.assertRaises(ValueError):
                installer.configure(Path(directory) / "missing", "test-value-with-no-real-secret")


if __name__ == "__main__":
    unittest.main()
