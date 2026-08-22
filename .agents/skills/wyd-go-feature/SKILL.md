---
name: wyd-go-feature
description: Implementar, corrigir ou auditar o WYD-Go e o client 7.48. Para client, protocolo, ABI, UI, input, render ou assets, exige consultar a descompilação Ghidra do WYD.exe 7.48 antes de editar; W2PP, Secrets, Micronics e TMProject são referências semânticas secundárias.
---

# WYD-Go feature

## Fluxo

1. Respeitar `wyd-go/AGENTS.md`; não reler documentação geral já disponível.
2. Localizar o caminho vivo com `rg` e preservar mudanças alheias.
3. Reproduzir o bug e identificar a fronteira: `wire`, `net`, `data`, `game`,
   `model`, `store` ou `client-source`.
4. Se a tarefa tocar client, protocolo, packet, ABI, struct, UI, input, render,
   assets ou comportamento observado do executável, ler primeiro
   [`references/ghidra-client748.md`](references/ghidra-client748.md) e consultar
   a função nativa correspondente na descompilação Ghidra do `WYD.exe` 7.48.
   Essa consulta é obrigatória mesmo quando TMProject/W2PP já têm código similar.
5. Para regra nativa ainda incerta depois da evidência 7.48, pesquisar somente
   o necessário: W2PP → Secrets 7.54 → Micronics → `wyd-dev-knowledge`.
6. Portar semântica, nunca layout, offset ou endereço de outra versão. Layout,
   offset, opcode, tamanho e lifecycle do client precisam vir do 7.48.
   `client-source/tmproject` é uma source de versão única: depois de confirmar o
   equivalente nativo, remover o caminho exclusivo de 7.59+ em vez de manter
   branches de compatibilidade entre versões. Dados novos só podem permanecer
   quando traduzidos para uma representação comprovadamente suportada pelo 7.48.
7. Implementar no arquivo da feature; manter handlers como roteadores.
8. Comentar no próprio código todo trecho editado ou implementado, explicando
   intenção, contrato ou motivo técnico. Não usar comentário redundante que
   apenas repita a instrução executada.
9. Testar a transição real, inclusive falha, rollback e relogin quando aplicável.
10. Rodar testes, vet, build do servidor e `git diff --check`.

## Gate obrigatório do client 7.48

Antes de alterar código relacionado ao client, registrar na análise de trabalho:

- executável e SHA-256 usados como referência;
- funções Ghidra consultadas;
- callers/callees relevantes;
- offsets e tamanhos comprovados, quando houver wire/ABI;
- diferença observada entre o nativo 7.48 e o TMProject recompilável;
- o que foi portado como comportamento e o que deliberadamente não foi portado.

Não editar por tentativa visual quando o fluxo pode ser recuperado da
descompilação. Se a função não estiver identificada, localizar por strings,
xrefs, opcode, tamanho, constantes, imports ou call graph antes de implementar.
Se ainda não houver evidência suficiente, marcar como `não confirmado` e não
transformar a hipótese em contrato.

## Invariantes

- `World` é o único escritor de gameplay.
- `ExtendedScore` é autoritativo; wire legado nunca volta ao cálculo.
- Cliente é não confiável; revalidar tudo server-side.
- Anti-dupe: validar → mutar cópia → persistir → publicar.
- Sincronizar dono, observers, visibilidade e persistência na mesma feature.
- Consultas locais usam índice espacial.
- `Merchant != 0` nunca entra em combate.
- Inventário: 64/63; cargo: 128/120.
- A source recompilável possui somente ABI, UI, protocolo e lifecycle 7.48;
  referências mais novas não permanecem como implementação alternativa.

## Referências sob demanda

- Para qualquer trabalho no client compilável ou no ABI 7.48, ler
  `references/ghidra-client748.md`; esta referência não é opcional nesse escopo.
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
