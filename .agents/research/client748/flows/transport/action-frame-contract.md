---
id: action-frame-contract
title: Envelope compartilhado de Action, ActionStop e Illusion
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-21
---

# Envelope compartilhado de Action, ActionStop e Illusion

## Pergunta

Qual tamanho deve ser exigido antes de entregar `0x366`, `0x367` ou `0x368`
ao `TMHuman`, que converte o mesmo buffer para `MSG_Action` e le campos ate
`Route[24]`?

## Fronteira de evidência

- UTILIZADA: ficha nativa `flows/transport/missing-entity-request.md`, que liga
  `FUN_00492E7D -> FUN_00482903` ao consumo de Action e ao lookup da entidade.
- UTILIZADA: evidencia nativa ja registrada junto de `MSG_Action` para
  `FUN_00524BBB`, com o construtor 7.48 e os offsets de posicao, velocidade,
  efeito, destino e rota.
- UTILIZADA: source atual `TMHuman::OnPacketEvent`, `MSG_Action`,
  `ObjectManager` e `ReceivedPacketDispatch`.
- UTILIZADA: `wire.ActionStop`, `wire.PlayerMove`, `wire.MobMove`,
  `wire.IllusionMove`, dispatch e testes do WYD-Go.
- NAO APLICAVEL: assets, controles e recursos visuais; o corte altera somente
  a validacao do envelope recebido.
- INDISPONIVEL NESTA RETOMADA: corpus externo do Ghidra; nenhuma nova claim
  nativa foi criada alem das evidencias versionadas acima.

## Fluxo nativo 7.48

### Entrada observável

- `0x366` reproduz deslocamento normal ou estado estacionario publicado pelo
  servidor; `0x367` encerra a acao enviada pelo client; `0x368` materializa a
  Illusion com o mesmo layout.
- Os tres opcodes carregam 52 bytes. Posicao fica em `+12`, Speed em `+16`,
  Effect em `+20`, destino em `+24` e a rota de 24 bytes em `+28`.
- A saida observavel e movimento, parada ou efeito de Illusion no objeto cujo
  ID consta no header.

### Callers

O dispatcher nativo da Field `FUN_00492E7D` encaminha Action ao caminho
`FUN_00482903`. Na source recompilavel, `ObjectManager::OnPacketEvent` percorre
a arvore e entrega o mesmo frame a `TMHuman::OnPacketEvent`.

### Função principal

`TMHuman::OnPacketEvent` reconhece os tres tipos antes do switch, converte o
buffer para `MSG_Action`, le `TargetX/Y` e `Effect`, e seleciona
`OnPacketMove`, `OnPacketChaosCube` ou `OnPacketIllusion`. Antes deste corte,
os opcodes nao estavam em `ReceivedPacketDispatch::ExpectedSize`; somente o
header minimo era exigido antes dessas leituras.

### Callees

- `OnPacketMove` consome posicao, velocidade, destino e rota para interpolar o
  deslocamento.
- `OnPacketIllusion` usa destino, Effect e estado da cena para salto, camera,
  animacao e consumo visual de MP.
- O lookup de entidade pode emitir `0x369/16` quando o ID ainda nao existe.

### Saídas e erros

Frame exato e entregue uma vez sem copia. Nulo, todos os prefixos abaixo de 52
bytes, excesso, `Header.Size`, `Header.Type` ou opcode externo divergentes sao
rejeitados antes do callback. A rejeicao nao move entidade, nao toca camera e
nao emite pedido de entidade ausente.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| Action valido | entidade visivel | `OnPacketMove` | rota em reproducao | animacao/interpolacao | retorna resultado do handler |
| ActionStop valido | entidade visivel | mesmo envelope por Effect | acao encerrada | postura/posicao sincronizada | retorna resultado do handler |
| Illusion valida | caster materializado | `OnPacketIllusion` | salto aplicado | camera, som e MP visual | retorna resultado do handler |
| frame invalido | qualquer | `ReceivedPacketDispatch` | estado preservado | nenhum callback | retorna `false` |

### Vtables, vptrs e receptores

O ObjectManager distribui pela arvore de objetos e o receptor final e o
`TMHuman` correspondente ao ID. O contrato nao altera vtable, slot virtual ou
tipo concreto; apenas impede que um frame incompleto alcance o receptor.

### Ownership

O transporte empresta o buffer somente durante o callback. O gate nao copia
nem conserva o payload; os handlers materializam seu proprio estado de rota.

### Falha parcial

A validacao ocorre antes da primeira leitura tipada e de qualquer mutacao.
Assim, truncamento nao pode produzir destino parcial, rota contaminada, pedido
espurio de entidade ou alteracao de camera.

### Cleanup e teardown

O contrato nao cria alocacao, timer ou objeto. Rotas e efeitos continuam sob o
cleanup existente do `TMHuman` e da FieldScene.

### Shutdown

N/A para o gate. Disconnect encerra a cena e impede novas entregas; nao ha
callback ou buffer pendente criado por esta validacao.

### Logout e relogin

Uma nova FieldScene recebe snapshots e movimentos novos do servidor. O mesmo
gate vale para sincronizacao inicial de postura, reentrada na visao e movimento
posterior; nenhum frame e reaproveitado entre sessoes.

## Wire, ABI e recursos

Direcao C<->S, tamanho fixo de 52 bytes para os tres opcodes:

| Campo | Offset | Largura | Semântica |
| --- | ---: | ---: | --- |
| `MSG_STANDARD` | 0 | 12 | size, key, checksum, type, id e tick |
| `PosX/PosY` | 12 | 4 | origem/posicao visual |
| `Speed` | 16 | 4 | velocidade do trecho |
| `Effect` | 20 | 4 | movimento, parada ou efeito especial |
| `TargetX/TargetY` | 24 | 4 | destino atual |
| `Route` | 28 | 24 | passos de rota, quando presentes |

`ActionFrameContract.h` centraliza opcodes, tamanho e offsets.
`MSG_Action` continua na fachada legada e possui asserts contra todas essas
constantes. Nao ha packing, asset ou payload novo.

## Mapeamento atual

### Source recompilável

`ReceivedPacketDispatch::ExpectedSize` agora registra `0x366`, `0x367` e
`0x368` como 52 bytes. A entrada compara tamanho real/declarado e os dois
discriminantes antes de qualquer cast do `TMHuman`.

### WYD-Go

Os builders `ActionStop`, `PlayerMove`, `MobMove` e `IllusionMove` ja produzem
52 bytes com os mesmos offsets. O gate C->S tambem exige 52 bytes para os tres
opcodes; movimento e efeitos continuam validados e publicados pelo servidor.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| opcodes | `0x366/0x367/0x368` | constantes corretas | mesmos valores | manter |
| layout | 52B e offsets registrados | struct/asserts corretos | builders equivalentes | manter |
| recepcao S->C | consumidor tipado comprovado | so header minimo | emissor sempre 52B | exigir frame exato |
| autoridade | client reproduz estado | handlers visuais | servidor valida/publica | preservar |

## Decisões

- Classificar opcodes e layout como `PARIDADE_NATIVA` com evidencia ja
  versionada.
- Classificar o novo guard central como `MODERNIZACAO_COMPATIVEL`: preserva o
  wire e endurece a entrada do peer ativo sem alegar equivalencia com o gate
  nativo de caller ainda nao resolvido.
- Cobrir os tres opcodes no mesmo contrato porque compartilham ABI e consumidor.
- Nao alterar semantica de rota, Effect, velocidade ou autoridade do servidor.

## Lacunas

- Falta exercitar no `project.exe` real caminhada continua, parada, reentrada
  na visao, movimento de mob e Illusion.
- O caller/direcao de `FUN_0055890A` permanece nao resolvido; esta ficha nao o
  usa como prova de que o nativo rejeitava frames S->C truncados.

## Validação

- Pesquisa: evidencias nativas versionadas e todos os produtores/consumidores
  ativos dos tres opcodes foram cruzados; corpus externo indisponivel, sem nova
  inferencia de Ghidra.
- Automacao: teste C++ cobre tamanho publicado, nulo, todos os prefixos,
  excesso, discriminantes divergentes, entrega unica, campos consumidos e
  imutabilidade para cada opcode. Testes Go existentes cobrem os builders e o
  gate de 52 bytes.
- Client real: pendente; build e testes nao promovem a `CLIENT_TESTED`.
