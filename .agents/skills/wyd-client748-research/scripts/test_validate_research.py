#!/usr/bin/env python3
"""Testes do gate estrutural das fichas de pesquisa 7.48."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from validate_research import LIFECYCLE_SUBSECTIONS, validate


NATIVE_HASH = "8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593"


def research_record(status: str = "TRACED", lifecycle: bool = True) -> str:
    subsystem = "lifecycle" if lifecycle else "transport"
    record_id = "scene-transition-lifecycle" if lifecycle else "packet-dispatch-flow"
    record_title = "Troca de cena e lifecycle" if lifecycle else "Dispatch de packet"
    lifecycle_sections = ""
    if lifecycle:
        lifecycle_sections = """
### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| estado 7 | cena ativa | FUN_004B37C9 | estado 5 | marca nó | cleanup pendente no tick |

### Vtables, vptrs e receptores

O receptor foi seguido até o vptr `0x005A45FC`; o slot relevante é `+0x54`.

### Ownership

O manager possui a árvore; a cena permanece owned pelo nó até o cleanup.

### Falha parcial

A inicialização destrói a cena recém-alocada antes de retornar falha.

### Cleanup e teardown

`FUN_004B16C0` remove e destrói os nós marcados no tick seguinte.

### Shutdown

N/A: esta transição não encerra o processo e converge ao teardown do manager.

### Logout e relogin

N/A: o fluxo termina antes do login; a justificativa limita este contrato.
"""
    client_real = (
        "executado no client748/project.exe, hash registrado no cenário"
        if status == "CLIENT_TESTED"
        else "não executado"
    )
    return f"""---
id: {record_id}
title: {record_title}
subsystem: {subsystem}
status: {status}
native_sha256: {NATIVE_HASH}
updated: 2026-08-28
---

# {record_title}

## Pergunta

Qual transição observável ocorre?

## Fronteira de evidência

- Executável/hash: confirmado

## Fluxo nativo 7.48

### Entrada observável

O pedido de estado 7 entra por `FUN_004B3500` e publica a nova cena.

### Callers

`FUN_004B37C9` é o caller confirmado.

### Função principal

`FUN_004B3500` executa a transição.

### Callees

`FUN_0054AC09` anexa a cena à árvore.

### Saídas e erros

Sucesso anexa; falha destrói a alocação parcial.

## Estado e lifecycle
{lifecycle_sections}

## Wire, ABI e recursos

N/A: não há wire nesta transição local.

## Mapeamento atual

### Source recompilável

ObjectManager é o candidato de adaptação.

### WYD-Go

N/A: a troca é local ao client.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| troca | confirmada | divergente | pista | N/A | portar |

## Decisões

Portar somente o claim confirmado.

## Lacunas

Nenhuma dentro da transição delimitada.

## Validação

- Pesquisa: Ghidra reaberto
- Automação: validador executado
- Client real: {client_real}
"""


class ValidateResearchTests(unittest.TestCase):
    def validate_text(self, text: str) -> list[str]:
        # Arquivo temporário exerce o mesmo caminho UTF-8 usado pela CLI real.
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "flow.md"
            path.write_text(text, encoding="utf-8")
            errors, _ = validate(path)
            return errors

    def test_mature_lifecycle_record_passes(self) -> None:
        for status in ("TRACED", "CONTRACT", "CLIENT_TESTED"):
            with self.subTest(status=status):
                self.assertEqual([], self.validate_text(research_record(status)))

    def test_each_lifecycle_field_is_required(self) -> None:
        complete = research_record()
        for heading in LIFECYCLE_SUBSECTIONS:
            with self.subTest(heading=heading):
                start = complete.index(f"### {heading}")
                next_heading = complete.find("\n### ", start + 4)
                next_section = complete.find("\n## ", start + 4)
                candidates = [index for index in (next_heading, next_section) if index >= 0]
                end = min(candidates) if candidates else len(complete)
                broken = complete[:start] + f"### {heading}\n\nUNRESOLVED\n" + complete[end:]
                errors = self.validate_text(broken)
                self.assertTrue(
                    any(heading in error for error in errors),
                    msg=f"gate ausente para {heading}: {errors}",
                )

    def test_transition_matrix_requires_a_data_row(self) -> None:
        text = research_record().replace(
            "| estado 7 | cena ativa | FUN_004B37C9 | estado 5 | marca nó | cleanup pendente no tick |\n",
            "",
        )
        errors = self.validate_text(text)
        self.assertTrue(any("Matriz de transições" in error for error in errors))

    def test_na_requires_a_justification(self) -> None:
        text = research_record().replace(
            "N/A: esta transição não encerra o processo e converge ao teardown do manager.",
            "N/A",
        )
        errors = self.validate_text(text)
        self.assertTrue(any("Shutdown" in error for error in errors))

    def test_unresolved_with_explanation_does_not_close_gate(self) -> None:
        text = research_record().replace(
            "O manager possui a árvore; a cena permanece owned pelo nó até o cleanup.",
            "UNRESOLVED: ainda é necessário identificar o owner.",
        )
        errors = self.validate_text(text)
        self.assertTrue(any("Ownership" in error for error in errors))

    def test_located_lifecycle_may_remain_incomplete(self) -> None:
        text = research_record("LOCATED").replace(
            "O receptor foi seguido até o vptr `0x005A45FC`; o slot relevante é `+0x54`.",
            "UNRESOLVED",
        )
        self.assertEqual([], self.validate_text(text))

    def test_non_lifecycle_traced_uses_general_gate(self) -> None:
        text = research_record(lifecycle=False)
        self.assertEqual([], self.validate_text(text))


if __name__ == "__main__":
    unittest.main()
