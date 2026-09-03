---
id: character-stat-fields-update
title: Atualizacao dos campos de estado da janela Character
subsystem: ui-lifecycle
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Atualizacao dos campos de estado da janela Character

## Pergunta

Como o client nativo 7.48 vincula e atualiza Att Speed, C.POINT, o ornamento
HOLD e o emblema Kingdom na janela Character, inclusive durante atualizacoes
de Score/equipamento e depois de logout/relogin?

## Fronteira de evidência

- Executavel: `client748/wyd.exe nativo+patches/WYD.exe`, com o SHA-256 do
  frontmatter.
- Projeto Ghidra: `WYD748Native_20260821.gpr`.
- Raizes nativas: `FUN_00435B13` (construcao/binding da FieldScene) e
  `FUN_004431E4` (projecao do estado nos controles).
- Source atual:
  `client-source/tmproject/Projects/TMProject/TMFieldScene.cpp` e
  `ResourceControl.h`.
- Servidor atual: `internal/wire/codec.go`,
  `internal/wire/session_packets_test.go` e o pacote de entrada no mundo.
- Escopo: controles de UI existentes. Nenhum opcode, tamanho de packet,
  packing, persistencia ou regra autoritativa foi alterado.

## Fluxo nativo 7.48

### Entrada observável

Ao construir a FieldScene, `FUN_00435B13` resolve os controles do recurso
`FieldScene2.bin` e chama `FUN_004431E4` para publicar o primeiro snapshot do
personagem. Atualizacoes posteriores de Score, equipamento e skill voltam a
alcançar `FUN_004431E4`; abrir Character apenas torna visivel o root que ja
possui os valores mais recentes.

### Callers

O callgraph recuperado para `FUN_004431E4` inclui `FUN_00435B13`,
`FUN_004662C5` e `FUN_00489A3E`, alem dos caminhos adjacentes de atualizacao de
Score, equipamento e skill. Assim, o contrato nao depende somente do clique
que abre Character.

O export textual nao nomeia um caller direto de `FUN_00435B13`; a entrada foi
fechada no lifecycle de construcao da FieldScene. Ausencia de caller textual
nao foi tratada como prova de codigo morto.

### Funcoes principais e controles

| Controle | Tipo | Semantica nativa |
| --- | --- | --- |
| `1110` / `TMT_CI_DFBT` | texto | Att Speed em percentual |
| `1168` / `TMP_EXP_HOLD` | painel | ornamento `HOLD`, visivel somente quando o Hold/FakeExp reservado e positivo |
| `1376` / `TMT_CI_FAKEEXPPOINT` | texto | C.POINT no formato da mensagem 304 |
| `1377` / `TMP_KINGDOMFLAG` | painel | emblema Kingdom derivado do manto valido |

`FUN_004431E4` calcula a apresentacao de Att Speed com mastery, modificadores
de velocidade, DEX, bonus de classe, ability 26 e penalidade de freeze. Esse
valor pertence a `1110`; `1104` e outro campo do recurso e nao pode ser usado
como alias.

O texto C.POINT usa o Hold/FakeExp reservado e um decimo do intervalo do nivel
atual. O painel `1168` e uma decoracao separada: valor zero deve deixa-lo
oculto, sem apagar nem substituir o texto `1376`.

O emblema Kingdom usa `1377`. Ele permanece oculto quando nao existe objeto de
manto valido ou quando o skin representa a ausencia de reino (`Skin0 == 19`).
O grupo moderno `65768/65771..65774` nao substitui esse controle no recurso
7.48.

### Callees

Entre os callees confirmados de `FUN_004431E4` estao `FUN_00442FF7`,
`FUN_004431D9`, `FUN_004FA8A1`, `FUN_004FF388` e os helpers de resolucao,
texto, cor, textura e visibilidade dos controles. `FUN_004431D9` permanece o
no-op nativo ja documentado; ele nao fornece fallback para nenhum desses
campos.

### Saidas e erros

- Estado valido: os quatro controles refletem o mesmo snapshot local.
- Hold/FakeExp zero: `1168` fica oculto; `1376` continua recebendo seu texto.
- Sem manto/reino: `1377` fica oculto.
- Controle opcional ausente: a atualizacao desse campo e ignorada sem abortar
  os demais valores de Character.
- Painel moderno ausente no recurso 7.48: seus textos de descricao permanecem
  opcionais e nao podem causar dereference nulo.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondicao | Funcao/call | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- | --- |
| construcao da FieldScene | `FieldScene2.bin` carregado | `FUN_00435B13 -> FUN_004431E4` | bindings nativos ativos | `1168` e `1377` partem ocultos | controle ausente permanece opcional |
| atualizacao de Score/equip/skill | personagem local valido | callers -> `FUN_004431E4` | textos e visibilidade recalculados | somente UI local | um controle ausente nao bloqueia os demais |
| Hold/FakeExp `0` | snapshot valido | update de Character | `1168` oculto | `1376` atualizado | sem estado visual stale |
| personagem sem reino | manto nulo ou `Skin0 == 19` | update de Character | `1377` oculto | nenhuma descricao moderna exigida | sem dereference nulo |
| logout/relogin | FieldScene anterior destruida | teardown -> nova construcao | novos controles e valores | bindings antigos descartados | nenhum ponteiro/visible state e reutilizado |

### Vtables, vptrs e receptores

`FUN_00435B13` e o inicializador concreto do agregado FieldScene e
`FUN_004431E4` recebe o mesmo objeto para atualizar os filhos do container. O
delta recompilavel nao transporta vptrs, offsets de objeto ou enderecos do
executavel nativo.

### Ownership

O container da FieldScene possui os controles; `ObjectManager` possui
`m_nFakeExp` e o snapshot de `STRUCT_MOB`; o personagem local possui a
representacao do manto. A rotina de UI apenas le esses estados e altera
texto/cor/textura/visibilidade. Nao transfere ownership nem modifica Score.

### Falha parcial

Cada binding e validado separadamente. Em particular, o painel Kingdom pode
existir sem os textos modernos de tooltip; o emblema ainda e atualizado sem
acessar ponteiros nulos. HOLD e Kingdom sao ocultados ja na inicializacao para
que o estado default serializado no asset nao apareca antes do primeiro
snapshot.

### Cleanup e teardown

Os controles continuam filhos do container. O deleting destructor da
FieldScene converge em `FUN_004358DA`, que libera a arvore; esta adaptacao nao
cria callback, heap object ou recurso adicional.

### Shutdown

O shutdown reutiliza o teardown da cena/container. Nenhum estado global novo
foi introduzido e nenhum controle e acessado depois da destruicao.

### Logout e relogin

Ao sair do mundo, a FieldScene anterior e destruida. A nova entrada executa o
binding e a atualizacao inicial novamente, partindo com `1168` e `1377`
ocultos e reaplicando somente os estados recebidos para o novo personagem.

## Wire, ABI e recursos

O campo `MSG_UpdateEtc::Hold` continua sendo o reservado de EXP do protocolo
legado. `wire.UpdateEtc` escreve zero nesse offset e
`TestUpdateEtcDoesNotLeakChaosIntoHold` prova que CP/Chaos nao e serializado em
Hold. A entrada no mundo tambem deixa `Ext1.Data[0]` zerado. CP permanece no
byte legado de `CreateMob`, com conversao autoritativa feita no servidor.

Portanto, exibir `HOLD` em um personagem com Hold zero era defeito de binding
e visibilidade do client, nao ausencia de uma funcao server-side.

## Mapeamento atual

### Source recompilavel

- Att Speed agora escreve em `1110`.
- C.POINT resolve explicitamente `1376` e usa o intervalo do nivel atual.
- `1168` inicia oculto e so fica visivel com `m_nFakeExp > 0`.
- `1377` inicia oculto e so aparece com manto/reino valido.
- O binding posterior `65768` nao sobrescreve `1377` no modo 7.48.
- Painel e textos modernos opcionais sao protegidos contra ponteiro nulo.

### WYD-Go

Nenhuma mudanca foi necessaria. O servidor ja mantem Score/CP autoritativos e
envia Hold zero no pacote legado.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisao |
| --- | --- | --- | --- | --- |
| Att Speed | texto `1110` | escrevia em `1104` | fornece estado autoritativo | portar binding nativo |
| C.POINT | texto `1376` | binding/formula incompletos | CP nao ocupa Hold | restaurar texto nativo |
| HOLD | painel `1168`, condicional | default do asset podia permanecer visivel | envia Hold zero | ocultar por estado |
| Kingdom | painel `1377`, condicional | `65768` sobrescrevia o binding nativo | sem mudanca | preservar ID/lifecycle 7.48 |

## Decisões

- Classificar o lote como `PARIDADE_NATIVA`.
- Preservar o Score e CP autoritativos do servidor; nao inventar um novo
  packet para corrigir um estado visual.
- Manter os controles modernos somente fora do modo compat 7.48 e sempre como
  opcionais.
- Recalcular visibilidade em toda atualizacao e reconstruir os bindings no
  relogin, em vez de depender do default do asset.

## Lacunas

- Abrir Character no `client748/project.exe` e confirmar Att Speed em `1110`.
- Confirmar que Kingdom e HOLD permanecem ocultos para personagem sem reino e
  Hold zero, inclusive apos atualizacao de Score/equipamento.
- Confirmar C.POINT nos dois valores do formato nativo.
- Repetir depois de logout/relogin. Ate isso, nao promover para
  `CLIENT_TESTED`.

## Validação

- Pesquisa: `FUN_00435B13`, `FUN_004431E4`, callers/callees, IDs
  `1110/1168/1376/1377`, ownership, teardown e relogin fechados para este
  delta.
- Servidor: `UpdateEtc` e seu teste impedem vazamento de CP/Chaos em Hold.
- Client source: `IMPLEMENTED / STATICALLY VERIFIED`; `Build-Client.ps1`
  concluiu `Release|Win32` com zero erros e 13 warnings C4018 preexistentes e
  instalou `client748/project.exe` com SHA-256
  `B51D48ACF691B84A6B577DBB07E4981CED6F54DFB2616567C907863A9B9AE6BC`.
- Automação: `go test -count=1 ./internal/wire`, `validate_research.py` e
  `git diff --check` passaram para o lote.
- Client real: nao executado; `CLIENT_TESTED` nao e alegado.
