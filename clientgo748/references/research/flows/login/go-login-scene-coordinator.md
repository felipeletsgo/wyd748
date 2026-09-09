---
id: go-login-scene-coordinator
title: Coordenador Go de login e cenas 7.48
subsystem: login
status: CONTRACT
mode: MODERNIZACAO_COMPATIVEL
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Coordenador Go de login e cenas 7.48

## Pergunta

Compor o contrato nativo de login já documentado com o lifecycle de cenas Go,
sem duplicar o consumidor de packets ou permitir que uma tela altere o estado
autoritativo.

## Fronteira de evidência

O contrato de packets e fases vem da ficha `login-session.md`; a fronteira de
cenas usa `FUN_004B3500`, `FUN_004B37C9` e `FUN_0055D066` documentadas no
lifecycle nativo. Este lote adapta somente a composição interna em Go.

## Matriz de fontes

| Fonte | Estado | Uso |
| --- | --- | --- |
| Binário nativo 7.48 / Ghidra | UTILIZADA | fichas `login-session.md` e lifecycle fornecem contratos e invariantes |
| Source Go e testes | UTILIZADA | `internal/login`, `internal/loginflow`, `internal/app`, `internal/scene` |
| WYD-Go | UTILIZADA | contrato de sessão e transporte existente; nenhum código alterado |
| Assets 7.48 | NÃO APLICÁVEL | cenas usam apenas primitivas de depuração; nenhum asset visual novo foi ligado |
| TMProject | UTILIZADA secundariamente | comparação estrutural, sem cópia de código ou ABI |
| Guias | NÃO APLICÁVEL | não definem este contrato |
| W2PP, Secrets, Micronics | EXCLUÍDA | fontes bugadas, proibidas pela campanha |

## Fluxo nativo 7.48

Os callers nativos são o ObjectManager e as rotinas de teardown citadas acima;
os callees são as factories/objetos de cena e seus destructors. O client Go não
reutiliza endereços, vtables ou offsets.

### Callers

`Application.Run` chama o coordenador por callbacks de sessão e por frame.

### Callees

`Coordinator` chama `login.Controller`, `login.Dispatcher` e `Navigator`.

## Estado e lifecycle

### Entrada observável

Conexão, packets de login e desconexão são entregues à thread principal.

### Matriz de transições

`Application` conecta a sessão e chama `Coordinator.SessionConnected`, que move
`SessionState` para `Connecting`. O `Controller` é o único emissor de
intenções; o `Dispatcher` é o único consumidor dos packets recebidos. Depois de
`PollEvents`, a aplicação drena e entrega os eventos ao coordenador, despacha o
input e chama `Synchronize`. A sincronização observa a fase final do frame e
agenda no máximo uma transição no `scene.Manager`.

| Fase | Cena lógica |
| --- | --- |
| `Disconnected`, `Connecting`, `Authenticating` | `Login` |
| `CharacterSelect` | `CharacterSelect` |
| `EnteringWorld` | `Loading` |
| `InWorld`, `LoggingOut` | `World` |

As cenas lógicas não possuem socket, renderer ou recursos externos. `Enter` e
`Update` validam novamente a fase aceita, detectando dessincronização antes de
renderizar. Desconexão limpa o estado; logout permanece em `World` enquanto a
confirmação não chega. Não há wire/ABI novo.

### Vtables, vptrs e receptores

N/A: a API Go não expõe nem copia vtables nativas.

### Ownership

`SessionState` pertence ao coordenador; `Application` possui o manager e as
instâncias de cena. Nenhuma cena possui socket ou renderer.

### Falha parcial

Falha do controller mantém rollback do estado; falha do navigator é propagada
sem mutar a fase.

### Cleanup e teardown

O fechamento da aplicação encerra sessão, manager e recursos gráficos em ordem;
as cenas recebem `Exit` e `Close` uma vez.

### Shutdown

`SessionDisconnected` limpa a prova de sessão e a sincronização retorna a Login.

### Logout e relogin

`LoggingOut` permanece em World até `0x116`; a confirmação retorna a
CharacterSelect e uma nova conexão reinicia em Login.

## Wire, ABI e recursos

N/A: nenhum opcode, layout, ABI ou asset novo foi criado; os packets são os
contratos existentes de `internal/login`.

## Mapeamento atual

Source: `internal/loginflow/coordinator.go`, `scenes.go`, `internal/ui` e
`internal/app/application.go`. Nativo: fichas `login-session.md` e
`go-scene-manager.md`.

## Matriz de delta

| Claim | Nativo 7.48 | Source Go | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| fase dirige tela | ObjectManager troca objetos | Coordinator mapeia fases para IDs | comparação estrutural | contrato de sessão | modernizar internamente |
| consumidor de packets | dispatch central | Dispatcher único | comparação secundária | server-authoritative | manter |
| troca e cleanup | FUN_004B3500/FUN_004B37C9/FUN_0055D066 | Manager com transição pendente | não é prova nativa | N/A | manter invariantes |

## Decisões

- Classificação: `MODERNIZACAO_COMPATIVEL`.
- Implementar cenas lógicas sem copiar código, ABI ou ownership do TMProject.
- As cenas visuais de seleção, carregamento e mundo são uma modernização
  interna compatível: consomem somente snapshots já aceitos e não alteram
  packets, fases ou ownership do renderer.
- A seleção ignora slots vazios, percorre os quatro slots sem pular ocupados e
  só chama o controller depois de uma confirmação válida.
- Não declarar `CLIENT_TESTED` até executar login/logout/relogin no executável.

## Lacunas

- validar conexão e ciclo completo contra o servidor WYD-Go;
- adicionar fonte de texto/fontes e rastrear recursos visuais no client real;
- substituir os retângulos de depuração por assets protegidos depois que o
  contrato de recursos estiver fechado;
- validar reconexão e logout/relogin no executável Windows.

## Validação

- `go test -count=1 ./...`: aprovado.
- `go test -race -count=1 ./internal/protocol ./internal/app ./internal/login ./internal/loginflow`: aprovado.
- Build Debug do executável Go: aprovado.
- Client real: pendente.

## Estado de validação

- `AUTOMATED TESTED`: `go test -count=1 ./...` e teste de ordem do frame.
- `AUTOMATED TESTED`: `go test -race` nos pacotes protocol/app/login/loginflow.
- `AUTOMATED TESTED`: controles da cena de login (foco, edição, validação,
  envio único, rollback de erro e limpeza da senha) em `internal/ui`.
- `CLIENT_TESTED`: pendente; o executável real ainda não percorreu login,
  seleção, mundo, logout e relogin.
