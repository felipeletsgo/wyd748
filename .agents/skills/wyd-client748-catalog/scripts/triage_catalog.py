#!/usr/bin/env python3
"""Rank the 7.48 native-function inventory without changing research state."""

from __future__ import annotations

import argparse
import csv
import hashlib
import io
import json
import sys
from collections import Counter
from pathlib import Path


REQUIRED_FIELDS = {
    "entry",
    "symbol",
    "evidence_class",
    "research_status",
    "source_cpp_h_references",
    "ghidra_direct_callers_count",
    "ghidra_direct_callees_count",
    "ghidra_incoming_flow_refs",
    "ghidra_incoming_data_refs",
    "ghidra_is_thunk",
}
REFERENCE_SHA256 = "8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593"


def integer(row: dict[str, str], field: str) -> int:
    value = (row.get(field) or "").strip()
    if not value:
        return 0
    try:
        return int(value, 10)
    except ValueError as exc:
        raise ValueError(f"campo {field!r} inválido em {row.get('entry')!r}: {value!r}") from exc


def boolean(row: dict[str, str], field: str) -> bool:
    return (row.get(field) or "").strip().lower() in {"1", "true", "yes"}


def verify_binary(path: Path, expected_sha256: str) -> None:
    # Hash the exact historical executable before ranking any address-derived evidence.
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    actual_sha256 = digest.hexdigest().upper()
    expected = expected_sha256.strip().upper()
    if actual_sha256 != expected:
        raise ValueError(
            f"SHA-256 divergente em {path}: esperado {expected}, encontrado {actual_sha256}"
        )


def classify(row: dict[str, str]) -> tuple[str, int, str, str]:
    status = (row.get("research_status") or "UNMAPPED").strip() or "UNMAPPED"
    evidence = (row.get("evidence_class") or "CORPUS_ONLY").strip() or "CORPUS_ONLY"
    source_refs = integer(row, "source_cpp_h_references")
    callers = integer(row, "ghidra_direct_callers_count")
    callees = integer(row, "ghidra_direct_callees_count")
    flow_refs = integer(row, "ghidra_incoming_flow_refs")
    data_refs = integer(row, "ghidra_incoming_data_refs")
    is_thunk = boolean(row, "ghidra_is_thunk")

    # This rank orders investigation work only; it never promotes evidence status.
    if evidence != "CORPUS_ONLY" or status != "UNMAPPED":
        lane = "DOCUMENTED_FIRST"
        # Existing evidence is the fastest route to real coverage. Keep this
        # lane above heuristic fan-out candidates so a documented gap is
        # closed before the queue starts a new speculative branch.
        score = 1000 + min(callers, 30) * 2 + min(callees, 50) + min(flow_refs, 100)
        reason = "já possui ficha, handoff ou estado de pesquisa; fechar a evidência"
        next_action = "reabrir a função e um caller/callee no Ghidra; atualizar a ficha"
    elif source_refs > 0:
        lane = "SOURCE_REFERENCED"
        score = 85 + min(source_refs, 20) * 3
        reason = "há referência direta na source recompilável"
        next_action = "comparar a chamada da source com a função 7.48 no Ghidra"
    elif callers == 0 and (flow_refs > 0 or data_refs > 0):
        lane = "INDIRECT_OR_CALLBACK"
        score = 65 + min(flow_refs, 100) + min(data_refs, 50) // 2
        reason = "não há caller direto; há xref de fluxo/dados para revisar"
        next_action = "resolver vtable, callback, thunk, bodyrefs e referências de dados"
    elif callers >= 5 or flow_refs >= 10 or callers + callees >= 20:
        lane = "HIGH_FANOUT"
        score = 50 + min(callers, 30) * 2 + min(callees, 50) + min(flow_refs, 100)
        reason = "alto fan-out ou muitas referências de entrada"
        next_action = "tratar como raiz de componente e fechar uma transição observável"
    else:
        lane = "CORPUS_TRIAGE"
        score = 10 + min(callers, 10) * 2 + min(callees, 20) + min(flow_refs, 20)
        reason = "somente catalogada; agrupar por string, caller, callee ou subsistema"
        next_action = "buscar entrada observável e agrupar com funções do mesmo fluxo"

    if is_thunk:
        score -= 15
        reason += "; thunk deve ser confirmado antes de priorizar implementação"

    return lane, score, reason, next_action


def load_rows(path: Path, expected: int) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        reader = csv.DictReader(stream, delimiter="\t")
        fields = set(reader.fieldnames or [])
        missing = REQUIRED_FIELDS - fields
        if missing:
            missing_text = ", ".join(sorted(missing))
            raise ValueError(f"schema incompleto em {path}: {missing_text}")
        rows = list(reader)

    entries = [row.get("entry", "").strip().upper() for row in rows]
    if any(not entry for entry in entries):
        raise ValueError(f"há entrada vazia em {path}")
    if len(entries) != len(set(entries)):
        raise ValueError(f"há entradas duplicadas em {path}")
    if expected > 0 and len(rows) != expected:
        raise ValueError(f"esperadas {expected} funções, encontradas {len(rows)} em {path}")

    ranked: list[dict[str, str | int]] = []
    for row in rows:
        lane, score, reason, next_action = classify(row)
        ranked.append(
            {
                "entry": row["entry"].strip().upper(),
                "symbol": row["symbol"].strip(),
                "lane": lane,
                "score": score,
                "research_status": row["research_status"].strip() or "UNMAPPED",
                "evidence_class": row["evidence_class"].strip() or "CORPUS_ONLY",
                "source_refs": integer(row, "source_cpp_h_references"),
                "direct_callers": integer(row, "ghidra_direct_callers_count"),
                "direct_callees": integer(row, "ghidra_direct_callees_count"),
                "incoming_flow_refs": integer(row, "ghidra_incoming_flow_refs"),
                "incoming_data_refs": integer(row, "ghidra_incoming_data_refs"),
                "is_thunk": boolean(row, "ghidra_is_thunk"),
                "reason": reason,
                "next_action": next_action,
            }
        )

    ranked.sort(key=lambda item: (-int(item["score"]), str(item["entry"])))
    for rank, item in enumerate(ranked, start=1):
        item["rank"] = rank
    return ranked


def summary(rows: list[dict[str, str | int]], expected: int) -> str:
    lanes = Counter(str(row["lane"]) for row in rows)
    statuses = Counter(str(row["research_status"]) for row in rows)
    lines = [
        "WYD 7.48 catalog triage",
        f"functions={len(rows)} expected={expected if expected > 0 else 'disabled'}",
        "lanes=" + ", ".join(f"{key}:{lanes[key]}" for key in sorted(lanes)),
        "statuses=" + ", ".join(f"{key}:{statuses[key]}" for key in sorted(statuses)),
        "top:",
    ]
    for row in rows[:20]:
        lines.append(
            f"{row['rank']:4} {row['entry']} {row['lane']:<22} "
            f"score={row['score']:3} {row['next_action']}"
        )
    return "\n".join(lines)


def tsv(rows: list[dict[str, str | int]]) -> str:
    fields = [
        "rank",
        "entry",
        "symbol",
        "lane",
        "score",
        "research_status",
        "evidence_class",
        "source_refs",
        "direct_callers",
        "direct_callees",
        "incoming_flow_refs",
        "incoming_data_refs",
        "is_thunk",
        "reason",
        "next_action",
    ]
    # DictWriter requires a file-like writer; StringIO keeps TSV generation in memory and deterministic.
    output = io.StringIO()
    writer = csv.DictWriter(output, fieldnames=fields, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    writer.writerows({field: row[field] for field in fields} for row in rows)
    return output.getvalue()


def json_report(
    rows: list[dict[str, str | int]],
    selected: list[dict[str, str | int]],
    expected: int,
) -> str:
    # Keep corpus-wide counts separate from the bounded top payload used by callers.
    return json.dumps(
        {
            "functions": len(rows),
            "selected_functions": len(selected),
            "expected_functions": expected,
            "lanes": dict(Counter(str(row["lane"]) for row in rows)),
            "statuses": dict(Counter(str(row["research_status"]) for row in rows)),
            "top": selected[:20],
        },
        ensure_ascii=False,
        indent=2,
    ) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path("."))
    parser.add_argument("--input", type=Path, help="TSV de funções; por padrão usa o inventário do repo")
    parser.add_argument(
        "--binary",
        type=Path,
        help="binário 7.48; por padrão usa a referência histórica WYD.exe do repo",
    )
    parser.add_argument(
        "--expected-sha256",
        default=REFERENCE_SHA256,
        help="SHA-256 esperado para o binário de referência",
    )
    parser.add_argument("--output", type=Path, help="arquivo de saída; sem isso imprime no stdout")
    parser.add_argument("--format", choices=("summary", "tsv", "json"), default="summary")
    parser.add_argument("--top", type=int, default=20, help="linhas no TSV/JSON; use 0 para todas")
    parser.add_argument("--expected-functions", type=int, default=4146)
    args = parser.parse_args()

    input_path = args.input or args.repo / ".agents" / "research" / "client748" / "inventory" / "functions.tsv"
    binary_path = args.binary or args.repo / "client748" / "wyd.exe nativo+patches" / "WYD.exe"
    try:
        verify_binary(binary_path, args.expected_sha256)
        rows = load_rows(input_path, args.expected_functions)
    except (OSError, ValueError) as exc:
        print(f"triage_catalog: {exc}", file=sys.stderr)
        return 2

    selected = rows if args.top == 0 else rows[: max(args.top, 0)]
    if args.format == "summary":
        rendered = summary(rows, args.expected_functions)
    elif args.format == "tsv":
        rendered = tsv(selected)
    else:
        rendered = json_report(rows, selected, args.expected_functions)

    if args.output:
        try:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(rendered, encoding="utf-8", newline="\n")
        except OSError as exc:
            print(f"triage_catalog: não foi possível gravar {args.output}: {exc}", file=sys.stderr)
            return 2
    else:
        print(rendered, end="" if rendered.endswith("\n") else "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
