---
name: wyd-client748-research
description: Rastrear transições observáveis do client nativo WYD 7.48 antes de adaptar comportamento, protocolo, UI, input, render, assets ou lifecycle.
---

# Pesquisa do client 7.48

Use esta skill para qualquer fronteira legada do client ou para validar a
integração de uma extensão coordenada. O fluxo obrigatório é:

```text
catálogo -> callgraph -> fluxo observável -> adaptação -> validação
```

## Autoridade

Avalie e registre por frente o binário nativo 7.48, o projeto/descompilação
Ghidra, assets 7.48, source atual do TMProject, `wydgo748/`, testes e demais
referências. Marque cada fonte como `UTILIZADA`, `NÃO APLICÁVEL` ou
`CONTRADITÓRIA`, com justificativa. O binário e o Ghidra prevalecem; TMProject
7.69, guias e histórico são referências secundárias. W2PP, Secrets e Micronics
são excluídos e não podem orientar, validar ou contradizer a decisão.

## Classificação e gates

- `PARIDADE_NATIVA`: reproduz comportamento legado; exige ficha `TRACED` e,
  para wire/ABI/loader/recurso, `CONTRACT` antes da adaptação.
- `MODERNIZACAO_COMPATIVEL`: muda a implementação interna preservando o
  contrato; reutiliza a ficha comprovada e restringe a prova às fronteiras
  afetadas.
- `EXTENSAO_COORDENADA`: adiciona contrato ausente no nativo; exige formato,
  versão/capability quando necessário, IDs/opcodes sem colisão, validação,
  rollback/fallback e testes dos dois projetos. Não é paridade.

## O que rastrear

Uma ficha descreve uma transição: entrada/evento, raiz nativa, callers/callees,
estado e mutações, erros, wire/ABI/recursos, source/server, decisão e
validação. Confirme no Ghidra chamadas indiretas, vtables, callbacks,
ownership, falha parcial, cleanup e teardown. Em lifecycle, resolva também
shutdown e logout/relogin ou marque `N/A` com justificativa.

Para packets, registre direção, opcode, tamanho, offsets, packing e signedness.
Para UI/assets, registre recurso materializado, binding, input, ownership,
fechamento e teardown. Ausência de caller textual não prova código morto.

## Continuação e saída

Leia apenas esta skill, `AGENTS.md`, o handoff e as referências necessárias ao
escopo. Confira status, diff scoped e fingerprints; reutilize evidência estável.
Registre a ficha em `.agents/research/` e a decisão durável em `DOCS/` sem
duplicar conteúdo. Use `validate_research.py` quando fichas ou schema mudarem.

`STATICALLY VERIFIED`, `AUTOMATED TESTED` e `CLIENT-TESTED` são estados
distintos. Build ou teste estático nunca promove uma frente a `CLIENT_TESTED`.
