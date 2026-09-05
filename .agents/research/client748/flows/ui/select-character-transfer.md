---
id: select-character-transfer
title: Resposta de transferencia e renomeacao na selecao de personagem
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-05
---

# Resposta de transferencia na selecao

## Pergunta

Quais contratos de `MSG_ReqTransper` devem ser preservados ao adaptar o
handler de selecao, incluindo erro, renomeacao e remocao do personagem local?

## Fronteira de evidência

- UTILIZADA: identidade do binario historico conferida por SHA-256 acima.
- UTILIZADA: projeto `WYD748Native_20260821`, aberto read-only/noanalysis;
  export `exports/select-character-transfer-focused.tsv`, com as raizes
  `004a626e`, `004a32dd`, `0055890a` e slot exato `005a44b8`.
- UTILIZADA: descompilacao dessas tres funcoes no corpus `20260821/decompiled`.
- UTILIZADA: source atual em `TMProject748/internal/app/scenes/TMSelectCharScene.cpp`
  e struct em `internal/core/Basedef.h`.
- UTILIZADA: busca no WYD-Go `internal/**/*.go`; nao localizou handler nomeado
  Transper nem literal `0xFAA`. Isto nao prova ausencia de dispatch numerico.
- UTILIZADA: `UI/SelCharScene2.bin`, SHA-256
  `3ED90543644E3AF96EEC99C16FDE36DCA21BE91632EFEACE31A92065BD6F1E3B`.
  Leitura pelos tamanhos de UIBinary.h atingiu EOF exato: 85 registros/4820
  bytes. Panel 1568 (offset 2556), botoes 628/629 (2600/2644), edit 1569
  (2744), todos filhos de 1568; botao 4613 (144), filho de 5654.
- NAO APLICAVEL: TMProject posterior e guias como prova nativa; a procedencia
  dos deltas atuais ainda precisa de comparacao. W2PP/Secrets/Micronics excluidos.

## Fluxo nativo 7.48

### Callers

O Ghidra confirma `005a44b8 -> FUN_004a626e`, slot `+4` da vtable de selecao
`005a44b4`. Ainda falta fechar o receptor do dispatch para este claim.

### Callees

`FUN_004a626e` chama `FUN_0049889a` (base) e `FUN_00403df2` (mensagem).
O ramo `0xFAA` usa metodos virtuais dos controles e o slot `+0x64` do
ObjectManager para remover o humano. Os receptores exigem revisao especifica.

`FUN_004a32dd` monta `0xFAA`, Result zero, slot selecionado e dois nomes;
chama `FUN_0042550e` com tamanho `0x34` e marca operacao em andamento.
`FUN_0055890a` rejeita `0xFAA` cujo tamanho difere de `0x34`.
Sua alcancabilidade no nativo permanece nao resolvida na ficha
`transport/packet-size-gate.md`: o par tamanho/opcode nao prova um gate vivo.

## Estado e lifecycle

O receptor zera a flag em `scene+0x26e1c` antes de rejeitar Slot negativo.
Result zero limpa hometown, guild, moeda, EXP, nome, equipamento e score do
slot, solicita exclusao do humano, zera seu ponteiro e seleciona slot -1.
Result um abre o painel de renomeacao. Demais resultados mostram erro.
Todas as mensagens desse ramo nativo duram `0xdac` (3500 ms).
O fluxo de cancelamento no emissor oculta o painel e zera a flag.
Timeout, foco, teardown, falha parcial e reentrada ainda nao estao fechados.

Coleta seguinte: `select-character-transfer-bindings.tsv` confirma initializer
`0049f0e7`, construtor `0049ee30` e os dez primeiros slots da vtable
`005a44b4` (10 lidos, zero erros). Initializer resolve 1568/1569 e oculta
1568 quando presente; construtor zera moving/time. Slot `+0x20` aponta a
`FUN_004a2aef`: seu corpo desabilita os quatro botoes enquanto moving=1,
mas nao implementa timeout de transferencia. Os timers de 2/3 segundos sao
debounces de criar/login, nao autorizacao para cancelar a transferencia.
Na source, `ObjectManager::DeleteObject(TreeNode*)` aceita nullptr e apenas
marca/remete a DeleteOwner quando ha objeto. Portanto o fallback de nome
nao introduz uma chamada insegura ao descartar um humano ja ausente.

## Wire, ABI e recursos

CONFIRMED no parser/emissor: opcode `0xFAA`, comprimento 52 bytes; Result
signed de 32 bits em +12, Slot signed de 32 bits em +16. Nomes no emissor
ocupam os offsets +20 e +36. A struct candidata possui os mesmos campos.
Ainda nao ha teste byte a byte dedicado nesta ficha.
O sucesso nativo habilita controle `0x1205` (4613); bindings materiais dos
demais controles precisam ser conferidos nos assets e initializer.

## Mapeamento atual

`TMSelectCharScene::OnPacketEvent` recebe `char*` sem comprimento do frame.
Antes deste corte, o caso `MSG_ReqTransper_Opcode` rejeitava apenas Slot
negativo e Result zero podia desreferenciar humano nulo ao formar o nome.
O corte local rejeita Slot >=4 e trata edit/humano/painel opcional; os emissores
e a confirmacao por mouse tambem validam o indice antes de acessar arrays.
O armazenamento do frame ainda deve ser validado antes do callback legado.

No corte seguinte, `ObjectManager::OnPacketView` passa a usar
`wire/ReceivedPacketDispatch.h` antes de `OnPacketEvent`. A mesma funcao
exercitada pelos testes exige para 0xFAA tamanho real/declarado 52 e Type
coerente com metadados. Nao toca estado da cena na rejeicao. O guard de Slot
continua na cena, depois do callback base; eventos locais continuam separados.
`CharacterTransferPacket.h` possui a definicao unica reexportada por Basedef,
preservando int signed e offsets 0/12/16/20/36 com asserts.

## Matriz de delta

| Claim | Nativo | Source atual | Decisao |
| --- | --- | --- | --- |
| Tamanho | 52 bytes | struct equivalente aparente | fixar teste apos fechar contrato |
| Slot | rejeita negativo | rejeita fora de [0,4) | endurecimento local, sem novo wire |
| Erros 2/3/default | 3500 ms | 2000 ms | preservar por enquanto; delta observavel |
| Result 4 | mensagem generica | mensagem 1131 | procedencia/utilidade pendente |
| Sucesso | limpa slot e habilita 4613 | tambem altera camera/paineis/selecao | fechar ownership e recursos |
| Result 1 | abre rename | tambem foca controle 1569 | verificar binding e lifecycle |

## Decisões

O claim de paridade integral permanece LOCATED. O corte de endurecimento
local e `MODERNIZACAO_COMPATIVEL`: recusar Slot fora de [0,4) antes de usar
arrays nos dois emissores e no receptor; nao cria opcode, layout, UI ou nova
transicao valida. Os arrays atuais e o emissor de login ja estabelecem esse
dominio. Preservar a limpeza de moving antes da rejeicao recebida.
Tolerar edit/painel opcional como ja faz o initializer: no sucesso sem edit,
usar nome do humano existente ou copia terminada dos 16 bytes do slot; manter
a limpeza do slot mesmo sem humano. Result 1 continua exibindo mensagem e so
abre rename quando ha painel. Nao criar controle nem alterar foco valido.
Essas defesas sao implementacao local, nao alegacao de seguranca nativa.
Preservar duracoes, mensagens e transicoes existentes ate decidir seus deltas.
O gate de frame e endurecimento local baseado no pacote de 52 bytes emitido
em 004A32DD, layout da source e parser 0055890A; nao afirma que este ultimo
seja chamado pelo nativo. Nao promove o lifecycle completo a CONTRACT.
Nao tratar transferencia como o fluxo `0x52A` de migracao
de servidor: possuem entradas e contratos diferentes.

## Lacunas

- Resolver slots virtuais de controle e teardown completo.
- Fechar cancelamento/reentrada; FrameMove nao fornece timeout de transferencia.
- Fechar origem server-side e lifecycle para promover paridade integral.
- Callbacks diretos char* fora de OnPacketView nao adquirem automaticamente
  a validacao de frame; manter eventos locais separados e revisar novos callers.
- Executar testes dinamicos do endurecimento de Slot e nulabilidade.
- Exercitar sucesso, resultados de erro, cancelamento e reentrada no candidato.

## Validação

Headless concluido sem SCRIPT ERROR; linha program confere hash nativo,
tres registros function e slot_outgoing exato presentes. Nenhuma promocao.
Release atual compilado/instalado via Build-Client.ps1, 118 checks PASS:
`89359107F67A84164D408930B50C5C78B55ADFA16087CC3B8318BB250F757FE1`.
Esses checks nao cobrem transferencia. Nao CLIENT-TESTED.

Depois do endurecimento: Debug e Release passaram por Build-Client.ps1 com
118 checks e instalacao/hash conferidos. Debug:
`77A58757BDCC628A426A326506C1E49164BF5E85357F669E0CE08954777FC105`.
Release instalado:
`08805042EA57594405431136EB00A9528ED16EC8B45C862487508313234186AD`.
STATICALLY VERIFIED para compilacao/inspecao dos guards; os 118 checks
existentes nao sao teste automatizado dos novos caminhos de UI.

Gate de frame subsequente: `ReceivedPacketDispatchTests.cpp` testa o mesmo
helper usado por ObjectManager com bytes conhecidos, armazenamento desalinhado,
todos os prefixos 0..51, frame nulo/excedente, Size e opcode divergentes,
entrega unica do frame exato, nao mutacao e fallback de outro opcode.
185 checks PASS em Debug e Release; asserts de layout PASS. Classificacao
AUTOMATED TESTED apenas para essa fronteira pura, nao lifecycle/UI/socket.
Debug instalado: `06F932BFB88371297B517ACEF95E2105D5793FB4A92DB4B896365CF99234FA75`.
Release instalado: `A256F702F9487AAD9E38DA2A0DA9EFBC57CB89C532CBC6B556A1ABFABE5DFC54`.
XML/paths do projeto e filtros conferidos; ficha continua LOCATED.
