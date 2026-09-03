---
id: ui-quest-panel-layout-lifecycle
title: Layout e lifecycle do painel Quest 7.48
subsystem: ui-layout-quest
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Layout e lifecycle do painel Quest 7.48

## Pergunta

Como o client 7.48 materializa, posiciona, abre, alimenta e encerra o painel
Quest, e quais IDs e transições a source precisa preservar para que botão,
tecla, abas, listas, memo, `Esc` e relogin operem sobre a mesma janela?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256
  `8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; pseudocódigo de
  `FUN_00441823`, `FUN_004662C5`, `FUN_0049E50F`, `FUN_00453C59`,
  `FUN_0044DF53` e `FUN_004358DA`.
- Assets: `UI\FieldScene.txt`/`UI\FieldScene2.txt`, `QuestSubjects.txt`,
  `QuestSubjects2.txt`, `QuestSubjects3.txt`, `QuestSubjects4.txt` e
  `QuestMessage.txt`.
- Source atual: `TMFieldScene::InitializeCompatFieldScene`,
  `PositionCompatQuestPanel`, `SetQuestPanelVisible`, `SelectQuestTab`,
  `OnControlEvent`, `OnKeyQuestLog`, `OnESC` e `AirMove_ShowUI` em
  `client-source/tmproject/Projects/TMProject/TMFieldScene.cpp`.

## Fluxo nativo 7.48

### Entrada observável

- Evento: click no botão `315`, tecla `X`, seleção das abas
  `329/330/331/333`, seleção nas listas `321/325/327/335`, memo `332`, botão
  fechar `323`, `Esc` ou fechamento por movimento entre cenas.
- Precondições e estado inicial: FieldScene materializada, container e root
  `320` válidos; controles opcionais só participam quando o recurso os criou.
- Saída observável: painel centralizado no viewport, uma das quatro
  listas/conteúdos ativa, conteúdo correspondente carregado e estado do botão
  `315` coerente com a visibilidade.

### Callers

- `FUN_00435B13` chama `FUN_00441823` durante a inicialização da FieldScene.
- A vtable `PTR_FUN_005A4294` expõe o initializer da FieldScene em `+0x4C` e
  o receptor de controles `FUN_004662C5` em `+0x58`.
- `FUN_004662C5` recebe os controles Quest: `315`, `321`, `323`, `325`,
  `327`, `329`, `330`, `331`, `332`, `333` e `335`.
- `FUN_00453C59` encaminha a tecla `Esc` para `FUN_0044DF53`.
- O dispatcher de teclado da FieldScene chega ao caminho da tecla `X`, que
  deve voltar ao mesmo controle nativo `315` usado pelo click.

### Função principal

`FUN_00441823` resolve e vincula o grupo Quest completo:

| Papel | ID nativo |
| --- | --- |
| toggle inferior | `315` |
| root | `320` |
| lista/conteúdo 1 | `321/322` |
| fechar/título | `323/324` |
| lista/conteúdo 2 | `325/326` |
| lista/conteúdo 3 | `327/328` |
| abas 1–4 | `329/330/331/333` |
| memo | `332` |
| lista/conteúdo 4 | `335/334` |

O root `320` recebe centralização exata nos dois eixos:

```text
x = (viewportWidth - panelWidth) / 2
y = (viewportHeight - panelHeight) / 2
```

O initializer oculta as quatro listas e os quatro conteúdos antes de definir
o estado inicial. Também carrega a tabela de níveis/mensagens e, quando
aplicável, popula as listas conforme nível e classe do personagem.

### Callees

- `FUN_004662C5`, no branch `315`, alterna o root, recarrega as quatro listas
  de assuntos e sincroniza a seleção do botão.
- Os branches `329/330/331/333` escolhem exatamente uma aba e seu par de
  lista/conteúdo.
- Os branches `321/325/327/335` chamam `FUN_0049E50F` para carregar o bloco
  de conteúdo correspondente à linha selecionada.
- `FUN_0049E50F` valida o destino antes de carregar o intervalo textual; um
  conteúdo não materializado não deve ser desreferenciado.
- O branch `323` encerra a janela, e `332` abre o conteúdo do memo pelo fluxo
  de mensagens da FieldScene.
- `FUN_0044DF53` fecha Quest na cascata de `Esc`, preservando a prioridade
  entre janelas.

### Saídas e erros

- Root ausente impede a transição de visibilidade sem fabricar um painel.
- Lista ou conteúdo ausente afeta somente aquela aba; as demais continuam
  disponíveis e nenhum loader recebe destino nulo.
- ObjectManager ausente impede a recarga dependente do personagem, mas não o
  fechamento seguro da janela.
- Fechar Quest não altera estado de missão no servidor nem mantém captura de
  UI; somente a apresentação local e o estado do toggle são alterados.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| bootstrap FieldScene | container/recurso carregados | `FUN_00435B13 -> FUN_00441823` | grupo vinculado; painel oculto | carrega tabelas e centraliza root | controles ausentes permanecem nulos |
| click `315` ou tecla `X` | root válido | `FUN_004662C5` | visibilidade alternada | recarrega assuntos e sincroniza toggle | sem root, nenhuma transição |
| aba `329/330/331/333` | par da aba disponível | `FUN_004662C5` | uma aba ativa | alterna lista/conteúdo | par ausente é ignorado |
| linha `321/325/327/335` | conteúdo destino válido | `FUN_004662C5 -> FUN_0049E50F` | texto da seleção carregado | usa intervalo da linha | destino nulo é rejeitado |
| memo `332` | controles de mensagem válidos | `FUN_004662C5` | mensagem exibida | usa `QuestMessage.txt` | sem destino, não abre parcialmente |
| fechar `323` | painel visível | `FUN_004662C5` | painel oculto | sincroniza botão | idempotente quando já oculto |
| `Esc` | Quest é a próxima UI da cascata | `FUN_00453C59 -> FUN_0044DF53` | painel oculto | preserva prioridade de fechamento | demais janelas ficam para o próximo `Esc` |
| AirMove/troca de cena | Quest visível | fechamento da UI/cena | painel oculto ou cena destruída | libera captura visual | nenhum ponteiro sobrevive à cena |
| logout/relogin | FieldScene ativa | lifecycle comum de cena | nova FieldScene | refaz bindings, textos e posição | não reutiliza controles antigos |

### Vtables, vptrs e receptores

A vtable FieldScene é `PTR_FUN_005A4294`. O initializer em `+0x4C` converge
em `FUN_00435B13`, que chama `FUN_00441823`; o receptor em `+0x58` é
`FUN_004662C5`. Inicialização, callbacks e fechamento operam sobre o mesmo
objeto FieldScene e os mesmos membros Quest.

### Ownership

O `SControlContainer` é dono dos controles materializados pelo recurso. A
FieldScene mantém ponteiros emprestados para root, toggle, abas, listas,
conteúdos, fechar e memo. Os helpers de posição, seleção e visibilidade não
adquirem ownership nem alocam substitutos para controles ausentes.

### Falha parcial

O layout compatível pode conter somente os IDs nativos, enquanto o TMProject
posterior também referencia IDs altos. Cada binding e cada destino textual
deve ser protegido individualmente. A ausência de um controle posterior não
pode impedir abrir, trocar aba ou fechar o grupo 7.48.

### Cleanup e teardown

O fechamento visual oculta o root e sincroniza o toggle; não existe estado de
gameplay Quest adquirido por esse helper. Ao destruir a FieldScene, o deleting
destructor converge em `FUN_004358DA`, que encerra o grafo de controles sob o
container da cena.

### Shutdown

Quest não possui thread ou owner global independente. O shutdown segue o
lifecycle comum da FieldScene; ponteiros para listas, conteúdos e memo deixam
de ser válidos quando a cena entra em teardown.

### Logout e relogin

Logout/troca de personagem destrói a FieldScene atual. No retorno ao mundo, o
initializer refaz os bindings, centraliza `320`, recarrega as cinco tabelas e
restabelece a visibilidade inicial. Nenhum ponteiro nem seleção da instância
anterior é reaproveitado. Esse fluxo ainda precisa ser exercitado no
`project.exe` antes de receber `CLIENT-TESTED`.

## Wire, ABI e recursos

Esta adaptação não altera wire, packing, structs nem opcodes. O fluxo é local
à UI e usa os IDs materializados pelo recurso 7.48. Os controles importados
`65793` e `1054256+` podem continuar no layout posterior, mas não substituem
`315` e `320..335` quando `FieldScene2` está ativo. Os cinco arquivos Quest
são recursos de entrada e não devem ser renomeados pelo helper.

## Mapeamento atual

### Source recompilável

`InitializeCompatFieldScene` vincula `315` e o grupo `320..335`.
`PositionCompatQuestPanel` aplica o centro exato no bootstrap e no open.
`SetQuestPanelVisible` centraliza recarga, aba inicial, visibilidade, som e
estado do botão. `SelectQuestTab` coordena os quatro pares. Click moderno,
click nativo, tecla `X`, fechar, `Esc` e AirMove convergem nesses helpers; os
loaders e controles opcionais possuem guards.

### WYD-Go

Não há mudança server-side neste lote. A janela apresenta dados dos arquivos
Quest locais; o servidor continua autoritativo sobre gameplay e progresso.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject posterior | Decisão |
| --- | --- | --- | --- | --- |
| controles Quest | `315`, `320..335` | vinculados no compat | usava também `65793` e `1054256+` | `PARIDADE_NATIVA` |
| posição | centro exato X/Y | reaplicada no bootstrap/open | Y usava fator `0.6` da altura | `PARIDADE_NATIVA` |
| tecla `X` | converge no ID `315` | envia `315` no compat | enviava somente `65793` | `PARIDADE_NATIVA` |
| abas/listas | quatro pares nativos | aceita IDs nativos e posteriores | callbacks somente posteriores | `PARIDADE_NATIVA` |
| fechamento | botão, `Esc` e AirMove convergem | helper único | caminhos diretos divergiam | `PARIDADE_NATIVA` |
| wire/gameplay | sem delta | inalterado | inalterado | preservar |

## Decisões

- Usar o grupo realmente materializado no recurso 7.48 sem remover o layout
  posterior compatível.
- Centralizar posição, seleção de aba e visibilidade em helpers únicos.
- Encaminhar a tecla `X` ao ID nativo `315` no modo compatível.
- Proteger cada controle e loader sem materializar widgets artificiais.
- Não alterar protocolo nem atribuir autoridade de missão ao client.

## Lacunas

- Testar click `315` e tecla `X`, inclusive abrir e fechar repetidamente.
- Testar as quatro abas, listas e respectivos conteúdos.
- Testar memo, botão `323`, `Esc` e fechamento por AirMove.
- Repetir depois de troca de cena e logout/relogin.
- Comparar centro e hitboxes em `800x600`, `1024x768` e `1280x960`.
- Não promover a `CLIENT-TESTED` antes desses fluxos.

## Validação

- Pesquisa: initializer, callbacks, loader, vtable, ownership, falha parcial,
  cleanup, teardown, shutdown e relogin estão registrados; maturidade
  `TRACED`.
- Entrega: `IMPLEMENTED / STATICALLY VERIFIED`; o ID nativo `315`, os helpers
  e os guards foram inspecionados na source. `Build-Client.ps1` concluiu
  `Release|Win32` com zero erros e 13 warnings C4018 preexistentes, sem o
  warning C4805 da comparação de visibilidade, e instalou
  `client748/project.exe` com SHA-256
  `B51D48ACF691B84A6B577DBB07E4981CED6F54DFB2616567C907863A9B9AE6BC`.
- Automação: `validate_research.py` e `git diff --check` passaram para o lote.
- Client real: não executado; `CLIENT-TESTED` não é alegado.
