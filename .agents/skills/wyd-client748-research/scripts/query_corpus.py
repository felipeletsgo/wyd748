#!/usr/bin/env python3
"""Consulta reproduzível do corpus exportado do Ghidra para o WYD 7.48."""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path


FUNCTION_RE = re.compile(r"FUN_([0-9A-Fa-f]{8})")


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


def load_index(corpus: Path) -> dict[str, Path]:
    rows: dict[str, Path] = {}
    lines = (corpus / "functions.tsv").read_text(encoding="utf-8-sig").splitlines()
    for line in lines[1:]:
        columns = line.split("\t")
        if len(columns) >= 4:
            rows[columns[0].upper().zfill(8)] = corpus / columns[3]
    return rows


def normalize_entry(value: str) -> str:
    value = value.upper().removeprefix("FUN_").removeprefix("0X")
    if not re.fullmatch(r"[0-9A-F]{1,8}", value):
        raise SystemExit(f"entrada inválida: {value}")
    return value.zfill(8)


def command_flow(corpus: Path, entry_arg: str) -> int:
    index = load_index(corpus)
    entry = normalize_entry(entry_arg)
    path = index.get(entry)
    if path is None or not path.is_file():
        raise SystemExit(f"função {entry} não existe no índice")
    content = path.read_text(encoding="utf-8", errors="replace")
    callees = sorted({match.upper() for match in FUNCTION_RE.findall(content) if match.upper() != entry})
    needle = f"FUN_{entry}"
    callers: list[str] = []
    for candidate in corpus.glob("*.c"):
        if candidate == path:
            continue
        text = candidate.read_text(encoding="utf-8", errors="ignore")
        if needle in text:
            callers.append(candidate.name)
    print(f"corpus: {corpus}")
    print(f"function: FUN_{entry}")
    print(f"file: {path}")
    print("syntactic_callers:")
    for caller in callers or ["UNRESOLVED"]:
        print(f"  - {caller}")
    print("syntactic_callees:")
    for callee in callees or ["UNRESOLVED"]:
        print(f"  - FUN_{callee}" if callee != "UNRESOLVED" else f"  - {callee}")
    print("warning: chamadas indiretas e xrefs exigem o projeto Ghidra")
    return 0


def command_search(corpus: Path, term: str, limit: int) -> int:
    found = 0
    for path in corpus.glob("*.c"):
        for number, line in enumerate(
            path.read_text(encoding="utf-8", errors="ignore").splitlines(), 1
        ):
            if term.lower() in line.lower():
                print(f"{path.name}:{number}:{line.strip()}")
                found += 1
                if found >= limit:
                    print(f"limit reached: {limit}")
                    return 0
    print(f"matches: {found}")
    return 0


def command_stats(corpus: Path, repo: Path) -> int:
    index = load_index(corpus)
    referenced: dict[str, dict[Path, list[int]]] = {}
    roots = [repo / ".agents", repo / "client-source", repo / "internal"]
    for root in roots:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.suffix.lower() not in {".md", ".cpp", ".h", ".go"}:
                continue
            # Keep source locations because a missing FUN_ token may identify an
            # instruction inside an indexed function instead of a missing function.
            for number, line in enumerate(
                path.read_text(encoding="utf-8", errors="ignore").splitlines(), 1
            ):
                for match in FUNCTION_RE.findall(line):
                    entry = match.upper()
                    relative = path.relative_to(repo)
                    referenced.setdefault(entry, {}).setdefault(relative, []).append(
                        number
                    )
    missing = sorted(set(referenced) - set(index))
    print(f"corpus_functions: {len(index)}")
    print(f"repo_native_functions_referenced: {len(referenced)}")
    print(f"references_missing_from_corpus: {len(missing)}")
    for entry in missing:
        print(f"  - FUN_{entry}")
        for path, occurrences in sorted(
            referenced[entry].items(), key=lambda item: item[0].as_posix()
        ):
            lines = ",".join(str(number) for number in sorted(set(occurrences)))
            print(
                f"    - {path.as_posix()}: lines={lines} "
                f"occurrences={len(occurrences)}"
            )
    if missing:
        print(
            "note: uma referência ausente pode ser rótulo documental ou endereço "
            "interno de outra função; não prova ausência no binário"
        )
        print(
            "next_step: no Ghidra, abra cada endereço, resolva a função "
            "proprietária e confira entry point, xrefs, callers e callees"
        )
    return 1 if missing else 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--corpus")
    sub = parser.add_subparsers(dest="command", required=True)
    flow = sub.add_parser("flow")
    flow.add_argument("entry")
    search = sub.add_parser("search")
    search.add_argument("term")
    search.add_argument("--limit", type=int, default=100)
    stats = sub.add_parser("stats")
    stats.add_argument("--repo", type=Path, default=Path.cwd())
    args = parser.parse_args()
    corpus = discover_corpus(args.corpus)
    if args.command == "flow":
        return command_flow(corpus, args.entry)
    if args.command == "search":
        return command_search(corpus, args.term, args.limit)
    return command_stats(corpus, args.repo.resolve())


if __name__ == "__main__":
    sys.exit(main())
