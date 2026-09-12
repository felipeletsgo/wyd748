# Mapa completo da documentação

Este arquivo é o índice de localização. A regra de manutenção é simples:
documentos duráveis ficam em `DOCS/`; evidências de agente ficam em
`.agents/research/` ou `.agents/handoffs/`; nenhuma source deve conter docs.
Nomes históricos encontrados dentro de handoffs são preservados como contexto,
mas não representam a arquitetura ativa.

## Documentação durável do produto

### Arquitetura e planejamento

- [`architecture-map.md`](architecture-map.md)
- [`target-architecture.md`](target-architecture.md)
- [`implementation-plan.md`](implementation-plan.md)
- [`file-migration.md`](file-migration.md)

### Estado e escopo

- [`IMPLEMENTATION_STATUS.md`](IMPLEMENTATION_STATUS.md)
- [`IMPLEMENTED.md`](IMPLEMENTED.md)
- [`ROADMAP.md`](ROADMAP.md)
- [`SCORE.md`](SCORE.md)

### Client

- [`client/port-748.md`](client/port-748.md)
- [`client/ui-windows.md`](client/ui-windows.md)
- [`client/native-reference-patches.md`](client/native-reference-patches.md)

### Servidor e dados

- [`EMULATOR.md`](EMULATOR.md)
- [`features-macro.md`](features-macro.md)
- [`basedef-documentation.md`](basedef-documentation.md)
- [`server/boss-data.md`](server/boss-data.md)

### Protocolo e contratos

- [`contracts-baseline.md`](contracts-baseline.md)
- [`login-dispatch.md`](login-dispatch.md)
- [`wire-opcode-catalog.md`](wire-opcode-catalog.md)
- [`protocol/packet-fixtures.md`](protocol/packet-fixtures.md)

### Guias e referências externas

- [`GUIA/`](GUIA/) contém os guias legados mantidos fora das sources.
- [`reference-upstream.md`](reference-upstream.md) identifica a referência
  upstream usada pelo projeto.

## Documentação operacional do agente

- [`.agents/skills/`](../.agents/skills/) contém as skills ativas; cada pasta
  tem um único `SKILL.md` como entrada.
- [`.agents/research/`](../.agents/research/) contém catálogo, triagem,
  callgraph, fichas e evidência do client nativo 7.48.
- [`.agents/handoffs/`](../.agents/handoffs/) contém registros de continuidade
  e decisões temporais. Eles não são fonte de verdade quando divergem da
  árvore atual.

## Regra de atualização

Ao criar ou mover um documento:

1. escolher uma única localização em `DOCS/`, `.agents/research/` ou
   `.agents/handoffs/`;
2. corrigir links relativos e referências à raiz `tmproject/` ou `wydgo748/`;
3. atualizar este mapa se o assunto ou a localização mudar;
4. verificar que não há documentação, scripts temporários ou artefatos
   gerados dentro das sources.
