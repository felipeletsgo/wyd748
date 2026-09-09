---
id: login-session
title: Account login, character selection, and world entry
subsystem: login
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Account login, character selection, and world entry

## Pergunta

Como o client nativo WYD 7.48 autentica uma conta, recebe os quatro slots de
personagem, seleciona um slot e materializa o snapshot inicial do mundo sem
aceitar respostas truncadas, fora de ordem ou pertencentes a outra cena?

## Fronteira de evidência

- Executável/hash: `references/ghidra/input/WYD.exe`, SHA-256 do frontmatter;
  fonte `UTILIZADA` para identidade e comportamento observável.
- Projeto/corpus Ghidra: projeto 7.48 e exports autocontidos; funções
  `FUN_00484D44`, `FUN_004AC985`, `FUN_004A32DD`, `FUN_00435B13`,
  `FUN_0055890A`, `FUN_004B263E`, `FUN_00492E7D`, `FUN_004A626E`,
  `FUN_0042550E`, `FUN_00424C2C`, `FUN_00424DFE` e `FUN_00425266`;
  fonte `UTILIZADA` para callgraph, validação e wire.
- Assets 7.48: `NÃO APLICÁVEL` nesta unidade de contrato wire; nenhuma tela ou
  imagem é escolhida, criada ou alterada.
- Source Go atual: `internal/protocol` e `internal/login`; fonte `UTILIZADA`
  para ownership do frame decifrado, limites de transporte, layouts, parsers e
  máquina de estados.
- WYD-Go e testes: `internal/wire/source_client.go`, `score.go`, `opcodes.go`,
  `internal/game/handlers.go`, `security.go` e respectivos testes; fonte
  `UTILIZADA` para conferir a projeção server-authoritative.
- TMProject: snapshot em `references/tmproject/`; fonte `UTILIZADA` somente
  como comparação de nomes e estruturas depois da prova nativa, nunca como
  autoridade de ABI ou como código a copiar.
- W2PP, Secrets e Micronics: excluídos desta campanha por decisão do projeto;
  não orientam, validam ou contradizem este contrato.

## Fluxo nativo 7.48

### Entrada observável

- Evento: depois do handshake TCP, o jogador confirma conta e senha; após a
  lista, confirma um dos slots ocupados `0..3`.
- Precondições e estado inicial: transporte conectado, nenhum login concorrente
  e campos de credencial materializados em buffers fixos; a seleção exige a
  resposta de conta válida e um slot existente.
- Saída observável: o client envia `0x20D/116`, recebe `0x10A/2360`, envia
  `0x213/36` e recebe `0x114/2104`; a última resposta solicita a cena Field.

### Callers

- `FUN_004AC985` cobre o caminho inicial/alternativo que chega ao envio de
  autenticação.
- `FUN_00435B13` inicializa a cena e os controles usados para selecionar o
  personagem.
- `FUN_004A32DD` recebe a confirmação visual, valida o slot e inicia o envio
  de `0x213`.
- `FUN_0055890A` valida opcode e tamanho antes de qualquer handler de cena.
- `FUN_004B263E` despacha a resposta para a cena ativa; `FUN_00492E7D` é o
  receptor da Field nos caminhos que reenviam login ou já estão no mundo.

### Função principal

`FUN_00484D44` materializa o login `0x20D` com 116 bytes, versão 748 e o
layout fixo deste contrato. Na seleção, `FUN_004A32DD` monta `0x213` com 36
bytes, slot `int32` em `+12`, `Force` em `+16` e `SecretCode[16]` em `+20`.
Somente slots `0..3` são aceitos; após o envio, os controles são desabilitados
e um debounce de aproximadamente dois segundos evita repetição imediata.

### Callees

- `FUN_0042550E -> FUN_00424C2C -> FUN_00424DFE -> FUN_00425266` fecha o
  caminho de envio, checksum, cifra e socket já implementado em
  `internal/protocol`.
- `FUN_0055890A` rejeita tamanho incompatível antes do dispatcher.
- `FUN_004B263E` entrega respostas completas ao receptor da cena corrente.
- `FUN_004A626E` consome a confirmação `0x114` e solicita a cena Field.

### Saídas e erros

- Slot negativo, maior que três ou vazio não produz uma seleção válida.
- Opcode ou tamanho diferente do contrato é rejeitado antes de mutar a sessão.
- Uma segunda lista ou confirmação fora do estado esperado não substitui o
  snapshot ativo.
- Falha de autenticação permanece fora do estado autenticado e o servidor
  publica uma mensagem em inglês ou encerra a sessão conforme o erro.
- Queda parcial limpa a tentativa, os dados de seleção e o snapshot de mundo;
  uma nova conexão não herda `SecretCode`, `ClientID` ou personagem anterior.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| conectar | `Disconnected` | transporte/handshake | `Connecting` | cria sessão TCP | falha volta a `Disconnected` |
| enviar `0x20D` | socket pronto | `FUN_004AC985 -> FUN_00484D44` | `Authenticating` | credenciais são copiadas para packet próprio | falha de envio limpa a tentativa |
| receber `0x10A` | `Authenticating`; 2360 bytes | `FUN_0055890A -> FUN_004B263E` | `CharacterSelect` | materializa slots, cargo e prova da sessão | resposta inválida não altera estado |
| enviar `0x213` | `CharacterSelect`; slot ocupado | `FUN_004A32DD` | `EnteringWorld` | desabilita controles; arma debounce | slot/estado inválido não envia |
| receber `0x114` | `EnteringWorld`; 2104 bytes | dispatcher -> `FUN_004A626E` | `InWorld` | materializa snapshot e solicita Field | resposta inválida não altera estado |
| desconectar | qualquer estado | callback do transporte | `Disconnected` | limpa conta, seleção e mundo | fechamento idempotente |
| logout confirmado | `InWorld` | fluxo `0x215 -> 0x116` | `CharacterSelect` | descarta somente estado do mundo | sessão autenticada permanece |
| relogin de personagem | `CharacterSelect` | novo `0x213 -> 0x114` | `InWorld` | cria snapshot novo | não reutiliza ID/snapshot antigo |

### Vtables, vptrs e receptores

O receiver é sempre a cena ativa alcançada pelo dispatcher
`FUN_004B263E`. A inicialização da seleção passa por `FUN_00435B13`; a mudança
para Field termina em `FUN_004A626E`. O client Go não porta vptrs ou offsets de
objetos C++: preserva a transição observável em uma máquina de estados tipada.

### Ownership

O transporte possui os bytes recebidos até produzir um `protocol.Packet`, que
possui sua cópia decifrada. O pacote `login` interpreta primeiro em valores
temporários e só transfere o agregado completo para o estado da sessão depois
da validação. Senha e packet de autenticação não ficam armazenados no estado de
seleção. O controller apaga a senha fornecida e o packet temporário em todos os
caminhos depois que `Send` retorna. A cena de mundo recebe uma cópia do snapshot
aceito.

### Falha parcial

Nenhum parser publica resultado parcial. Se qualquer limite, opcode, tamanho,
slot ou campo obrigatório falhar, o agregado anterior permanece inalterado. Se
o envio de seleção falhar, o estado pode retornar à seleção; se o socket cair,
todo o estado da conta é descartado.

### Cleanup e teardown

Desconexão apaga `SecretCode`, lista, cargo, slot pendente e snapshot do mundo.
Logout de personagem apaga apenas slot pendente e snapshot do mundo, mantendo
a lista autenticada para uma nova seleção. O owner da sessão fecha o socket
antes das cenas conforme o contrato de lifecycle já implementado.

### Shutdown

O shutdown global chama o fechamento idempotente da sessão e depois desmonta
cenas e renderer. Não há goroutine ou buffer adicional pertencente ao pacote
`login` nesta unidade.

### Logout e relogin

O ciclo em uma sessão autenticada reutiliza a lista válida, mas nunca o
`ClientID`, a posição ou o `MOB` da entrada anterior. A transição completa por
`0x215/0x116` permanece detalhada em
`lifecycle/character-logout-selectchar-relogin.md`.

## Wire, ABI e recursos

Todos os inteiros são little-endian e começam depois do header `_MSG` de 12
bytes.

| Packet | Direção | Opcode | Tamanho | Layout comprovado |
| --- | --- | ---: | ---: | --- |
| Account Login | C->S | `0x20D` | 116 | conta `+12/16`, senha `+28/12`, versão `+40/u32=748`, `DBNeedSave +44/u32=0`, zero `+48/52`, adapter `+100/4*u32` |
| Character List | S->C | `0x10A` | 2360 | `SecretCode +12/16`, padding `+28/4`, `SelChar +32/1272`, cargo `+1304/128*8`, gold `+2328/u32`, conta `+2332/16`, SSN `+2348/+2352` |
| Character Login | C->S | `0x213` | 36 | slot `+12/i32`, Force `+16/i32`, `SecretCode +20/16` |
| Enter World | S->C | `0x114` | 2104 | posição `+12/+14`, `MOB +16/1224`, slot `+1240/u16`, ClientID `+1242/u16`, Weather `+1244/u16`, `ShortSkill +1246/16`, alinhamento `+1262/2`, Ext1 `+1264` |

`STRUCT_SELCHAR` contém `HomeTownX[4] +0`, `HomeTownY[4] +8`, nomes
`[4][16] +16`, quatro Scores de 140 bytes em `+80`, quatro conjuntos de 18
itens de oito bytes em `+640`, guilds em `+1216`, moedas em `+1224` e EXP
`uint64` em `+1240`. `Score` contém 35 palavras `uint32`; `Item` contém índice
`uint16` e seis bytes de efeitos. `STRUCT_MOB` tem 1224 bytes; o snapshot
inicial inclui nome, guild/classe, gold/EXP, scores base e atual, 18 equipamentos,
64 itens de carry, máscaras de skills e atalhos. Nenhum asset é tocado.

## Mapeamento atual

### Source recompilável

`internal/protocol` implementa handshake, framing, checksum, cifra,
fragmentação, escrita parcial e ownership. Sua goroutine leitora transfere
packets e desconexão por uma fila limitada; `internal/app.Application` drena
essa fila na thread principal antes de atualizar a cena. `internal/login` é o
único owner dos layouts acima e não é importado por `protocol`, evitando ciclo
e mistura entre transporte e gameplay. Seus builders, parsers, `SessionState`,
`Dispatcher` e `Controller` cobrem autenticação, seleção, entrada, logout,
desconexão, rollback de envio e relogin em valores independentes do buffer
recebido.

O `KeyWord` preserva o campo nativo de um byte. No fluxo histórico,
`FUN_00424DFE` usa o byte baixo de `_rand()` quando recebe zero; no client Go o
valor padrão vem de `crypto/rand`, com fonte injetável nos testes. Esta é uma
`MODERNIZACAO_COMPATIVEL`: não altera tamanho, posição, cifra nem interpretação
do servidor e evita compartilhar estado pseudoaleatório global.

### WYD-Go

`internal/game.onLogin` lê conta, senha e versão nos offsets comprovados;
autentica, recalcula estado autoritativo e envia `wire.CharList`. O handler de
`0x213` exige 36 bytes, interpreta slot como `int32`, materializa o personagem
e publica `wire.EnterWorld` antes dos demais packets de sincronização.

## Matriz de delta

| Claim | Nativo 7.48 | Source Go atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| opcodes/tamanhos | seis contratos comprovados no gate | `internal/login` materializa e valida os mesmos bytes | nomes/layouts coincidem como comparação | valida/emite os mesmos bytes | `PARIDADE_NATIVA` no wire |
| layouts | offsets, packing e signedness comprovados | representação Go tipada implementada e testada | referência secundária | serializers e guards existentes | manter um único owner Go dos layouts |
| estado | cenas bloqueiam repetição e seguem seleção -> Field | `SessionState` valida ordem, rollback e limpeza | arquitetura não foi copiada | servidor rejeita replay | `MODERNIZACAO_COMPATIVEL` interna |
| autoridade | client envia intenção e apresenta snapshot | preservada | pode conter regras client-side não confiáveis | estado autoritativo | não aceitar score/slot como verdade local |
| extensão | nenhuma | nenhuma necessária | N/A | nenhuma necessária | não criar opcode novo |

## Decisões

- Classificar wire e comportamento externo como `PARIDADE_NATIVA`.
- Classificar parser e máquina de estados Go como
  `MODERNIZACAO_COMPATIVEL`, preservando o contrato externo.
- Classificar a fila limitada e a drenagem na thread principal como
  `MODERNIZACAO_COMPATIVEL`; elas não mudam bytes, ordem ou direção do wire.
- Validar o packet inteiro antes de qualquer mutação observável.
- Manter `SecretCode` opaco e reproduzi-lo somente no `0x213` da mesma sessão.
- Não iniciar UI real até os quatro contratos passarem testes byte a byte.
- Não copiar classes, globals, vtables ou código do TMProject.

## Lacunas

- O controller dos envios `0x20D`, `0x213` e `0x215` ainda deve ser ligado às
  cenas Go; sua fronteira de transporte já está implementada e testada.
- As cenas Login, CharacterSelect, Loading e World ainda devem reagir às
  transições já publicadas na thread principal.
- Falhas de autenticação publicadas por outros opcodes terão contratos próprios
  no dispatcher.
- O restante da sequência pós-`0x114` pertence ao dispatcher/mundo mínimo.
- Teste contra o servidor e logout/relogin do executável Go ainda são
  necessários; nenhum deles é alegado nesta ficha.

## Validação

- Pesquisa: callgraph, opcodes, tamanhos, offsets, signedness do slot e
  lifecycle correlacionados no corpus nativo do hash registrado.
- Automação: `go test ./internal/login ./internal/protocol ./internal/app
  -count=1`, `go test -race` nos mesmos pacotes e `go test ./... -count=1`
  cobrem layouts, offsets, campos reservados, agregados, erro sem mutação,
  ordem de respostas, fila limitada, thread principal, desconexão, logout e
  relogin sem snapshot antigo.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
