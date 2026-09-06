---
id: guild-deprivate-request
title: Remoção de membro da guild 0x28C
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Remoção de membro da guild 0x28C

## Pergunta

Qual frame o client 7.48 envia ao confirmar a expulsão de um membro da guild?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado `exports/guild-deprivate-flow.tsv`.
- Assets: `NÃO APLICÁVEL`; o contrato é um envelope de 16 bytes.
- Source atual: `TMFieldScene.cpp`, `Basedef.h` e `internal/wire/GuildDeprivatePacket.h`.
- Servidor: `internal/game/guild.go`, `security.go`, `world.go` e testes de guild.
- TMProject posterior/guias: `NÃO APLICÁVEL` para o ABI; W2PP, Secrets e Micronics foram excluídos da decisão.

## Fluxo nativo 7.48

No evento de confirmação `0x330`, `FUN_004640E5` zera 16 bytes, grava
`Type=0x28C`, o ID local e o alvo em `+12`, então chama `FUN_0055F2DD`.
`FUN_0055890A` valida o opcode somente com tamanho `0x10`.

## Estado e lifecycle

| Evento/estado | Pré-condição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| confirmar expulsão | líder confirma alvo na caixa nativa | `FUN_004640E5 -> FUN_0055F2DD` | servidor recebe intenção | caixa segue lifecycle existente | alvo inválido não é enviado pelo servidor |
| servidor recebe | sessão autenticada, guild e cargo válidos | `World.onGuildDeprivate` | membro é removido/persistido | publicação aos envolvidos | rejeição sem mutação parcial |
| logout/relogin | nova sessão | lifecycle geral | guild é reconstruída | request antigo não é repetido | sem estado próprio do packet |

O buffer vive na pilha e é emprestado ao transporte. `TargetID` é intenção; o
servidor valida líder, alvo, rank e persistência antes de publicar.

## Wire, ABI e recursos

Direção C->S, opcode `0x28C`, exatamente 16 bytes:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | envelope WYD 7.48 |
| `+12` | `TargetID` | `int32` | cliente ID do membro alvo |

Não há payload adicional nem resposta específica no frame.

## Mapeamento atual

`TMFieldScene::OnMsgBoxEvent` caso `816` agora usa `MSG_GuildDeprivate` e
`MSG_GuildDeprivate_Opcode`, substituindo o literal decimal `652` e a fachada
genérica `MSG_STANDARDPARM` sem mudar alvo, caixa ou callback. O WYD-Go já exige
16 bytes e mantém a autoridade da guild no servidor.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire | `0x28C/16`, `TargetID@12` | equivalente, antes literal decimal | gate/handler equivalentes | extrair tipo dedicado |
| autoridade | client só indica alvo | source não envia estado de guild | servidor decide rank/alvo/persistência | preservar |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT`.
- Criar `GuildDeprivatePacket.h` com opcode, campo e asserts.
- Substituir somente o emissor do caso `816`.
- Não adicionar rota ao `ReceivedPacketDispatch`, pois `0x28C` é C->S.

## Lacunas

- Expulsão real, rejeição por cargo, publicação, troca de cena e relogin ainda
  não foram executados no `client748/project.exe`; não é `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra read-only usou hash nativo esperado; export registra branch,
  campos, envio e gate de tamanho.
- Automação: fixture C++ verifica opcode, tamanho, offset, alvo e bytes
  reservados; testes e builds serão registrados após a edição.
- Client real: ainda não executado.
