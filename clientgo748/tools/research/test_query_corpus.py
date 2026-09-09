#!/usr/bin/env python3
"""Regression tests for the read-only 7.48 corpus query helper."""

from __future__ import annotations

import contextlib
import importlib.util
import io
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("query_corpus.py")
SPEC = importlib.util.spec_from_file_location("query_corpus", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class QueryCorpusTests(unittest.TestCase):
    def test_flow_finds_callers_regardless_of_hex_case(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            corpus = Path(temporary)
            (corpus / "functions.tsv").write_text(
                "address\tname\tsize\tfile\n"
                "004B3500\tFUN_004B3500\t1\tFUN_004b3500.c\n"
                "0054AC09\tFUN_0054AC09\t1\tFUN_0054ac09.c\n",
                encoding="utf-8",
            )
            (corpus / "FUN_004b3500.c").write_text(
                "void FUN_004b3500(void) { }\n",
                encoding="utf-8",
            )
            (corpus / "FUN_0054ac09.c").write_text(
                "void FUN_0054ac09(void) { FUN_004b3500(); }\n",
                encoding="utf-8",
            )

            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                result = MODULE.command_flow(corpus, "004B3500")

            self.assertEqual(result, 0)
            self.assertIn(
                "syntactic_callers:\n  - FUN_0054ac09.c", output.getvalue()
            )


if __name__ == "__main__":
    unittest.main()
