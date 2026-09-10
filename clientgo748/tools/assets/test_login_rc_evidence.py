"""Regressão documental do asset local; não declara paridade com o loader nativo."""

import struct
import unittest
from pathlib import Path


class LoginResourceEvidenceTests(unittest.TestCase):
    def test_local_inline_resource_boundaries(self):
        data = (Path(__file__).resolve().parents[2] /
                "assets/current/UI/LoginScene2.bin").read_bytes()
        # Fronteiras observadas no arquivo, independentes de sizeof do TMProject.
        expected = [(0, 12, 769, 176), (180, 1, 4608, 36),
                    (220, 13, 5121, 184), (408, 13, 5122, 184),
                    (596, 2, 4609, 164), (764, 2, 4610, 164),
                    (932, 2, 4611, 164), (1100, 1, 305, 36)]
        end = 0
        for offset, kind, control_id, size in expected:
            self.assertEqual(end, offset)
            self.assertEqual(struct.unpack_from("<ii", data, offset),
                             (kind, control_id))
            end = offset + 4 + size
        self.assertEqual(end, len(data))
        # O tipo EDIT começa em 220: não é nPickable nem padding do painel.
        self.assertEqual(struct.unpack_from("<i", data, 220)[0], 13)
        self.assertEqual(struct.unpack_from("<i", data, 224)[0], 5121)


if __name__ == "__main__":
    unittest.main()
