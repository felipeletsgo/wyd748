---
id: cargo-gold-transfer-confirmations
title: Confirmações de depósito e saque de gold do Cargo
subsystem: transport
status: UNMAPPED
native_sha256: UNRESOLVED
updated: 2026-09-21
---

# Confirmações de depósito e saque de gold do Cargo

## Pergunta

Qual envelope o TMProject deve exigir antes de `OnPacketDeposit` e
`OnPacketWithdraw` aplicarem a confirmação publicada pelo WYD-Go?

## Fronteira de evidência

- UTILIZADA: source atual do client, incluindo construção dos pedidos,
  `TMFieldScene::OnPacketEvent`, `OnPacketDeposit` e `OnPacketWithdraw`.
- UTILIZADA: `wire.CargoGoldTransfer`, `onCargoGold`, gate de segurança e testes
  do WYD-Go, que fixam 16 bytes e o valor `uint32` em `+12`.
- UTILIZADA: `world-state-parameter-contracts.md` para os snapshots
  autoritativos subsequentes `0x339` e `0x337`.
- NÃO APLICÁVEL: assets não alteram este envelope.
- INDISPONÍVEL NESTA RETOMADA: corpus textual externo. Não há nova claim de
  instrução ou endereço nativo para `0x387/0x388`.

## Fluxo nativo 7.48

### Entrada observável

O client envia `0x388` para depósito e `0x387` para saque com um DWORD de
quantidade em `+12`. O peer ativo devolve o mesmo opcode e quantidade como
confirmação, seguido dos snapshots autoritativos de Cargo e personagem.

### Callers

Na source recompilável, `TMFieldScene::OnPacketEvent` encaminha `0x388` a
`OnPacketDeposit` e `0x387` a `OnPacketWithdraw`. Endereços do binário nativo
não foram promovidos nesta ficha.

### Função principal

Os dois handlers fazem cast para `MSG_STANDARDPARM` e leem `Parm` em `+12`.
Antes deste corte, o dispatcher admitia qualquer frame com o header mínimo de
12 bytes, permitindo leitura além do buffer em uma confirmação truncada.

### Callees

Os handlers atualizam os contadores locais exibidos. O WYD-Go valida NPC,
saldo, teto de gold e persistência antes de publicar a confirmação.

### Saídas e erros

Frames nulos, truncados, excedentes ou com `Header.Size`/`Header.Type`
divergentes são rejeitados antes dos handlers. Rejeições de regra de negócio
continuam sem confirmação de sucesso e sem mutação autoritativa.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| depósito confirmado | frame 16B válido | `OnPacketDeposit` | UI transfere a quantia para Cargo | snapshots `0x339/0x337` reconciliam | inválido não chega ao handler |
| saque confirmado | frame 16B válido | `OnPacketWithdraw` | UI transfere a quantia para personagem | snapshots `0x339/0x337` reconciliam | inválido não chega ao handler |
| envelope inválido | qualquer | `ReceivedPacketDispatch` | estado preservado | nenhum callback | retorna `false` |

### Vtables, vptrs e receptores

O receptor permanece `TMFieldScene`; o gate só valida o envelope antes do
callback legado.

### Ownership

O buffer é emprestado durante a chamada. O gate não retém nem altera bytes.

### Falha parcial

Não há escrita local antes de validar os 16 bytes completos. Persistência e
rollback de saldo permanecem responsabilidade do servidor.

### Cleanup e teardown

O gate não aloca recursos e não adiciona cleanup.

### Shutdown

N/A. Disconnect interrompe novas confirmações.

### Logout e relogin

Novo login recebe `CargoGold` no char-list e não depende da confirmação antiga.

## Wire, ABI e recursos

`0x387/0x388`, C<->S, 16 bytes:

| Campo | Offset | Largura |
| --- | ---: | ---: |
| header | 0 | 12 |
| quantidade | 12 | 4 |

`Header.ID` permanece `SceneField` nas confirmações do WYD-Go. O contrato não
introduz recurso visual nem altera o limite econômico de 2.000.000.000.

## Mapeamento atual

### Source recompilável

`CargoGoldTransferContract.h` centraliza opcodes, tamanho e offset.
`ReceivedPacketDispatch` exige 16 bytes exatos antes dos casts legados, e
asserts fixam a equivalência com `MSG_STANDARDPARM`.

### WYD-Go

`onCargoGold` valida e persiste a operação, publica `CargoGoldTransfer` com o
mesmo opcode e quantidade e depois envia `UpdateCargoGold` e `UpdateEtc`.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| envelope de confirmação | não reaberto nesta retomada | cast 16B sem gate exato | builder e receptor usam 16B | fixar contrato ativo em 16B |
| autoridade de saldo | snapshots observáveis já documentados | aplica confirmação na UI | valida, persiste e reconcilia | preservar servidor autoritativo |

## Decisões

- Classificar o gate como `MODERNIZACAO_COMPATIVEL`: ele preserva o contrato
  já usado pelos dois projetos sem afirmar nova paridade de instruções nativas.
- Não adicionar validação semântica local da quantidade; os snapshots
  subsequentes continuam sendo a fonte autoritativa e evitam duplicar regra.
- Manter depósito e saque no mesmo contrato porque compartilham ABI e lifecycle.

## Lacunas

- Falta executar depósito e saque no client real, incluindo rejeição por saldo,
  teto e distância do NPC.
- Uma afirmação de `PARIDADE_NATIVA` para os handlers exige restaurar o corpus
  textual ou produzir nova evidência equivalente do binário 7.48.
- O estado permanece `UNMAPPED` no catálogo nativo, embora o contrato entre a
  source recompilável e o servidor ativo esteja implementado e testável.

## Validação

- Automação C++ cobre todos os prefixos, excesso, nulo, discriminantes,
  imutabilidade e os dois opcodes.
- Teste Go fixa 16 bytes, opcode, `SceneField` e quantidade em `+12`; testes do
  handler cobrem sucesso e rejeição por saldo.
- Client real: pendente; build e testes não promovem a `CLIENT_TESTED`.
