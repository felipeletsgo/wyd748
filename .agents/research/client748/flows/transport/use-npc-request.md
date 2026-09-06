---
id: use-npc-request
title: Interação com NPC 0x28B
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Interação com NPC 0x28B

## Pergunta

Qual frame o client 7.48 envia ao clicar ou confirmar a interação com um NPC?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado `exports/use-npc-flow.tsv`.
- Assets: `NÃO APLICÁVEL`; o contrato é um envelope de 20 bytes.
- Source atual: `TMFieldScene.cpp`, `Basedef.h` e `internal/wire/UseNPCPacket.h`.
- Servidor: `internal/game/handlers.go`, `security.go`, `world.go` e testes de interação com NPC.
- TMProject posterior/guias: `NÃO APLICÁVEL` para o ABI; W2PP, Secrets e Micronics foram excluídos da decisão.

## Fluxo nativo 7.48

`FUN_004640E5` possui um branch de clique com evento `0x0D`: zera 20 bytes,
grava `Type=0x28B`, o ID local, o NPC em `+12` e `ClickOk=0` em `+16`, então
chama `FUN_0055F2DD`. Os eventos `0x0F`, `0x0A`, `0x6CE`, `0xE9` e `0x3A`
reutilizam o frame, mas gravam `ClickOk=1` para a confirmação nativa.

### Callees e gate

- `FUN_0055F2DD` aplica o throttle e envia o frame.
- `FUN_0055890A` contém o caso `0x28B` e rejeita qualquer tamanho diferente de
  `0x14` (`0055890a_FUN_0055890a.c`, case 0x28B).

## Estado e lifecycle

| Evento/estado | Pré-condição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| clique NPC | NPC selecionado e cena ativa | `FUN_004640E5 -> FUN_0055F2DD` | servidor recebe intenção | `ClickOk=0` | alvo inválido é rejeitado pelo servidor |
| confirmação | caixa nativa aceita | mesmo caminho | servidor revalida o contexto | `ClickOk=1` | confirmação sem NPC válido não muta estado |
| servidor recebe | sessão, alcance e visibilidade válidos | `World.onUseNPC` | abre contexto autoritativo | loja/skill/craft/quest conforme allowlist | aviso e nenhum contexto em falha |
| logout/relogin | nova cena | lifecycle geral | NPC/contexto é reconstruído | frame antigo não é repetido | sem estado próprio do packet |

O buffer vive na pilha e é emprestado ao transporte. `TargetID` e `ClickOk`
são intenções; o servidor valida NPC, alcance, visibilidade, gold, itens e
estado antes de qualquer publicação.

## Wire, ABI e recursos

Direção C->S, opcode `0x28B`, exatamente 20 bytes:

| Offset | Campo | Tipo | Semântica |
| ---: | --- | --- | --- |
| `+0` | `Header` | `MSG_STANDARD` | envelope WYD 7.48 |
| `+12` | `TargetID` | `int32` | ID do NPC selecionado |
| `+16` | `ClickOk` | `int32` | `0` clique/abertura, `1` confirmação |

O nativo não envia resposta no mesmo frame. O servidor pode responder por
listas, avisos ou estado da feature conforme o tipo do NPC.

## Mapeamento atual

Os emissores de `TMFieldScene` que usavam `MSG_Quest_Opcode` e
`MSG_STANDARDPARM2` agora usam `MSG_UseNPC` e `MSG_UseNPC_Opcode`, sem alterar
os valores de `TargetID`, `ClickOk`, timers ou callbacks. O WYD-Go já exige
20 bytes e interpreta o mesmo par de campos antes da allowlist de NPC.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire | `0x28B/20`, `TargetID@12`, `ClickOk@16` | equivalente, antes fachada genérica | gate/handler equivalentes | extrair tipo dedicado |
| confirmação | branches `0` e `1` | valores preservados | allowlist revalida contexto | manter |
| autoridade | client só indica alvo | source não envia estado de NPC | servidor decide contexto | preservar |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT`.
- Criar `UseNPCPacket.h` com opcode, campos e asserts.
- Substituir somente a fachada genérica dos seis emissores, sem mudar semântica de NPC/quest.
- Não adicionar rota ao `ReceivedPacketDispatch`, pois `0x28B` é C->S.

## Lacunas

- Clique/confirmar NPC, abertura de cada contexto, recusa, troca de cena e relogin ainda não foram executados no `client748/project.exe`; não é `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra read-only usou hash nativo esperado; export registra os dois grupos de eventos, campos, envio e gate de tamanho.
- Automacao: fixture C++ verifica opcode, tamanho, offsets e `ClickOk`; `go test -count=1 ./...`, XML/caminhos e `git diff --check` passaram. Debug e Release via `Build-Client.ps1 -Rebuild` passaram com 1936 checks/asserts; o Release instalado tem SHA-256 `C7406A06EAA5C8CCA2FEC7BC8726DC646E11512957DEE4F431D56F4F2BE01FFD`.
- Client real: ainda não executado.
