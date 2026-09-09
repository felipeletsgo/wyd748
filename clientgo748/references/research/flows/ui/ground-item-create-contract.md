---
id: ground-item-create-contract
title: Materializacao de item no chao
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

# Materializacao de item no chao

## Pergunta

Qual e o contrato `0x26E` que cria ou atualiza um item do chao e quais dados
podem ser usados com seguranca pelo renderer do client 7.48?

## Fronteira de evidência

- `UTILIZADA` — binario nativo do hash acima, projeto Ghidra
  `WYD748Native_20260821.gpr` e descompilacao de `FUN_00492E7D`,
  `FUN_004856C3` e `FUN_0055890A`.
- `UTILIZADA` — source atual `MSG_CreateItem`, `OnPacketCreateItem`, ItemList,
  containers do mundo e `ReceivedPacketDispatch`.
- `UTILIZADA` — WYD-Go `wire.CreateItem`, teste byte a byte e emissores de
  visibilidade de itens/objetos do mapa.
- `UTILIZADA` — `ItemList.bin` ativo possui o contrato local de 6.500 entradas,
  protegido por `MAX_ITEMLIST`.
- `NAO APLICAVEL` — guia KR nao documenta este wire. Sources 7.54, W2PP,
  Secrets e Micronics nao foram consultadas.

## Fluxo nativo 7.48

### Callers

`FUN_00492E7D @ 0x00492E7D` encaminha `0x26E` diretamente para
`FUN_004856C3 @ 0x004856C3`. `FUN_0055890A @ 0x0055890A` aceita esse opcode
somente quando `Size=0x20`.

### Callees

`FUN_004856C3` procura objeto existente por `ItemID@0x10`, lê o item de oito
bytes em `+0x12` e escolhe gate, cannon ou item comum. Inicializa mesh/estado,
rotacao `+0x1A`, posicao `GridX/GridY@0x0C/0x0E`, altura `+0x1C`, flag de
criacao `+0x1D` e owner `+0x1E`. Os callees relevantes incluem
`FUN_0054CD07` para habilidades, `FUN_004F0C50`/`FUN_004F1960`/
`FUN_004F3A9B` para tipos de objeto, `FUN_0054AC09` para insercao no mundo e
`FUN_005554CC` para efeito de propriedade.

O handler consulta a tabela global de definicoes pelo `sIndex` recebido. O
nativo pressupoe pacote e indice validos; a source compatível rejeita valores
fora do ItemList carregado antes do primeiro lookup.

## Estado e lifecycle

| Evento | Precondicao | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- |
| criar item comum | frame 32B e `sIndex` valido | `TMItem` no container | posicao, mesh e som opcional | alocacao falha sem inserir |
| criar gate/objeto | habilidade 34 positiva | `TMGate` inicializado | heightmap/estado atualizados | ground/container ausente rejeita |
| atualizar ID existente | objeto ja encontrado | mesmo owner atualizado | sem segundo child | tipo segue item recebido |
| indice invalido | `sIndex<=0` ou `>=6500` | mundo intacto | nenhum lookup fora da tabela | retorno imediato |
| remover/relogar | `0x16F` ou troca de cena | owner normal destroi objeto | container/cena limpam recursos | frame nao e retido |

O transporte empresta os 32 bytes durante o callback. Os objetos criados sao
entregues ao container somente depois da inicializacao; objeto existente mantem
o owner anterior. O gate nao copia nem retem o frame.

## Wire, ABI e recursos

Contrato S->C little-endian:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`, `Type=0x26E` |
| `0x0C` | 2 | `GridX` |
| `0x0E` | 2 | `GridY` |
| `0x10` | 2 | `ItemID` |
| `0x12` | 8 | `STRUCT_ITEM` |
| `0x1A` | 1 | `Rotate` |
| `0x1B` | 1 | `State` |
| `0x1C` | 1 | `Height` |
| `0x1D` | 1 | `Create` |
| `0x1E` | 2 | `Owner` |
| total | 32 | tamanho exigido por `FUN_0055890A` |

O `sIndex` referencia as 6.500 linhas carregadas do ItemList 7.48. Nenhum
offset de objeto C++ nativo e transportado ou copiado para a source atual.

## Mapeamento atual

`GroundItemCreateContract.h` nomeia opcode, tamanho, offsets e dominio do
ItemList. `Basedef.h` mantem a struct concreta e fixa cada campo por asserts.
`ReceivedPacketDispatch` exige 32 bytes reais/declarados e Type igual ao opcode
externo antes do cast. `OnPacketCreateItem` valida mensagem, owners da cena e
`sIndex` antes de consultar habilidades ou `g_pItemList`. O WYD-Go emite o
mesmo layout nos snapshots de visibilidade e na publicacao de drops.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisao |
| --- | --- | --- | --- | --- |
| opcode/tamanho | `0x26E/32` | literal sem gate exato | contrato e gate | `PARIDADE_NATIVA` |
| offsets | `12..30` | struct com assert apenas de tamanho | asserts por campo | `PARIDADE_NATIVA` |
| ItemList | lookup direto | podia indexar fora de 6.500 | dominio validado | `MODERNIZACAO_COMPATIVEL` |
| objeto/efeitos | branches nativos | implementacao existente | preservados | manter |

## Decisões

- Validar envelope antes do cast e `sIndex` antes de qualquer lookup.
- Preservar objetos, posicao, rotacao, sons e ownership para frames validos.
- Rejeitar lifecycle parcial quando a cena ainda nao possui ground/container.
- Nao criar asset, packet ou fallback de outra versao.

## Lacunas

- Executar aparicao, atualizacao e remocao de loot no candidato.
- Confirmar gate/objeto permanente e owner visual no fluxo real.
- Testar relogin e troca de regiao com itens visiveis.

## Validação

- Pesquisa: `FUN_00492E7D -> FUN_004856C3` fecha o consumidor e
  `FUN_0055890A` confirma `0x26E/0x20`.
- Automacao: fixture cobre todos os prefixos, excesso, nulo, divergencias de
  Size/Type, offsets, preservacao dos bytes e limites do ItemList.
  `go test -count=1 ./internal/wire` e `go vet ./internal/wire` passaram.
- Build Release passou pelo `Build-Client.ps1` com 24626 checks e asserts
  estaticos. XML e registro unico do header passaram. Candidato instalado:
  `A2272382F20F09377C0D2F3090612A96B9D82E208A8F380409D1AC175001FBFF`.
- Estado `AUTOMATED TESTED`; client real ainda nao executado, portanto nao e
  `CLIENT_TESTED`.
