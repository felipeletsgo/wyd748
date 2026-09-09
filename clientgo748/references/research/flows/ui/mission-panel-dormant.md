---
id: mission-panel-dormant
title: Clique de confirmação do painel Mission sem contrato ativo
subsystem: ui
status: UNMAPPED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Clique de confirmação do painel Mission sem contrato ativo

## Pergunta

O painel posterior `CMission` pode enviar seu payload usando o opcode nativo
Tiny `0x3C0`, ou deve permanecer preservado e dormente até possuir contrato
client/server próprio?

## Fronteira de evidência

- Executável/hash: baseline identificada pelo SHA-256 do frontmatter.
- Projeto/corpus Ghidra: contrato legado de combine e gate de tamanho já
  registrados em `flows/transport/packet-size-gate.md`; busca focal não
  localizou os IDs modernos do painel Mission no corpus nativo.
- Fronteira nativa interceptada: `FUN_0055890A @ 0x0055890A` registra o gate
  legado de opcode/tamanho; não é uma implementação de Mission.
- Assets/source: painel `86017`, botão `86019`, resultados `86020..86043` e
  necessidades `86077..86084`; `CMission`, `SetVisibleMissionPanel()` e o
  dispatcher de controles em `TMFieldScene.cpp`.
- Servidor: `internal/wire/opcodes.go` reserva `0x3C0` para
  `OpCombineTiny`; handlers de combine consomem o formato legado.

## Fluxo nativo 7.48

### Entrada observável

O fluxo legado relevante é a intenção Tiny `0x3C0`; ele não é uma entrada de
Mission. A UI Mission e seu botão são posteriores e não possuem transição
nativa 7.48 confirmada.

### Callers

Não aplicável à extensão Mission. O dispatcher da source possui o case
`86019`; não foi atribuído caller nativo a esse ID.

### Função principal

`CMission::DoCombine()` é um stub vazio na source atual e na referência W2PP.
Nenhum construtor de packet Mission foi implementado.

### Callees

O stub não chama callee. Antes deste lote, o dispatcher fechava o painel após a
chamada vazia, produzindo aparência de confirmação sem operação correspondente.

### Saídas e erros

A decisão segura é não enviar e não fechar o painel no clique de confirmação.
Isso impede sucesso visual falso e colisão com o protocolo Tiny.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| abrir Mission | painel posterior materializado | `SetVisibleMissionPanel(1)` | visível | staging local | sem wire |
| clicar `86019` | contrato ausente | dispatcher | permanece visível | nenhum | no-op explícito |
| fechar/cancelar | painel visível | lifecycle existente | oculto | limpa staging | sem wire |

### Vtables, vptrs e receptores

Os controles pertencem ao `SControlContainer` da Field. Não há callback nativo
ou receptor de packet Mission comprovado.

### Ownership

O container continua dono dos controles; `CMission` mantém ponteiros não
possuidores e estado de staging.

### Falha parcial

Sem envio não existe mutação server-side para desfazer. O clique bloqueado não
descarta o staging nem simula sucesso.

### Cleanup e teardown

O fechamento existente chama `ClearMissionPannel()`, restaura cores/grids e
zera o opcode. Nenhum recurso novo é alocado neste lote.

### Shutdown

O teardown continua pertencendo à destruição da árvore da Field.

### Logout e relogin

Não há packet ou persistência Mission. A nova cena começa com a estrutura
zerada por `TakeItResource()`.

## Wire, ABI e recursos

- `MSG_CombineItem`: 84 bytes, oito itens e oito posições Carry.
- `MSG_Mission`: 88 bytes, pois acrescenta `MissionNo` ao mesmo conteúdo.
- `0x3C0`: contrato C para S nativo Tiny de 84 bytes, confirmado na source,
  `WYD748Compat.cpp` e `internal/wire`.
- O painel Mission não possui opcode, direção, resposta, validação, rollback ou
  capability definidos. Portanto ainda não é uma extensão coordenada ativa.

## Mapeamento atual

### Source recompilável

O código e os controles manuais de `CMission` são preservados. O tipo do header
permanece zero após cleanup, o case `86019` é no-op explícito e um
`static_assert` mantém o payload de 88 bytes distinto do combine legado.

### WYD-Go

Não existe handler Mission. `0x3C0` permanece exclusivamente Tiny.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| painel Mission | não confirmado | controles preservados | extensão posterior incompleta | ausente | manter dormente |
| payload | Tiny tem 84 bytes | Mission tem 88 bytes | formatos distintos | espera Tiny em `0x3C0` | não aliasar |
| confirmação `86019` | sem equivalente | chamava stub e fechava | sucesso visual falso | sem handler | tornar no-op |
| evolução futura | N/A | ponto preservado | arquitetura reutilizável | editável | exigir contrato coordenado |

## Decisões

- Classificar Mission como extensão posterior preservada, ainda não
  materializada como `EXTENSAO_COORDENADA`.
- Rejeitar a associação Mission para Tiny `0x3C0`.
- Não remover a UI nem inventar packet; manter o botão indisponível por no-op
  até formato, opcode, validação, resposta, rollback e relogin serem definidos.

## Lacunas

- Definir a regra funcional desejada para Mission.
- Projetar opcode sem colisão, request/response, validação autoritativa,
  persistência, rejeição, rollback e capability/fallback.
- Materializar e testar sucesso, cancelamento, teardown e relogin nos dois
  lados antes de ativar `86019`.

## Validação

- Pesquisa: colisão de opcode e diferença 84/88 conferidas na source e no
  servidor; ausência dos IDs é busca focal, não prova de inexistência binária.
- Automação: `validate_research.py` e `git diff --check` aprovados; build
  Release/Win32 aprovado com zero erros e 13 warnings C4018 preexistentes.
- Candidato: `client748/project.exe`, SHA-256
  `87431F0B066FD782CE1231F1E76C1905671E8C0D2C23E9CA75EDD4DC351F9979`.
- Client real: não executado; Mission continua indisponível.
