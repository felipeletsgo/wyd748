# Inventário completo das funções nativas WYD 7.48

Estado: `INVENTORY_GENERATED`

Este relatório cobre todas as entradas presentes em `functions.tsv`. Ele identifica o material disponível e cria uma fila de pesquisa; não transforma pseudocódigo exportado em compreensão ou contrato.

## Artefatos

- Corpus: `C:\Users\felipe\Tools\GhidraAnalysis\20260821\decompiled`
- Inventário TSV: `C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go\.agents\research\client748\inventory\functions.tsv`
- Binário de referência: `client748/wyd.exe nativo+patches/WYD.exe`
- A confirmação de hash permanece obrigatória antes de usar endereços.

## Contagem

- Funções no índice Ghidra: **4146**
- Funções citadas no handoff de paridade: **42**
- Funções citadas nas fichas formais: **148**
- Funções distintas documentadas: **185**
- Documentadas e presentes no índice: **185**
- Referências documentadas resolvidas como endereço interno: **2**
- Referências documentadas ainda não resolvidas: **0**
- Funções com referência direta na source C++/H: **70**
- Funções no catálogo estrutural Ghidra: **4146**
- Linhas com callgraph direto Ghidra: **4146**
- Funções sem xref FLOW de entrada no catálogo: **2062**
- SHA-256 registrado pelo projeto Ghidra: **8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593**

### Classificação inicial do corpus

| Classe | Quantidade | Próxima ação |
| --- | ---: | --- |
| `PARITY_STATIC_EVIDENCE` | 37 | fechar callers/callees e fluxo no Ghidra |
| `FLOW_SHEET` | 143 | completar ficha a partir de `LOCATED` |
| `PARITY_AND_FLOW` | 5 | consolidar evidências |
| `CORPUS_ONLY` | 3961 | triagem por entrypoint/caller |
| `UNRESOLVED_REFERENCE` | 0 | resolver endereço diretamente no Ghidra |

## Limites da classificação

- `CORPUS_ONLY` significa somente que a função ainda não aparece nas fichas/handoff selecionados; não significa que seja código morto.
- Callers/callees são candidatos obtidos por tokens nos `.c`; chamadas indiretas, callbacks, vtables e destinos calculados continuam exigindo revisão no projeto Ghidra. O campo `ghidra_*` registra somente relações diretas reconhecidas pela análise.
- `functions.tsv` é o índice de decompilação; `ghidra-functions.tsv` é o catálogo estrutural versionado desta análise. Os dois devem conter exatamente as mesmas 4.146 entradas antes de serem combinados.
- `STATICALLY_EVIDENCED` não significa `TRACED`, `CONTRACT` ou `CLIENT_TESTED`.
- Uma função nativa pode ser wrapper, runtime, helper compartilhado ou código não alcançável no fluxo 7.48; nesses casos o resultado correto será documentar a decisão, não fabricar uma tradução para o TMProject.

## Critério para considerar compreendida

Uma função só sai da fila quando estiver vinculada a uma transição observável com entrada, callers/callees confirmados no Ghidra, estado, side effects, erros, teardown/relogin quando aplicável, equivalente na source e validação proporcional. Packet/ABI exige `CONTRACT`; execução no candidato exige `CLIENT_TESTED`.

Referências resolvidas dentro do corpo de uma função:

- `FUN_00452733` -> `FUN_00452728`
- `FUN_0047E4D6` -> `FUN_0047E49D`
