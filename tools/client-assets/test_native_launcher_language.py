"""Validate the native launcher extension without running its patch/update flow."""
import argparse
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path

import build_native_launcher_language as extension
from build_wyd_launcher import parse_dialog, resource


class NativeLauncherTests(unittest.TestCase):
    def test_original_code_data_and_imports_are_retained(self):
        expected = extension.patch_version_format(self.baseline)
        for name, _, _, size, offset in self.before.sections:
            if name == b".rsrc":
                continue
            after = next(s for s in self.after.sections if s[0] == name)
            self.assertEqual(expected[offset:offset + size], self.candidate[after[4]:after[4] + size])
        self.assertEqual(self.before.imports(), self.after.imports()[:-1])
        self.assertEqual(len(self.after.imports()), len(self.before.imports()) + 1)
        _, _, _, dll_name, iat = struct.unpack("<IIIII", self.after.imports()[-1])
        name_at = self.after.offset(dll_name)
        self.assertEqual(self.candidate[name_at:self.candidate.index(0, name_at)], extension.DLL_NAME[:-1])
        self.assertEqual(self.after.import_address(b"InitializeClientLanguage"), self.after.base + iat)

    def test_version_format_shows_release_line(self):
        at = self.after.offset(extension.VERSION_FORMAT_RVA)
        text = self.candidate[at:self.candidate.index(0, at)]
        self.assertEqual(text + b"\0", extension.VERSION_FORMAT_AFTER)
        self.assertEqual(text.count(b"%"), 2)  # same two integer conversions as native
        self.assertEqual(text.decode() % (48001 // 1000, 48001 % 1000), "Ready to play.  (Version 7.48.001)")
        native = self.before.offset(extension.VERSION_FORMAT_RVA)
        changed = [i for i, (a, b) in enumerate(zip(self.baseline, extension.patch_version_format(self.baseline))) if a != b]
        self.assertTrue(changed and all(native <= i < native + extension.VERSION_FORMAT_SLOT for i in changed))
        with self.assertRaisesRegex(ValueError, "version format slot"):
            extension.patch_version_format(self.candidate)

    def test_entry_restores_registers_and_reaches_original_entry(self):
        at = self.after.offset(self.after.entry)
        code = self.candidate[at:at + 31]
        self.assertEqual(code[:4], bytes.fromhex("9c60ff15"))
        self.assertEqual(code[8:12], bytes.fromhex("85c00f84"))
        self.assertEqual(16 + struct.unpack_from("<i", code, 12)[0], 23)
        self.assertEqual(code[16:19], bytes.fromhex("619de9"))
        self.assertEqual(self.after.entry + 23 + struct.unpack_from("<i", code, 19)[0], self.before.entry)
        self.assertEqual(code[23:27], bytes.fromhex("6a01ff15"))
        self.assertEqual(struct.unpack_from("<I", code, 27)[0], self.before.import_address(b"ExitProcess"))

    def test_language_controls_preserve_original_controls(self):
        before = parse_dialog(resource(self.baseline, 5, 102, 1042))
        after = parse_dialog(resource(self.candidate, 5, 102, 1042))
        self.assertEqual(before["items"], after["items"][:-2])
        self.assertEqual(after["items"][-1]["id"], extension.LANGUAGE_CONTROL)
        self.assertEqual(after["items"][-1]["style"] & 3, 3)  # dropdown list, no free-form paths
        self.assertEqual(after["items"][-2]["id"], extension.LANGUAGE_LABEL)
        self.assertGreater(after["cy"], before["cy"])
        for kind, name, lang in [(5, 130, 1033), (5, 133, 1033), (16, 1, 1042)]:
            self.assertEqual(resource(self.baseline, kind, name, lang), resource(self.candidate, kind, name, lang))

    def test_added_sections_have_separate_code_and_writable_imports(self):
        self.assertEqual(self.after.sections[-2][0], b".wlang")
        self.assertEqual(self.after.sections[-1][0], b".wlcfg")
        for index, expected in [(-2, 0x60000020), (-1, 0xC0000040)]:
            entry = self.after.table + (self.after.count + index) * 40
            self.assertEqual(struct.unpack_from("<I", self.candidate, entry + 36)[0], expected)
        for _, _, rva, size, raw in self.after.sections:
            self.assertEqual(rva % self.after.section_alignment, 0)
            self.assertEqual(raw % self.after.file_alignment, 0)
            self.assertEqual(size % self.after.file_alignment, 0)
            self.assertLessEqual(raw + size, len(self.candidate))

    def test_unrecognized_base_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "pinned operational"):
            extension.build(self.candidate_path, self.candidate_path.with_suffix(".rejected"))

    def test_windows_loader_and_bootstrap_without_updating(self):
        # Only this disposable test copy replaces the original entry with
        # ExitProcess(0). Thus the real Windows loader executes the added import
        # and initializer, then exits before the original patcher can run.
        directory = extension.ROOT / "build/client-locales"
        directory.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="native-bootstrap-", dir=directory) as temporary:
            folder = Path(temporary)
            image = bytearray(self.candidate)
            entry = self.after.offset(self.before.entry)
            image[entry:entry + 8] = b"\x6a\x00\xff\x15" + struct.pack("<I", self.before.import_address(b"ExitProcess"))
            executable = folder / "WYDLauncher.exe"
            executable.write_bytes(image)
            shutil.copyfile(self.dll_path, folder / extension.DLL_NAME.rstrip(b"\0").decode())
            result = subprocess.run([str(executable)], cwd=folder, timeout=15, capture_output=True)
            self.assertEqual(result.returncode, 0, f"loader/bootstrap failed: {result.returncode:#x}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--candidate", required=True, type=Path)
    parser.add_argument("--dll", required=True, type=Path)
    options = parser.parse_args()
    NativeLauncherTests.baseline = options.source.read_bytes()
    if extension.digest(NativeLauncherTests.baseline) != extension.BASE_SHA256:
        raise SystemExit("test baseline hash mismatch")
    NativeLauncherTests.candidate_path = options.candidate.resolve()
    NativeLauncherTests.candidate = options.candidate.read_bytes()
    NativeLauncherTests.dll_path = options.dll.resolve()
    NativeLauncherTests.before = extension.Image(NativeLauncherTests.baseline)
    NativeLauncherTests.after = extension.Image(NativeLauncherTests.candidate)
    unittest.main(argv=[__file__], verbosity=2)
