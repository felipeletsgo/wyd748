#!/usr/bin/env python3
"""Generate a reproducible source inventory for TMProject748.

The generated files contain source facts only. Compatibility, provenance and
research conclusions belong to the separate curated audit layer.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import io
import json
import os
import re
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Iterable, Iterator, Sequence

try:
    from tree_sitter import Language, Node, Parser
    import tree_sitter_cpp
except ImportError as exc:  # pragma: no cover - CLI dependency error
    raise SystemExit(
        "missing dependencies; install: python -m pip install -r "
        ".agents/skills/wyd-client748-catalog/scripts/"
        "requirements-source-inventory.txt"
    ) from exc


CPP_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl"}
HEADER_EXTENSIONS = {".h", ".hh", ".hpp", ".hxx", ".inl"}
IGNORED_DIRECTORY_NAMES = {".git", ".vs", "bin", "build", "debug", "obj", "release", "x64"}
CONFIGURATION = "Release"
PLATFORM = "Win32"

FILE_FIELDS = (
    "path", "ownership", "file_kind", "project_roles", "main_release_win32",
    "test_release_win32", "bytes", "lines", "sha256", "parser",
    "raw_parse_errors", "parser_normalizations", "parse_errors", "symbols",
    "function_lines", "preprocessor_lines",
    "comment_lines", "blank_lines", "other_lines", "boundary_lines",
    "classified_lines",
)

PARSER_IGNORABLE_TOKENS = re.compile(
    rb"(?<![A-Za-z0-9_])(?:CALLBACK|APIENTRY|WINAPI|__stdcall|_In_|_In_opt_)(?![A-Za-z0-9_])"
)
PARSER_TEXT_MACRO = re.compile(
    rb"(?<![A-Za-z0-9_])(?:_T|TEXT)\(((?:L|u8|u|U)?\"(?:\\.|[^\"\\])*\")\)"
)
PARSER_POINTER_ARRAY_NEW = re.compile(
    rb"(?<![A-Za-z0-9_])new\s+[A-Za-z_]\w*(?:::\w+)*\s*(\*)\s*(?=\[)"
)
PARSER_ENDIF_SUFFIX = re.compile(rb"(?m)^([ \t]*#[ \t]*endif\b)([^\r\n]*)")
PARSER_STRUCT_CAST = re.compile(rb"\(const\s+(struct\s+)[A-Za-z_]\w*\s*\*\s*\)\s*&")
PARSER_COMMA_RETRY_PREFIX = re.compile(
    rb"(\|\|\s*\()"
    rb"([A-Za-z_]\w*\s*\+=\s*\d+,\s*"
    rb"[A-Za-z_]\w*(?:\.[A-Za-z_]\w*)+\s*=\s*[A-Za-z_]\w*\([^,\r\n]*\),\s*)"
    rb"(?=[A-Za-z_]\w*\()"
)
PARSER_SAFE_RELEASE_STATEMENT = re.compile(
    rb"(?m)^([ \t]*)SAFE_RELEASE\(([^()\r\n]*)\)([ \t]*)(\r?)$"
)
SYMBOL_FIELDS = (
    "source_id", "path", "ownership", "project_roles", "kind", "symbol",
    "start_line", "end_line", "signature", "signature_sha256",
    "body_sha256", "parse_clean",
)


@dataclass(frozen=True)
class ProjectMembership:
    main_items: dict[str, str]
    test_items: dict[str, str]


@dataclass(frozen=True)
class SymbolFact:
    kind: str
    symbol: str
    start_line: int
    end_line: int
    signature: str
    body_sha256: str


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def blank_span(buffer: bytearray, start: int, end: int) -> None:
    """Blank a parser-only span without changing byte offsets or line breaks."""
    for index in range(start, end):
        if buffer[index] not in (10, 13):
            buffer[index] = 32


def parser_input(data: bytes) -> tuple[bytes, int]:
    """Normalize grammar-only legacy constructs while preserving all offsets."""
    normalized = bytearray(data)
    normalizations = 0

    for match in PARSER_IGNORABLE_TOKENS.finditer(data):
        blank_span(normalized, match.start(), match.end())
        normalizations += 1

    for match in PARSER_TEXT_MACRO.finditer(data):
        blank_span(normalized, match.start(), match.start(1))
        blank_span(normalized, match.end(1), match.end())
        normalizations += 1

    for match in PARSER_POINTER_ARRAY_NEW.finditer(data):
        blank_span(normalized, match.start(1), match.end(1))
        normalizations += 1

    for match in PARSER_ENDIF_SUFFIX.finditer(data):
        suffix = match.group(2).strip()
        if suffix and not suffix.startswith((b"//", b"/*")):
            blank_span(normalized, match.start(2), match.end(2))
            normalizations += 1

    for match in PARSER_STRUCT_CAST.finditer(data):
        blank_span(normalized, match.start(1), match.end(1))
        normalizations += 1

    for match in PARSER_COMMA_RETRY_PREFIX.finditer(data):
        blank_span(normalized, match.start(2), match.end(2))
        normalizations += 1

    for match in PARSER_SAFE_RELEASE_STATEMENT.finditer(data):
        blank_span(normalized, match.start(1), match.start(2))
        normalized[match.end(2)] = ord(";")
        normalizations += 1

    return bytes(normalized), normalizations


def normalize_relative(path: Path, root: Path) -> str:
    return path.resolve().relative_to(root.resolve()).as_posix()


def condition_matches(condition: str | None) -> bool:
    if not condition:
        return True
    compact = re.sub(r"\s+", "", condition).lower()
    return f"{CONFIGURATION}|{PLATFORM}".lower() in compact


def parse_bool(value: str | None) -> bool:
    return (value or "").strip().lower() in {"true", "1", "yes"}


def project_items(project_path: Path, source_root: Path) -> dict[str, str]:
    """Return source-root-relative entries and Release|Win32 build state."""
    tree = ET.parse(project_path)
    result: dict[str, str] = {}
    project_dir = project_path.parent.resolve()
    for item in tree.getroot().iter():
        tag = item.tag.rsplit("}", 1)[-1]
        if tag not in {"ClCompile", "ClInclude"}:
            continue
        include = item.attrib.get("Include")
        if not include:
            continue
        resolved = (project_dir / Path(include.replace("\\", os.sep))).resolve()
        try:
            relative = normalize_relative(resolved, source_root)
        except ValueError:
            continue
        excluded = False
        for child in item:
            if child.tag.rsplit("}", 1)[-1] == "ExcludedFromBuild" and condition_matches(child.attrib.get("Condition")):
                excluded = parse_bool(child.text)
        result[relative] = "include" if tag == "ClInclude" else ("excluded" if excluded else "compile")
    return result


def enumerate_source_files(source_root: Path) -> list[Path]:
    files: list[Path] = []
    for path in source_root.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in CPP_EXTENSIONS:
            continue
        directories = path.relative_to(source_root).parts[:-1]
        if any(part.lower() in IGNORED_DIRECTORY_NAMES for part in directories):
            continue
        files.append(path)
    return sorted(files, key=lambda value: value.relative_to(source_root).as_posix().lower())


def ownership_for(relative: str) -> str:
    parts = PurePosixPath(relative).parts
    lowered = {part.lower() for part in parts}
    if "dependencies" in lowered:
        return "THIRD_PARTY"
    if "tests" in lowered:
        return "TEST"
    if ".generated." in parts[-1].lower():
        return "GENERATED"
    return "PRODUCT"


def project_roles(relative: str, membership: ProjectMembership) -> tuple[str, str, str]:
    main = membership.main_items.get(relative, "unlisted")
    tests = membership.test_items.get(relative, "unlisted")
    roles = []
    if main != "unlisted":
        roles.append(f"main:{main}")
    if tests != "unlisted":
        roles.append(f"tests:{tests}")
    return ";".join(roles) if roles else "unlisted", main, tests


def walk(node: Node) -> Iterator[Node]:
    yield node
    for child in node.children:
        yield from walk(child)


def node_text(node: Node, data: bytes) -> bytes:
    return data[node.start_byte:node.end_byte]


def normalized_text(data: bytes) -> str:
    return " ".join(data.decode("utf-8", errors="replace").split())


def end_line_inclusive(node: Node) -> int:
    row, column = node.end_point
    if column == 0 and row > node.start_point[0]:
        row -= 1
    return row + 1


def first_descendant(node: Node, types: set[str]) -> Node | None:
    for candidate in walk(node):
        if candidate.type in types:
            return candidate
    return None


def function_name(node: Node, data: bytes) -> str:
    declarator = node if node.type == "function_declarator" else first_descendant(node, {"function_declarator"})
    if declarator is None:
        return "<anonymous>"
    name = declarator.child_by_field_name("declarator")
    while name is not None and name.type in {
        "function_declarator", "parenthesized_declarator", "pointer_declarator", "reference_declarator",
    }:
        nested = name.child_by_field_name("declarator")
        if nested is None:
            break
        name = nested
    symbol = normalized_text(node_text(name, data)) if name is not None else "<anonymous>"
    if symbol == "<anonymous>" or "::" in symbol:
        return symbol

    scope: list[str] = []
    ancestor = node.parent
    while ancestor is not None:
        if ancestor.type in {"class_specifier", "struct_specifier", "union_specifier", "namespace_definition"}:
            scope_name = ancestor.child_by_field_name("name")
            if scope_name is not None:
                scope.append(normalized_text(node_text(scope_name, data)))
        ancestor = ancestor.parent
    return "::".join((*reversed(scope), symbol)) if scope else symbol


def signature_for_function(node: Node, data: bytes) -> str:
    body = node.child_by_field_name("body")
    end_byte = body.start_byte if body is not None else node.end_byte
    return normalized_text(data[node.start_byte:end_byte]).rstrip(" :")


def symbol_fact(node: Node, data: bytes, kind: str) -> SymbolFact:
    if kind == "function":
        symbol = function_name(node, data)
        signature = signature_for_function(node, data)
        body = node.child_by_field_name("body")
        body_hash = sha256_bytes(node_text(body, data)) if body is not None else ""
    elif kind == "lambda":
        symbol = f"<lambda@{node.start_point[0] + 1}:{node.start_point[1] + 1}>"
        signature = normalized_text(node_text(node, data).split(b"{", 1)[0])
        body = node.child_by_field_name("body")
        body_hash = sha256_bytes(node_text(body or node, data))
    elif kind == "macro":
        name = node.child_by_field_name("name")
        symbol = normalized_text(node_text(name, data)) if name is not None else "<macro>"
        signature = normalized_text(node_text(node, data))
        body_hash = sha256_bytes(node_text(node, data))
    else:
        symbol = function_name(node, data)
        signature = normalized_text(node_text(node, data)).rstrip(";")
        body_hash = ""
    return SymbolFact(kind, symbol, node.start_point[0] + 1, end_line_inclusive(node), signature, body_hash)


def collect_symbols(root: Node, data: bytes) -> list[SymbolFact]:
    facts: list[SymbolFact] = []
    for node in walk(root):
        if node.type == "function_definition":
            facts.append(symbol_fact(node, data, "function"))
        elif node.type == "lambda_expression":
            facts.append(symbol_fact(node, data, "lambda"))
        elif node.type == "preproc_function_def":
            facts.append(symbol_fact(node, data, "macro"))
        elif node.type in {"declaration", "field_declaration"}:
            if first_descendant(node, {"function_declarator"}) is None:
                continue
            ancestor = node.parent
            while ancestor is not None and ancestor.type != "function_definition":
                ancestor = ancestor.parent
            if ancestor is None:
                facts.append(symbol_fact(node, data, "declaration"))
    return sorted(facts, key=lambda fact: (fact.start_line, fact.end_line, fact.kind, fact.symbol))


def count_parse_errors(root: Node) -> int:
    return sum(1 for node in walk(root) if node.type == "ERROR" or node.is_missing)


def line_sets(root: Node, symbols: Sequence[SymbolFact], line_count: int) -> dict[str, set[int]]:
    buckets = {name: set() for name in ("function", "preprocessor", "comment")}
    for fact in symbols:
        if fact.kind in {"function", "lambda"}:
            buckets["function"].update(range(fact.start_line, fact.end_line + 1))
    for node in walk(root):
        if node.type.startswith("preproc"):
            buckets["preprocessor"].update(range(node.start_point[0] + 1, end_line_inclusive(node) + 1))
        elif node.type == "comment":
            buckets["comment"].update(range(node.start_point[0] + 1, end_line_inclusive(node) + 1))
    valid = set(range(1, line_count + 1))
    for bucket in buckets.values():
        bucket.intersection_update(valid)
    return buckets


def classify_lines(data: bytes, root: Node | None, symbols: Sequence[SymbolFact], ownership: str) -> dict[str, int]:
    lines = data.splitlines()
    counts = {name: 0 for name in ("function", "preprocessor", "comment", "blank", "other", "boundary")}
    if ownership == "THIRD_PARTY":
        for line in lines:
            counts["blank" if not line.strip() else "boundary"] += 1
        return counts
    assert root is not None
    sets = line_sets(root, symbols, len(lines))
    for number, line in enumerate(lines, start=1):
        if number in sets["function"]:
            counts["function"] += 1
        elif not line.strip():
            counts["blank"] += 1
        elif number in sets["preprocessor"]:
            counts["preprocessor"] += 1
        elif number in sets["comment"]:
            counts["comment"] += 1
        else:
            counts["other"] += 1
    return counts


def tsv_text(fields: Sequence[str], rows: Iterable[dict[str, object]]) -> str:
    stream = io.StringIO(newline="")
    writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


def build_inventory(source_root: Path, main_project: Path, test_project: Path) -> tuple[str, str, dict[str, int]]:
    parser = Parser(Language(tree_sitter_cpp.language()))
    membership = ProjectMembership(project_items(main_project, source_root), project_items(test_project, source_root))
    file_rows: list[dict[str, object]] = []
    symbol_rows: list[dict[str, object]] = []
    totals = {key: 0 for key in (
        "files", "lines", "product_files", "third_party_files", "generated_files", "test_files",
        "symbols", "raw_parse_errors", "parser_normalizations", "parse_errors",
        "unlisted_files", "unlisted_product_files", "main_compile_files", "test_compile_files",
    )}
    for path in enumerate_source_files(source_root):
        relative = normalize_relative(path, source_root)
        ownership = ownership_for(relative)
        roles, main_role, test_role = project_roles(relative, membership)
        data = path.read_bytes()
        physical_lines = len(data.splitlines())
        root: Node | None = None
        facts: list[SymbolFact] = []
        raw_errors = 0
        normalizations = 0
        errors = 0
        parser_name = "boundary-only"
        if ownership != "THIRD_PARTY":
            raw_root = parser.parse(data).root_node
            raw_errors = count_parse_errors(raw_root)
            normalized_data, normalizations = parser_input(data)
            root = parser.parse(normalized_data).root_node
            errors = count_parse_errors(root)
            facts = collect_symbols(root, data)
            parser_name = "tree-sitter-cpp+legacy-normalization"
        counts = classify_lines(data, root, facts, ownership)
        classified = sum(counts.values())
        if classified != physical_lines:
            raise RuntimeError(f"line classification mismatch for {relative}: {classified} != {physical_lines}")
        file_rows.append({
            "path": relative,
            "ownership": ownership,
            "file_kind": "header" if path.suffix.lower() in HEADER_EXTENSIONS else "translation_unit",
            "project_roles": roles,
            "main_release_win32": main_role,
            "test_release_win32": test_role,
            "bytes": len(data),
            "lines": physical_lines,
            "sha256": sha256_bytes(data),
            "parser": parser_name,
            "raw_parse_errors": raw_errors,
            "parser_normalizations": normalizations,
            "parse_errors": errors,
            "symbols": len(facts),
            "function_lines": counts["function"],
            "preprocessor_lines": counts["preprocessor"],
            "comment_lines": counts["comment"],
            "blank_lines": counts["blank"],
            "other_lines": counts["other"],
            "boundary_lines": counts["boundary"],
            "classified_lines": classified,
        })
        for fact in facts:
            source_key = "|".join((relative, fact.kind, fact.symbol, fact.signature)).encode("utf-8")
            symbol_rows.append({
                "source_id": sha256_bytes(source_key)[:20],
                "path": relative,
                "ownership": ownership,
                "project_roles": roles,
                "kind": fact.kind,
                "symbol": fact.symbol,
                "start_line": fact.start_line,
                "end_line": fact.end_line,
                "signature": fact.signature,
                "signature_sha256": sha256_bytes(fact.signature.encode("utf-8")),
                "body_sha256": fact.body_sha256,
                "parse_clean": "yes" if errors == 0 else "no",
            })
        totals["files"] += 1
        totals["lines"] += physical_lines
        totals["symbols"] += len(facts)
        totals["raw_parse_errors"] += raw_errors
        totals["parser_normalizations"] += normalizations
        totals["parse_errors"] += errors
        totals["product_files"] += int(ownership == "PRODUCT")
        totals["third_party_files"] += int(ownership == "THIRD_PARTY")
        totals["generated_files"] += int(ownership == "GENERATED")
        totals["test_files"] += int(ownership == "TEST")
        totals["unlisted_files"] += int(roles == "unlisted")
        totals["unlisted_product_files"] += int(ownership == "PRODUCT" and roles == "unlisted")
        totals["main_compile_files"] += int(main_role == "compile")
        totals["test_compile_files"] += int(test_role == "compile")
    symbol_rows.sort(key=lambda row: (str(row["path"]).lower(), int(row["start_line"]), str(row["kind"]), str(row["symbol"])))
    return tsv_text(FILE_FIELDS, file_rows), tsv_text(SYMBOL_FIELDS, symbol_rows), totals


def write_if_changed(path: Path, content: str) -> bool:
    current = path.read_text(encoding="utf-8") if path.exists() else None
    if current == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="")
    return True


def default_repo_root() -> Path:
    return Path(__file__).resolve().parents[4]


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=default_repo_root())
    action = parser.add_mutually_exclusive_group()
    action.add_argument("--write", action="store_true")
    action.add_argument("--check", action="store_true")
    parser.add_argument("--fail-on-parse-errors", action="store_true")
    parser.add_argument("--source-root", type=Path)
    parser.add_argument("--main-project", type=Path)
    parser.add_argument("--test-project", type=Path)
    parser.add_argument("--files-output", type=Path)
    parser.add_argument("--symbols-output", type=Path)
    return parser.parse_args(argv)


def resolve_arg(value: Path | None, default: str, repo: Path) -> Path:
    path = value or Path(default)
    return path if path.is_absolute() else repo / path


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    repo = args.repo.resolve()
    source_root = resolve_arg(args.source_root, "tmproject/TMProject748", repo).resolve()
    main_project = resolve_arg(args.main_project, "tmproject/TMProject748/TMProject748.vcxproj", repo).resolve()
    test_project = resolve_arg(args.test_project, "tmproject/TMProject748/tests/ArchitectureTests.vcxproj", repo).resolve()
    files_output = resolve_arg(args.files_output, ".agents/research/client748/inventory/source-files.tsv", repo)
    symbols_output = resolve_arg(args.symbols_output, ".agents/research/client748/inventory/source-symbols.tsv", repo)
    for required in (source_root, main_project, test_project):
        if not required.exists():
            print(f"missing required input: {required}", file=sys.stderr)
            return 2
    files_text, symbols_text, totals = build_inventory(source_root, main_project, test_project)
    exit_code = 0
    if args.write:
        totals["outputs_changed"] = sum(
            int(write_if_changed(path, content))
            for path, content in ((files_output, files_text), (symbols_output, symbols_text))
        )
    elif args.check:
        stale = [
            str(path) for path, content in ((files_output, files_text), (symbols_output, symbols_text))
            if not path.exists() or path.read_text(encoding="utf-8") != content
        ]
        totals["stale_outputs"] = len(stale)
        if stale:
            totals["stale_paths"] = stale
            exit_code = 1
    if args.fail_on_parse_errors and totals["parse_errors"]:
        exit_code = 1
    print(json.dumps(totals, ensure_ascii=False, sort_keys=True))
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
