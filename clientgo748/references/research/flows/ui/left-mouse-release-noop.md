---
id: left-mouse-release-noop
title: Liberação do botão esquerdo sem dispatch no client 7.48
subsystem: ui-input
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Liberação do botão esquerdo sem dispatch no client 7.48

## Pergunta

O corpo vazio de `EventTranslator::OnLMouseReleased()` é uma lacuna ou o
comportamento do input nativo 7.48?

## Fronteira de evidência

- Executável/hash: baseline identificada pelo SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `FUN_004AEC3D`, `FUN_004AF854` e `FUN_004AF910` no
  corpus `GhidraAnalysis/20260821/decompiled`.
- Assets: não aplicável; a transição precede o dispatch da árvore de controles.
- Source atual: `EventTranslator::ReadInputEventData()`,
  `OnLMousePressed()` e `OnLMouseReleased()` em `EventTranslator.cpp`.
- Servidor: não há packet ou mutação server-side nessa borda.

## Fluxo nativo 7.48

### Entrada observável

- Evento: o polling DirectInput observa botão esquerdo solto após frame
  anterior pressionado.
- Precondições: estado atual zero e estado anterior diferente de zero.
- Saída: retorna ao polling sem emitir evento de cena.

### Callers

`CONFIRMED`: `FUN_004AEC3D` chama `FUN_004AF854` enquanto o botão está
pressionado e `FUN_004AF910` somente na borda pressionado para solto.

### Função principal

`CONFIRMED`: `FUN_004AF910 @ 0x004AF910` retorna imediatamente, sem ler ou
escrever estado.

### Callees

`CONFIRMED`: `FUN_004AF910` não possui callee. Em contraste,
`FUN_004AF854` valida o cursor e encaminha `WM_LBUTTONDOWN (0x201)` à cena.

### Saídas e erros

Não há branch de erro, alocação, callback ou dispatch de `WM_LBUTTONUP`. O
poller apenas publica o estado atual para o próximo frame.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| esquerdo pressionado | botão atual ativo | `FUN_004AF854` | pressionado | `WM_LBUTTONDOWN` | retorna ao poller |
| esquerdo liberado | atual zero, anterior ativo | `FUN_004AF910` | liberado | nenhum | retorna ao poller |

### Vtables, vptrs e receptores

O press usa a chamada virtual da cena atual; o release não consulta receptor,
vtable ou vptr.

### Ownership

O estado dos botões pertence ao tradutor de input. O no-op não adquire objeto.

### Falha parcial

Não aplicável: o release não inicia operação.

### Cleanup e teardown

Não há recurso próprio a limpar; o estado anterior é atualizado pelo poller.

### Shutdown

Não há ação específica no shutdown.

### Logout e relogin

Não há estado persistente do release atravessando a reconstrução da cena.

## Wire, ABI e recursos

Não há wire, packet, ABI de recurso ou ID de controle. `0x201` pertence apenas
ao press; adicionar `0x202` por simetria mudaria o contrato observado.

## Mapeamento atual

### Source recompilável

`ReadInputEventData()` detecta a mesma borda e chama
`OnLMouseReleased()`. O corpo permanece vazio com comentário de paridade;
`OnLMousePressed()` continua emitindo `513/WM_LBUTTONDOWN`.

### WYD-Go

Não afetado.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| press esquerdo | dispatch `0x201` | dispatch `513` | implementado | N/A | manter |
| release esquerdo | retorno puro | corpo vazio | stub vazio | N/A | manter no-op |
| evento simétrico `0x202` | não emitido aqui | não emitido | não implementar | N/A | rejeitar inferência |

## Decisões

- Classificar o corpo vazio como `PARIDADE_NATIVA` com evidência estática.
- Manter o no-op e documentar o motivo junto à implementação.
- Não promover todo o lifecycle de input a `TRACED`; esta ficha decide apenas
  a borda de release.

## Lacunas

Nenhuma lacuna bloqueia o no-op. Claims sobre drag, captura de controle ou
outros releases pertencem aos fluxos específicos desses controles.

## Validação

- Pesquisa: branches de press/release e os dois callees conferidos no corpus.
- Automação: `validate_research.py` e `git diff --check` aprovados; build
  Release/Win32 aprovado com zero erros e 13 warnings C4018 preexistentes.
- Candidato: `client748/project.exe`, SHA-256
  `87431F0B066FD782CE1231F1E76C1905671E8C0D2C23E9CA75EDD4DC351F9979`.
- Client real: não executado; nenhum claim `CLIENT_TESTED`.
