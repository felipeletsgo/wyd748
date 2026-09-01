---
id: premium-npc-click-dormant
title: Clique em NPC Premium sem contrato recuperado
subsystem: ui-gameplay
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Clique em NPC Premium sem contrato recuperado

## Pergunta

O stub `TMFieldScene::MouseClick_PremiumNPC()` deve construir uma intenção de
rede, ou permanecer como ponto de extensão até o handler real ser comprovado?

## Fronteira de evidência

- Executável/hash: baseline identificada pelo SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `FUN_00516105 @ 0x00516105`.
- Assets: nenhuma janela ou binding Premium foi atribuído nesta investigação.
- Source atual: caller no clique de NPC da Field e stub
  `MouseClick_PremiumNPC()` em `TMFieldScene.cpp`.
- Servidor: nenhum opcode Premium foi ligado a esse stub.

## Fluxo nativo 7.48

### Entrada observável

A source intercepta humano com `ID >= 1000` e cabeça `57`, chama o stub e
consome o clique para impedir que o NPC siga ao fluxo de ataque.

### Callers

O caller source está no dispatcher de clique da Field. O caller nativo
específico de uma ação Premium permanece `UNRESOLVED`.

### Função principal

`FUN_00516105` reconhece como merchant as cabeças `54`, `55`, `56`, `57`,
`51`, `68` e `67`, além dos tipos Merchant `1..14`. A função classifica a
entidade; não constrói request Premium e não é equivalente ao stub.

### Callees

O stub source e `FUN_00516105` não fornecem callee de rede para a feature.

### Saídas e erros

Sem contrato comprovado, o clique é consumido sem envio. Isso preserva a
proteção contra ataque ao merchant sem inventar operação Premium.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| clicar cabeça 57 | NPC `ID >= 1000` | stub Premium | cena inalterada | clique consumido | sem wire |

### Vtables, vptrs e receptores

Nenhum receptor ou callback Premium foi resolvido. A classificação Merchant
opera sobre o objeto humano existente.

### Ownership

Não há alocação nem transferência de ownership.

### Falha parcial

Não aplicável: nenhuma operação é iniciada.

### Cleanup e teardown

Não há estado Premium para limpar.

### Shutdown

Não há ação específica.

### Logout e relogin

Não há estado persistente ou callback atravessando a cena.

## Wire, ABI e recursos

Opcode, direção, tamanho, campos, resposta e recurso de UI permanecem
`UNRESOLVED`. Nenhum layout posterior será copiado por inferência.

## Mapeamento atual

### Source recompilável

O stub é preservado com comentário explícito. O caller continua consumindo o
clique da cabeça `57`, mantendo a entidade fora do ataque comum.

### WYD-Go

Não afetado; nenhum handler foi criado.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject/W2PP | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| cabeça 57 é merchant | `FUN_00516105` confirma | caller trata como NPC | compatível | N/A | manter classificação |
| ação Premium | não localizada | stub vazio | W2PP também vazio | ausente | não implementar por inferência |
| clique consumido | proteção provável | confirmado na source | preservado | N/A | manter |

## Decisões

- Preservar o ponto de extensão e o bloqueio do fluxo de ataque.
- Não atribuir semântica de handler a `FUN_00516105`.
- Exigir evidência nativa adicional para paridade ou contrato client/server
  explícito para uma futura extensão Premium.

## Lacunas

- Resolver no Ghidra se existe dispatcher específico para cabeça `57`.
- Se a feature for desejada, definir UI, opcode, request/response, autoridade,
  rejeição, teardown e relogin nos dois lados.

## Validação

- Pesquisa: classificação Merchant confirmada em `FUN_00516105`; handler
  Premium permanece não localizado.
- Automação: `validate_research.py` e `git diff --check` aprovados; build
  Release/Win32 aprovado com zero erros e 13 warnings C4018 preexistentes.
- Candidato: `client748/project.exe`, SHA-256
  `87431F0B066FD782CE1231F1E76C1905671E8C0D2C23E9CA75EDD4DC351F9979`.
- Client real: não executado; nenhum claim `CLIENT_TESTED`.
