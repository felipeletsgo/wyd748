---
name: wyd-go-feature
description: Implementar, corrigir ou auditar mecânicas nativas no emulador WYD-Go/client 7.48: protocolo, combate, skills, NPCs, crafting, inventário, progressão, IA, visibilidade e persistência. Usar quando o comportamento divergir de W2PP, Secrets ou Micronics.
---

# WYD-Go feature

## Fluxo

1. Respeitar `wyd-go/AGENTS.md`; não reler documentação geral já disponível.
2. Localizar o caminho vivo com `rg` e preservar mudanças alheias.
3. Reproduzir o bug e identificar a fronteira: `wire`, `net`, `data`, `game`,
   `model` ou `store`.
4. Para regra nativa incerta, pesquisar somente o necessário:
   W2PP → Secrets 7.54 → Micronics/client 7.48 → `wyd-dev-knowledge`.
5. Portar semântica, nunca layout, offset ou endereço de outra versão.
6. Implementar no arquivo da feature; manter handlers como roteadores.
7. Testar a transição real, inclusive falha, rollback e relogin quando aplicável.
8. Rodar testes, vet, build do servidor e `git diff --check`.

## Invariantes

- `World` é o único escritor de gameplay.
- `ExtendedScore` é autoritativo; wire legado nunca volta ao cálculo.
- Cliente é não confiável; revalidar tudo server-side.
- Anti-dupe: validar → mutar cópia → persistir → publicar.
- Sincronizar dono, observers, visibilidade e persistência na mesma feature.
- Consultas locais usam índice espacial.
- `Merchant != 0` nunca entra em combate.
- Inventário: 64/63; cargo: 128/120.

## Referências sob demanda

- Ler `references/emulator-contracts.md` apenas para o assunto afetado
  (protocolo, NPC/craft, IA, combate ou persistência).
- Em auditoria de código morto/duplicação, ler `references/audit.md`.
- Para protocolo, fórmulas ou funções nativas ausentes, usar
  `wyd-dev-knowledge`; não carregar essa base para mudanças puramente locais.
- Para hooks no plugin Micronics, usar `add-hook` e `build-deploy`.

## Conclusão

Concluir quando a regra for autoritativa, o feedback do client estiver correto,
falha/relogin preservarem estado, observers estiverem sincronizados e a
validação relevante passar. Atualizar somente a documentação afetada.
