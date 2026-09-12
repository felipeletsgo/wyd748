---
name: wyd-client748-catalog
description: Catalogar o corpus nativo WYD 7.48 e escolher raízes de callgraph para investigação do client.
---

# Catálogo do client 7.48

Use esta skill antes de uma investigação de paridade nativa. Ela mantém o
censo e prioriza raízes; não transforma uma linha de `functions.tsv` em função
compreendida e não substitui `wyd-client748-research`.

## Regras

- Parta da feature/função viva em `tmproject/` e propague para candidatos
  nativos por seed, string, fingerprint, vtable e callgraph.
- O binário nativo em
  `references/client748/WYD.exe`, o projeto Ghidra e a
  descompilação estudada são a referência primária para claims 7.48.
- `functions.tsv`, exports, pontuação e `EXACT_MATCH` são instrumentos de
  descoberta. Callbacks, thunks, vtables, `bodyrefs` e `rawrelatives` exigem
  confirmação no Ghidra.
- Reutilize hashes, seeds, triagem e correlações confirmadas quando os inputs
  não mudaram. Não refaça uma varredura global em uma continuação.
- Uma extensão coordenada pode ser planejada sem equivalente nativo, desde que
  seja declarada como tal e tenha contrato client/server separado.

## Procedimento

1. Verifique `git status --short`, `HEAD`, o handoff do escopo e os fingerprints.
2. Só execute o triador quando o corpus/catalogo mudou ou quando não existir
   fila válida para a nova raiz:

   ```powershell
   python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
   ```

3. Escolha um lote pequeno ligado a uma entrada observável; priorize
   `bootstrap -> login -> cenas -> dispatcher -> mundo -> logout/relogin`.
4. Registre raiz, motivo, candidatos, evidência e próximo símbolo em
   `.agents/research/`. Entregue transições completas à skill de pesquisa.

## Estados

Use somente `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`, `IMPLEMENTED` ou
`CLIENT_TESTED`. `LOCATED` é localização, não autorização para afirmar
paridade; `CLIENT_TESTED` exige execução do client construído.

Leia `references/catalog-strategy.md` somente ao mudar schema, ranking ou
critério de cobertura.
