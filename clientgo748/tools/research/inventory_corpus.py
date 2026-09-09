#!/usr/bin/env python3
"""Gera um inventário auditável de todas as funções exportadas do Ghidra 7.48.

O inventário é uma fila de trabalho, não um contrato nativo. O grafo de
chamadas obtido dos .c exportados é apenas textual; xrefs, callbacks, vtables
e chamadas indiretas continuam exigindo confirmação no projeto Ghidra.
"""

from __future__ import annotations

import argparse
import csv
import os
import re
from collections import Counter, defaultdict
from pathlib import Path


FUNCTION_RE = re.compile(r"FUN_([0-9A-Fa-f]{8})")
HEX_ENTRY_RE = re.compile(r"^[0-9A-Fa-f]{8}$")
TRACKED_SUFFIXES = {".md", ".cpp", ".h", ".go"}
GHIDRA_CATALOG_RELATIVE = Path(
    ".agents/research/client748/inventory/ghidra-functions.tsv"
)


def discover_corpus(explicit: str | None) -> Path:
    candidates: list[Path] = []
    if explicit:
        candidates.append(Path(explicit))
    env = os.environ.get("WYD748_GHIDRA_CORPUS")
    if env:
        candidates.append(Path(env))
    tools = Path.home() / "Tools" / "GhidraAnalysis"
    if tools.exists():
        candidates.extend(sorted(tools.glob("*/decompiled"), reverse=True))
    for candidate in candidates:
        if (candidate / "functions.tsv").is_file():
            return candidate.resolve()
    raise SystemExit(
        "corpus não encontrado; passe --corpus ou defina WYD748_GHIDRA_CORPUS"
    )


def normalize_entry(value: str) -> str:
    value = value.upper().removeprefix("FUN_").removeprefix("0X")
    if not HEX_ENTRY_RE.fullmatch(value.zfill(8)):
        raise SystemExit(f"entrada inválida: {value}")
    return value.zfill(8)


def load_index(corpus: Path) -> dict[str, dict[str, str]]:
    rows: dict[str, dict[str, str]] = {}
    with (corpus / "functions.tsv").open(
        encoding="utf-8-sig", newline=""
    ) as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        for row in reader:
            entry = normalize_entry(row.get("entry", ""))
            rows[entry] = {
                "name": row.get("name", ""),
                "decompile_status": row.get("decompile_status", ""),
                "file": row.get("file", ""),
            }
    return rows


def load_ghidra_catalog(
    repo: Path,
) -> tuple[dict[str, dict[str, str]], dict[str, str]]:
    """Load the structural catalog exported directly by the Ghidra project."""

    path = repo / GHIDRA_CATALOG_RELATIVE
    if not path.is_file():
        return {}, {}

    metadata: dict[str, str] = {}
    table_lines: list[str] = []
    with path.open(encoding="utf-8-sig", newline="") as handle:
        for line in handle:
            if line.startswith("#"):
                key, separator, value = line[1:].partition("=")
                if separator:
                    metadata[key.strip()] = value.strip()
                continue
            if line.strip():
                table_lines.append(line)

    rows: dict[str, dict[str, str]] = {}
    reader = csv.DictReader(table_lines, delimiter="\t")
    required = {"entry", "symbol", "direct_callers", "direct_callees"}
    if not reader.fieldnames or not required.issubset(reader.fieldnames):
        raise SystemExit(
            f"catálogo Ghidra inválido: cabeçalho ausente em {path}"
        )
    for row in reader:
        entry = normalize_entry(row.get("entry", ""))
        if entry in rows:
            raise SystemExit(f"catálogo Ghidra duplicado: FUN_{entry}")
        rows[entry] = {key: value or "" for key, value in row.items()}
    if not rows:
        raise SystemExit(f"catálogo Ghidra vazio: {path}")
    return rows, metadata


def integer_field(row: dict[str, str], key: str) -> int:
    try:
        return int(row.get(key, "0"))
    except ValueError:
        return 0


def read_function_texts(corpus: Path, index: dict[str, dict[str, str]]) -> dict[str, str]:
    texts: dict[str, str] = {}
    for entry, metadata in index.items():
        path = corpus / metadata["file"]
        if path.is_file():
            # errors=replace keeps the complete inventory reproducible even if a
            # single decompiler export contains an invalid byte sequence.
            texts[entry] = path.read_text(encoding="utf-8", errors="replace")
        else:
            texts[entry] = ""
    return texts


def extract_documented_entries(repo: Path, relative_files: list[str]) -> set[str]:
    entries: set[str] = set()
    for relative in relative_files:
        path = repo / relative
        if not path.is_file():
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        entries.update(match.upper() for match in FUNCTION_RE.findall(text))
    return entries


def extract_flow_entries(repo: Path) -> set[str]:
    root = repo / ".agents" / "research" / "client748" / "flows"
    entries: set[str] = set()
    if not root.is_dir():
        return entries
    for path in root.rglob("*.md"):
        if path.name == "TEMPLATE.md":
            continue
        entries.update(
            match.upper() for match in FUNCTION_RE.findall(
                path.read_text(encoding="utf-8", errors="replace")
            )
        )
    return entries


def resolve_documented_entries(
    entries: set[str],
    index: dict[str, dict[str, str]],
    ghidra_catalog: dict[str, dict[str, str]],
) -> tuple[set[str], dict[str, set[str]], set[str]]:
    """Mapeia referências documentais para entradas ou donos de corpo.

    Alguns handoffs registram um VA interno (`FUN_XXXXXXXX`) encontrado no
    meio do pseudocódigo, não um novo entry point. Resolver pelo intervalo do
    corpo preserva o catálogo Ghidra e evita fabricar funções para preencher
    uma lacuna textual.
    """
    resolved: set[str] = set()
    internal: dict[str, set[str]] = defaultdict(set)
    unresolved: set[str] = set()
    body_ranges: list[tuple[int, int, str]] = []
    for owner, row in ghidra_catalog.items():
        try:
            body_min = int(row.get("body_min", ""), 16)
            body_max = int(row.get("body_max", ""), 16)
        except ValueError:
            continue
        if body_min <= body_max:
            body_ranges.append((body_min, body_max, owner))

    for entry in entries:
        if entry in index:
            resolved.add(entry)
            continue
        try:
            address = int(entry, 16)
        except ValueError:
            unresolved.add(entry)
            continue
        owners = {
            owner
            for body_min, body_max, owner in body_ranges
            if body_min <= address <= body_max
        }
        if len(owners) == 1:
            owner = next(iter(owners))
            resolved.add(owner)
            internal[entry].add(owner)
        else:
            # Zero owners ou múltiplos donos continuam pendentes: uma
            # sobreposição não fornece evidência suficiente para escolher.
            unresolved.add(entry)
    return resolved, internal, unresolved


def source_reference_counts(repo: Path) -> Counter[str]:
    counts: Counter[str] = Counter()
    source_root = repo / "client-source"
    if not source_root.is_dir():
        return counts
    for path in source_root.rglob("*"):
        if path.suffix.lower() not in {".cpp", ".h"}:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        counts.update(match.upper() for match in FUNCTION_RE.findall(text))
    return counts


def build_call_graph(
    index: dict[str, dict[str, str]], texts: dict[str, str]
) -> tuple[dict[str, set[str]], dict[str, set[str]]]:
    callees: dict[str, set[str]] = defaultdict(set)
    callers: dict[str, set[str]] = defaultdict(set)
    known = set(index)
    for entry, text in texts.items():
        for target in FUNCTION_RE.findall(text):
            target = target.upper()
            if target == entry or target not in known:
                continue
            callees[entry].add(target)
            callers[target].add(entry)
    return callees, callers


def classify(
    entry: str,
    parity_entries: set[str],
    flow_entries: set[str],
    known: set[str],
) -> tuple[str, str, str]:
    in_parity = entry in parity_entries
    in_flow = entry in flow_entries
    if entry not in known:
        return "UNRESOLVED_REFERENCE", "not-in-corpus", "resolve-directly-in-ghidra"
    if in_parity and in_flow:
        return "PARITY_AND_FLOW", "LOCATED_OR_STATIC_EVIDENCE", "complete-ghidra-flow"
    if in_parity:
        return "PARITY_STATIC_EVIDENCE", "STATICALLY_EVIDENCED", "complete-ghidra-flow"
    if in_flow:
        return "FLOW_SHEET", "LOCATED", "complete-ghidra-flow"
    return "CORPUS_ONLY", "UNMAPPED", "triage-by-entrypoint-or-caller"


def write_inventory(
    output: Path,
    corpus: Path,
    index: dict[str, dict[str, str]],
    texts: dict[str, str],
    callees: dict[str, set[str]],
    callers: dict[str, set[str]],
    parity_entries: set[str],
    flow_entries: set[str],
    source_refs: Counter[str],
    ghidra_catalog: dict[str, dict[str, str]],
) -> Counter[str]:
    output.parent.mkdir(parents=True, exist_ok=True)
    counts: Counter[str] = Counter()
    fieldnames = [
        "entry",
        "symbol",
        "decompile_status",
        "decompile_file",
        "file_present",
        "evidence_class",
        "research_status",
        "next_action",
        "textual_callers_count",
        "textual_callers",
        "textual_callees_count",
        "textual_callees",
        "source_cpp_h_references",
        "callgraph_basis",
        "ghidra_catalog_present",
        "ghidra_calling_convention",
        "ghidra_signature",
        "ghidra_body_min",
        "ghidra_body_max",
        "ghidra_body_address_count",
        "ghidra_is_thunk",
        "ghidra_thunk_target",
        "ghidra_direct_callers_count",
        "ghidra_direct_callers",
        "ghidra_direct_callees_count",
        "ghidra_direct_callees",
        "ghidra_incoming_ref_count",
        "ghidra_incoming_flow_refs",
        "ghidra_incoming_data_refs",
        "ghidra_incoming_other_refs",
    ]
    with output.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        for entry in sorted(index):
            metadata = index[entry]
            evidence, status, next_action = classify(
                entry, parity_entries, flow_entries, set(index)
            )
            present = bool(texts[entry])
            entry_callers = sorted(callers.get(entry, set()))
            entry_callees = sorted(callees.get(entry, set()))
            # O catálogo é a evidência estrutural do programa Ghidra; o grafo
            # textual continua disponível para mostrar divergências de export.
            ghidra = ghidra_catalog.get(entry, {})
            counts[evidence] += 1
            writer.writerow(
                {
                    "entry": entry,
                    "symbol": ghidra.get("symbol") or f"FUN_{entry}",
                    "decompile_status": metadata["decompile_status"],
                    "decompile_file": metadata["file"],
                    "file_present": str(present).lower(),
                    "evidence_class": evidence,
                    "research_status": status,
                    "next_action": next_action,
                    "textual_callers_count": len(entry_callers),
                    "textual_callers": ";".join(f"FUN_{item}" for item in entry_callers),
                    "textual_callees_count": len(entry_callees),
                    "textual_callees": ";".join(f"FUN_{item}" for item in entry_callees),
                    "source_cpp_h_references": source_refs.get(f"FUN_{entry}", 0),
                    "callgraph_basis": (
                        "ghidra-direct-plus-exported-decompile-text"
                        if ghidra_catalog
                        else "exported-decompile-text-only"
                    ),
                    "ghidra_catalog_present": str(bool(ghidra)).lower(),
                    "ghidra_calling_convention": ghidra.get(
                        "calling_convention", ""
                    ),
                    "ghidra_signature": ghidra.get("signature", ""),
                    "ghidra_body_min": ghidra.get("body_min", ""),
                    "ghidra_body_max": ghidra.get("body_max", ""),
                    "ghidra_body_address_count": ghidra.get(
                        "body_address_count", ""
                    ),
                    "ghidra_is_thunk": ghidra.get("is_thunk", ""),
                    "ghidra_thunk_target": ghidra.get("thunk_target", ""),
                    "ghidra_direct_callers_count": ghidra.get(
                        "direct_callers_count", ""
                    ),
                    "ghidra_direct_callers": ghidra.get("direct_callers", ""),
                    "ghidra_direct_callees_count": ghidra.get(
                        "direct_callees_count", ""
                    ),
                    "ghidra_direct_callees": ghidra.get("direct_callees", ""),
                    "ghidra_incoming_ref_count": ghidra.get(
                        "incoming_ref_count", ""
                    ),
                    "ghidra_incoming_flow_refs": ghidra.get(
                        "incoming_flow_refs", ""
                    ),
                    "ghidra_incoming_data_refs": ghidra.get(
                        "incoming_data_refs", ""
                    ),
                    "ghidra_incoming_other_refs": ghidra.get(
                        "incoming_other_refs", ""
                    ),
                }
            )
    return counts


def write_summary(
    output: Path,
    inventory_path: Path,
    corpus: Path,
    index: dict[str, dict[str, str]],
    counts: Counter[str],
    parity_entries: set[str],
    flow_entries: set[str],
    source_refs: Counter[str],
    ghidra_catalog: dict[str, dict[str, str]],
    ghidra_metadata: dict[str, str],
    documented_internal: dict[str, set[str]],
    unresolved_references: set[str],
) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    known = set(index)
    documented = parity_entries | flow_entries
    unresolved = sorted(unresolved_references)
    present_documented = sorted(documented & known)
    ghidra_direct_callgraph_rows = sum(
        1
        for row in ghidra_catalog.values()
        if "direct_callers_count" in row and "direct_callees_count" in row
    )
    ghidra_no_incoming_flow = sum(
        1 for row in ghidra_catalog.values() if integer_field(row, "incoming_flow_refs") == 0
    )
    lines = [
        "# Inventário completo das funções nativas WYD 7.48",
        "",
        "Estado: `INVENTORY_GENERATED`",
        "",
        "Este relatório cobre todas as entradas presentes em `functions.tsv`. "
        "Ele identifica o material disponível e cria uma fila de pesquisa; "
        "não transforma pseudocódigo exportado em compreensão ou contrato.",
        "",
        "## Artefatos",
        "",
        f"- Corpus: `{corpus}`",
        f"- Inventário TSV: `{inventory_path}`",
        "- Binário de referência: `client748/wyd.exe nativo+patches/WYD.exe`",
        "- A confirmação de hash permanece obrigatória antes de usar endereços.",
        "",
        "## Contagem",
        "",
        f"- Funções no índice Ghidra: **{len(index)}**",
        f"- Funções citadas no handoff de paridade: **{len(parity_entries)}**",
        f"- Funções citadas nas fichas formais: **{len(flow_entries)}**",
        f"- Funções distintas documentadas: **{len(documented)}**",
        f"- Documentadas e presentes no índice: **{len(present_documented)}**",
        f"- Referências documentadas resolvidas como endereço interno: **{len(documented_internal)}**",
        f"- Referências documentadas ainda não resolvidas: **{len(unresolved)}**",
        f"- Funções com referência direta na source C++/H: **{sum(1 for value in source_refs.values() if value)}**",
        f"- Funções no catálogo estrutural Ghidra: **{len(ghidra_catalog)}**",
        f"- Linhas com callgraph direto Ghidra: **{ghidra_direct_callgraph_rows}**",
        f"- Funções sem xref FLOW de entrada no catálogo: **{ghidra_no_incoming_flow}**",
        f"- SHA-256 registrado pelo projeto Ghidra: **{ghidra_metadata.get('program_sha256', 'não disponível')}**",
        "",
        "### Classificação inicial do corpus",
        "",
        "| Classe | Quantidade | Próxima ação |",
        "| --- | ---: | --- |",
        f"| `PARITY_STATIC_EVIDENCE` | {counts['PARITY_STATIC_EVIDENCE']} | fechar callers/callees e fluxo no Ghidra |",
        f"| `FLOW_SHEET` | {counts['FLOW_SHEET']} | completar ficha a partir de `LOCATED` |",
        f"| `PARITY_AND_FLOW` | {counts['PARITY_AND_FLOW']} | consolidar evidências |",
        f"| `CORPUS_ONLY` | {counts['CORPUS_ONLY']} | triagem por entrypoint/caller |",
        f"| `UNRESOLVED_REFERENCE` | {len(unresolved)} | resolver endereço diretamente no Ghidra |",
        "",
        "## Limites da classificação",
        "",
        "- `CORPUS_ONLY` significa somente que a função ainda não aparece nas "
        "fichas/handoff selecionados; não significa que seja código morto.",
        "- Callers/callees são candidatos obtidos por tokens nos `.c`; chamadas "
        "indiretas, callbacks, vtables e destinos calculados continuam exigindo "
        "revisão no projeto Ghidra. O campo `ghidra_*` registra somente relações "
        "diretas reconhecidas pela análise.",
        "- `functions.tsv` é o índice de decompilação; `ghidra-functions.tsv` é "
        "o catálogo estrutural versionado desta análise. Os dois devem conter "
        "exatamente as mesmas 4.146 entradas antes de serem combinados.",
        "- `STATICALLY_EVIDENCED` não significa `TRACED`, `CONTRACT` ou "
        "`CLIENT_TESTED`.",
        "- Uma função nativa pode ser wrapper, runtime, helper compartilhado ou "
        "código não alcançável no fluxo 7.48; nesses casos o resultado correto "
        "será documentar a decisão, não fabricar uma tradução para o TMProject.",
        "",
        "## Critério para considerar compreendida",
        "",
        "Uma função só sai da fila quando estiver vinculada a uma transição "
        "observável com entrada, callers/callees confirmados no Ghidra, estado, "
        "side effects, erros, teardown/relogin quando aplicável, equivalente "
        "na source e validação proporcional. Packet/ABI exige `CONTRACT`; "
        "execução no candidato exige `CLIENT_TESTED`.",
    ]
    if unresolved:
        lines.extend(["", "Referências ainda ausentes do índice textual:", ""])
        lines.extend(f"- `FUN_{entry}`" for entry in unresolved)
    if documented_internal:
        lines.extend(["", "Referências resolvidas dentro do corpo de uma função:", ""])
        for address, owners in sorted(documented_internal.items()):
            lines.append(
                f"- `FUN_{address}` -> "
                + ", ".join(f"`FUN_{owner}`" for owner in sorted(owners))
            )
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    parser.add_argument("--corpus")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(".agents/research/client748/inventory/functions.tsv"),
    )
    parser.add_argument(
        "--summary-output",
        type=Path,
        default=Path(".agents/research/client748/inventory/README.md"),
    )
    args = parser.parse_args()
    repo = args.repo.resolve()
    corpus = discover_corpus(args.corpus)
    index = load_index(corpus)
    ghidra_catalog, ghidra_metadata = load_ghidra_catalog(repo)
    if ghidra_catalog:
        # Falhar fechado evita misturar análises de binários/projetos Ghidra
        # diferentes e produzir uma falsa cobertura de 4.146 funções.
        corpus_entries = set(index)
        catalog_entries = set(ghidra_catalog)
        missing_from_catalog = sorted(corpus_entries - catalog_entries)
        extra_in_catalog = sorted(catalog_entries - corpus_entries)
        if missing_from_catalog or extra_in_catalog:
            raise SystemExit(
                "catálogo Ghidra e corpus divergem: "
                f"ausentes={missing_from_catalog[:5]} "
                f"extras={extra_in_catalog[:5]}"
            )
    texts = read_function_texts(corpus, index)
    callees, callers = build_call_graph(index, texts)
    parity_raw = extract_documented_entries(
        repo, [".agents/handoffs/client748-parity.md"]
    )
    flow_raw = extract_flow_entries(repo)
    parity_entries, parity_internal, parity_unresolved = resolve_documented_entries(
        parity_raw, index, ghidra_catalog
    )
    flow_entries, flow_internal, flow_unresolved = resolve_documented_entries(
        flow_raw, index, ghidra_catalog
    )
    documented_internal: dict[str, set[str]] = defaultdict(set)
    for address, owners in parity_internal.items():
        documented_internal[address].update(owners)
    for address, owners in flow_internal.items():
        documented_internal[address].update(owners)
    unresolved_references = parity_unresolved | flow_unresolved
    source_refs = source_reference_counts(repo)
    counts = write_inventory(
        (repo / args.output).resolve(),
        corpus,
        index,
        texts,
        callees,
        callers,
        parity_entries,
        flow_entries,
        source_refs,
        ghidra_catalog,
    )
    write_summary(
        (repo / args.summary_output).resolve(),
        (repo / args.output).resolve(),
        corpus,
        index,
        counts,
        parity_entries,
        flow_entries,
        source_refs,
        ghidra_catalog,
        ghidra_metadata,
        documented_internal,
        unresolved_references,
    )
    print(f"corpus_functions: {len(index)}")
    print(f"parity_entries: {len(parity_entries)}")
    print(f"flow_sheet_entries: {len(flow_entries)}")
    print(f"documented_entries: {len(parity_entries | flow_entries)}")
    print(f"ghidra_catalog_entries: {len(ghidra_catalog)}")
    print("classes:")
    for key in sorted(counts):
        print(f"  {key}: {counts[key]}")
    print(f"inventory: {(repo / args.output).resolve()}")
    print(f"summary: {(repo / args.summary_output).resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
