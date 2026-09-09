---
id: application-bootstrap-window
title: Application bootstrap and primary window
subsystem: lifecycle
status: CLIENT_TESTED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Application bootstrap and primary window

## Pergunta

Como o client nativo WYD 7.48 cria o objeto principal da aplicação, carrega a
configuração de vídeo, registra e cria a janela, inicializa os managers e entra
no message loop; e como o novo client Go pode preservar essa fronteira
observável com ownership e rollback explícitos?

## Fronteira de evidência

- Binário nativo e projeto Ghidra: `references/ghidra/input/WYD.exe`, com o
  SHA-256 do frontmatter, e corpus versionado em `references/ghidra/corpus/`.
- Funções centrais: `FUN_0055F7F9`, `FUN_0055BC0A`, `FUN_0055CA18`,
  `FUN_0055D345`, `FUN_0055D066` e o WndProc `FUN_0055FA89`.
- Assets oficiais 7.48: `assets/current/` foi utilizado para confirmar a raiz
  local, mas nenhum loader ou formato visual é atribuído a esta ficha.
- Código Go atual: `internal/app`, `internal/platform/win32` e
  `internal/graphics/wgl` implementam a unidade moderna delimitada abaixo.
- WYD-Go: **NÃO APLICÁVEL**; esta etapa não abre socket nem altera wire.
- TMProject: **UTILIZADA somente como comparação estrutural secundária**; não
  houve cópia, tradução mecânica ou importação de ABI/código C++.
- W2PP, Secrets e Micronics: excluídos por política e não utilizados.

Esta ficha cobre apenas o processo, a janela primária, o contexto gráfico
vazio e seu teardown. Scene, input, protocolo e loaders de asset exigem fichas
próprias. A classificação é `MODERNIZACAO_COMPATIVEL`: Win32/WGL continuam
observáveis na mesma fronteira, mas o ownership interno é refeito em Go.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0055F7F9` cria o objeto da aplicação e chama o slot virtual `+0x04`,
resolvido pela vtable `0x005A6104` como `FUN_0055BC0A`. O bootstrap parte de
defaults 800x600, lê `config.txt`, valida a resolução com
`EnumDisplaySettingsA`, registra `FUN_0055FA89` como WndProc e cria a janela
com `CreateWindowExA` depois de `AdjustWindowRect`.

Após foco e inicialização de dados por `FUN_0054F0F2`, o fluxo chama
`FUN_0055CA18` para criar renderer e managers. Somente em sucesso o caller
instala o hook global e chama o slot virtual `+0x0C`, `FUN_0055D345`, que
mantém o message loop e os frames.

### Callers

O caller concreto do bootstrap é `FUN_0055F7F9`. Ele constrói a aplicação,
chama o slot `+0x04/FUN_0055BC0A`, instala o hook apenas após sucesso, executa
o slot `+0x0C/FUN_0055D345`, conclui COM/OLE e chama o deleting destructor.

A vtable efetiva em `0x005A6104` resolve os quatro slots necessários:

| Slot | Função | Papel |
| --- | --- | --- |
| `+0x00` | deleting destructor | encerra o owner da aplicação |
| `+0x04` | `FUN_0055BC0A` | configuração, janela e bootstrap |
| `+0x08` | `FUN_0055D066` | teardown dos recursos adquiridos |
| `+0x0C` | `FUN_0055D345` | message loop e execução |

### Função principal: `FUN_0055BC0A`

O fluxo nativo usa área cliente 800x600 por padrão. Ele registra a classe,
ajusta o retângulo externo, cria a janela e define foco antes de iniciar dados
e renderer. Falhas retornam zero e apresentam mensagens em inglês, incluindo
`Initialize Data Failed`, `Initialize Render Failed`,
`Initialize Interface Failed`, `Initialize IME Failed` e
`Initialize Socket Failed`.

O client Go preserva a ordem externa `janela -> contexto gráfico -> loop` e
faz rollback local em cada falha. Configuração avançada, scene, interface,
IME e socket não são simulados nesta unidade.

### Callees

`FUN_0055BC0A` chama `RegisterClassA`, `AdjustWindowRect`,
`CreateWindowExA`, `SetFocus`, `FUN_0054F0F2` e `FUN_0055CA18`.
`FUN_0055CA18` evidencia a ordem aproximada RenderDevice, SoundManager, BGM,
Interface/Input, TimerManager, ObjectManager e SocketManager. O message loop
fica em `FUN_0055D345`; o caminho terminal converge em `FUN_0055D066`.

## Estado e lifecycle

### Entrada observável

A entrada do novo fluxo é `cmd/wydclient`: configuração válida cria a janela
Win32 com área cliente 800x600 e, em seguida, um contexto WGL/OpenGL associado
àquela janela. O título e todas as falhas apresentadas pelo processo estão em
inglês.

### Matriz de transições

| Evento/estado | Precondição | Ação | Estado resultante | Erro/saída |
| --- | --- | --- | --- | --- |
| iniciar | configuração válida | abre janela Win32 | janela adquirida | erro em inglês sem renderer |
| janela adquirida | handle válido | cria DC, pixel format e WGL context | renderer adquirido | rollback gráfico e fecha janela |
| renderer adquirido | contexto corrente | processa mensagens e apresenta frames | executando | X ou Alt+F4 solicita saída |
| fechar | execução ou cancelamento | fecha renderer e depois janela | encerrado | teardown idempotente |

### Vtables, vptrs e receptores

O nativo usa a vtable `0x005A6104` e os receptores descritos acima. O Go não
importa vtable, offsets ou layout do objeto nativo: interfaces tipadas injetam
janela e renderer, preservando somente a ordem e os efeitos observáveis.

### Ownership

`app.Application` possui os recursos depois que cada inicialização retorna
sucesso. `wgl.Renderer` possui o HDC e o HGLRC; `win32.Window` possui o HWND.
O fechamento ocorre na ordem renderer/contexto, DC e finalmente janela. Cada
`Close` pode ser chamado novamente sem repetir a liberação.

### Falha parcial

Falha de abertura da janela não transfere ownership ao orquestrador. Falha
durante WGL desfaz somente contexto/DC já adquiridos; o orquestrador fecha a
janela. Managers não implementados não são fingidos nem recebem estado vazio.

### Cleanup e teardown

O contexto deixa de ser corrente antes de `wglDeleteContext`; o DC é liberado
com `ReleaseDC`; por fim `DestroyWindow` conduz `WM_DESTROY` e
`PostQuitMessage`. Testes do orquestrador verificam a ordem e a idempotência.

### Shutdown

O X e Alt+F4 convergem em `WM_CLOSE`; o orquestrador encerra o loop, desmonta
o renderer e então destrói a janela. `WM_DESTROY` publica o término do message
loop. O contrato Field/packet de encerramento não
se aplica antes da implementação de scene e socket e permanece na ficha
`application-close-global-shutdown.md`.

### Logout e relogin

N/A: esta unidade não autentica, não entra em Field e não mantém sessão. O
fluxo futuro deve usar `character-logout-selectchar-relogin.md` sem confundir
logout de personagem com shutdown do processo.

## Wire, ABI e recursos

Não há packet ou ABI de rede nesta unidade. A fronteira de plataforma usa tipos
Win32 de tamanho explícito e layout host, mas não exporta handles para domínio,
protocolo ou assets. O contexto solicitado é OpenGL legado por WGL para manter
compatibilidade com a família de assets/render do client, sem atribuir ainda
formato ou loader a `.wyt`, `.wys`, `.msh`, `.msa` ou `.ani`.

## Mapeamento atual

- `internal/platform/win32`: classe, janela 800x600 de área cliente, messages,
  X, Alt+F4 e fechamento idempotente.
- `internal/graphics/wgl`: pixel format, contexto OpenGL, viewport, clear,
  swap e teardown reverso.
- `internal/app`: aquisição ordenada, loop, rollback e dependências injetáveis.
- `cmd/wydclient`: composição real; não há mais executável apenas textual.

## Matriz de delta

| Claim | Nativo 7.48 | Client Go | Decisão |
| --- | --- | --- | --- |
| tamanho inicial | área cliente 800x600 por default | área cliente 800x600 | preservar |
| plataforma gráfica | Win32 e renderer legado | Win32 e OpenGL/WGL | preservar fronteira |
| ordem | janela, dados, renderer/managers, loop | janela, renderer, loop | implementar incrementalmente |
| falha parcial | retorna zero e exibe erro | erro tipado e rollback explícito | modernizar com equivalência |
| ownership | aplicação desmonta managers | owners Go fecham em ordem reversa | modernizar com equivalência |
| mensagens | textos de falha em inglês | erros visíveis em inglês | preservar |

## Decisões

- Evidência: `TRACED`; callers, callees, entrada, ownership e saída desta
  fronteira estão resolvidos estaticamente.
- Classificação: `MODERNIZACAO_COMPATIVEL`.
- A implementação usa Go puro, `purego` e `x/sys/windows`; não usa CGO nem
  código do TMProject.
- O primeiro asset visual será uma unidade separada depois do contrato do
  loader nativo; não interpretar formatos proprietários por suposição.
- O contrato estreito de processo, janela e contexto vazio foi promovido para
  `CLIENT_TESTED` depois da execução do `wydclient.exe` produzido. Isso não
  promove scene, input, asset ou protocolo, que continuam fora desta ficha.

## Lacunas

- Selecionar uma textura oficial e rastrear seu loader nativo antes da primeira
  conversão canônica pelo futuro `assetc`.
- A falha de bootstrap permanece coberta por injeção nos testes do
  orquestrador; ainda não existe uma opção de produto que force falha WGL no
  executável sem adulterar o ambiente.
- Completar data, input, interface, audio, timer, object e socket em fichas e
  unidades próprias; a ordem em `FUN_0055CA18` não os torna implementados.

## Validação

- Pesquisa estática: `FUN_0055F7F9`, vtable `0x005A6104`,
  `FUN_0055BC0A`, `FUN_0055CA18`, `FUN_0055D345` e `FUN_0055D066`
  correlacionados no corpus do executável hasheado.
- Automação executada: testes do orquestrador e dos contratos puros, build
  Windows e `tools/Test-ClientBootstrap.ps1`.
- Client real: em 2026-09-09, o smoke iniciou duas instâncias do
  `bin/wydclient.exe`, localizou cada HWND pela classe
  `WYDClientGo748Window` e pelo PID, confirmou área cliente 800x600,
  redimensionou para 960x720 e verificou saída com código zero por `WM_CLOSE`
  e pelo fluxo de Alt+F4. A validação observa lifecycle e tamanho; não alega
  renderização de assets.
