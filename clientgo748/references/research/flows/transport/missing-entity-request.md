---
id: missing-entity-request
title: Recuperação de entidade ausente por 0x369
subsystem: world-visibility
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Recuperação de entidade ausente por 0x369

## Pergunta

Como o client 7.48 pede a materialização de uma entidade referenciada por
movimento/ataque, qual é o layout exato e quais limites impedem esse pedido de
virar uma consulta global do mapa?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado
  `exports/request-mob-by-id-flow.tsv`, SHA-256
  `77EC34161BBB36E3F28B1B25C3FBF09EE2DAC8A0996AD24C7525F3943FD68B9F`.
- Assets: `NÃO APLICÁVEL`; o request não contém recurso ou UI.
- Source atual: `TMFieldScene.cpp` e `Basedef.h` em
  `client-source/tmproject/TMProject748/`.
- Servidor: `internal/game/character_session.go`, `internal/game/security.go`,
  `internal/game/session_drops_test.go` e `internal/wire/opcodes.go`.
- TMProject posterior e guias: `NÃO APLICÁVEL`; o ABI foi decidido pelo
  binário/Ghidra 7.48 e pelo ecossistema atual. W2PP, Secrets e Micronics
  permaneceram excluídos.

## Fluxo nativo 7.48

### Entrada observável

- Evento: Action ou Attack recebido referencia um ID que o ObjectManager ainda
  não materializou.
- Precondições e estado inicial: FieldScene ativa, frame original válido e
  busca local do ID sem resultado.
- Saída observável: o client envia `0x369/16` com o ID ausente; o servidor pode
  responder com o snapshot próprio da entidade.

### Callers

`FUN_00492E7D`, dispatcher da FieldScene, chama `FUN_00482903` para o caminho
de Action e `FUN_00489A3E` para o caminho de Attack. Ambos verificam se a
entidade referenciada já existe antes de construir o pedido.

### Função principal

`FUN_00482903` consulta o slot virtual `+0x34` do ObjectManager com o ID do
frame. Quando o resultado é nulo, zera 16 bytes, grava `Type=0x369`, ID local no
header, copia o WORD ausente para `+12` e chama `FUN_0055F2DD` com `0x10`.

`FUN_00489A3E` repete o mesmo envelope quando o atacante do packet não está
materializado. O buffer também é zerado antes de escrever o WORD em `+12`,
portanto `+14..15` permanecem reservados e zero.

### Callees

- Busca virtual `ObjectManager +0x34`: decide se o request é necessário.
- `_memset`: zera o frame e o WORD reservado.
- `FUN_0055F2DD`: envia os 16 bytes sem reter a pilha do caller.
- `FUN_0055890A`: exige `0x369` com tamanho exato `0x10`.

### Saídas e erros

- Entidade já materializada: nenhuma mensagem é enviada.
- ID ausente: o request é emitido uma vez pelo handler atual.
- Frame truncado/excedente não deve chegar ao consumidor tipado.
- O request não autoriza resposta para ID zero, para o próprio player, fora da
  visão, em outro espaço de gameplay ou em instância privada alheia.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| Action com ID conhecido | entidade local existe | `FUN_00492E7D -> FUN_00482903` | sem mudança | nenhum request | retorna sem envio |
| Action com ID ausente | lookup nulo | `FUN_00482903 -> FUN_0055F2DD` | aguarda snapshot | envia `0x369/16` | falha de envio não cria entidade |
| Attack com atacante ausente | lookup nulo | `FUN_00489A3E -> FUN_0055F2DD` | aguarda snapshot | envia mesmo contrato | packet original segue seu fluxo |
| request aceito no WYD-Go | ID visível e no mesmo espaço | `onREQMobByID` | client pode rematerializar | envia CreateMob/Player/Trade próprio | alvo inválido não responde |

### Vtables, vptrs e receptores

`FUN_00492E7D` é o receptor de packet da FieldScene na vtable já documentada.
`FUN_00482903` usa o slot virtual `+0x34` do ObjectManager para lookup; não
adiciona slot nem muda vtable. `0x369` é somente C->S, sem receptor homônimo.

### Ownership

O frame de 16 bytes vive na pilha do handler e é emprestado ao transporte
durante o envio. A resposta usa o contrato da entidade correspondente e cria
seu próprio objeto pelo lifecycle normal da cena.

### Falha parcial

Um envio recusado não cria placeholder local. O servidor responde apenas após
revalidar player, entidade, visibilidade, distância e instância; rejeição não
altera mapas de visibilidade nem estado persistente.

### Cleanup e teardown

O request não possui estado persistente nem timer próprio. Objetos recuperados
pertencem à FieldScene e seguem seu teardown normal.

### Shutdown

`N/A`: o frame não cria owner global, thread ou recurso; o transporte e a cena
são encerrados pelos fluxos gerais já documentados.

### Logout e relogin

Não há replay do request após logout. Uma nova FieldScene reconstrói a visão a
partir dos snapshots do servidor e pode emitir novos `0x369` se observar uma
referência antes da entidade correspondente.

## Wire, ABI e recursos

Direção C->S, opcode `0x369`, exatamente 16 bytes:

| Campo | Offset | Largura/tipo | Evidência |
| --- | ---: | --- | --- |
| `MSG_STANDARD` | `+0` | 12 bytes | header 7.48 |
| `MobID` | `+12` | int16/WORD | write nativo em `0048295E` |
| `Reserved` | `+14` | 2 bytes zero | frame zerado antes do write |

O gate `FUN_0055890A` compara o tamanho com `0x10`. Não há packing especial,
asset, ID de controle ou resposta `0x369` S->C.

## Mapeamento atual

### Source recompilável

`MSG_REQMobByID` em Basedef já expressa `short MobID` e `short Reserved`. O
caminho de Action usa esse tipo; o caminho de Attack usa `MSG_STANDARDPARM`,
que produz os mesmos quatro bytes em `+12`, mas mistura semanticamente outro
contrato. Ambos zeram o armazenamento.

### WYD-Go

O gate aceita exatamente 16 bytes e `onREQMobByID` lê o WORD em `+12`. Mob,
player e ghost shop só são enviados quando visíveis e próximos; instâncias
privadas são revalidadas. Os testes já cobrem recuperação, repetição para
reparar perda local, próprio ID e isolamento de instância.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| wire `0x369` | C->S, 16 bytes, WORD em +12 | layout correto em Basedef | não decide ABI | gate/parser equivalentes | extrair contrato |
| dois emissores | mesmo frame zerado | tipos semânticos distintos | N/A | um parser | usar `MSG_REQMobByID` nos dois |
| resposta | contrato próprio da entidade | cria pelo dispatcher normal | N/A | revalida e envia snapshot | manter autoridade server-side |
| lifecycle | sem pending persistente | sem estado adicional | N/A | sem mutação na rejeição | preservar |

## Decisões

- Classificar o wire como `PARIDADE_NATIVA/CONTRACT`.
- Extrair o tipo para `internal/wire` com asserts de tamanho e offsets.
- Usar o tipo dedicado nos dois emissores, preservando bytes e ordem.
- Não adicionar `0x369` ao dispatcher recebido, pois não há direção S->C.
- Manter todas as validações server-side existentes.

## Lacunas

- A perda real de Action/Attack, rematerialização de mob/player/shop, troca de
  cena e relogin ainda não foram exercitados no `project.exe`.

## Validação

- Pesquisa: export Ghidra headless read-only terminou sem `SCRIPT ERROR`, com
  SHA-256 esperado e `instruction_search` cobrindo construtor, lookup, offsets
  e envio de `FUN_00482903`; o segundo emissor e o gate foram reabertos.
- Automação: `go test -count=1 ./...` passou; Debug e Release via
  `Build-Client.ps1` passaram com 1927 checks/asserts. A fixture C++ cobre
  opcode, tamanho, MobID, WORD reservado e os dois discriminantes do header.
  Release instalado com SHA-256
  `E6772EA2C4E2A5449AB0B8E9CC46735D12903B66D7CE2680A3F98C88876DCC7`.
- Client real: não executado; não `CLIENT_TESTED`.
