#!/usr/bin/env python3
"""Correlaciona fingerprints Ghidra sem promover evidência de pesquisa.

O resultado é uma fila diferencial reproduzível. EXACT_MATCH e STRONG_MATCH
descrevem somente a correspondência estrutural entre dois programas; revisão
Ghidra do fluxo continua obrigatória antes de TRACED, CONTRACT ou adaptação.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import math
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable


HASH_FIELDS = (
    "bytes_sha256",
    "mnemonic_sha256",
    "normalized_sha256",
    "pcode_sha256",
    "cfg_sha256",
)
REQUIRED_FIELDS = {
    "entry",
    "symbol",
    "full_symbol",
    "body_address_count",
    "instruction_count",
    "basic_block_count",
    "cfg_edge_count",
    "incoming_data_refs",
    "is_thunk",
    "strings",
    "imports",
    "constants",
    "direct_callers_count",
    "direct_callers",
    "direct_callees_count",
    "direct_callees",
    *HASH_FIELDS,
}
CLASS_ORDER = {
    "EXACT_MATCH": 3,
    "STRONG_MATCH": 2,
    "CANDIDATE": 1,
    "NO_MATCH": 0,
}


@dataclass(frozen=True)
class Fingerprint:
    entry: str
    symbol: str
    full_symbol: str
    body_size: int
    instructions: int
    blocks: int
    edges: int
    incoming_data_refs: int
    is_thunk: bool
    strings: frozenset[str]
    imports: frozenset[str]
    constants: frozenset[str]
    callers: frozenset[str]
    callees: frozenset[str]
    hashes: dict[str, str] = field(compare=False, hash=False)

    @property
    def neighbors(self) -> frozenset[str]:
        return self.callers | self.callees

    @property
    def exact_key(self) -> tuple[object, ...]:
        # Combinar três representações reduz colisões causadas por wrappers e
        # instruções curtas; cardinalidade ainda é verificada nos dois lados.
        return (
            self.hashes["normalized_sha256"],
            self.hashes["pcode_sha256"],
            self.hashes["cfg_sha256"],
            self.instructions,
            self.blocks,
            self.edges,
        )


@dataclass
class Corpus:
    path: Path
    metadata: dict[str, str]
    rows: list[Fingerprint]


@dataclass
class PairScore:
    native: Fingerprint
    source: Fingerprint
    base_score: float
    score: float
    components: dict[str, float]
    neighbor_hits: int = 0
    neighbor_evidence: int = 0

    @property
    def reason(self) -> str:
        labels: list[str] = []
        for key in (
            "bytes",
            "normalized",
            "pcode",
            "mnemonic",
            "cfg",
            "strings",
            "imports",
            "constants",
        ):
            value = self.components.get(key, 0.0)
            if value > 0:
                labels.append(f"{key}={value:.1f}")
        labels.extend(
            [
                f"instructions={self.components.get('instructions', 0.0):.1f}",
                f"shape={self.components.get('shape', 0.0):.1f}",
            ]
        )
        if self.neighbor_evidence:
            labels.append(
                f"callgraph={self.neighbor_hits}/{self.neighbor_evidence}"
            )
        return ";".join(labels)


def integer(row: dict[str, str], field_name: str) -> int:
    value = (row.get(field_name) or "").strip()
    try:
        return int(value or "0", 10)
    except ValueError as exc:
        raise ValueError(
            f"campo {field_name!r} inválido para {row.get('entry')!r}: {value!r}"
        ) from exc


def boolean(row: dict[str, str], field_name: str) -> bool:
    return (row.get(field_name) or "").strip().lower() in {"1", "true", "yes"}


def tokens(value: str) -> frozenset[str]:
    return frozenset(item for item in (value or "").split("|") if item)


def neighbor_entries(value: str) -> frozenset[str]:
    entries: set[str] = set()
    for item in (value or "").split(";"):
        entry = item.split(":", 1)[0].strip().upper()
        if entry:
            entries.add(entry)
    return frozenset(entries)


def validate_hash(value: str, label: str) -> str:
    normalized = value.strip().upper()
    if len(normalized) != 64 or any(character not in "0123456789ABCDEF" for character in normalized):
        raise ValueError(f"SHA-256 inválido em {label}: {value!r}")
    return normalized


def load_corpus(path: Path, expected_sha256: str) -> Corpus:
    metadata: dict[str, str] = {}
    data_lines: list[str] = []
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        for line in stream:
            if line.startswith("# "):
                key, separator, value = line[2:].rstrip("\r\n").partition("=")
                if separator:
                    metadata[key.strip()] = value.strip()
                continue
            if line.strip():
                data_lines.append(line)

    if metadata.get("schema_version") != "1":
        raise ValueError(
            f"schema_version incompatível em {path}: {metadata.get('schema_version')!r}"
        )
    actual_sha256 = validate_hash(metadata.get("program_sha256", ""), str(path))
    expected = validate_hash(expected_sha256, f"hash esperado de {path}")
    if actual_sha256 != expected:
        raise ValueError(
            f"program_sha256 divergente em {path}: esperado {expected}, encontrado {actual_sha256}"
        )
    if not data_lines:
        raise ValueError(f"TSV sem cabeçalho em {path}")

    reader = csv.DictReader(data_lines, delimiter="\t")
    fields = set(reader.fieldnames or [])
    missing = REQUIRED_FIELDS - fields
    if missing:
        raise ValueError(f"schema incompleto em {path}: {', '.join(sorted(missing))}")

    rows: list[Fingerprint] = []
    seen: set[str] = set()
    for raw in reader:
        entry = (raw.get("entry") or "").strip().upper()
        if not entry:
            raise ValueError(f"entry vazio em {path}")
        if entry in seen:
            raise ValueError(f"entry duplicado em {path}: {entry}")
        seen.add(entry)
        hashes = {
            name: validate_hash(raw.get(name, ""), f"{path}:{entry}:{name}")
            for name in HASH_FIELDS
        }
        rows.append(
            Fingerprint(
                entry=entry,
                symbol=(raw.get("symbol") or "").strip(),
                full_symbol=(raw.get("full_symbol") or "").strip(),
                body_size=integer(raw, "body_address_count"),
                instructions=integer(raw, "instruction_count"),
                blocks=integer(raw, "basic_block_count"),
                edges=integer(raw, "cfg_edge_count"),
                incoming_data_refs=integer(raw, "incoming_data_refs"),
                is_thunk=boolean(raw, "is_thunk"),
                strings=tokens(raw.get("strings", "")),
                imports=tokens(raw.get("imports", "")),
                constants=tokens(raw.get("constants", "")),
                callers=neighbor_entries(raw.get("direct_callers", "")),
                callees=neighbor_entries(raw.get("direct_callees", "")),
                hashes=hashes,
            )
        )
    if not rows:
        raise ValueError(f"nenhuma função em {path}")
    return Corpus(path=path, metadata=metadata, rows=rows)


def ratio(left: int, right: int) -> float:
    if left == right == 0:
        return 1.0
    maximum = max(left, right)
    return 0.0 if maximum == 0 else min(left, right) / maximum


def jaccard(left: frozenset[str], right: frozenset[str]) -> float:
    if not left and not right:
        return 0.0
    return len(left & right) / len(left | right)


def score_pair(native: Fingerprint, source: Fingerprint) -> PairScore:
    components: dict[str, float] = {}
    components["bytes"] = 8.0 if native.hashes["bytes_sha256"] == source.hashes["bytes_sha256"] else 0.0
    components["normalized"] = 24.0 if native.hashes["normalized_sha256"] == source.hashes["normalized_sha256"] else 0.0
    components["pcode"] = 22.0 if native.hashes["pcode_sha256"] == source.hashes["pcode_sha256"] else 0.0
    components["mnemonic"] = 12.0 if native.hashes["mnemonic_sha256"] == source.hashes["mnemonic_sha256"] else 0.0
    components["cfg"] = 10.0 if native.hashes["cfg_sha256"] == source.hashes["cfg_sha256"] else 0.0
    components["strings"] = 8.0 * jaccard(native.strings, source.strings)
    components["imports"] = 8.0 * jaccard(native.imports, source.imports)
    components["constants"] = 5.0 * jaccard(native.constants, source.constants)
    components["instructions"] = 6.0 * ratio(native.instructions, source.instructions)
    components["body"] = 2.0 * ratio(native.body_size, source.body_size)
    components["blocks"] = 2.0 * ratio(native.blocks, source.blocks)
    components["edges"] = 1.0 * ratio(native.edges, source.edges)
    components["degree"] = 1.0 * (
        ratio(len(native.callers), len(source.callers))
        + ratio(len(native.callees), len(source.callees))
    )
    components["shape"] = (
        components["body"]
        + components["blocks"]
        + components["edges"]
        + components["degree"]
    )
    components["thunk"] = 1.0 if native.is_thunk == source.is_thunk else 0.0
    # Os componentes somam no máximo 111; normalizar mantém thresholds e
    # relatórios fáceis de interpretar sem esconder a composição da nota.
    raw = sum(value for key, value in components.items() if key != "shape")
    normalized = raw * (100.0 / 111.0)
    return PairScore(
        native=native,
        source=source,
        base_score=normalized,
        score=normalized,
        components=components,
    )


def build_indices(rows: Iterable[Fingerprint]) -> tuple[
    dict[str, dict[str, list[Fingerprint]]], dict[int, list[Fingerprint]]
]:
    hash_indices: dict[str, dict[str, list[Fingerprint]]] = {
        field_name: defaultdict(list) for field_name in HASH_FIELDS
    }
    instruction_index: dict[int, list[Fingerprint]] = defaultdict(list)
    for row in rows:
        for field_name in HASH_FIELDS:
            hash_indices[field_name][row.hashes[field_name]].append(row)
        instruction_index[row.instructions].append(row)
    return hash_indices, instruction_index


def candidate_rows(
    native: Fingerprint,
    source_rows: list[Fingerprint],
    hash_indices: dict[str, dict[str, list[Fingerprint]]],
    instruction_index: dict[int, list[Fingerprint]],
) -> set[Fingerprint]:
    candidates: set[Fingerprint] = set()
    for field_name in HASH_FIELDS:
        candidates.update(hash_indices[field_name].get(native.hashes[field_name], ()))

    tolerance = max(2, math.ceil(max(native.instructions, 1) * 0.20))
    for count in range(max(0, native.instructions - tolerance), native.instructions + tolerance + 1):
        candidates.update(instruction_index.get(count, ()))
    if not candidates:
        candidates.update(source_rows)
    return candidates


def initial_scores(native: Corpus, source: Corpus, candidate_limit: int) -> dict[str, list[PairScore]]:
    hash_indices, instruction_index = build_indices(source.rows)
    scores: dict[str, list[PairScore]] = {}
    for native_row in native.rows:
        pairs = [
            score_pair(native_row, source_row)
            for source_row in candidate_rows(
                native_row, source.rows, hash_indices, instruction_index
            )
        ]
        pairs.sort(key=lambda pair: (-pair.score, pair.source.entry))
        # Limitar depois da pontuação barata preserva candidatos por hash e
        # impede que funções minúsculas dominem memória e tempo do callgraph.
        scores[native_row.entry] = pairs[:candidate_limit]
    return scores


def unique_counts(rows: Iterable[Fingerprint], attribute: str) -> Counter[object]:
    if attribute == "exact_key":
        return Counter(row.exact_key for row in rows)
    return Counter(row.hashes[attribute] for row in rows)


def reciprocal_best(scores: dict[str, list[PairScore]]) -> dict[str, str]:
    best: dict[str, tuple[float, str]] = {}
    tied: set[str] = set()
    for native_entry, pairs in scores.items():
        for pair in pairs:
            current = best.get(pair.source.entry)
            if current is None or pair.score > current[0] + 1e-9:
                best[pair.source.entry] = (pair.score, native_entry)
                tied.discard(pair.source.entry)
            elif abs(pair.score - current[0]) <= 1e-9 and native_entry != current[1]:
                tied.add(pair.source.entry)
    return {
        source_entry: native_entry
        for source_entry, (_, native_entry) in best.items()
        if source_entry not in tied
    }


def is_unique_exact(
    pair: PairScore,
    native_exact: Counter[object],
    source_exact: Counter[object],
    native_bytes: Counter[object],
    source_bytes: Counter[object],
) -> bool:
    exact_key = pair.native.exact_key
    structural = (
        exact_key == pair.source.exact_key
        and native_exact[exact_key] == 1
        and source_exact[exact_key] == 1
    )
    byte_hash = pair.native.hashes["bytes_sha256"]
    byte_exact = (
        byte_hash == pair.source.hashes["bytes_sha256"]
        and native_bytes[byte_hash] == 1
        and source_bytes[byte_hash] == 1
    )
    return structural or byte_exact


def classify_best(
    pairs: list[PairScore],
    reciprocal: dict[str, str],
    native_exact: Counter[object],
    source_exact: Counter[object],
    native_bytes: Counter[object],
    source_bytes: Counter[object],
) -> tuple[str, float, bool]:
    if not pairs:
        return "NO_MATCH", 0.0, False
    best = pairs[0]
    second_score = pairs[1].score if len(pairs) > 1 else 0.0
    margin = best.score - second_score
    reciprocal_match = reciprocal.get(best.source.entry) == best.native.entry
    if is_unique_exact(
        best, native_exact, source_exact, native_bytes, source_bytes
    ) and reciprocal_match:
        return "EXACT_MATCH", margin, True

    stable_feature = (
        best.components.get("normalized", 0.0) > 0
        or best.components.get("pcode", 0.0) > 0
        or (
            best.components.get("mnemonic", 0.0) > 0
            and best.components.get("cfg", 0.0) > 0
        )
        or best.components.get("strings", 0.0) >= 4.0
        or best.components.get("imports", 0.0) >= 4.0
    )
    tiny = min(best.native.instructions, best.source.instructions) <= 3
    if (
        best.score >= 78.0
        and margin >= 8.0
        and reciprocal_match
        and stable_feature
        and not tiny
    ):
        return "STRONG_MATCH", margin, True
    if best.score >= 45.0 or stable_feature:
        return "CANDIDATE", margin, reciprocal_match
    return "NO_MATCH", margin, reciprocal_match


def anchor_map(
    scores: dict[str, list[PairScore]], native: Corpus, source: Corpus
) -> dict[str, str]:
    native_exact = unique_counts(native.rows, "exact_key")
    source_exact = unique_counts(source.rows, "exact_key")
    native_bytes = unique_counts(native.rows, "bytes_sha256")
    source_bytes = unique_counts(source.rows, "bytes_sha256")
    reciprocal = reciprocal_best(scores)
    anchors: dict[str, str] = {}
    for entry, pairs in scores.items():
        classification, _, _ = classify_best(
            pairs,
            reciprocal,
            native_exact,
            source_exact,
            native_bytes,
            source_bytes,
        )
        if classification in {"EXACT_MATCH", "STRONG_MATCH"}:
            anchors[entry] = pairs[0].source.entry
    return anchors


def apply_callgraph_support(
    scores: dict[str, list[PairScore]], native_by_entry: dict[str, Fingerprint], anchors: dict[str, str]
) -> None:
    for native_entry, pairs in scores.items():
        native_neighbors = native_by_entry[native_entry].neighbors
        mapped_neighbors = {
            anchors[neighbor]
            for neighbor in native_neighbors
            if neighbor in anchors
        }
        for pair in pairs:
            pair.score = pair.base_score
            pair.neighbor_evidence = len(mapped_neighbors)
            pair.neighbor_hits = len(mapped_neighbors & pair.source.neighbors)
            if pair.neighbor_evidence:
                support = pair.neighbor_hits / pair.neighbor_evidence
                pair.score += min(12.0, pair.neighbor_hits * 4.0) * support
        pairs.sort(key=lambda item: (-item.score, item.source.entry))


def correlate(native: Corpus, source: Corpus, candidate_limit: int) -> dict[str, list[PairScore]]:
    if candidate_limit < 2:
        raise ValueError("candidate_limit deve ser pelo menos 2 para medir ambiguidade")
    scores = initial_scores(native, source, candidate_limit)
    native_by_entry = {row.entry: row for row in native.rows}
    # Duas passagens permitem que matches estruturais confirmados auxiliem os
    # vizinhos sem transformar propagação em uma cadeia ilimitada de hipóteses.
    for _ in range(2):
        anchors = anchor_map(scores, native, source)
        apply_callgraph_support(scores, native_by_entry, anchors)
    return scores


def render_tsv(
    output: Path,
    native: Corpus,
    source: Corpus,
    scores: dict[str, list[PairScore]],
    selected_entries: set[str],
    top: int,
) -> Counter[str]:
    native_exact = unique_counts(native.rows, "exact_key")
    source_exact = unique_counts(source.rows, "exact_key")
    native_bytes = unique_counts(native.rows, "bytes_sha256")
    source_bytes = unique_counts(source.rows, "bytes_sha256")
    reciprocal = reciprocal_best(scores)
    fields = [
        "native_entry",
        "native_symbol",
        "candidate_rank",
        "source_entry",
        "source_symbol",
        "classification",
        "score",
        "best_margin",
        "reciprocal_best",
        "neighbor_hits",
        "neighbor_evidence",
        "evidence",
    ]
    counts: Counter[str] = Counter()
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", encoding="utf-8", newline="") as stream:
        stream.write("# schema_version=1\n")
        stream.write(f"# native_program_sha256={native.metadata['program_sha256']}\n")
        stream.write(f"# source_program_sha256={source.metadata['program_sha256']}\n")
        stream.write("# warning=structural correlation only; research_status unchanged\n")
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        for native_row in native.rows:
            if selected_entries and native_row.entry not in selected_entries:
                continue
            pairs = scores.get(native_row.entry, [])
            classification, margin, _ = classify_best(
                pairs,
                reciprocal,
                native_exact,
                source_exact,
                native_bytes,
                source_bytes,
            )
            counts[classification] += 1
            if not pairs:
                writer.writerow(
                    {
                        "native_entry": native_row.entry,
                        "native_symbol": native_row.full_symbol or native_row.symbol,
                        "candidate_rank": 0,
                        "classification": "NO_MATCH",
                        "score": "0.00",
                        "best_margin": "0.00",
                        "reciprocal_best": "false",
                        "neighbor_hits": 0,
                        "neighbor_evidence": 0,
                        "evidence": "nenhum candidato estrutural",
                    }
                )
                continue
            for rank, pair in enumerate(pairs[:top], start=1):
                row_class = classification if rank == 1 else "CANDIDATE"
                writer.writerow(
                    {
                        "native_entry": native_row.entry,
                        "native_symbol": native_row.full_symbol or native_row.symbol,
                        "candidate_rank": rank,
                        "source_entry": pair.source.entry,
                        "source_symbol": pair.source.full_symbol or pair.source.symbol,
                        "classification": row_class,
                        "score": f"{pair.score:.2f}",
                        "best_margin": f"{margin:.2f}" if rank == 1 else "",
                        "reciprocal_best": str(
                            reciprocal.get(pair.source.entry) == native_row.entry
                        ).lower(),
                        "neighbor_hits": pair.neighbor_hits,
                        "neighbor_evidence": pair.neighbor_evidence,
                        "evidence": pair.reason,
                    }
                )
    return counts


def verify_binary(path: Path, expected_sha256: str) -> None:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    actual = digest.hexdigest().upper()
    expected = validate_hash(expected_sha256, f"hash esperado de {path}")
    if actual != expected:
        raise ValueError(
            f"SHA-256 divergente em {path}: esperado {expected}, encontrado {actual}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", type=Path, required=True, help="fingerprints do WYD 7.48")
    parser.add_argument("--source", type=Path, required=True, help="fingerprints do build TMProject")
    parser.add_argument("--native-sha256", required=True)
    parser.add_argument("--source-sha256", required=True)
    parser.add_argument("--native-binary", type=Path, help="opcional: revalidar o binário nativo")
    parser.add_argument("--source-binary", type=Path, help="opcional: revalidar o build da source")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--native-entry",
        action="append",
        default=[],
        help="limita o relatório; repetível. A correlação ainda usa o corpus inteiro.",
    )
    parser.add_argument("--top", type=int, default=3, help="candidatos gravados por função")
    parser.add_argument("--candidate-limit", type=int, default=250)
    args = parser.parse_args()

    try:
        if args.top < 1:
            raise ValueError("--top deve ser pelo menos 1")
        if args.native_binary:
            verify_binary(args.native_binary, args.native_sha256)
        if args.source_binary:
            verify_binary(args.source_binary, args.source_sha256)
        native = load_corpus(args.native, args.native_sha256)
        source = load_corpus(args.source, args.source_sha256)
        selected = {entry.strip().upper() for entry in args.native_entry if entry.strip()}
        known_entries = {row.entry for row in native.rows}
        missing = selected - known_entries
        if missing:
            raise ValueError(f"native entries ausentes: {', '.join(sorted(missing))}")
        scores = correlate(native, source, args.candidate_limit)
        counts = render_tsv(args.output, native, source, scores, selected, args.top)
    except (OSError, ValueError) as exc:
        print(f"correlate_fingerprints: {exc}", file=sys.stderr)
        return 2

    selected_count = sum(counts.values())
    rendered_counts = ", ".join(
        f"{classification}:{counts[classification]}"
        for classification in sorted(counts, key=lambda value: -CLASS_ORDER[value])
    )
    print(
        f"correlated={selected_count} native={len(native.rows)} source={len(source.rows)} "
        f"classes={rendered_counts} output={args.output}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
