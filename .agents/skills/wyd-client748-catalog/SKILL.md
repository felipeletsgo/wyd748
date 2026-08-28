---
name: wyd-client748-catalog
description: Catalogar, validar e priorizar as funções nativas do client WYD 7.48 usando o corpus Ghidra e o callgraph antes de escolher ou adaptar um fluxo no TMProject.
---

# Catálogo do client WYD 7.48

Use esta skill quando a tarefa envolver o programa completo de mapeamento,
retomada de pesquisa, escolha da próxima função/raiz ou auditoria de heranças
7.59/7.69+ no TMProject. Ela é a primeira trilha do trabalho e não substitui a
skill `wyd-client748-research`: seu resultado é uma fila de pesquisa, não um
contrato comportamental.

## Autoridade e limites

- O binário de referência é `client748/wyd.exe nativo+patches/WYD.exe`.
- Confirme o SHA-256 antes de interpretar endereços ou exports.
- `functions.tsv` e `ghidra-functions.tsv` devem representar as mesmas 4.146
  entradas do corpus 7.48.
- O corpus exportado acelera a descoberta; callers, callees indiretos, vtables,
  callbacks, dados e lifecycle precisam ser confirmados no projeto Ghidra.
- TMProject, W2PP e referências posteriores sugerem semântica, mas não criam
  contrato 7.48.
- A triagem nunca promove uma função para `TRACED`, `CONTRACT`, `IMPLEMENTED`
  ou `CLIENT_TESTED`.

## Procedimento obrigatório

1. Verificar `git status --short --branch` e preservar qualquer alteração alheia.
2. Executar o triador sem modificar o catálogo:

   ```powershell
   python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
   ```

3. Escolher uma raiz ou grupo pequeno de funções pela fila e formular uma
   pergunta observável. Não transformar cada linha do TSV em uma tarefa isolada.
4. Consultar `query_corpus.py` para localizar strings e pseudocódigo, e então
   abrir a função, seus xrefs e suas chamadas no projeto Ghidra.
5. Para entradas sem caller direto, revisar vtable, callback, thunk, referência
   de dados, `bodyrefs` e `rawrelatives` antes de concluir que são inativas.
6. Entregar a função/raiz e sua transição para
   `wyd-client748-research`, que cria ou atualiza uma ficha por fluxo.
7. Só depois de `TRACED`/`CONTRACT` o `wyd-go-feature` pode adaptar o delta
   comprovado no TMProject ou no WYD-Go.

## Lanes de pesquisa

O script escolhe uma lane primária, apenas para ordenar trabalho:

- `DOCUMENTED_FIRST`: já aparece em ficha, handoff ou evidência não textual;
- `SOURCE_REFERENCED`: possui referência direta no C++/header recompilável;
- `INDIRECT_OR_CALLBACK`: não possui caller direto e exige revisão de callback,
  vtable, thunk ou referência de dados;
- `HIGH_FANOUT`: tem muitos callers/callees ou referências de entrada e pode
  explicar várias transições;
- `CORPUS_TRIAGE`: ainda não tem evidência suficiente e deve ser agrupada por
  caller, callee, string ou subsistema.

A lane e uma pontuação são somente instrumentos de triagem. A prioridade nunca
substitui a pergunta concreta, a ficha ou a confirmação Ghidra.

## Saída esperada

Ao terminar a triagem, registrar no handoff ou na ficha:

- contagem total e eventual divergência do corpus;
- lane e motivo da raiz escolhida;
- entrada e hash do binário consultado;
- export/query usado e a limitação conhecida;
- próximo comando ou função reproduzível.

Para o fluxo pesquisado, use a ficha em `.agents/research/client748/flows/` e
classifique cada claim como `CONFIRMED`, `PROBABLE`, `HYPOTHESIS` ou `REJECTED`.
Não usar este catálogo para fabricar stubs no TMProject ou preencher nomes
ausentes no Ghidra.

## Prioridade dos fluxos

Quando não houver uma urgência de produção, seguir a ordem:

1. bootstrap, configuração, lifecycle e shutdown;
2. transporte, framing, criptografia e login;
3. dispatcher, cenas e entrada no mundo;
4. tick, input, movimento e entidades;
5. inventário, equipamento, NPC, loja e trade;
6. combate, skills, affects e score;
7. UI, HUD, render e assets;
8. helpers, wrappers e rotinas sem impacto observável.

A ordem pode mudar por evidência de um fluxo bloqueante, mas a decisão deve ser
documentada e não pode importar ABI, packet, recurso ou offset de versão nova.

Leia [references/catalog-strategy.md](references/catalog-strategy.md) para o
schema, a política de ranking e os critérios de conclusão da campanha.
