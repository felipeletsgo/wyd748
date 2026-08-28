#!/usr/bin/env python3
"""Valida maturidade e estrutura das fichas de fluxo do client 7.48."""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from pathlib import Path


STATUSES = {"UNMAPPED", "LOCATED", "TRACED", "CONTRACT", "CLIENT_TESTED"}
REQUIRED_KEYS = {"id", "title", "subsystem", "status", "native_sha256", "updated"}
REQUIRED_SECTIONS = {
    "Pergunta",
    "Fronteira de evidência",
    "Fluxo nativo 7.48",
    "Estado e lifecycle",
    "Wire, ABI e recursos",
    "Mapeamento atual",
    "Matriz de delta",
    "Decisões",
    "Lacunas",
    "Validação",
}
LIFECYCLE_SUBSECTIONS = (
    "Entrada observável",
    "Matriz de transições",
    "Vtables, vptrs e receptores",
    "Ownership",
    "Falha parcial",
    "Cleanup e teardown",
    "Shutdown",
    "Logout e relogin",
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
    match = re.search(
        rf"^### {re.escape(heading)}\s*$([\s\S]*?)(?=^### |^## |\Z)",
        text,
        re.MULTILINE,
    )
    return match.group(1).strip() if match else ""


def is_lifecycle_flow(metadata: dict[str, str]) -> bool:
    # A classificação explícita pelo subsystem é preferida; id/título mantêm o
    # gate ativo nas fichas históricas que já nomeiam lifecycle antes do schema.
    identity = " ".join(
        metadata.get(key, "") for key in ("id", "title", "subsystem")
    ).casefold()
    return "lifecycle" in identity


def has_resolved_content(body: str) -> bool:
    if not body.strip():
        return False
    plain = re.sub(r"^[\s>*-]+", "", body.strip()).strip()
    # Só N/A com justificativa integral fecha um item não aplicável. Marcadores
    # de lacuna continuam bloqueando a promoção mesmo quando trazem explicação.
    na_marker = r"(?:N/?A|NÃO SE APLICA|NAO SE APLICA)"
    na_match = re.fullmatch(
        rf"{na_marker}\s*:\s*(\S[\s\S]*\S|\S)", plain, re.IGNORECASE
    )
    if na_match:
        return True
    unresolved = (
        r"^(?:UNRESOLVED|TODO|PENDENTE|NÃO CONFIRMAD[OA]|NAO CONFIRMAD[OA])"
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
    # Cabeçalho mais ao menos uma transição concreta.
    return len(table_rows) >= 2


def validate(path: Path) -> tuple[list[str], str | None]:
    text = path.read_text(encoding="utf-8")
    metadata = front_matter(text)
    errors: list[str] = []
    missing = REQUIRED_KEYS - set(metadata)
    if missing:
        errors.append(f"front matter ausente: {', '.join(sorted(missing))}")
        return errors, metadata.get("status")
    status = metadata["status"]
    if status not in STATUSES:
        errors.append(f"status inválido: {status}")
    if not re.fullmatch(r"[a-z0-9]+(?:-[a-z0-9]+)*", metadata["id"]):
        errors.append("id deve usar kebab-case")
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", metadata["updated"]):
        errors.append("updated deve usar YYYY-MM-DD")
    headings = set(re.findall(r"^## (.+?)\s*$", text, re.MULTILINE))
    absent = REQUIRED_SECTIONS - headings
    if absent:
        errors.append(f"seções ausentes: {', '.join(sorted(absent))}")
    if status in {"LOCATED", "TRACED", "CONTRACT", "CLIENT_TESTED"}:
        if metadata["native_sha256"] == "UNRESOLVED":
            errors.append(f"{status} exige native_sha256")
        if not re.search(r"FUN_[0-9A-Fa-f]{8}|0x[0-9A-Fa-f]{8}", text):
            errors.append(f"{status} exige função/endereço nativo")
    if status in MATURE_STATUSES:
        for heading in ("Callers", "Callees"):
            body = section_body(text, heading)
            if not body or "UNRESOLVED" in body:
                errors.append(f"{status} exige {heading} resolvidos")
        if is_lifecycle_flow(metadata):
            for heading in LIFECYCLE_SUBSECTIONS:
                body = section_body(text, heading)
                if not has_resolved_content(body):
                    errors.append(
                        f"{status} lifecycle exige {heading} resolvido ou N/A justificado"
                    )
            matrix = section_body(text, "Matriz de transições")
            if has_resolved_content(matrix) and not transition_matrix_has_data(matrix):
                errors.append(
                    f"{status} lifecycle exige ao menos uma linha na Matriz de transições"
                )
    if status in {"CONTRACT", "CLIENT_TESTED"}:
        if "| Claim |" not in text or "## Decisões\n" not in text:
            errors.append(f"{status} exige matriz de delta e decisões")
    if status == "CLIENT_TESTED" and re.search(
        r"Client real:\s*(pendente|não executado)", text, re.IGNORECASE
    ):
        errors.append("CLIENT_TESTED exige execução real registrada")
    return errors, status


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    args = parser.parse_args()
    root = args.repo.resolve() / ".agents" / "research" / "client748" / "flows"
    files = sorted(path for path in root.rglob("*.md") if path.name != "TEMPLATE.md")
    if not files:
        print(f"nenhuma ficha encontrada em {root}")
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
