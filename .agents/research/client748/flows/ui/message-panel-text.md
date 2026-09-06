---
id: message-panel-text
title: Aviso textual da cena MessagePanel 0x101
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-05
---

# Aviso textual da cena MessagePanel 0x101

## Pergunta

Qual envelope o client 7.48 aceita para o aviso textual `0x101` e qual limite
de texto a source recompilavel deve preservar antes de entregar o frame a
`TMScene::OnPacketEvent`?

## Fronteira de evidência

- `UTILIZADA` — executavel historico
  `client748/wyd.exe nativo+patches/WYD.exe`, identificado pelo SHA-256 do
  frontmatter e mantido somente como referencia.
- `UTILIZADA` — projeto/corpus Ghidra canonico, em especial o gate
  `FUN_0055890A @ 0x0055890A` e o receptor
  `FUN_0049889A @ 0x0049889A`.
- `UTILIZADA` — source atual: `TMScene::OnPacketEvent`, `Basedef.h`,
  `MessagePanelPacket.h` e `ReceivedPacketDispatch.h`.
- `UTILIZADA` — WYD-Go: `internal/wire.MessagePanel`, `OpMessagePanel` e os
  testes de `MessagePanel`/`DaySync`.
- `NAO APLICAVEL` — assets: este corte nao altera recurso, geometria, fonte ou
  binding do painel existente.
- `NAO APLICAVEL` — W2PP, Secrets e Micronics: fontes excluidas desta campanha.

## Fluxo nativo 7.48

### Entrada observavel

O transporte recebe um frame `S->C`. `FUN_0055890A` aceita o opcode `0x101`
somente quando o comprimento do frame e exatamente `0x6C` (108 bytes). Na
familia tratada pela cena base, `FUN_0049889A` exige `Header.ID == 0`.

### Callers

O callgraph focado ja registrado para o receptor da cena confirma quatro
callers diretos de `FUN_0049889A`: `FUN_004339D8`, `FUN_00492E7D`,
`FUN_004A626E` e `FUN_004AE3D6`. A entrada virtual em `0x005A4440` encaminha
as cenas compativeis ao mesmo receptor.

### Funcao principal

Para `ID=0` e `Type=0x101`, `FUN_0049889A` interpreta o corpo textual a partir
de `+0x0C`. Antes do uso, zera os bytes absolutos `+0x6A` e `+0x6B`, os dois
ultimos bytes do corpo de 96 bytes, impedindo que o texto ultrapasse o frame.
Os branches `0x102/0x104` permanecem no contrato opaco separado e nao ampliam
este claim.

### Callees

O branch textual entrega a mensagem a apresentacao pertencente a cena ativa.
O corte atual nao altera os callees de UI, a interpretacao de prefixos nem a
duracao do painel; apenas fecha o envelope antes do callback legado.

### Saidas e erros

- frame com 108 bytes, `Type=0x101` e metadado coerente: e entregue uma vez ao
  receptor existente;
- comprimento real/declarado divergente, frame nulo ou `Type` divergente: e
  recusado antes da cena;
- `Header.ID != 0`: fica fora da familia textual da cena base;
- nao existe ACK ou mutacao de gameplay associada ao aviso.

## Estado e lifecycle

### Matriz de transicoes

| Evento | Precondicao | Funcao | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- | --- |
| receber aviso | `0x101`, 108 bytes, `ID=0` | gate -> `FUN_0049889A` | painel recebe texto | apresentacao local | frame invalido nao chega ao receptor |
| prefixo especial | frame valido | parser atual de `TMScene` | depende da cena/prefixo | preservado por este corte | sem regra nova |
| troca de cena | painel pertence a cena | lifecycle existente | painel antigo destruido | nenhum frame retido | N/A |

### Vtables, vptrs e receptores

A referencia virtual `0x005A4440` liga o handler ao receptor da cena. O gate
novo nao escolhe painel nem entidade; ele entrega o mesmo `PacketView` ao
ObjectManager somente depois de validar o envelope.

### Ownership

O transporte empresta o buffer durante o callback. O gate le uma copia local
do header e nao retem, libera ou modifica o payload. O receptor existente
continua responsavel por terminar o texto antes de interpreta-lo.

### Falha parcial

A rejeicao acontece antes do callback e nao cria estado. Nao ha rollback de
gameplay, persistencia ou UI.

### Cleanup e teardown

O contrato nao cria objeto, timer, controle ou inscricao. O painel segue o
cleanup normal da cena.

### Shutdown

Nao ha estado novo a destruir nem mudanca na ordem de shutdown.

### Logout e relogin

O frame nao e persistido. Logout elimina a cena atual e relogin materializa o
painel pelo lifecycle existente; nenhum texto deste contrato sobrevive.

## Wire, ABI e recursos

Contrato nativo `S->C`, little-endian:

| Offset absoluto | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`, `Size=108`, `Type=0x101`, `ID=0` para a cena base |
| `0x0C` | 96 | `char String[96]` |
| total | 108 | frame canonico |

O campo e uma sequencia de bytes textual; signedness nao altera o wire. Nao ha
packing adicional entre o header e `String`. Este corte nao altera os recursos
visuais do `SMessagePanel`.

## Mapeamento atual

### Source recompilavel

`internal/wire/MessagePanelPacket.h` concentra opcode, estrutura e asserts de
tamanho/offset. `Basedef.h` reexporta esse contrato. `ReceivedPacketDispatch`
valida comprimento real/declarado e a igualdade entre opcode externo e
`Header.Type` antes de preservar o mesmo frame para `TMScene::OnPacketEvent`.

### WYD-Go

`wire.MessagePanel` constroi 108 bytes com `ID=0`, copia no maximo 95 bytes a
partir de `+12` e conserva NUL em `+107`. `DaySync` reutiliza o mesmo envelope
com um prefixo consumido pelo parser existente da cena.

## Matriz de delta

| Claim | Nativo 7.48 | Source antes | Source atual | Decisao |
| --- | --- | --- | --- | --- |
| opcode | `0x101` | literal nomeado em `Basedef.h` | contrato em `wire/` | `PARIDADE_NATIVA` |
| tamanho | 108 exatos | asserts globais | asserts locais + gate | `PARIDADE_NATIVA` |
| texto | corpo em `+12`, 96 bytes | mesmo layout | preservado | manter |
| dispatch | rejeita envelope divergente | validacao tardia na cena | gate antes do callback | `MODERNIZACAO_COMPATIVEL` |
| UI/prefixos | receptor da cena | implementacao viva | inalterados | preservar |

## Decisões

- Extrair somente o contrato wire e manter a implementacao unica atual do
  painel.
- Validar o envelope no mesmo gate incremental dos demais packets migrados,
  sem mover parse, prefixos ou apresentacao para a fronteira de transporte.
- Manter `0x102/0x104` e `0x105/0x106` em suas fichas independentes.

## Lacunas

- Exercitar no `client748/project.exe` aviso comum, os prefixos usados pela
  source, troca de cena e logout/relogin antes de qualquer claim
  `CLIENT_TESTED`.
- Confirmar visualmente duracao, truncamento e apresentacao com um servidor
  real; build e fixture nao cobrem renderizacao.

## Validação

- Pesquisa: gate, receptor, callers, tamanho, offset e ownership estao fechados
  no hash registrado.
- ABI: `static_assert` fixa 108 bytes e `String` em `+12` no build Win32.
- Automacao: a fixture C++ cobre truncamento, excesso, nulo, divergencias de
  `Type/Size`, entrega unica e preservacao integral do buffer; os testes Go
  cobrem `MessagePanel` e `DaySync`. Debug e Release passaram pelo
  `Build-Client.ps1` com 1708 checks/asserts em cada configuracao. XML e
  caminhos do projeto passaram; o candidato Release instalado possui SHA-256
  `7277CC4AD3ED65493D9375F149CDBD843C6B7A60826FA9360E79E60E2EBC4D25`.
- Client real: ainda nao executado; nenhuma alegacao `CLIENT_TESTED` e feita.
