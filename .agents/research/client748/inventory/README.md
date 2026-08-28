# Inventário completo das funções nativas WYD 7.48

Estado: `INVENTORY_GENERATED`

Este relatório cobre todas as entradas presentes em `functions.tsv`. Ele identifica o material disponível e cria uma fila de pesquisa; não transforma pseudocódigo exportado em compreensão ou contrato.

## Estratégia operacional

O programa segue a transição `catálogo -> callgraph -> fluxo observável ->
adaptação -> validação`. O catálogo é o censo e o triador fornece uma ordem
reproduzível; a compreensão só é atribuída pela ficha do fluxo após confirmação
no projeto Ghidra. Não criar uma ficha isolada para cada função nem usar o
TMProject 7.69+ como contrato do 7.48.

Para iniciar ou retomar uma frente:

```powershell
python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format json --top 20
```

O JSON conserva a contagem e as lanes de todo o corpus e limita somente a lista
`top`. Para gerar uma fila TSV completa, use `--format tsv --top 0 --output
<arquivo temporário>`; nunca substitua `functions.tsv` com a saída da triagem.

Os estados do programa são distintos: `UNMAPPED` (sem entrada de pesquisa),
`LOCATED` (localizado), `TRACED` (fluxo confirmado), `CONTRACT` (contrato
wire/ABI/recursos confirmado), `IMPLEMENTED` (delta aplicado) e
`CLIENT_TESTED` (fluxo real executado no `client748/project.exe`). O triador
apenas ordena; não promove nenhum estado.

## Artefatos

- Corpus: `C:\Users\felipe\Tools\GhidraAnalysis\20260821\decompiled`
- Inventário TSV: `C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go\.agents\research\client748\inventory\functions.tsv`
- Binário de referência: `client748/wyd.exe nativo+patches/WYD.exe`
- A confirmação de hash permanece obrigatória antes de usar endereços.

## Contagem

- Funções no índice Ghidra: **4146**
- Funções citadas no handoff de paridade: **39**
- Funções citadas nas fichas formais: **23**
- Funções distintas documentadas: **62**
- Documentadas e presentes no índice: **62**
- Referências documentadas resolvidas como endereço interno: **0**
- Referências documentadas ausentes do índice textual: **2**
- Funções com referência direta na source C++/H: **69**
- Funções no catálogo estrutural Ghidra: **4146**
- Linhas com callgraph direto Ghidra: **4146**
- Funções sem xref FLOW de entrada no catálogo: **2062**
- SHA-256 registrado pelo projeto Ghidra: **8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593**

### Classificação inicial do corpus

| Classe | Quantidade | Próxima ação |
| --- | ---: | --- |
| `PARITY_STATIC_EVIDENCE` | 39 | fechar callers/callees e fluxo no Ghidra |
| `FLOW_SHEET` | 23 | completar ficha a partir de `LOCATED` |
| `PARITY_AND_FLOW` | 0 | consolidar evidências |
| `CORPUS_ONLY` | 4084 | triagem por entrypoint/caller |
| `UNRESOLVED_REFERENCE` | 0 | resolver endereço diretamente no Ghidra |

## Limites da classificação

- `CORPUS_ONLY` significa somente que a função ainda não aparece nas fichas/handoff selecionados; não significa que seja código morto.
- Callers/callees são candidatos obtidos por tokens nos `.c`; chamadas indiretas, callbacks, vtables e destinos calculados continuam exigindo revisão no projeto Ghidra. O campo `ghidra_*` registra somente relações diretas reconhecidas pela análise.
- `functions.tsv` é o índice de decompilação; `ghidra-functions.tsv` é o catálogo estrutural versionado desta análise. Os dois devem conter exatamente as mesmas 4.146 entradas antes de serem combinados.
- `STATICALLY_EVIDENCED` não significa `TRACED`, `CONTRACT` ou `CLIENT_TESTED`.
- Uma função nativa pode ser wrapper, runtime, helper compartilhado ou código não alcançável no fluxo 7.48; nesses casos o resultado correto será documentar a decisão, não fabricar uma tradução para o TMProject.

## Critério para considerar compreendida

Uma função só sai da fila quando estiver vinculada a uma transição observável com entrada, callers/callees confirmados no Ghidra, estado, side effects, erros, teardown/relogin quando aplicável, equivalente na source e validação proporcional. Packet/ABI exige `CONTRACT`; execução no candidato exige `CLIENT_TESTED`.

Referências documentadas ainda não confirmadas no índice textual:

- `FUN_00452733` — referência observada no repositório; resolver diretamente
  no projeto Ghidra antes de atribuir qualquer endereço.
- `FUN_0047E4D6` — referência observada no repositório; resolver diretamente
  no projeto Ghidra antes de atribuir qualquer endereço.
