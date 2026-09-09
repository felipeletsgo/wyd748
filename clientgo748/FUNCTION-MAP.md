# Censo funcional

O mapeamento função a função está em
[`references/catalog/functions.tsv`](references/catalog/functions.tsv). O TSV
é deliberadamente tabular para permitir filtros e auditoria; não é uma lista
de pseudocódigos a traduzir linearmente.

Campos que decidem a maturidade:

- `entry` e `symbol`: endereço preferido e nome Ghidra/exportado;
- `research_status`: `UNMAPPED`, `LOCATED` ou estado de evidência estática;
- `evidence_class`: corpus, ficha, paridade ou prova estática;
- `textual_callers`/`textual_callees`: candidatos do export, sujeitos a
  confirmação de xrefs/vtables no Ghidra;
- `ghidra_direct_callers`/`ghidra_direct_callees`: relações já resolvidas no
  catálogo Ghidra;
- `next_action`: comando de pesquisa, não promessa de implementação.

Os 3.961 itens `CORPUS_ONLY` permanecem `UNMAPPED`. Ausência de caller textual
não prova código morto: revisar callback, thunk, vtable, `bodyrefs` e
`rawrelatives` antes de descartar uma entrada.

Para cada claim comportamental, a ficha em
`references/research/flows/<subsystem>/<flow>.md` é a unidade de evidência.
Uma ficha só pode ser promovida após resolver entrada observável, callers,
callees, estado, erro, teardown e, quando aplicável, wire/ABI/recurso.
