#!/usr/bin/env python3
"""Regression tests for the native 7.48 corpus inventory generator."""

from __future__ import annotations

import csv
import importlib.util
import sys
import tempfile
import unittest
from collections import defaultdict
from pathlib import Path


SCRIPT = Path(__file__).with_name("inventory_corpus.py")
SPEC = importlib.util.spec_from_file_location("inventory_corpus", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class InventoryCorpusTests(unittest.TestCase):
    def test_function_pattern_does_not_treat_pointer_symbols_as_functions(self) -> None:
        matches = MODULE.FUNCTION_RE.findall(
            "PTR_FUN_005A4294 references FUN_00401234 and MY_FUN_00405678"
        )

        self.assertEqual(matches, ["00401234"])

    def test_source_references_are_written_for_matching_entry(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "tmproject" / "TMProject748" / "NativeRoute.cpp"
            source.parent.mkdir(parents=True)
            source.write_text(
                "void route() { FUN_00401234(); FUN_00401234(); }\n",
                encoding="utf-8",
            )

            source_refs = MODULE.source_reference_counts(root)
            output = root / "functions.tsv"
            MODULE.write_inventory(
                output,
                root,
                {"00401234": {"decompile_status": "ok", "file": ""}},
                {"00401234": ""},
                defaultdict(set),
                defaultdict(set),
                set(),
                set(),
                source_refs,
                {},
            )

            with output.open(encoding="utf-8", newline="") as handle:
                row = next(csv.DictReader(handle, delimiter="\t"))

            self.assertEqual(source_refs["00401234"], 2)
            self.assertEqual(row["source_cpp_h_references"], "2")

    def test_source_reference_inside_body_is_assigned_to_its_owner(self) -> None:
        source_refs = MODULE.Counter({"00401235": 3})
        resolved, internal, unresolved = MODULE.resolve_source_reference_counts(
            source_refs,
            {"00401230": {}},
            {
                "00401230": {
                    "body_min": "00401230",
                    "body_max": "00401240",
                }
            },
        )

        self.assertEqual(resolved, MODULE.Counter({"00401230": 3}))
        self.assertEqual(internal, {"00401235": {"00401230"}})
        self.assertEqual(unresolved, set())


if __name__ == "__main__":
    unittest.main()
