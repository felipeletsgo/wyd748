#!/usr/bin/env python3
"""Gera uma fila de triagem para as 4.146 funções do client WYD 7.48.

As filas são heurísticas operacionais. Elas não afirmam semântica, alcance ou
paridade: cada função ainda precisa de uma transição observável e confirmação
no projeto Ghidra antes de ser promovida a uma ficha TRACED/CONTRACT.
"""

from __future__ import annotations

import argparse
import csv
from collections import Counter
from pathlib import Path

from inventory_corpus import discover_corpus


HINTS: tuple[tuple[str, int, tuple[str, ...]], ...] = (
    (
        "TRANSPORT_TEXT_HINT",
        2,
        (
            "cpsock",
            "socket",
            "recv(",
            "send(",
            "opcode",
            "checksum",
            "decrypt",
            "encrypt",
            "packet",
            "msg_",
            "wsa",
        ),
    ),
    (
        "UI_INPUT_TEXT_HINT",
        2,
        (
            "wm_key",
            "wm_char",
            "wm_ime",
            "ime",
            "control",
            "window",
            "messagebox",
            "focus",
            "tscene",
            "sgrid",
        ),
    ),
    (
        "RENDER_ASSET_TEXT_HINT",
        2,
        (
            "direct3d",
            "d3d",
            "texture",
            "mesh",
            "render",
            "draw",
            "sprite",
            "image",
            ".bin",
            ".bmp",
            ".tga",
        ),
    ),
    (
        "LIFECYCLE_TEXT_HINT",
        2,
        (
            "create",
            "destroy",
            "release",
            "initialize",
            "init(",
            "free(",
            "delete",
            "close(",
            "logout",
            "relogin",
            "scene",
        ),
    ),
)


def parse_count(row: dict[str, str], key: str) -> int:
    try:
        return int(row.get(key, "0") or 0)
    except ValueError:
        return 0


def load_inventory(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8-sig", newline="") as handle:
        rows = list(csv.DictReader(handle, delimiter="\t"))
    if not rows:
        raise SystemExit(f"inventário vazio: {path}")
    if rows[0].get("ghidra_catalog_present") != "true":
        raise SystemExit(
            "inventário sem catálogo Ghidra integrado; execute inventory_corpus.py"
        )
    return rows


def load_texts(corpus: Path, rows: list[dict[str, str]]) -> dict[str, str]:
    texts: dict[str, str] = {}
    for row in rows:
        path = corpus / row.get("decompile_file", "")
        texts[row["entry"]] = (
            path.read_text(encoding="utf-8", errors="replace").lower()
            if path.is_file()
            else ""
        )
    return texts


def assign(row: dict[str, str], text: str) -> tuple[int, list[str], str]:
    queues: list[str] = []
    reasons: list[str] = []
    status = row.get("research_status", "UNMAPPED")
    if status != "UNMAPPED":
        queues.append("DOCUMENTED_FIRST")
        reasons.append("já citado em handoff/ficha; fechar a evidência existente")
    if parse_count(row, "source_cpp_h_references") > 0:
        queues.append("SOURCE_BRIDGE")
        reasons.append("referência FUN_ direta na source client")
    if parse_count(row, "ghidra_direct_callees_count") >= 12:
        queues.append("HIGH_FANOUT_DISPATCH_CANDIDATE")
        reasons.append("alto fan-out direto reconhecido pelo Ghidra")
    if parse_count(row, "ghidra_direct_callers_count") == 0:
        queues.append("NO_DIRECT_CALLER_REVIEW")
        reasons.append("sem caller direto reconhecido; revisar callback/vtable/import")
    for queue, _, terms in HINTS:
        matches = [term for term in terms if term in text]
        if matches:
            queues.append(queue)
            reasons.append("pista textual: " + ", ".join(matches[:4]))
    if not queues:
        queues.append("CORPUS_REMAINDER")
        reasons.append("sem prioridade estrutural ou pista textual automática")

    # A menor prioridade numérica mantém evidência existente e referências
    # vivas antes das heurísticas, sem confundir a fila com uma classificação.
    if "DOCUMENTED_FIRST" in queues:
        priority = 0
    elif "SOURCE_BRIDGE" in queues:
        priority = 1
    elif any(queue in queues for queue, _, _ in HINTS):
        priority = 2
    elif "HIGH_FANOUT_DISPATCH_CANDIDATE" in queues:
        priority = 3
    elif "NO_DIRECT_CALLER_REVIEW" in queues:
        priority = 4
    else:
        priority = 5
    return priority, queues, " | ".join(reasons)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    parser.add_argument(
        "--inventory",
        type=Path,
        default=Path(".agents/research/client748/inventory/functions.tsv"),
    )
    parser.add_argument("--corpus")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(".agents/research/client748/inventory/research-queue.tsv"),
    )
    args = parser.parse_args()
    repo = args.repo.resolve()
    inventory = (repo / args.inventory).resolve()
    corpus = discover_corpus(args.corpus)
    rows = load_inventory(inventory)
    texts = load_texts(corpus, rows)
    output = (repo / args.output).resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "entry",
        "symbol",
        "priority",
        "queues",
        "reason",
        "research_status",
        "evidence_class",
        "ghidra_direct_callers_count",
        "ghidra_direct_callees_count",
        "ghidra_incoming_flow_refs",
        "source_cpp_h_references",
    ]
    counts: Counter[str] = Counter()
    priorities: Counter[int] = Counter()
    ordered: list[dict[str, str]] = []
    for row in rows:
        priority, queues, reason = assign(row, texts[row["entry"]])
        counts.update(queues)
        priorities[priority] += 1
        ordered.append(
            {
                "entry": row["entry"],
                "symbol": row["symbol"],
                "priority": str(priority),
                "queues": ";".join(queues),
                "reason": reason,
                "research_status": row.get("research_status", ""),
                "evidence_class": row.get("evidence_class", ""),
                "ghidra_direct_callers_count": row.get(
                    "ghidra_direct_callers_count", ""
                ),
                "ghidra_direct_callees_count": row.get(
                    "ghidra_direct_callees_count", ""
                ),
                "ghidra_incoming_flow_refs": row.get(
                    "ghidra_incoming_flow_refs", ""
                ),
                "source_cpp_h_references": row.get("source_cpp_h_references", ""),
            }
        )
    ordered.sort(key=lambda row: (int(row["priority"]), row["entry"]))
    with output.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        writer.writerows(ordered)
    print(f"queue_functions: {len(ordered)}")
    print("priorities: " + ", ".join(f"P{key}={priorities[key]}" for key in sorted(priorities)))
    for key in sorted(counts):
        print(f"  {key}: {counts[key]}")
    print(f"queue: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
