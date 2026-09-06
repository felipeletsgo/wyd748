---
id: challenge-confirm-request
title: Confirmação de disputa de zona 0x28F
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Confirmação de disputa de zona 0x28F

## Pergunta

Qual frame o client 7.48 envia ao confirmar a seleção de uma disputa de zona?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado `exports/challenge-confirm-flow.tsv`.
- Assets: `NÃO APLICÁVEL`; o contrato é um envelope de 20 bytes.
- Source atual: `TMFieldScene.cpp`, `Basedef.h` e `internal/wire/ChallengeConfirmPacket.h`.
- Servidor: `internal/game/teleports.go`, `security.go`, `world.go` e testes de disputa.
- TMProject posterior/guias: `NÃO APLICÁVEL` para o ABI; W2PP, Secrets e Micronics foram excluídos da decisão.

## Fluxo nativo 7.48

No estado nativo de confirmação (`state=5`), `FUN_004662C5` zera um buffer de
20 bytes, grava `Type=0x28F`, o ID local, `Parm1` derivado do alvo salvo e
`Parm2` da seleção atual, e chama `FUN_0055F2DD`. O gate `FUN_0055890A`
confirma que `0x28F` exige `0x14` bytes.

### Callees e estado

- `FUN_0055F2DD` aplica o throttle e envia o frame.
- O callback `FUN_004662C5` é o receptor da seleção de controles da FieldScene;
  a confirmação é posterior ao alvo salvo na janela contextual.

## Estado e lifecycle

| Evento/estado | Pré-condição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| confirmação | alvo salvo e modo selecionado | `FUN_004662C5 -> FUN_0055F2DD` | servidor recebe intenção | menu/caixa segue fechamento existente | sem seleção não envia |
| servidor recebe | sessão autenticada, frame 20B | handler `OpChallengeConfirm` | regra de disputa decide | aviso/estado conforme TMSrv | emulador mantém resposta segura |
| logout/relogin | nova cena | lifecycle geral | seleção transitória é descartada | frame antigo não é repetido | sem estado próprio do packet |

O buffer vive na pilha e é emprestado ao transporte. Os dois parâmetros são
intenções; o servidor continua autoridade sobre a disputa.

## Wire, ABI e recursos

Direção C->S, opcode `0x28F`, exatamente 20 bytes:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | envelope WYD 7.48 |
| `+12` | `Parm1` | `int32` | alvo salvo pela cena |
| `+16` | `Parm2` | `int32` | modo/seleção; source atual envia zero |

## Mapeamento atual

`TMFieldScene::OnMsgBoxEvent` caso `60` agora usa `MSG_ChallengeConfirm` e
`MSG_ChallengeConfirm_Opcode`, substituindo o literal decimal `655` e a
fachada `MSG_STANDARDPARM2` sem alterar `m_dwTID` ou `Parm2=0`. O servidor já
exige 20 bytes e mantém resposta segura para o sistema de disputa ainda não
habilitado.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire | `0x28F/20`, `Parm1@12`, `Parm2@16` | equivalente, antes literal decimal | gate/handler equivalentes | extrair tipo dedicado |
| confirmação | estado 5 e alvo salvo | caso 60 preservado | resposta segura | manter |
| autoridade | client só indica parâmetros | source não decide resultado | servidor decide | preservar |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT` para wire/ABI.
- Criar `ChallengeConfirmPacket.h` com opcode, campos e asserts.
- Substituir somente o emissor do caso `60`; não alterar regra de disputa.
- Não adicionar rota ao `ReceivedPacketDispatch`, pois `0x28F` é C->S.

## Lacunas

- Confirmação real, resposta de disputa, troca de cena e relogin ainda não foram
  executados no `client748/project.exe`; não é `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra read-only usou hash nativo esperado; export registra estado,
  campos, envio e gate de tamanho.
- Automação: fixture C++ verifica opcode, tamanho, offsets e parâmetros; testes
  e builds serão registrados após a edição.
- Client real: ainda não executado.
