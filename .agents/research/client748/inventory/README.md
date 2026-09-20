# Inventários do client WYD 7.48

Estado nativo: `INVENTORY_GENERATED`

Estado da source: `SOURCE_INVENTORY_GENERATED`

Este relatório cobre todas as entradas presentes em `functions.tsv`. Ele identifica o material disponível e cria uma fila de pesquisa; não transforma pseudocódigo exportado em compreensão ou contrato.

## Artefatos

- Corpus: `C:\Users\felipe\Tools\GhidraAnalysis\20260821\decompiled`
- Inventário TSV: `C:\Users\felipe\Documents\CLAUDE\SERVER 7.54\wyd-go\.agents\research\client748\inventory\functions.tsv`
- Binário de referência: `references/client748/WYD.exe`
- A confirmação de hash permanece obrigatória antes de usar endereços.

## Inventário reproduzível da source

- Arquivos: `source-files.tsv`
- Símbolos: `source-symbols.tsv`
- Escopo: `tmproject/TMProject748`, incluindo headers, testes e a fronteira `Dependencies/`
- Arquivos encontrados: **547** (**333** produto, **10** testes, **1** gerado e **203** terceiros)
- Linhas físicas classificadas: **445448**
- Símbolos indexados: **3977** (**2035** definições, **1831** declarações, **101** lambdas e **10** macros funcionais)
- Participação no projeto: **115** compilações e **212** headers em `Release|Win32`, além de **10** compilações no projeto de testes
- Arquivos não listados em projeto: **210**, dos quais **7** pertencem ao produto; os outros **203** são a fronteira de dependências
- Erros brutos do grammar: **37**; normalizações legadas com offsets preservados: **121**; erros restantes: **0**

O gerador fica em `.agents/skills/wyd-client748-catalog/scripts/source_inventory.py`. O gate reproduzível é `--check --fail-on-parse-errors`; as dependências Python estão fixadas no arquivo `requirements-source-inventory.txt`. As normalizações existem somente na visão entregue ao parser e não alteram a source, hashes, assinaturas ou offsets registrados. Os TSVs são fatos estruturais e não atribuem paridade, alcance, suporte ou estado de pesquisa.

### Sementes iniciais de entrada

Estas são fronteiras operacionais iniciais, não uma afirmação de cobertura completa do callgraph:

| Papel | Símbolo | Tipo | Local | `source_id` |
| --- | --- | --- | --- | --- |
| callback da janela principal | `WndProc` | `function` | `internal/app/scenes/NewApp.cpp:77` | `00b0bdb56f6afaa96e47` |
| entrada do processo Win32 | `wWinMain` | `function` | `internal/app/scenes/NewApp.cpp:1393` | `acbeb4d7c73897744a3d` |
| entrada da thread de guildmark | `Guildmark_Download` | `function` | `internal/app/scenes/TMFieldScene.cpp:31133` | `74281794d746fa3a5f15` |
| entrada da thread de guildmark | `Guildmark_Download` | `declaration` | `internal/app/scenes/TMFieldScene.h:1098` | `b60f31c05ac31d4077bb` |

## Contagem

- Funções no índice Ghidra: **4146**
- Funções citadas no handoff de paridade: **80**
- Funções citadas nas fichas formais: **331**
- Funções distintas documentadas: **343**
- Documentadas e presentes no índice: **343**
- Referências documentadas resolvidas como endereço interno: **4**
- Referências documentadas ainda não resolvidas: **0**
- Endereços `FUN_*` distintos citados na source C++/H: **78**
- Funções do catálogo ligadas à source C++/H: **78**
- Ocorrências `FUN_*` na source C++/H: **169**
- Referências da source resolvidas como endereço interno: **2**
- Referências da source ainda não resolvidas: **0**
- Funções no catálogo estrutural Ghidra: **4146**
- Linhas com callgraph direto Ghidra: **4146**
- Funções sem xref FLOW de entrada no catálogo: **2062**
- SHA-256 registrado pelo projeto Ghidra: **8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593**

### Classificação inicial do corpus

| Classe | Quantidade | Próxima ação |
| --- | ---: | --- |
| `PARITY_STATIC_EVIDENCE` | 12 | fechar callers/callees e fluxo no Ghidra |
| `FLOW_SHEET` | 263 | completar ficha a partir de `LOCATED` |
| `PARITY_AND_FLOW` | 68 | consolidar evidências |
| `CORPUS_ONLY` | 3803 | triagem por entrypoint/caller |
| `UNRESOLVED_REFERENCE` | 0 | resolver endereço diretamente no Ghidra |

## Limites da classificação

- `CORPUS_ONLY` significa somente que a função ainda não aparece nas fichas/handoff selecionados; não significa que seja código morto.
- Callers/callees são candidatos obtidos por tokens nos `.c`; chamadas indiretas, callbacks, vtables e destinos calculados continuam exigindo revisão no projeto Ghidra. O campo `ghidra_*` registra somente relações diretas reconhecidas pela análise.
- `functions.tsv` é o índice de decompilação; `ghidra-functions.tsv` é o catálogo estrutural versionado desta análise. Os dois devem conter exatamente as mesmas 4.146 entradas antes de serem combinados.
- `STATICALLY_EVIDENCED` não significa `TRACED`, `CONTRACT` ou `CLIENT_TESTED`.
- Uma função nativa pode ser wrapper, runtime, helper compartilhado ou código não alcançável no fluxo 7.48; nesses casos o resultado correto será documentar a decisão, não fabricar uma tradução para o TMProject.

## Critério para considerar compreendida

Uma função só sai da fila quando estiver vinculada a uma transição observável com entrada, callers/callees confirmados no Ghidra, estado, side effects, erros, teardown/relogin quando aplicável, equivalente na source e validação proporcional. Packet/ABI exige `CONTRACT`; execução no candidato exige `CLIENT_TESTED`.

Referências da source resolvidas dentro do corpo de uma função:

- `FUN_00452733` -> `FUN_00452728`
- `FUN_0047E4D6` -> `FUN_0047E49D`

Referências resolvidas dentro do corpo de uma função:

- `FUN_0044E1E8` -> `FUN_0044DF53`
- `FUN_004D9100` -> `FUN_004D90DD`
- `FUN_00506DF0` -> `FUN_00504A80`
- `FUN_00579210` -> `FUN_00579195`
