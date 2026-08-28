---
id: ui-focus-ime-lifecycle
title: Construção de cena, foco de controle e composição IME 7.48
subsystem: ui-lifecycle
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-28
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
chamadas indiretas ou lifecycle conferidos no projeto Ghidra. Por isso esta
ficha registra o fluxo localizado, mas permanece `LOCATED`.

## Fluxo nativo 7.48

### Entrada

`CONFIRMED`: a entrada de janela passa por
`FUN_0055FA89 -> FUN_0055DAB8`, sendo `FUN_0055DAB8` o dispatcher principal de
mensagens. O dispatch observado separa os eventos Win32 abaixo:

| Evento | Função nativa localizada |
| --- | --- |
| `WM_KEYDOWN` | `FUN_004AF484` |
| `WM_KEYUP` | `FUN_004AF4CC` |
| `WM_CHAR` | `FUN_004AF514` |
| `WM_IME_COMPOSITION` | `FUN_004AF550` |
| `WM_IME_NOTIFY` | `FUN_004AF5F6` |

`FUN_0040C544` é a transição de troca de foco: lê o controle atual em
`container + 0x12C`, notifica o controle antigo e o novo pela virtual `+0x68`,
alterna o modo IME para controles tipo `0x0D` e chama `FUN_004AF545`.

### Callers

`LOCATED`: o dispatcher de mensagens, o construtor de cena, a transição de
foco e os caminhos de teardown foram localizados no projeto/corpus, mas a
cadeia completa de xrefs para todos os callers indiretos ainda não foi fechada.
Os pontos de entrada observados incluem `FUN_0055DAB8`, `FUN_00493E70`,
`FUN_0040C544` e os callers de desmontagem `FUN_00431D00`, `FUN_004343A4`,
`FUN_0049EE30` e `FUN_004A8CCF`. Não promover a ficha a `TRACED` enquanto a
reabertura desses xrefs no projeto Ghidra não confirmar a cadeia completa.

### Função principal

`FUN_00493E70` constrói o estado de UI associado à cena: aloca um container de
`0x154` bytes em `scene + 0x28`, aloca o controle de candidatos IME de `0xE4C`
bytes em `scene + 0x88` e anexa os controles à árvore.

`FUN_004AF550` trata a composição final. Obtém o contexto com
`ImmGetContext`, limpa a composição em `scene + 0x454`, lê `GCS_RESULTSTR` em
um buffer de até `0x200` bytes, garante terminação NUL, libera o contexto e
encaminha o texto pela virtual `+0x20` do objeto global em `DAT_013B71E8`.

`FUN_004AF5F6` trata a lista de candidatos. Usa o controle em `scene + 0xA58`,
consulta `ImmGetCandidateListA`, realoca o armazenamento conforme necessário,
concatena candidatos no formato equivalente a `"%d:%s "` e atualiza a UI pelas
virtuais `+0x80` e `+0x60`.

### Callees

- `ImmGetContext`, `ImmGetCandidateListA` e as operações de contexto IME do
  Win32: recursos externos consumidos pelo fluxo.
- `FUN_004AF545`: sincroniza o modo/estado IME após uma troca de foco.
- `FUN_0054AC09`: define o parent e insere um filho na lista intrusiva.
- `FUN_0054AA45`: destrói filhos e remove o nó da lista.
- `FUN_00494C00`: desmonta a cena e converge com os destrutores.

Os efeitos foram localizados por pseudocódigo e exports focados. A semântica
completa dos callees e todos os callers indiretos ainda precisa ser reaberta no
projeto Ghidra antes de usar este documento como contrato de implementação.

### Saídas e erros

`CONFIRMED`: a composição final sai como texto para o objeto de cena global; a
lista de candidatos altera o controle candidato e sua apresentação. O foco
notifica o controle antigo e o novo. O fluxo usa buffers limitados e libera o
contexto IME.

`UNRESOLVED`: comportamento exato de falha de alocação, ausência de contexto
IME, retorno das virtuais e propagação de erro pelo dispatcher. Também não foi
provado se todo caminho de fechamento desativa a entrada antes da destruição.

## Estado e lifecycle

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

`PROBABLE`: a source recompilável pretende representar a mesma hierarquia por
`SControlContainer`/`TMScene`, mas a correspondência campo a campo e a ordem de
teardown ainda não foram fechadas para todos os caminhos de relogin, troca de
cena e fechamento de modal.

Lacunas específicas de lifecycle: detach sem destruição, reparenting,
parent/child nulo, prevenção de ciclos, logout/relogin completo, desativação de
input antes da destruição e todos os caminhos de alocação parcial.

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

- `NewApp.cpp` recebe `WM_KEYDOWN`, `WM_KEYUP`, `WM_CHAR`,
  `WM_IME_COMPOSITION` e `WM_IME_NOTIFY`, encaminhando eventos para
  `EventTranslator`.
- `EventTranslator.cpp` possui inicialização/finalização IME, foco de entrada,
  composição e lista de candidatos.
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
| Entrada de teclado e IME | dispatcher e funções `FUN_004AF4xx`/`FUN_004AF5xx` | `NewApp`/`EventTranslator` | referência semântica posterior | não aplicável ao lifecycle | reabrir xrefs antes de alterar |
| Root de candidatos | `scene + 0xA58`, criado no fluxo de `FUN_00493E70` | campo correspondente na cena | pode conter controles modernos extras | não aplicável | só usar se recurso 7.48 materializar |
| Foco | `container + 0x12C`, notificações em virtual `+0x68` | `SControlContainer`/controle focado | não decide ABI | não aplicável | confirmar campo e lifecycle antes de editar |
| Árvore | parent/siblings/primeiro filho em `+0x04..+0x10` | `TMScene`/container | sem autoridade sobre layout 7.48 | não aplicável | não portar offsets cegamente |
| Fechamento | remove/destrói filhos e desmonta cena | destrutores e `TMScene` | pode conservar branches 7.59 | estado server-side separado | fechar todos os roots e limpar foco |

## Decisões

- Manter a ficha em `LOCATED`: a entrada e os principais símbolos foram
  encontrados, mas callers/callees e teardown completo ainda não estão
  confirmados em nível suficiente para implementação comportamental.
- Não criar widget, ID ou branch 7.59 para satisfazer ponteiro nulo. Primeiro
  confirmar se o root existe no recurso 7.48; se não existir, o ponteiro é
  opcional e a transição principal deve continuar.
- Não tratar `ui-focus-ime.tsv` como prova independente. Reabrir a função e os
  xrefs no projeto Ghidra sempre que uma mudança depender deste fluxo.
- Não alterar `EventTranslator`, `NewApp`, `TMScene` ou controles nesta etapa;
  esta entrega é documental e não mudou código ativo do client.

## Lacunas

- xrefs diretos/indiretos completos de `FUN_0055FA89`, `FUN_0055DAB8`,
  `FUN_00493E70`, `FUN_0040C544`, `FUN_0054AC09`, `FUN_0054AA45` e
  `FUN_00494C00`;
- retorno e erro das virtuais de foco, composição e candidatos;
- detach/reparenting, ciclos e alocação parcial;
- teardown na troca de cena, logout e relogin;
- catálogo de IDs/recursos 7.48 e correspondência campo a campo com a source;
- cenário real no `client748/project.exe`, inclusive digitação, backspace,
  composição IME, foco, X/Esc e reabertura.

## Validação

- Pesquisa: hash do executável e pseudocódigo/export focado conferidos em
  2026-08-27.
- Automação: `query_corpus.py stats --repo .` encontrou 4.146 funções e 108
  referências nativas citadas no repositório; duas referências
  (`FUN_00452733` e `FUN_0047E4D6`) continuam ausentes do índice textual.
  `validate_research.py --repo .` aprovou a estrutura desta ficha em
  2026-08-28, sem promover sua maturidade além de `LOCATED`.
- Client real: não executado. Nenhuma alegação `CLIENT_TESTED` é permitida para
  este fluxo.
