#!/usr/bin/env python3
"""Validate structure and maturity of 7.48 client flow records."""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from pathlib import Path


STATUSES = {"UNMAPPED", "LOCATED", "TRACED", "CONTRACT", "CLIENT_TESTED"}
REQUIRED_KEYS = {"id", "title", "subsystem", "status", "native_sha256", "updated"}
# Accept existing records during migration; new records use the English names.
SECTION_ALIASES = {
    "Question": "Pergunta",
    "Evidence boundary": "Fronteira de evidência",
    "Native 7.48 flow": "Fluxo nativo 7.48",
    "State and lifecycle": "Estado e lifecycle",
    "Wire, ABI, and resources": "Wire, ABI e recursos",
    "Current mapping": "Mapeamento atual",
    "Delta matrix": "Matriz de delta",
    "Decisions": "Decisões",
    "Gaps": "Lacunas",
    "Validation": "Validação",
    "Observable entry": "Entrada observável",
    "Transition matrix": "Matriz de transições",
    "Vtables, vptrs, and receivers": "Vtables, vptrs e receptores",
    "Partial failure": "Falha parcial",
    "Cleanup and teardown": "Cleanup e teardown",
    "Logout and relogin": "Logout e relogin",
}
REQUIRED_SECTIONS = (
    "Question", "Evidence boundary", "Native 7.48 flow",
    "State and lifecycle", "Wire, ABI, and resources", "Current mapping",
    "Delta matrix", "Decisions", "Gaps", "Validation",
)
LIFECYCLE_SUBSECTIONS = (
    "Observable entry", "Transition matrix", "Vtables, vptrs, and receivers",
    "Ownership", "Partial failure", "Cleanup and teardown", "Shutdown",
    "Logout and relogin",
)
MATURE_STATUSES = {"TRACED", "CONTRACT", "CLIENT_TESTED"}


def front_matter(text: str) -> dict[str, str]:
    if not text.startswith("---\n"):
        return {}
    end = text.find("\n---\n", 4)
    if end < 0:
        return {}
    result: dict[str, str] = {}
    for line in text[4:end].splitlines():
        if ":" in line:
            key, value = line.split(":", 1)
            result[key.strip()] = value.strip().strip('"')
    return result


def section_body(text: str, heading: str) -> str:
    for name in (heading, SECTION_ALIASES.get(heading, heading)):
        match = re.search(
            rf"^### {re.escape(name)}\s*$([\s\S]*?)(?=^### |^## |\Z)",
            text,
            re.MULTILINE,
        )
        if match:
            return match.group(1).strip()
    return ""


def is_lifecycle_flow(metadata: dict[str, str]) -> bool:
    # Prefer subsystem; identity keeps the gate active for older records.
    identity = " ".join(
        metadata.get(key, "") for key in ("id", "title", "subsystem")
    ).casefold()
    return "lifecycle" in identity


def has_resolved_content(body: str) -> bool:
    if not body.strip():
        return False
    plain = re.sub(r"^[\s>*-]+", "", body.strip()).strip()
    # Only a justified N/A closes an inapplicable field. A gap marker does not.
    na_marker = r"(?:N/?A|NÃO SE APLICA|NAO SE APLICA)"
    na_match = re.fullmatch(
        rf"{na_marker}\s*:\s*(\S[\s\S]*\S|\S)", plain, re.IGNORECASE
    )
    if na_match:
        return True
    unresolved = (
        r"^(?:UNRESOLVED|TODO|PENDING|UNCONFIRMED|PENDENTE|NÃO CONFIRMAD[OA]|NAO CONFIRMAD[OA])"
        r"(?:\b|\s*:)|"
        rf"^{na_marker}[.!]?$"
    )
    return re.search(unresolved, plain, re.IGNORECASE) is None


def transition_matrix_has_data(body: str) -> bool:
    table_rows = []
    for line in body.splitlines():
        stripped = line.strip()
        if not stripped.startswith("|"):
            continue
        cells = [cell.strip() for cell in stripped.strip("|").split("|")]
        if cells and all(re.fullmatch(r":?-{3,}:?", cell) for cell in cells):
            continue
        table_rows.append(cells)
    # The header and at least one concrete transition are required.
    return len(table_rows) >= 2


def validate(path: Path) -> tuple[list[str], str | None]:
    text = path.read_text(encoding="utf-8")
    metadata = front_matter(text)
    errors: list[str] = []
    missing = REQUIRED_KEYS - set(metadata)
    if missing:
        errors.append(f"missing front matter: {', '.join(sorted(missing))}")
        return errors, metadata.get("status")
    status = metadata["status"]
    if status not in STATUSES:
        errors.append(f"invalid status: {status}")
    if not re.fullmatch(r"[a-z0-9]+(?:-[a-z0-9]+)*", metadata["id"]):
        errors.append("id must use kebab-case")
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", metadata["updated"]):
        errors.append("updated must use YYYY-MM-DD")
    headings = set(re.findall(r"^## (.+?)\s*$", text, re.MULTILINE))
    absent = [name for name in REQUIRED_SECTIONS
              if name not in headings and SECTION_ALIASES[name] not in headings]
    if absent:
        errors.append(f"missing sections: {', '.join(sorted(absent))}")
    if status in {"LOCATED", "TRACED", "CONTRACT", "CLIENT_TESTED"}:
        if metadata["native_sha256"] == "UNRESOLVED":
            errors.append(f"{status} requires native_sha256")
        if not re.search(r"FUN_[0-9A-Fa-f]{8}|0x[0-9A-Fa-f]{8}", text):
            errors.append(f"{status} requires a native function/address")
    if status in MATURE_STATUSES:
        for heading in ("Callers", "Callees"):
            body = section_body(text, heading)
            if not body or "UNRESOLVED" in body:
                errors.append(f"{status} requires resolved {heading}")
        if is_lifecycle_flow(metadata):
            for heading in LIFECYCLE_SUBSECTIONS:
                body = section_body(text, heading)
                if not has_resolved_content(body):
                    errors.append(
                        f"{status} lifecycle requires resolved {heading} or a justified N/A"
                    )
            matrix = section_body(text, "Transition matrix")
            if has_resolved_content(matrix) and not transition_matrix_has_data(matrix):
                errors.append(
                    f"{status} lifecycle requires a row in the Transition matrix"
                )
    if status in {"CONTRACT", "CLIENT_TESTED"}:
        if "| Claim |" not in text or not any(
            f"## {name}\n" in text for name in ("Decisions", SECTION_ALIASES["Decisions"])
        ):
            errors.append(f"{status} requires a delta matrix and decisions")
    if status == "CLIENT_TESTED" and re.search(
        r"(?:Real client|Client real):\s*(?:pending|not run|pendente|não executado)",
        text, re.IGNORECASE,
    ):
        errors.append("CLIENT_TESTED requires recorded real-client execution")
    return errors, status


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    args = parser.parse_args()
    root = args.repo.resolve() / ".agents" / "research" / "client748" / "flows"
    files = sorted(path for path in root.rglob("*.md") if path.name != "TEMPLATE.md")
    if not files:
        print(f"no records found in {root}")
        return 1
    failures = 0
    counts: Counter[str] = Counter()
    for path in files:
        errors, status = validate(path)
        if status:
            counts[status] += 1
        if errors:
            failures += 1
            for error in errors:
                print(f"ERROR {path.relative_to(args.repo.resolve())}: {error}")
        else:
            print(f"OK {path.relative_to(args.repo.resolve())}: {status}")
    print("status: " + ", ".join(f"{key}={counts[key]}" for key in sorted(counts)))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
