---
id: challenge-confirm-request
title: Confirmação de disputa de zona 0x28F
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-13
---

# Inscrição de disputa de zona 0x28E -> 0x18D -> 0x28F

## Pergunta

Qual é o contrato completo entre pedido, prompt nativo e confirmação da seleção
de uma disputa de zona no client 7.48?

## Fronteira de evidência

- Executável/hash: `references/client748/WYD.exe`, SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado `exports/challenge-confirm-flow.tsv`.
- Assets: `NÃO APLICÁVEL`; o contrato usa envelopes fixos de 16, 12 e 20 bytes.
- Source atual: `TMFieldScene.cpp`, `Basedef.h` e `internal/wire/ChallengeConfirmPacket.h`.
- Servidor: `internal/game/teleports.go`, `security.go`, `world.go` e testes de disputa.
- TMProject posterior/guias: `NÃO APLICÁVEL` para o ABI; fontes legadas externas foram excluídos da decisão.

## Fluxo nativo 7.48

O fluxo observável é:

1. client -> servidor: `0x28E`, 16 bytes, pedido para o alvo/NPC atual;
2. servidor -> client: `0x18D`, `MSG_STANDARD` exato de 12 bytes;
3. a FieldScene abre os controles nativos de seleção usando o alvo já salvo;
4. client -> servidor: `0x28F`, 20 bytes, com `Parm1@12` e `Parm2@16`.

### Callers

No estado nativo de confirmação (`state=5`), `FUN_004662C5` zera um buffer de
20 bytes, grava `Type=0x28F`, o ID local, `Parm1` derivado do alvo salvo e
`Parm2` da seleção atual, e chama `FUN_0055F2DD`. O gate `FUN_0055890A`
confirma que `0x28F` exige `0x14` bytes.

O prompt `0x18D` percorre o dispatcher recebido pela FieldScene como
`MSG_STANDARD` de 12 bytes. Ele não carrega a regra autoritativa da disputa:
serve para abrir a seleção contextual sobre o alvo que a cena já conhece.

### Callees

- `FUN_0055F2DD` aplica o throttle e envia o frame.
- O callback `FUN_004662C5` é o receptor da seleção de controles da FieldScene;
  a confirmação é posterior ao alvo salvo na janela contextual.
- `0x18D` entra pelo caminho de `ReceivedPacketDispatch`; `0x28F` não entra
  nesse dispatcher porque sua direção é C->S.

## Estado e lifecycle

| Evento/estado | Pré-condição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| pedido | NPC/alvo atual | C->S `0x28E/16B` | servidor valida interação | prepara contexto autoritativo | NPC/cidade inválidos rejeitam |
| prompt | contexto aceito | S->C `0x18D/12B` | FieldScene abre seleção nativa | alvo salvo é reutilizado | sem contexto não deve ser emitido |
| confirmação | alvo salvo e modo selecionado | `FUN_004662C5 -> FUN_0055F2DD`, C->S `0x28F/20B` | servidor recebe intenção | contexto é consumido uma vez | replay/TTL/dona divergente rejeitam |
| servidor recebe | sessão autenticada, contexto efêmero válido | handler `OpChallengeConfirm` | regra de disputa decide | taxa/registro dentro do commit | falha não deixa mutação parcial |
| logout/relogin | nova cena | lifecycle geral | seleção transitória é descartada | frame antigo não é repetido | sem estado próprio do packet |

O buffer do client vive na pilha e é emprestado ao transporte. Os parâmetros
são intenções; o servidor mantém o estado efêmero de NPC/cidade/guilda dona e
TTL e continua autoridade sobre a disputa.

## Wire, ABI e recursos

Contratos do fluxo:

- C->S `0x28E`: 16 bytes;
- S->C `0x18D`: `MSG_STANDARD`, exatamente 12 bytes;
- C->S `0x28F`: exatamente 20 bytes.

Layout de `0x28F`:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | envelope WYD 7.48 |
| `+12` | `Parm1` | `int32` | alvo salvo pela cena |
| `+16` | `Parm2` | `int32` | modo/seleção; source atual envia zero |

## Mapeamento atual

`TMFieldScene::OnMsgBoxEvent` caso `60` agora usa `MSG_ChallengeConfirm` e
`MSG_ChallengeConfirm_Opcode`, substituindo o literal decimal `655` e a
fachada `MSG_STANDARDPARM2` sem alterar `m_dwTID` ou `Parm2=0`. O servidor já
exige os tamanhos exatos e implementa o handshake completo: `0x28E` valida o
coletor e cria contexto efêmero; `GuildChallengePrompt()` emite `0x18D/12B`;
`0x28F` revalida NPC/cidade/dona/TTL, consome o contexto uma única vez e aplica
taxa/registro pelo caminho autoritativo. Replay, expiração, troca de dona ou
interação com outro NPC não reutilizam o contexto anterior. A revisão de
fechamento corrigiu rejeições antecipadas no handler (opção inválida, alvo
inválido ou fora de alcance) que ainda preservavam o contexto: agora o snapshot
é consumido na entrada, e só um novo pedido validado estabelece outro contexto.
Isso é `MODERNIZACAO_COMPATIVEL` interna do servidor, sem alterar o ABI nativo.

Nos dados ativos foram comprovados Balmus/Armia, Kara/Azran e Empis/Erion.
Não há coletor de Nippleheim atualmente, portanto nenhum NPC foi sintetizado.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| pedido/prompt | `0x28E/16` -> `0x18D/12` | dispatcher da FieldScene preservado | valida e emite prompt exato | preservar |
| confirmação | `0x28F/20`, `Parm1@12`, `Parm2@16`, estado 5 e alvo salvo | caso 60 preservado | gate/handler equivalentes | manter |
| autoridade | client só indica parâmetros | source não decide resultado | contexto efêmero + servidor decide | preservar |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT` para wire/ABI.
- Criar `ChallengeConfirmPacket.h` com opcode, campos e asserts.
- Preservar o emissor do caso `60` e a seleção nativa da FieldScene.
- Manter `0x18D` na rota recebida por `ReceivedPacketDispatch`; não adicionar
  `0x28F` nessa rota porque ele é C->S.
- No servidor, aceitar confirmação somente contra contexto autoritativo ainda
  válido e consumi-lo independentemente de sucesso/rejeição final.

## Lacunas

- O fluxo ainda não foi exercitado ponta a ponta no
  `tmproject/client748/project.exe` com dois clients/guildas; não é
  `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra read-only usou hash nativo esperado; export registra estado,
  campos, prompt, envio e gates de tamanho.
- Automação: fixture C++ verifica opcode, tamanho, offsets e parâmetros; testes
  Go exercitam `0x28E -> 0x18D -> 0x28F`, replay, TTL, troca de dona e troca de
  NPC, além de rejeição seguida de replay e recuperação com novo pedido.
  `ArchitectureTests` Release/Win32 passou 35.457 checks e o client
  Release/x86 compilou com `-NoDeploy`. Isso mantém o estado em `CONTRACT`, não
  `CLIENT_TESTED`.
- Client real: ainda não executado.
