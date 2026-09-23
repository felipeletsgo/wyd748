---
id: capsule-info-envelope
title: Envelope de consulta e resposta da Capsula Celestial
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Envelope de consulta e resposta da Capsula Celestial

## Pergunta

Qual layout `0xDC3` o client 7.48 pode copiar para seu cache de doze capsulas
depois da consulta `0x2CD`, e qual divergencia restou da source 7.69?

## Fronteira de evidência

- UTILIZADA: contrato 7.48 ja versionado em `wire.CNFCapsuleInfo`, seu teste de
  layout e comentarios que registram a copia nativa de 13 DWORDs/52 bytes.
- UTILIZADA: `FUN_00492E7D @ 0x00492E7D`, dispatcher nativo de packets da
  Field ja comprovado pelas fichas versionadas; o branch especifico de
  `0xDC3` nao foi reaberto nesta retomada.
- UTILIZADA: source atual de `MSG_CAPSULEINFO`, switch da Field,
  `OnPacketCapsuleInfo`, cache de `ObjectManager` e tooltip de `SGridControl`.
- UTILIZADA: handler WYD-Go `onCapsuleInfo`, que valida sessao, ownership do
  item e snapshot persistido antes de emitir a resposta.
- NAO APLICAVEL: asset ou coordenada visual; o delta corrige ABI e valida a
  fronteira de transporte antes do consumidor existente.
- INDISPONIVEL NESTA RETOMADA: export externo do Ghidra. A decisao reutiliza a
  validacao 7.48 ja registrada no codec/teste, sem nova claim de decompilacao.

## Fluxo nativo 7.48

### Entrada observavel

Ao passar o mouse sobre o item 3443 sem cache local, o client envia `0x2CD`
com 16 bytes e o indice da capsula em `Parm@12`. O servidor responde `0xDC3`
com 52 bytes; a Field atualiza uma das doze entradas e solicita refresh do
tooltip.

### Callers

`SGridControl::MouseOver` procura `CIndex` no cache. Em caso de falta, monta
`MSG_STANDARDPARM` e envia `MSG_RequestCapsuleInfo_Opcode`. O dispatcher da
Field encaminha `MSG_CapsuleInfo_Opcode` a `OnPacketCapsuleInfo`. No binario,
`FUN_00492E7D @ 0x00492E7D` e o dispatcher da Field ja identificado; esta
ficha nao atribui um callee nativo especifico ao branch `0xDC3` ainda ausente.

### Funcao principal

`OnPacketCapsuleInfo` compara `CIndex`, substitui ou aloca uma entrada do cache
e copia o struct inteiro. A source herdada declarava `Mastery[4]`, resultando
em 56 bytes e deslocando `skill` e `Quest`; o frame 7.48 possui somente duas
masteries, nove skills e 52 bytes.

### Callees

- `ObjectManager::m_stCapsuleInfo[12]` conserva a copia local.
- `SGridControl::UpdateCapsuleInfo` apresenta classe, nivel, atributos, nove
  skills e quest; ele nao depende de uma terceira ou quarta mastery.
- `wire.CNFCapsuleInfo` serializa exatamente o layout compacto validado.

### Saidas e erros

Buffer nulo, qualquer prefixo abaixo de 52 bytes, excesso, `Header.Size`
divergente ou opcode externo/interno diferente sao rejeitados antes do cast e
da copia. Uma resposta valida e entregue uma unica vez sem mutacao do frame.

## Estado e lifecycle

| Evento/estado | Precondicao | Funcao/call | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- | --- |
| cache miss | hover em selo 3443 | `SGridControl::MouseOver` | consulta `0x2CD` | nenhum dado local inventado | sessao/owner invalido nao recebe resposta |
| resposta | frame `0xDC3/52` valido | `OnPacketCapsuleInfo` | entrada inserida/substituida | tooltip marcado para refresh | frame invalido nao chega ao handler |
| cache cheio | doze indices distintos | fallback da Field | slot zero substituido | cache permanece limitado | sem alocacao adicional |
| troca de cena/logout | teardown do objeto global | lifecycle existente | cache reconstruido | nenhum buffer de rede retido | sem cleanup novo |
| relogin | nova consulta | mesmo roundtrip | snapshot autoritativo novo | ownership revalidado | cache antigo nao autoriza resposta |

### Vtables, vptrs e receptores

O receptor continua sendo `TMFieldScene`; o gate nao altera vtable, vptr,
assinatura de handler nem tipo dos controles.

### Ownership

O dispatcher empresta o frame durante o callback. A Field copia os 52 bytes
para armazenamento proprio; nenhuma referencia ao buffer recebido sobrevive.

### Falha parcial

A validacao ocorre antes da atribuicao integral do struct. Assim, truncamento
nao pode atualizar parcialmente indice, atributos, skills ou quest.

### Cleanup, shutdown, logout e relogin

O contrato nao cria recurso. Cache e tooltip continuam presos ao lifecycle da
Field/ObjectManager e uma nova sessao depende de consulta validada pelo server.

## Wire, ABI e recursos

Consulta C->S `0x2CD`, 16 bytes; resposta S->C `0xDC3`, 52 bytes:

| Campo da resposta | Offset | Largura | Semantica |
| --- | ---: | ---: | --- |
| `MSG_STANDARD` | 0 | 12 | size, key, checksum, type, id e tick |
| `CIndex` | 12 | 4 | indice persistido da capsula |
| `Class`, `Level` | 16 | 4 | classe visual e nivel |
| `sStr`, `sInt`, `sDex`, `sCon` | 20 | 8 | atributos base |
| `Mastery[2]` | 28 | 4 | dois WORDs do snapshot 7.48 |
| `skill[9]` | 32 | 18 | nove valores exibidos no tooltip |
| `Quest` | 50 | 2 | estado de quest |

`CapsuleInfoContract.h` centraliza opcode, tamanho, offsets e cardinalidades.
Os asserts de `MSG_CAPSULEINFO` impedem que a fachada volte ao layout 7.69.

## Mapeamento atual

### Source recompilavel

`MSG_CAPSULEINFO` usa duas masteries e mede 52 bytes. O switch nomeia o opcode
e `ReceivedPacketDispatch::ExpectedSize` exige o frame integral antes do cast.

### WYD-Go

`onCapsuleInfo` aceita somente consulta de 16 bytes de jogador em mundo,
resolve a capsula dentro da conta e confirma ownership do selo. O builder
`CNFCapsuleInfo` envia os mesmos offsets e cardinalidades do client corrigido.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisao |
| --- | --- | --- | --- | --- |
| resposta | `0xDC3`, 52B | literal `0xDC3`, struct 56B | `0xDC3`, 52B | corrigir ABI do client |
| mastery | dois WORDs | quatro WORDs herdados do 7.69 | dois WORDs | usar cardinalidade 7.48 |
| skill/quest | +32/+50 | +36/+54 | +32/+50 | alinhar offsets |
| autoridade | snapshot recebido | cache apenas visual | valida conta e selo | preservar |
| recepcao | consumidor copia frame | apenas header minimo | sempre 52B | exigir frame exato |

## Decisões

- Classificar a reducao de `Mastery[4]` para `[2]` e os offsets resultantes
  como `PARIDADE_NATIVA`, reutilizando o contrato 7.48 ja validado.
- Classificar o gate como `MODERNIZACAO_COMPATIVEL`: ele preserva o wire e
  impede leitura quatro bytes alem do frame valido.
- Manter ownership e selecao da capsula exclusivamente no WYD-Go.

## Lacunas

- O hover de uma capsula real precisa ser exercitado no `project.exe`, com
  cache miss/hit, mais de doze indices e logout/relogin.
- Esta ficha nao afirma ter isolado no Ghidra o branch nativo de rejeicao de
  frame malformado; usa o layout 7.48 ja registrado no codec e no teste.

## Validação

- Pesquisa: produtor, consumidor, cache, tooltip e autoridade foram cruzados
  com o contrato 7.48 versionado.
- Automacao: asserts C++ fixam 52 bytes e todos os offsets; o teste do gate
  cobre nulo, prefixos, excesso, discriminantes, entrega, campos e imutabilidade.
  O teste Go confirma opcode, tamanho, ID, primeiro/ultimo skill e quest.
- Client real: pendente; build e automacao nao promovem a `CLIENT_TESTED`.
