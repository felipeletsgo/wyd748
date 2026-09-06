---
id: guild-relations-request
title: Guerra e aliança entre guilds 0xE0E e 0xE12
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Guerra e aliança entre guilds 0xE0E e 0xE12

## Pergunta

Quais frames o client 7.48 envia ao confirmar guerra ou aliança entre guilds?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export headless read-only
  completo em `%TEMP%/guild-relations-flow-full.tsv` e recorte versionado em
  `exports/guild-relations-flow.tsv`.
- Assets: `NÃO APLICÁVEL`; os contratos são envelopes de 20 bytes.
- Source atual: `TMFieldScene.cpp`, `Basedef.h` e `internal/wire/GuildRelationPacket.h`.
- Servidor: `internal/game/guild.go`, `security.go`, `world.go`,
  `guild_war_test.go` e `guild_flow_test.go`.
- TMProject posterior/guias: `NÃO APLICÁVEL` para o ABI; W2PP, Secrets e
  Micronics foram excluídos da decisão.

## Fluxo nativo 7.48

### Callers

`FUN_004640E5` recebe os eventos de confirmação da caixa. No evento `0x331`,
zera 20 bytes, grava `Type=0xE0E`, o ID local, a guild local em `+12` e a guild
alvo em `+16`. O evento `0x35E` repete o layout com `Type=0xE12`.

### Callees

Os callsites `0x0046489F` e `0x00464950` chamam `FUN_0055F2DD` com tamanho
`0x14`, aplicando o throttle e enviando cada frame. Depois, ambos os branches
ocultam o painel contextual pelo slot virtual `+0x60`. `FUN_0055890A` possui o
caso `0xE0E` e compara o tamanho com `0x14`; o `0xE12` não aparece nesse gate,
mas seu construtor e o argumento do envio comprovam diretamente os 20 bytes.

## Estado e lifecycle

| Evento/estado | Pré-condição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| confirmar guerra | caixa `0x331`, personagem em guild | `FUN_004640E5 -> FUN_0055F2DD` | servidor recebe intenção `0xE0E` | painel contextual fecha | servidor rejeita guild/cargo/alvo inválido |
| confirmar aliança | caixa `0x35E`, personagem em guild | `FUN_004640E5 -> FUN_0055F2DD` | servidor recebe intenção `0xE12` | painel contextual fecha | servidor rejeita guild/cargo/alvo inválido |
| persistência | liderança e alvo revalidados | `World.onGuildWar/onGuildAlly` | relação é salva e publicada | rollback restaura snapshot em falha | aviso sem mutação parcial |
| logout/relogin | nova sessão | lifecycle geral | relação vem do estado persistido | request antigo não é repetido | sem estado próprio do packet |

Os buffers vivem na pilha e são emprestados ao transporte. Os IDs são
intenções; a guild real do remetente, liderança, alvo e persistência pertencem
ao servidor.

## Wire, ABI e recursos

Direção C->S, exatamente 20 bytes para os opcodes `0xE0E` e `0xE12`:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | envelope WYD 7.48 |
| `+12` | `GuildID` | `uint32` | guild local, copiada de um WORD e estendida com zero |
| `+16` | `TargetGuildID` | `uint32` | guild alvo salva pela caixa de confirmação |

Não há recurso visual novo nem payload adicional. O mesmo layout serve aos
dois comandos; o opcode seleciona a regra server-side.

## Mapeamento atual

Os casos `817/862` de `TMFieldScene::OnMsgBoxEvent` agora usam
`MSG_GuildRelation`, `MSG_GuildWar_Opcode` e `MSG_GuildAlly_Opcode`, substituindo
somente `MSG_STANDARDPARM2` e os literais decimais `3598/3602`. O WYD-Go já
exige 20 bytes, ignora a guild alegada como fonte de autoridade e cobre guerra,
aliança, rejeição e rollback em testes.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire guerra | `0xE0E/20`, guilds em `+12/+16` | equivalente, antes literal | gate/handler equivalentes | extrair tipo dedicado |
| wire aliança | `0xE12/20`, mesmo payload | equivalente, antes literal | gate/handler equivalentes | compartilhar layout nomeado |
| signedness | guild local é WORD estendido com zero | agora `uint32` explícito | parser usa `uint32` | preservar |
| autoridade | client apenas alega IDs | source não decide resultado | servidor revalida e persiste | preservar |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT` para wire/ABI.
- Manter um layout nomeado compartilhado, com opcodes distintos e asserts de
  tamanho, offsets e signedness.
- Substituir somente os dois emissores; não alterar regras, callbacks ou painel.
- Não adicionar rota ao `ReceivedPacketDispatch`, pois ambos são C->S.

## Lacunas

- Guerra e aliança reais, rejeições, atualização dos dois clientes, troca de
  cena e relogin ainda não foram executados no `client748/project.exe`; não é
  `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra 12.1.3 headless/read-only terminou sem `SCRIPT ERROR`; o
  export contém o hash nativo, os dois eventos, campos e envios de 20 bytes.
- Automação: fixture C++ cobre os dois opcodes, tamanho, offsets, signedness e
  campos; `go test -count=1 ./...`, XML/caminhos, validador documental e
  `git diff --check` passaram. Debug e Release via `Build-Client.ps1 -Rebuild`
  passaram com 1942 checks/asserts; o Release instalado tem SHA-256
  `891463810FDDF12E959A6DE0C64DE0A751CF4E6D0F6FBE9EB6DAD1C2C05FDDC7`.
- Client real: ainda não executado.
