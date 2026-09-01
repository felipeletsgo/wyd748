---
id: ui-focus-ime-lifecycle
title: Construção de cena, foco de controle e composição IME 7.48
subsystem: ui-lifecycle
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Construção de cena, foco de controle e composição IME 7.48

## Pergunta

Como o client nativo 7.48 constrói o container de controles, encaminha foco,
teclas e composição IME, e desmonta a árvore de controles sem depender de
widgets herdados da topologia 7.59+?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 acima.
- Projeto Ghidra: `WYD748Native_20260821.gpr`.
- Corpus auxiliar:
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Exports auxiliares: `.agents/research/client748/exports/ui-focus-ime.tsv`.
- Source atual: `EventTranslator.cpp`, `NewApp.cpp`, `TMScene.cpp`,
  `TMFieldScene.cpp` e `SControlContainer.cpp` em
  `client-source/tmproject/Projects/TMProject/`.
- Servidor: não há regra de gameplay neste fluxo; intenções originadas em
  controles continuam sujeitas à autoridade do WYD-Go.

Os exports são aceleradores de busca. Eles não substituem xrefs, tipos,
chamadas indiretas ou lifecycle conferidos no projeto Ghidra. O dispatcher,
os receptores virtuais, os no-ops deliberados e o teardown desta transição
foram reabertos no projeto/corpus; por isso a ficha avança a `TRACED`.

## Fluxo nativo 7.48

### Entrada observável

`CONFIRMED`: a entrada de janela passa por
`FUN_0055FA89 -> FUN_0055DAB8`, sendo `FUN_0055DAB8` o dispatcher principal de
mensagens. O dispatch observado separa os eventos Win32 abaixo:

| Evento | Função nativa localizada |
| --- | --- |
| `WM_KEYDOWN` | `FUN_004AF484` |
| `WM_KEYUP` | `FUN_004AF4CC` |
| `WM_CHAR` | `FUN_004AF514` |
| `WM_IME_COMPOSITION` | `FUN_004AF550(wParam & 0xFF, lParam)` |
| `WM_IME_ENDCOMPOSITION` | `FUN_004AF5F6(lParam, false)` |
| `WM_IME_NOTIFY`, abrir/mudar candidato | `FUN_004AF5EB()` e `FUN_004AF5F6(lParam, true)` |
| `WM_IME_NOTIFY`, fechar candidato | `FUN_004AF5F6(lParam, false)` |

`FUN_0040C544` é a transição de troca de foco: lê o controle atual em
`container + 0x12C`, notifica o controle antigo e o novo pela virtual `+0x68`,
alterna o modo IME para controles tipo `0x0D` e chama `FUN_004AF545`.

### Callers

`CONFIRMED`: `FUN_0055FA89` encaminha a janela para `FUN_0055DAB8`. Esse
dispatcher é o caller observável único das quatro entradas IME deste recorte.
`FUN_004AF550` propaga a composição pela virtual `+0x20` do ObjectManager; na
árvore da cena, ela chega ao container focado e à virtual correspondente do
controle. O receptor editável concreto é `FUN_00406F5E`.

O construtor `FUN_00493E70`, a troca de foco `FUN_0040C544` e os roots de
desmontagem `FUN_00431D00`, `FUN_004343A4`, `FUN_0049EE30` e `FUN_004A8CCF`
foram conferidos para delimitar construção, troca de cena e destruição. Não há
timer, packet ou worker mantendo referência ao controle candidato.

### Função principal

`FUN_00493E70` constrói o estado de UI associado à cena: aloca um container de
`0x154` bytes em `scene + 0x28`, aloca o controle de candidatos IME de `0xE4C`
bytes em `scene + 0x88` e anexa os controles à árvore.

`FUN_004AF550` trata a composição em andamento. Obtém o contexto com
`ImmGetContext`; quando `lParam & GCS_COMPSTR` (`0x8`) lê a string em um buffer
de até `0x200` bytes, caso contrário produz string vazia. Depois garante NUL,
libera o contexto e encaminha o texto pela virtual `+0x20` do ObjectManager em
`DAT_013B71E8`. Ausência de contexto retorna sem mutação.

`FUN_00406F5E`, receptor `SEditableText::OnIMEEvent`, aceita a composição
somente quando o controle está focado, copia até `0x100` bytes para seu buffer,
chama `Update()` e retorna `1`; sem foco delega à implementação base.

`FUN_004AF5F6` trata a lista de candidatos. Usa o controle em `scene + 0xA58`,
consulta `ImmGetCandidateListA`, realoca o armazenamento conforme necessário,
concatena candidatos no formato equivalente a `"%d:%s "` e atualiza a UI pelas
virtuais `+0x80` e `+0x60`. O controle nativo contém a página agregada, não um
controle separado por candidato.

### Callees

- `ImmGetContext`, `ImmGetCandidateListA` e as operações de contexto IME do
  Win32: recursos externos consumidos pelo fluxo.
- `FUN_004AF545`: callback chamado após troca de foco; é deliberadamente vazio
  no 7.48.
- `FUN_004AF5EB`: callback executado antes de abrir/mudar candidatos; também é
  deliberadamente vazio.
- `FUN_00406F5E`: receptor concreto da composição no `SEditableText` focado.
- `FUN_0054AC09`: define o parent e insere um filho na lista intrusiva.
- `FUN_0054AA45`: destrói filhos e remove o nó da lista.
- `FUN_00494C00`: desmonta a cena e converge com os destrutores.

As chamadas indiretas relevantes foram resolvidas pela cadeia
ObjectManager -> cena -> container -> controle focado. As funções base retornam
`0`; `FUN_00406F5E` retorna `1` quando consome a composição.

### Saídas e erros

`CONFIRMED`: a composição sai como texto para o objeto de cena global; a lista
de candidatos altera o controle candidato e sua apresentação. O foco notifica
o controle antigo e o novo. O contexto IME é liberado após cada consulta.

- sem contexto: composição/candidatos retornam silenciosamente;
- sem cena ou controle candidato: o fluxo de apresentação não age;
- lista vazia: o armazenamento é zerado e o controle pode ser ocultado;
- falha de alocação: a adaptação preserva a lista anterior e retorna, em vez de
  entregar ponteiro nulo ao Win32;
- comprimento de composição negativo e offsets de candidato fora do bloco: a
  adaptação descarta o dado inválido sem ler ou escrever fora do buffer.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| foco entra em editável | controle ligado à árvore | `FUN_0040C544` e virtual `+0x68` | editável focado, IME habilitado | notifica controles antigo/novo | controle ausente não recebe evento |
| `WM_IME_COMPOSITION` | dispatcher e contexto IME ativos | `FUN_0055DAB8 -> FUN_004AF550 -> FUN_00406F5E` | composição copiada no editável focado | `Update()` refaz o desenho | sem contexto retorna; sem foco delega à base |
| candidato abre ou muda | cena e controle candidato presentes | `FUN_004AF5EB -> FUN_004AF5F6(..., true)` | página agregada visível | consulta e substitui `CANDIDATELIST` | falha preserva lista anterior |
| candidato fecha ou composição termina | tradutor ativo | `FUN_004AF5F6(..., false)` | candidatos/composição ocultos | não conserva receptor de UI | controles opcionais são ignorados |
| troca de cena/logout | cena antiga marcada para exclusão | destructors e `FUN_00494C00` | árvore e foco antigos destruídos | libera filhos e estado modal | nenhum callback assíncrono permanece |
| relogin | nova cena construída | `FUN_00493E70` e lookup no evento | novos controles recebem input | tradutor consulta a cena corrente | estado anterior não é reutilizado |

### Vtables, vptrs e receptores

`FUN_0055DAB8` é o receptor Win32. A composição atravessa a virtual `+0x20` do
ObjectManager, a árvore da cena e o container focado até
`FUN_00406F5E`, receptor concreto de `SEditableText`. A lista candidata usa as
virtuais `+0x80` para texto e `+0x60` para visibilidade do controle nativo.
Na source, essas relações usam os tipos C++ existentes; offsets nativos não são
transportados.

### Ownership

A cena possui o container e seus controles. O container possui o foco como
referência não proprietária para um filho da própria árvore. `EventTranslator`
possui somente o contexto IME e a `CANDIDATELIST`; não possui cena nem controle
e consulta `g_pCurrentScene` a cada evento.

### Falha parcial

Ausência de contexto, cena ou controle produz retorno sem mutação. A source
valida tamanho, quantidade, página, tabela de offsets e terminação NUL antes de
publicar uma lista nova. Falha de alocação ou bloco inválido preserva a lista
anterior; composição inválida resulta em string vazia limitada ao buffer.

### Cleanup e teardown

Ao fechar candidatos, todos os controles candidatos da source são ocultos.
`SControlContainer` desmarca foco e zera referências de pick/modal antes de
destruir root e filhos. `FinalizeIME()` libera a `CANDIDATELIST`, reassocia o
contexto anterior e destrói o contexto criado pelo tradutor.

### Shutdown

No encerramento global, a cena desmonta a árvore antes da finalização do
ObjectManager e do `EventTranslator`. Não há timer, worker ou packet que possa
reentrar no controle após a destruição; os no-ops `FUN_004AF545` e
`FUN_004AF5EB` também não registram callbacks.

### Logout e relogin

Logout e troca de cena destroem container, foco e candidatos da cena anterior.
No relogin, a nova cena materializa outra árvore; como o tradutor sobrevivente
resolve `g_pCurrentScene` somente no momento do evento, não reutiliza ponteiros
da sessão anterior.

`CONFIRMED`: a árvore usa uma lista intrusiva com os campos observados:

```text
+0x04 parent
+0x08 sibling anterior
+0x0C próximo sibling
+0x10 primeiro filho
```

`FUN_0054AC09` liga parent/filho e insere o nó; `FUN_0054AA45` destrói filhos e
remove o nó; `FUN_00494C00` desmonta a cena. O foco atual fica no container em
`+0x12C`. O controle candidato fica em `scene + 0xA58`, enquanto a composição
visual aparece em `scene + 0x454`.

`CONFIRMED` na source: `TMScene::~TMScene()` destrói seu
`SControlContainer` antes de `ObjectManager::CleanUp()`. O container limpa o
estado focado antes de destruir root/filhos; `EventTranslator` é global, mas
não retém ponteiro de cena ou controle, e `FinalizeIME()` libera apenas sua
`CANDIDATELIST` e o contexto Win32.

Em troca de cena/logout, a cena antiga é marcada para exclusão e sua árvore é
destruída. No relogin, uma nova cena constrói novo container e novos controles;
o tradutor sobrevivente consulta sempre `g_pCurrentScene` no momento do evento.
Assim, nenhum foco ou candidato da cena anterior é reutilizado. Detach e
reparenting genéricos da árvore não fazem parte desta transição observável.

## Wire, ABI e recursos

Este fluxo não define um packet de gameplay nem autoriza transportar ABI para o
servidor. Os valores de tamanho e offsets acima são layout nativo de objetos
Win32/UI no executável 7.48, não structs wire da source C++.

O recurso materializado pelo fluxo é a árvore de controles da cena e o controle
de candidatos IME. Não foi provado, nesta ficha, um catálogo completo de IDs de
recurso equivalente ao layout 7.59. Um campo existente apenas na source nova é
opcional até que seu ID e sua criação sejam confirmados no recurso 7.48.

## Mapeamento atual

### Source recompilável

- `NewApp.cpp` encaminha `WM_IME_COMPOSITION` sem truncar `lParam`, trata
  abrir/mudar/fechar candidatos e encerra a composição com guardas de cena e
  controles opcionais.
- `EventTranslator.cpp` lê `GCS_COMPSTR`, propaga a composição e materializa a
  página agregada no primeiro dos dez controles candidatos mantidos pela
  source. Os nove controles restantes ficam ocultos.
- `SEditableText::OnIMEEvent` conserva a regra nativa: somente o editável
  focado copia a composição e atualiza seu desenho.
- O destrutor de `SControlContainer` desmarca e solta foco/pick/modais antes de
  destruir a árvore.
- `TMScene.cpp`, `TMFieldScene.cpp` e `SControlContainer.cpp` constroem,
  localizam, notificam e destroem controles.
- A source contém acessos a controles de gerações posteriores; eles não são
  evidência de que o controle existe no recurso 7.48 e devem ser protegidos ou
  removidos quando o fluxo nativo não os materializar.

### WYD-Go

Não há equivalente server-side direto para o lifecycle local de UI/IME. O Go
recebe apenas as intenções resultantes de controles e continua validando
permissão, alvo, distância, estado e persistência. Nenhum ponteiro de UI deve
ser usado como autoridade de gameplay.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Entrada de teclado e IME | dispatcher e funções `FUN_004AF4xx`/`FUN_004AF5xx` | `NewApp`/`EventTranslator` | truncava `lParam` e ignorava notify | não aplicável | restaurar dispatch nativo |
| Root de candidatos | um controle com página agregada | dez controles já materializados | estrutura posterior compatível | não aplicável | usar `[0]` e ocultar `[1..9]` |
| Foco/composição | virtual até `FUN_00406F5E` | container/`SEditableText` | receptor era no-op | não aplicável | copiar composição apenas com foco |
| Árvore | parent/siblings/primeiro filho em `+0x04..+0x10` | `TMScene`/container | sem autoridade sobre layout 7.48 | não aplicável | não portar offsets cegamente |
| Fechamento | remove/destrói filhos e desmonta cena | destrutores e `TMScene` | pode conservar branches 7.59 | estado server-side separado | fechar todos os roots e limpar foco |

## Decisões

- Promover a ficha a `TRACED`: entrada observável, callers/callees, receptor,
  estados de foco, erros, teardown e relogin estão resolvidos para esta
  transição. Não há wire/ABI para justificar `CONTRACT`.
- Classificar dispatcher, `GCS_COMPSTR`, receptor editável e apresentação da
  página candidata como `PARIDADE_NATIVA`.
- Classificar validação de ponteiros, comprimentos, alocação e offsets como
  `MODERNIZACAO_COMPATIVEL`, pois preserva a saída válida e torna falhas
  silenciosas determinísticas.
- Não criar widget, ID ou branch 7.59 para satisfazer ponteiro nulo. Primeiro
  confirmar se o root existe no recurso 7.48; se não existir, o ponteiro é
  opcional e a transição principal deve continuar.
- Não tratar `ui-focus-ime.tsv` como prova independente. Reabrir a função e os
  xrefs no projeto Ghidra sempre que uma mudança depender deste fluxo.
- Preservar `FUN_004AF545`/`UpdateCompositionPos` e
  `FUN_004AF5EB`/`OnIME2` como no-ops deliberados, sem inventar UI moderna.

## Lacunas

- testar no `client748/project.exe` composição real, troca de foco,
  abrir/mudar/fechar candidatos e ausência de contexto;
- testar troca de cena, logout e relogin durante/após composição;
- confirmar visualmente clipping da página agregada em resoluções suportadas.

## Validação

- Pesquisa: dispatcher `FUN_0055DAB8`, handlers `FUN_004AF545`,
  `FUN_004AF550`, `FUN_004AF5EB`, `FUN_004AF5F6`, receptor `FUN_00406F5E`,
  construção, foco e teardown conferidos no projeto/corpus do hash registrado.
- Adaptação: implementada na source em 2026-09-01. O build oficial Release
  Win32 v145 passou com zero erros e dois warnings C4305/C4309 preexistentes,
  instalando `client748/project.exe` com SHA-256
  `1DF5956AC134BCAEB5C072E84B77EF9BBDFF6EDE30DAC8ACBE8616375CED6082`.
- Estado de entrega: `STATICALLY VERIFIED`; o build não promove a ficha a
  `CLIENT_TESTED`.
- Client real: não executado. Nenhuma alegação `CLIENT_TESTED` é permitida para
  este fluxo.
