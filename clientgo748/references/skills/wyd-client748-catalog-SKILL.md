---
name: wyd-client748-catalog
description: Catalogar e correlacionar rapidamente funções do client WYD 7.48 com a source atual, priorizando raízes e callgraph antes de claims de paridade ou adaptação.
---

# Catálogo do client WYD 7.48

Esta skill mantém o censo nativo e escolhe raízes de pesquisa. Ela não exige
análise linear das 4.146 funções e não substitui `wyd-client748-research`.

## Fast path

Em continuação, use o handoff, `git status`, diff scoped e fingerprints já
registrados. Não rerode triagem, inventário, export ou SHA-256 quando os
respectivos inputs não mudaram. Execute o triador somente em campanha nova,
mudança do corpus/catálogo ou escolha de uma raiz sem fila válida.

Parta preferencialmente da source viva:

```text
função/feature TMProject
-> candidatos nativos por seed, string, fingerprint, vtable e callgraph
-> vizinhos que decidem a transição
-> ficha do fluxo
```

Agrupe helpers pela transição e reduza a prioridade de runtime/D3DX. Seeds
confirmados são cache de pesquisa: reutilize-os e propague pelos callers,
callees e slots adjacentes.

## Autoridade e limites

- A referência nativa é `client748/wyd.exe nativo+patches/WYD.exe`.
- `functions.tsv` e `ghidra-functions.tsv` censam as mesmas 4.146 entradas.
- Export e correlação aceleram descoberta; xrefs indiretos, vtables, callbacks
  e lifecycle que sustentam um claim são confirmados no projeto Ghidra.
- `EXACT_MATCH`, lane e pontuação são candidatos, não maturidade.
- TMProject posterior pode fornecer arquitetura candidata, mas não fatos do
  nativo 7.48.
- O catálogo é gate para novo claim de paridade, não para uma extensão
  coordenada explicitamente projetada sem alegação nativa.

## Procedimento

1. Preservar mudanças alheias e decidir se há cache válido da fila.
2. Quando necessário, executar:

   ```powershell
   python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
   ```

3. Escolher uma raiz ou lote pequeno ligado a uma entrada observável. Para
   migração, começar pela função atual da source; para pesquisa aberta, usar a
   fila por subsistema/fan-out.
4. Consultar o corpus para candidatos e abrir no Ghidra somente a raiz e os
   vizinhos que decidem a pergunta.
5. Sem caller direto, revisar vtable, callback, thunk, `bodyrefs` e
   `rawrelatives`; zero textual não prova código morto.
6. Entregar a transição a `wyd-client748-research` e registrar a descoberta no
   mesmo ciclo, antes de trocar de lote.

## Lanes

- `DOCUMENTED_FIRST`: evidência existente a fechar;
- `SOURCE_REFERENCED`: correlação direta com a source atual;
- `INDIRECT_OR_CALLBACK`: vtable, callback, thunk ou dado;
- `HIGH_FANOUT`: raiz que explica múltiplas transições;
- `CORPUS_TRIAGE`: ainda precisa ser agrupada.

A lane apenas ordena. O caminho crítico tem precedência prática:
bootstrap → login → cenas → dispatcher → mundo → logout/relogin.

## Saída

Registrar somente o delta novo: raiz/lote, motivo, binário/corpus identificados,
candidatos descartados ou confirmados e próximo símbolo/comando. Não duplicar
contagens e hashes estáveis em cada sessão.

Use a ficha em `.agents/research/client748/flows/` para claims nativos. Uma
feature deliberadamente nova deve ser documentada como extensão coordenada,
sem fabricar equivalente ou maturidade nativa.

Leia [references/catalog-strategy.md](references/catalog-strategy.md) ao mudar o
schema, ranking ou critério de cobertura; não é leitura de toda retomada.
