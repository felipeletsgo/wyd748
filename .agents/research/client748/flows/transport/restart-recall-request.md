---
id: restart-recall-request
title: Pedido de renascimento/recall 0x289
subsystem: combat-progression
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

# Pedido de renascimento/recall 0x289

## Pergunta

Qual frame o client 7.48 envia para renascer depois da morte e em quais
transições o mesmo opcode é usado pelo recall periódico?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; export focado
  `exports/restart-recall-flow.tsv`, SHA-256
  `BC97BBAD136A733A391F9E1732D26AC1EBEBE95A2EA2045D29AC896DDBFAA6CF`.
- Assets: `NÃO APLICÁVEL`; o frame só contém header.
- Source atual: `TMFieldScene.cpp` e `Basedef.h` em
  `client-source/tmproject/TMProject748/`.
- Servidor: `internal/game/world.go`, `internal/game/handlers.go`,
  `internal/game/security.go` e testes de morte/restart.
- TMProject posterior e guias: `NÃO APLICÁVEL`; W2PP, Secrets e Micronics
  permaneceram excluídos.

## Fluxo nativo 7.48

### Entrada observável

- Evento: personagem morto recebe comando de renascimento, ou recall/retorno
  periódico decide voltar ao ponto seguro.
- Precondições: estado de morte/recall vencido, personagem e socket ativos;
  o client não inventa posição nem score no frame.
- Saída observável: `0x289/12` com o ID do personagem no header; a regra de
  renascimento é autoritativa no servidor.

### Callers

- `FUN_00476006` envia `0x289/12` após janela de cinco segundos no fluxo de
  morte/recall e limpa os flags locais relacionados.
- `FUN_004776C3` envia o mesmo frame no tick de manutenção após a janela de
  morte prolongada, também com 12 bytes zerados antes do opcode.
- O gate `FUN_0055890A` aceita `0x289` somente com 12 bytes.

### Função principal e erros

As duas funções zeram um buffer de 12 bytes, gravam o `WORD` do servidor em
`Header.ID`, `Type=0x289` e chamam `FUN_0055F2DD`. Nenhum payload adicional é
transmitido. Estado inválido, timer ainda ativo ou personagem em condição não
recallável não gera o frame.

### Callees

- `FUN_0055F2DD` enquadra e envia o buffer de 12 bytes.
- `FUN_0055890A` valida o tamanho exato do opcode.
- Os helpers de UI/efeito chamados depois do envio não alteram o wire.

## Estado e lifecycle

| Evento | Precondição | Função/call | Resultado | Erro/saída |
| --- | --- | --- | --- | --- |
| renascimento | personagem morto e cooldown aceito | `FUN_00476006` | envia request | timer ainda ativo não envia |
| recall periódico | morte/recall vencido | `FUN_004776C3` | envia o mesmo request | flag ausente não envia |
| servidor recebe | frame 12B e player autenticado | `onRestart` | `recallPlayer` autoritativo | HP não zero, sessão inválida ou cooldown rejeita |
| logout/relogin | nova FieldScene | lifecycle geral | nenhum request antigo é repetido | estado é reconstruído pelo servidor |

Ownership é apenas o buffer de stack emprestado ao transporte; não há recurso,
timer ou estado persistente próprio do packet. O teardown usa o lifecycle geral
de FieldScene e sessão.

## Wire, ABI e recursos

Direção C->S, opcode `0x289`, tamanho exato 12 bytes: somente `MSG_STANDARD`.
`Header.ID` identifica o personagem local; `Header.Type` é `MSG_Recall_Opcode`.
O gate nativo e `security.go` coincidem. Não há rota S->C nem payload de
posição, HP, EXP ou score.

## Mapeamento atual

`MSG_Recall_Opcode` já existia em `Basedef.h`; os emissores usam um literal e o
símbolo. WYD-Go já valida 12 bytes e `onRestart` chama `recallPlayer`, que
revalida morte, cooldown, espaço de gameplay e persistência.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| wire `0x289` | header de 12B | layout equivalente | gate/handler equivalentes | extrair opcode/header |
| dois emissores | recall e manutenção usam o mesmo frame | caminhos preservados | um handler autoritativo | manter |
| autoridade | client só pede | source não envia estado | servidor decide renascimento | preservar |

## Decisões

- Classificar como `PARIDADE_NATIVA/CONTRACT`.
- Criar `RestartRecallPacket.h` como dono do opcode e do header de 12 bytes.
- Trocar o literal restante no emissor, sem alterar timers ou lifecycle.
- Não adicionar rota ao dispatcher recebido.

## Lacunas

- O fluxo de morte, cooldown, recall, troca de cena e relogin ainda não foi
  executado no `project.exe`.

## Validação
- Pesquisa: export Ghidra read-only terminou sem `SCRIPT ERROR`, hash esperado e
  registros de função/caller/callee para os dois emissores e o gate.
- Automação: `go test -count=1 ./...` passou; Debug e Release via
  `Build-Client.ps1` passaram com 1928 checks/asserts. A fixture cobre opcode,
  tamanho e ID no header. Release instalado com SHA-256
  `6EEB158C59829A8D65E41F8CE6962B62CBAB084B2AF56B9BAB32B1079D15F35E`.
- Client real: não executado; não `CLIENT_TESTED`.
