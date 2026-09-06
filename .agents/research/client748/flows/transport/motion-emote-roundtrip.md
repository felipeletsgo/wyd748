---
id: motion-emote-roundtrip
title: Envio e aplicação de motion/emote 0x36A
subsystem: world-input-motion
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Envio e aplicação de motion/emote 0x36A

## Pergunta

Qual contrato o client 7.48 envia ao escolher um emote, como o retorno libera
o próximo input e quais valores podem atravessar o servidor sem permitir que o
client fabrique efeitos reservados?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado
  `exports/motion-emote-flow.tsv`, SHA-256
  `75038059083E12532ED4D83B573C3F5D2DFA9C3CE3706FB073CA29D1F99015CD`.
- Assets: `NÃO APLICÁVEL`; este corte não altera animações, sons ou efeitos.
- Source atual: `TMFieldScene.cpp`, `TMHuman.cpp` e `Basedef.h` em
  `client-source/tmproject/TMProject748/`.
- Servidor: `internal/game/character_session.go`, `internal/game/visibility.go`,
  `internal/game/security.go`, `internal/wire/codec.go` e seus testes.
- TMProject posterior e guias: `NÃO APLICÁVEL`; o contrato foi decidido pelo
  binário/Ghidra 7.48, pela source ativa e pelo WYD-Go. W2PP, Secrets e
  Micronics permaneceram excluídos.

## Fluxo nativo 7.48

### Entrada observável

- Evento: teclado numérico/menu escolhe um emote, ou o click no personagem
  alterna sentar/levantar.
- Precondições e estado inicial: FieldScene ativa, personagem local vivo,
  debounce de 500 ms vencido e nenhuma motion anterior aguardando retorno.
- Saída observável: o client envia `0x36A/20`; ao receber o frame com seu ID,
  aplica a animação e limpa a motion pendente, liberando o próximo emote.

### Callers

- `FUN_004541F3` encaminha os comandos `0x9CA8..0x9CB1` para
  `FUN_00455950` com teclas `0x60..0x69`.
- `FUN_00454763` também encaminha teclado numérico para `FUN_00455950`.
- O slot exato `0x005A429C` da vtable FieldScene aponta para
  `FUN_004625DA`, que envia a alternância de sentar/levantar no click.
- O slot exato `0x005A5580` aponta para `FUN_0052EAA9`, dispatcher do humano
  que entrega `0x36A` a `FUN_005296E8`.

### Função principal

`FUN_00455950` zera um buffer de 20 bytes, grava `Type=0x36A`, o ID local,
`Motion` em `+12` e `Direction=0` em `+16`, e chama `FUN_0055F2DD` com tamanho
`0x14`. O caminho de click em `FUN_004625DA` constrói o mesmo frame.

No recebimento, `FUN_0052EAA9` seleciona `0x36A` e chama `FUN_005296E8`.
Esse handler lê `Motion` signed em `+12`, `Parm` signed em `+14` e o DWORD/float
em `+16`. `Motion=100` cria firework; `Parm=1` aplica variantes de personagem,
`Parm=2` limpa morte e `Parm=3` cria level-up. Para `Motion < 256`, o ID local
limpa a motion pendente e a animação é aplicada.

### Callees

- `FUN_0055F2DD` enquadra e envia o buffer construído na pilha.
- `FUN_0055890A` é o gate nativo e aceita `0x36A` somente com `0x14` bytes.
- `FUN_0052EAA9 -> FUN_005296E8` forma o caminho de recepção do humano.
- `FUN_005296E8 -> FUN_00523533` aplica a motion/direção; os branches especiais
  criam efeitos e os entregam ao container da cena.

### Saídas e erros

- Tecla fora de `0x60..0x69`, personagem morto, motion incompatível, debounce
  ativo ou motion já pendente não enviam outro frame.
- Os emissores de emote do 7.48 usam `Parm=0` e `Direction=0`. Motions
  observadas são `13`, `15..24`, `25` e `27`.
- `Motion=100` e `Parm=1..3` chegam ao client por ações do servidor; aceitá-los
  como intenção do client permitiria fabricar efeitos visuais.
- Frame recebido com tamanho diferente de 20 é rejeitado antes do handler.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| escolher emote | vivo, debounce livre, sem pending | `FUN_004541F3/FUN_00454763 -> FUN_00455950` | motion local fica pendente | envia `0x36A/20` | condição inválida consome sem envio |
| click sentar/levantar | humano local e estado permitido | `FUN_004625DA` | `25` ou `27` pendente | envia `0x36A/20` | estado incompatível não envia |
| retorno do próprio ID | frame válido e humano localizado | `FUN_0052EAA9 -> FUN_005296E8` | pending volta a none | aplica motion | motion >= 256 não anima |
| retorno de observer | humano remoto localizado | `FUN_005296E8` | animação do remoto atualizada | som/efeito conforme campos | humano ausente não recebe dispatch |
| efeito autoritativo | `100` ou Parm especial vindo do servidor | `FUN_005296E8` | efeito/motion aplicado | aloca efeito quando necessário | alocação nula não é anexada |

### Vtables, vptrs e receptores

`0x005A429C -> FUN_004625DA` é o receptor de input da FieldScene.
`0x005A5580 -> FUN_0052EAA9` é o receptor de packet do humano. Os dois slots
foram exportados por endereço exato e convergem no mesmo contrato de 20 bytes.

### Ownership

Os emissores possuem o buffer na pilha e `FUN_0055F2DD` não o retém. Na
recepção, o buffer pertence ao transporte e é apenas emprestado ao humano.
Efeitos criados pelos branches `100/3` transferem ownership ao container da
cena quando a alocação é válida.

### Falha parcial

Falha ou bloqueio antes do envio não publica pending novo. Frame inválido não
chega ao humano. Um efeito que não pode ser criado não muda a animação base; os
demais branches continuam retornando consumo único.

### Cleanup e teardown

Motion pendente e animação pertencem à instância do humano. O teardown da
FieldScene remove humanos e o container de efeitos; nenhum ponteiro para o
buffer do packet sobrevive ao callback.

### Shutdown

`N/A`: o contrato não cria thread, socket ou owner global. O shutdown geral
destrói a cena e o transporte pelos fluxos já documentados.

### Logout e relogin

Logout destrói a FieldScene e as instâncias de humano. No retorno, pending é
inicializado como none e nenhuma motion da sessão anterior é restaurada. O
roundtrip real ainda precisa ser exercitado no `project.exe`.

## Wire, ABI e recursos

Direção bidirecional, opcode `0x36A`, exatamente 20 bytes:

| Campo | Offset | Largura/tipo | Evidência |
| --- | ---: | --- | --- |
| `MSG_STANDARD` | `+0` | 12 bytes | header 7.48 |
| `Motion` | `+12` | int16 signed | `MOVSX [packet+0x0C]` |
| `Parm` | `+14` | int16 signed | `MOVSX [packet+0x0E]` |
| `Direction` | `+16` | float32/DWORD | leitura `[packet+0x10]` |

O layout usa alinhamento Win32 natural e não possui padding final. O gate
`FUN_0055890A` compara o tamanho com `0x14`. Nenhum asset ou ID de UI faz parte
do frame.

## Mapeamento atual

### Source recompilável

`MSG_Motion` em `Basedef.h` já possui os mesmos campos e tamanho. Os emissores
de `TMFieldScene` zeram o frame, fixam `Parm/Direction` em zero e marcam
`m_SendeMotion`; `TMHuman::OnPacketFireWork` limpa essa marca somente quando o
retorno carrega o ID local.

### WYD-Go

`OpMotion` e o gate de 20 bytes existem. `wire.Motion` publica `Motion@12` e
`Parm@14`, deixando `Direction@16` zerado. O handler C->S atual apenas reconhece
o tamanho e descarta o frame, portanto não devolve o ID autoritativo ao emissor
nem publica o emote aos observers.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| wire `0x36A` | 20 bytes e offsets confirmados | layout equivalente em Basedef | não decide ABI | builder/gate equivalentes | extrair sem alterar ABI |
| emote C->S | Parm/Direction zero; motions `13,15..25,27` | emissores equivalentes | N/A | frame descartado | revalidar e publicar |
| ID do retorno | humano local limpa pending pelo Header.ID | callback equivalente | N/A | deve usar ID da sessão | ignorar ID declarado pelo client |
| efeitos especiais | `100` e Parm `1..3` são consumo S->C | callback equivalente | N/A | builders server-side existentes | rejeitar como intenção C->S |
| lifecycle | retorno libera próximo emote | pending depende do retorno | N/A | sem retorno | incluir emissor no fan-out |

## Decisões

- Classificar o wire e o roundtrip como `PARIDADE_NATIVA/CONTRACT`.
- Extrair `MSG_Motion` para `internal/wire` preservando nome, signedness,
  packing, tamanho e emissores.
- No WYD-Go, aceitar somente as motions produzidas pelos inputs 7.48 com
  `Parm=0`; reconstruir o frame com o ID autoritativo e `Direction=0`.
- Publicar para o próprio jogador e observers visíveis pelo fan-out existente.
- Manter `Motion=100` e `Parm=1..3` exclusivos dos builders server-side.

## Lacunas

- O roundtrip, as dez teclas, sentar/levantar, dois clients, troca de cena e
  relogin ainda não foram exercitados no `project.exe`.
- A direção diferente de zero não é produzida pelos emissores 7.48 estudados e
  permanece fora deste corte.

## Validação

- Pesquisa: export Ghidra headless read-only finalizou sem `SCRIPT ERROR`, com
  hash do programa esperado, slots exatos e resumos `instruction_search` para
  emissor e receptor.
- Automação: `go test -count=1 ./...` passou; Debug e
  Release via `Build-Client.ps1` passaram com 1925 checks/asserts. O Release foi
  instalado com SHA-256
  `DB0BEE35327ED0E6DEBD987BB3F515554D4EF5E7FFF01D212593E5AA1D68DB3E`.
- Client real: não executado; não `CLIENT_TESTED`.
