---
id: go-scene-manager
title: SceneManager e eventos do client Go 7.48
subsystem: lifecycle
status: CONTRACT
mode: MODERNIZACAO_COMPATIVEL
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# SceneManager e eventos do client Go 7.48

## Pergunta

Como organizar cenas, eventos de janela e teardown no client Go sem copiar ABI
ou código do client nativo, preservando os invariantes de ownership observados
no lifecycle nativo 7.48?

## Fronteira de evidência

- Executável nativo e projeto Ghidra: `references/ghidra/input/WYD.exe`, com o
  SHA-256 registrado no front matter.
- Funções de criação/substituição e teardown: `FUN_004B3500`, `FUN_004B37C9` e
  `FUN_0055D066`, correlacionadas nas fichas `scene-transition.md`,
  `field-scene-rebuild-after-server-move.md` e
  `application-close-global-shutdown.md`.
- Source recompilável: `clientgo748/internal/scene`, `internal/input`,
  `internal/app` e `internal/platform/win32`.
- WYD-Go: **NÃO APLICÁVEL**; não há estado server-side nem packet nesta
  unidade.
- TMProject: **UTILIZADA somente como comparação estrutural secundária**; não
  houve cópia, tradução mecânica ou importação de ABI/C++.
- Assets, guias, W2PP, Secrets e Micronics: não aplicáveis ou excluídos pela
  política desta campanha.

Esta ficha registra uma `MODERNIZACAO_COMPATIVEL` interna. O lifecycle nativo
amplo continua `LOCATED` em `scene-transition.md`; a API Go não é apresentada
como uma vtable nativa.

## Fluxo nativo 7.48

### Entrada observável

O executável nativo entra e abandona cenas pelo ObjectManager durante o
bootstrap, troca de Field e fechamento global. Os callers e receptores estão
registrados nas fichas nativas indicadas acima. A implementação Go reutiliza
somente os invariantes de ordem e ownership, sem transportar offsets.

### Callers

No client Go, `Application.New` chama `Manager.Start` para a cena inicial e
`Application.Run` chama `Dispatch`, `Update`, `Render` e `Close`. No nativo, os
callers pertencem ao ObjectManager e às rotinas de teardown das fichas legadas.

### Função principal

`Manager.Update` é a fronteira de transição: cria a fábrica pendente, chama
`Enter`, libera a cena anterior somente após sucesso e então atualiza a cena
ativa. `Manager.Close` executa `Exit`/`Close` uma única vez.

### Callees

O manager chama apenas a `Factory` selecionada e os métodos da interface
`Scene`. A plataforma fornece eventos por `Window.PollEvents`; cenas não
chamam Win32 diretamente. No nativo, os callees incluem os destructors da
árvore de objetos e os receptores de Field já documentados.

### Saídas e erros

Fábrica ausente, transição duplicada ou `Enter` falho retorna erro sem
substituir a cena anterior. Falhas de `Exit`, `Close`, `Update` ou `Render` são
devolvidas ao owner da aplicação; o manager não esconde o erro.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Ação | Estado resultante | Erro/saída |
| --- | --- | --- | --- | --- |
| bootstrap → cena inicial | factory válida | `Start` + `Enter` | cena ativa | erro encerra bootstrap |
| cena A → cena B | A ativa, pedido pendente | troca na fronteira de `Update` | B ativa; A liberada | `Enter` falho preserva A |
| evento de janela | manager aberto | `Dispatch` para a cena | estado da cena | erro propagado |
| fechamento | cena opcional ativa | `Exit`, `Close` | manager fechado | operação idempotente |
| relogin | sessão futura solicita nova tela | nova cena sem referência residual | preparado | wire ainda não implementado |

### Vtables, vptrs e receptores

Não aplicável à API Go. As funções nativas e seus receptores permanecem nas
fichas legadas; nenhum vptr ou offset foi copiado.

### Ownership

O manager é owner da cena ativa e da fábrica pendente. A `Application` continua
owner do renderer, source de assets e janela. Cenas não fecham esses recursos.

### Falha parcial

Se `Enter` falhar, a nova cena recebe `Close` e a anterior permanece ativa. Se
uma operação da cena falhar durante o frame, o erro sobe para `Application.Run`
e o caminho de fechamento libera o manager uma vez.

### Cleanup e teardown

Na troca, a nova cena entra antes de `Exit`/`Close` da anterior. No encerramento,
o manager é fechado antes do renderer, source de assets e janela.

### Shutdown

O fechamento da aplicação chama `Manager.Close` mesmo em caminho de erro. A
operação é idempotente para que `defer` não repita callbacks.

### Logout e relogin

O manager já permite solicitar uma tela nova sem manter referência da cena
anterior, mas login, seleção de personagem, Field e reconexão ainda exigem as
fichas e contratos de transporte próprios.

## Wire, ABI e recursos

N/A: esta unidade não cria packet, ABI ou recurso visual. A janela mantém a
fronteira Win32 já documentada; mensagens são convertidas em tipos
`input.Event` neutros antes de chegarem às cenas.

## Mapeamento atual

### Source recompilável

`internal/scene/scene.go` contém o manager e as invariantes de transição;
`internal/input/input.go` contém eventos independentes de Win32;
`internal/app/application.go` integra o ciclo e fecha o manager antes dos
owners gráficos. `internal/platform/win32` apenas traduz mensagens de janela.

### WYD-Go

N/A: nenhum estado server-side ou contrato de sessão é alterado.

## Matriz de delta

| Claim | Nativo 7.48 | Source Go | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| owner de cena | ObjectManager possui árvore/cena | `scene.Manager` possui cena ativa | comparação estrutural | N/A | modernizar internamente |
| troca | criação/substituição com cleanup | transição em `Update` | comparação secundária | N/A | manter lifecycle |
| falha parcial | cleanup do objeto criado | `Enter` falho fecha nova cena e preserva anterior | não é prova nativa | N/A | proteger |
| teardown | ObjectManager antes da janela | manager antes de renderer/source/window | comparação secundária | N/A | manter ordem |
| wire | não aplicável | nenhum packet novo | não aplicável | não aplicável | não implementar |

## Decisões

- Evidência nativa: `CONFIRMED` para a fronteira de cenas e a necessidade de
  cleanup; a forma da API Go é modernização, não paridade.
- Classificação: `MODERNIZACAO_COMPATIVEL`.
- Ação: modernizar a estrutura interna e manter as fronteiras externas já
  testadas; não copiar ABI, vtable ou código TMProject.
- Não promover o claim nativo amplo `scene-transition.md`, que permanece
  `LOCATED`.

## Lacunas

- ligar cenas reais de login, seleção e Field ao manager;
- integrar troca disparada por pacote, logout e relogin após fechar os contratos
  de transporte;
- executar a troca customizada no `project.exe` para obter `CLIENT_TESTED`.

## Validação

- `go test ./...`: aprovado.
- `GOOS=windows GOARCH=amd64 go build ./...`: aprovado.
- `Build-ClientGo.ps1 -Configuration Debug`: aprovado.
- `Test-ClientBootstrap.ps1` nos ambientes default e protegido: criação da
  janela, resize, `WM_CLOSE` e `Alt+F4` aprovados.
- `git diff --check`: aprovado.
- Execução visual e integração com transporte: pendentes até o manager
  receber as cenas de login e mundo.
