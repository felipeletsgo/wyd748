#!/usr/bin/env python3
"""Testes determinísticos do correlator estrutural, sem depender do Ghidra."""

from __future__ import annotations

import csv
import hashlib
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("correlate_fingerprints.py")
SPEC = importlib.util.spec_from_file_location("correlate_fingerprints", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


FIELDS = [
    "entry",
    "symbol",
    "full_symbol",
    "namespace",
    "calling_convention",
    "signature",
    "body_min",
    "body_max",
    "body_address_count",
    "instruction_count",
    "basic_block_count",
    "cfg_edge_count",
    "incoming_data_refs",
    "is_thunk",
    "thunk_target",
    "bytes_sha256",
    "mnemonic_sha256",
    "normalized_sha256",
    "pcode_sha256",
    "cfg_sha256",
    "strings",
    "imports",
    "constants",
    "direct_callers_count",
    "direct_callers",
    "direct_callees_count",
    "direct_callees",
]


def digest(label: str) -> str:
    return hashlib.sha256(label.encode("ascii")).hexdigest().upper()


def row(
    entry: str,
    symbol: str,
    family: str,
    *,
    instructions: int = 12,
    hashes: dict[str, str] | None = None,
    cfg_family: str | None = None,
) -> dict[str, object]:
    selected = hashes or {
        "bytes_sha256": digest(f"{family}:bytes"),
        "mnemonic_sha256": digest(f"{family}:mnemonic"),
        "normalized_sha256": digest(f"{family}:normalized"),
        "pcode_sha256": digest(f"{family}:pcode"),
        "cfg_sha256": digest(f"{cfg_family or family}:cfg"),
    }
    return {
        "entry": entry,
        "symbol": symbol,
        "full_symbol": symbol,
        "namespace": "Global",
        "calling_convention": "__cdecl",
        "signature": "void f(void)",
        "body_min": entry,
        "body_max": entry,
        "body_address_count": instructions * 4,
        "instruction_count": instructions,
        "basic_block_count": 3,
        "cfg_edge_count": 3,
        "incoming_data_refs": 0,
        "is_thunk": "false",
        "thunk_target": "",
        **selected,
        "strings": "c2NlbmU",
        "imports": "S0VSTkVMMzIuZGxs",
        "constants": "0x54|0x64",
        "direct_callers_count": 0,
        "direct_callers": "",
        "direct_callees_count": 0,
        "direct_callees": "",
    }


def write_corpus(path: Path, program_hash: str, rows: list[dict[str, object]]) -> None:
    with path.open("w", encoding="utf-8", newline="") as stream:
        stream.write("# schema_version=1\n")
        stream.write(f"# program_sha256={program_hash}\n")
        writer = csv.DictWriter(stream, fieldnames=FIELDS, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


class CorrelateFingerprintsTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.native_hash = digest("native-program")
        self.source_hash = digest("source-program")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def load(self, native_rows, source_rows):
        native_path = self.root / "native.tsv"
        source_path = self.root / "source.tsv"
        write_corpus(native_path, self.native_hash, native_rows)
        write_corpus(source_path, self.source_hash, source_rows)
        return (
            MODULE.load_corpus(native_path, self.native_hash),
            MODULE.load_corpus(source_path, self.source_hash),
        )

    def test_unique_structural_fingerprint_is_exact(self) -> None:
        native, source = self.load(
            [row("00401000", "FUN_00401000", "scene")],
            [row("00501000", "ObjectManager::ChangeScene", "scene")],
        )
        scores = MODULE.correlate(native, source, 10)
        reciprocal = MODULE.reciprocal_best(scores)
        classification, _, reciprocal_match = MODULE.classify_best(
            scores["00401000"],
            reciprocal,
            MODULE.unique_counts(native.rows, "exact_key"),
            MODULE.unique_counts(source.rows, "exact_key"),
            MODULE.unique_counts(native.rows, "bytes_sha256"),
            MODULE.unique_counts(source.rows, "bytes_sha256"),
        )
        self.assertEqual("EXACT_MATCH", classification)
        self.assertTrue(reciprocal_match)

    def test_duplicate_tiny_fingerprints_remain_candidates(self) -> None:
        native, source = self.load(
            [
                row("00402000", "FUN_00402000", "tiny", instructions=2),
                row("00402010", "FUN_00402010", "tiny", instructions=2),
            ],
            [
                row("00502000", "A::Wrapper", "tiny", instructions=2),
                row("00502010", "B::Wrapper", "tiny", instructions=2),
            ],
        )
        scores = MODULE.correlate(native, source, 10)
        reciprocal = MODULE.reciprocal_best(scores)
        for entry in ("00402000", "00402010"):
            classification, _, _ = MODULE.classify_best(
                scores[entry],
                reciprocal,
                MODULE.unique_counts(native.rows, "exact_key"),
                MODULE.unique_counts(source.rows, "exact_key"),
                MODULE.unique_counts(native.rows, "bytes_sha256"),
                MODULE.unique_counts(source.rows, "bytes_sha256"),
            )
            self.assertEqual("CANDIDATE", classification)

    def test_unique_high_score_without_exact_cfg_is_strong(self) -> None:
        native_row = row("00403000", "FUN_00403000", "lifecycle", cfg_family="native")
        source_row = row("00503000", "ObjectManager::FrameMove", "lifecycle", cfg_family="source")
        source_row["bytes_sha256"] = digest("lifecycle:source-bytes")
        native, source = self.load([native_row], [source_row])
        scores = MODULE.correlate(native, source, 10)
        reciprocal = MODULE.reciprocal_best(scores)
        classification, _, _ = MODULE.classify_best(
            scores["00403000"],
            reciprocal,
            MODULE.unique_counts(native.rows, "exact_key"),
            MODULE.unique_counts(source.rows, "exact_key"),
            MODULE.unique_counts(native.rows, "bytes_sha256"),
            MODULE.unique_counts(source.rows, "bytes_sha256"),
        )
        self.assertEqual("STRONG_MATCH", classification)

    def test_program_hash_is_mandatory(self) -> None:
        path = self.root / "corpus.tsv"
        write_corpus(path, self.native_hash, [row("00404000", "FUN_00404000", "hash")])
        with self.assertRaisesRegex(ValueError, "program_sha256 divergente"):
            MODULE.load_corpus(path, self.source_hash)


if __name__ == "__main__":
    unittest.main()
