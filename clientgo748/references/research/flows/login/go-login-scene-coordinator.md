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
| Assets 7.48 | UTILIZADA | `UI/loginbox2.wyt` e `UI/ServerList2.wyt` oficiais fornecem as skins modernas do login e da seleção; `SelServerScene2.bin` é a composição nativa da cena |
| TMProject | UTILIZADA secundariamente e autorizada | `TMSelectServerScene::InitializeScene` foi consultado para a sequência servidor -> login e para a composição viewport do painel/logos; usado somente para comparação de semântica, sem cópia de código, ABI ou ownership |
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

Na cena de seleção, confirmar `CONNECT` apenas grava o endpoint e a prova local
`serverSelected`; a sessão continua `Disconnected` enquanto a cena de login é
revelada. Isso reproduz a ordem observada em
`TMSelectServerScene::HandleEvent`: `B_SERVER_SEL_OK` oculta a lista e mostra o
painel, enquanto somente `B_LOGIN_OK`, depois de validar o formulário, chama
`ConnectServer` e envia `MSG_AccountLogin` (`0x20D`). Na implementação Go,
`Authenticate` chama `Application.ConnectSession` nesse mesmo ponto; a
conexão então chama `Coordinator.SessionConnected`, que move `SessionState` para
`Connecting`. O `Controller` é o único emissor de
intenções; o `Dispatcher` é o único consumidor dos packets recebidos. Depois de
`PollEvents`, a aplicação drena e entrega os eventos ao coordenador, despacha o
input e chama `Synchronize`. A sincronização observa a fase final do frame e
agenda no máximo uma transição no `scene.Manager`.

| Fase | Cena lógica |
| --- | --- |
| `Disconnected`, sem servidor confirmado | `ServerSelection` |
| `Disconnected`, servidor confirmado | `Login` |
| `Connecting`, `Authenticating` | `Login` |
| `CharacterSelect` | `CharacterSelect` |
| `EnteringWorld` | `Loading` |
| `InWorld`, `LoggingOut` | `World` |

As cenas lógicas não possuem socket nem ownership de renderer; as cenas de
seleção e login recebem explicitamente texturas oficiais imutáveis e as
apresentam pelo backend. A seleção chama apenas o callback de confirmação do
endpoint; o callback de autenticação aplica a conexão depois que o formulário
foi submetido. `serverSelected` é limpo na desconexão, impedindo que um estado
antigo pule novamente a tela de seleção.
`Enter` e
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
`internal/app/application.go`. Assets: `CLIENT OFICIAL 7.48/UI/loginbox2.wyt`
e `CLIENT OFICIAL 7.48/UI/ServerList2.wyt`.
Nativo: fichas `login-session.md` e `go-scene-manager.md`.

## Matriz de delta

| Claim | Nativo 7.48 | Source Go | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| fase dirige tela | ObjectManager troca objetos | Coordinator mapeia fases para IDs | fluxo compatível de login/personagem/mundo, usado como comparação | contrato de sessão | modernizar internamente |
| composição da tela de login | painel centralizado na viewport; logos ancorados em `screenWidth/2` com ajustes nativos de altura | painel e logos renderizados por `loginLayoutFor` | `TMSelectServerScene::InitializeScene` fornece a geometria candidata compatível | assets oficiais 7.48 | modernizar internamente preservando a composição observável |
| consumidor de packets | dispatch central | Dispatcher único | comparação secundária | server-authoritative | manter |
| troca e cleanup | FUN_004B3500/FUN_004B37C9/FUN_0055D066 | Manager com transição pendente | não é prova nativa | N/A | manter invariantes |

## Decisões

- Classificação: `MODERNIZACAO_COMPATIVEL`.
- Implementar cenas lógicas sem copiar código, ABI ou ownership do TMProject.
- A cena visual de login é uma modernização compatível baseada no asset oficial
  7.48; o TMProject auxilia somente na composição compatível do fluxo. As cenas
  de seleção, carregamento e mundo continuam esqueletos visuais e consomem
  somente snapshots já aceitos, sem alterar packets, fases ou ownership do
  renderer.
- A seleção ignora slots vazios, percorre os quatro slots sem pular ocupados e
  só chama o controller depois de uma confirmação válida.
- A lista de servidores é fornecida por configuração (`WYD_SERVER_LIST`,
  `Name|host:port;...`) e cada endpoint é validado antes de ser apresentado;
  sem a variável, existe uma entrada local segura para desenvolvimento.
- A seleção de servidor separa destaque da linha e confirmação: clicar numa
  linha apenas muda o índice; `CONNECT` ou Enter confirma o endpoint e revela a
  tela de login. A conexão permanece adiada até o envio das credenciais. Sem
  `WYD_SERVER_ADDRESS`, a entrada usa o endpoint padrão `127.0.0.1:8281`; a
  inicialização da janela não abre o socket antes da confirmação.
- `loginbox.wyt`, `loginicon.wyt`, `LoginScene.bin` e `SelServerScene.bin` não
  são referenciados pelo client Go ativo. Foram removidos de `assets/current`
  para reduzir a distribuição; a cópia oficial e as referências Ghidra foram
  preservadas como corpus read-only, pois o executável nativo ainda carrega a
  família antiga quando selecionado por sua própria configuração.
- Não declarar `CLIENT_TESTED` até executar login/logout/relogin no executável.

## Lacunas

- validar conexão e ciclo completo contra o servidor WYD-Go, incluindo duas
  entradas configuradas e retorno à seleção após desconexão;
- adicionar a fonte/atlas nativo de texto e rastrear recursos visuais no client
  real;
- substituir os elementos visuais provisórios de seleção, carregamento e mundo
  por assets protegidos depois que o contrato de recursos estiver fechado;
- validar reconexão e logout/relogin no executável Windows.

## Validação

- `go test -count=1 ./...`: aprovado.
- `go test -race -count=1 ./internal/protocol ./internal/app ./internal/login ./internal/loginflow`: aprovado.
- Build Debug do executável Go: aprovado.
- Smoke de bootstrap Windows (janela, resize, `WM_CLOSE` e `Alt+F4`): aprovado;
  o fluxo autenticado ainda não foi exercitado.
- Client real: pendente.

## Estado de validação

- `AUTOMATED TESTED`: `go test -count=1 ./...` e teste de ordem do frame.
- `AUTOMATED TESTED`: `go test -race` nos pacotes protocol/app/login/loginflow.
- `AUTOMATED TESTED`: controles da cena de login (foco, edição, validação,
  envio único, rollback de erro e limpeza da senha) em `internal/ui`.
- `CLIENT_TESTED`: pendente; o executável real ainda não percorreu login,
  seleção, mundo, logout e relogin.
