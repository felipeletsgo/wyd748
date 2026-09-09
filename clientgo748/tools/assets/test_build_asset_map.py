import tempfile
import unittest
from pathlib import Path

from build_asset_map import canonical_size


class CanonicalSizeTests(unittest.TestCase):
    def test_text_size_is_independent_of_checkout_line_endings(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            windows_text = root / "table.txt"
            linux_text = root / "table-copy.txt"
            windows_text.write_bytes(b"first\r\nsecond\r\n")
            linux_text.write_bytes(b"first\nsecond\n")

            self.assertEqual(canonical_size(windows_text), canonical_size(linux_text))

    def test_binary_size_remains_byte_exact(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / "texture.wyt"
            data = b"WT10\r\n\x00\xff"
            binary.write_bytes(data)

            self.assertEqual(canonical_size(binary), len(data))


if __name__ == "__main__":
    unittest.main()
