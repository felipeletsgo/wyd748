---
name: wyd-client748-catalog
description: Atualizar o censo nativo WYD 7.48 ou localizar/priorizar raízes de investigação ainda sem seed; não repetir catálogo para continuar fluxo já localizado.
---

# Catálogo do client 7.48

Use esta skill quando faltar uma raiz ou quando a tarefa mudar o censo/ranking.
Com seed/ficha válida, seguir diretamente para `wyd-client748-research`.
O censo prioriza raízes; uma linha de `functions.tsv` não representa uma função
compreendida. Catalogação não é pré-requisito repetido de cada adaptação.

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

1. Reutilize a entrada única do `AGENTS.md`. Consulte o seed/censo existente;
   valide identidade apenas se a evidência necessária estiver ausente ou mudou.
2. Só execute o triador quando o corpus/catalogo mudou ou quando não existir
   fila válida para a nova raiz:

   ```powershell
   python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
   ```

3. Escolha um lote pequeno ligado a uma entrada observável; priorize
   `bootstrap -> login -> cenas -> dispatcher -> mundo -> logout/relogin`.
4. Registre raiz, motivo, candidatos, evidência e próximo símbolo em
   `.agents/research/`. Ao localizar a raiz, entregue os candidatos e lacunas
   à pesquisa; não espere compreender todo o callgraph para encerrar catalogação.

### Inventário reproduzível da source

Quando a tarefa exigir cobertura do código do TMProject748, instale as
dependências fixadas em `scripts/requirements-source-inventory.txt` num ambiente
isolado e execute `scripts/source_inventory.py --repo <raiz> --write`. Use
`--check --fail-on-parse-errors` no gate para detectar inventários
desatualizados ou erros de parser restantes. Os TSVs gerados
registram somente fatos de arquivo, projeto, linha e símbolo; nunca grave neles
decisões humanas de compatibilidade ou promova estado de pesquisa.

## Estados

Use somente `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`, `IMPLEMENTED` ou
`CLIENT_TESTED`. `LOCATED` é localização, não autorização para afirmar
paridade; `CLIENT_TESTED` exige execução do client construído.

Leia `references/catalog-strategy.md` somente ao mudar schema, ranking ou
critério de cobertura.
