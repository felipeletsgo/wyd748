---
id: ui-player-interaction-menu-lifecycle
title: Menu de interação com jogador por Ctrl+clique direito
subsystem: ui-input-player-interaction
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-04
---

# Menu de interação com jogador por Ctrl+clique direito

## Pergunta

Como o client 7.48 abre o menu contextual sobre outro jogador e como o
`TMProject` compatível deve materializar, posicionar, despachar e fechar esse
menu sem depender dos controles 7.59+?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256
  `8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
- Recurso nativo: `FieldScene2.bin`, root do menu `640`, texto `645` e
  controles filhos `620..645`, `816..817`, `862..863` e `912..915`.
- Source atual:
  `TMFieldScene::InitializeCompatFieldScene`, `OnMouseEventCompat`,
  `PGTVisible` e `OnControlEvent` em
  `client-source/tmproject/Projects/TMProject/TMFieldScene.cpp`.
- Escopo: paridade nativa de input/UI; não altera wire, ABI ou autoridade do
  servidor.

## Fluxo nativo 7.48

### Entrada observável

- `dwFlags == 517` (`WM_RBUTTONUP`) com `wParam & 8` (`MK_CONTROL`).
- O alvo precisa ser `m_pMouseOverHuman` válido e não possuir a cor de borda
  `0x8800FF00`, usada pelo nativo para bloquear o menu naquele estado.
- O próprio personagem em AutoTrade não abre o menu sobre si mesmo.

### Inicialização e materialização

`FUN_00435B13` resolve o root `640`, texto `645` e os botões nativos:

| Função visual | ID |
| --- | ---: |
| Party/Grupo | `641` |
| Guild | `642` |
| Trade | `643` |
| Cancelar | `644` |
| Challenge | `620` |
| Challenge 1v1, 5v5, 10v10, AllvAll | `639`, `621`, `622`, `623` |
| Guild drop/war/ally/invite | `816`, `817`, `862`, `863` |
| Guild invite common/chief 1/2/3 | `912`, `913`, `914`, `915` |

O root é modal no slot `m_pModalControl[2]` e começa invisível. A posição é
recalculada no viewport:

```text
x = viewportWidth / 2 - menu.width / 2
y = viewportHeight / 2 - menu.height / 2
```

### Abertura

O ramo nativo de `FUN_0055DAB8` valida o modo AutoTrade e o alvo e chama
`FUN_0044E1E8`/`PGTVisible`. A rotina preenche o alvo (`m_dwOpID` e
`m_pPGTOver`), mostra Party, Guild, Trade e Challenge, oculta as variantes e
torna o root `640` visível.

### Callers

- `FUN_0055DAB8` é o receptor do mouse e contém o teste `517 + Ctrl`.
- O callback da FieldScene é `FUN_004662C5`, no slot `+0x58` da vtable, e
  despacha os IDs dos botões para convite Party, Guild, Trade, Challenge e
  Cancelar.

### Função principal

`FUN_0055DAB8` valida o modificador, o alvo e AutoTrade antes de chamar a
rotina de abertura contextual.

### Callees

- `FUN_0044E1E8`/`PGTVisible` preenchem o alvo, alternam os botões e exibem o
  root modal.
- `FUN_004662C5` encaminha os callbacks dos controles para as ações de Party,
  Guild, Trade e Challenge.
- `FUN_0044DF53` percorre o fechamento por `Esc`.

### Saídas e erros

Alvo inválido, cor de borda bloqueante ou AutoTrade sobre o próprio personagem
terminam o evento sem abrir a UI. Com árvore de controles incompleta, a source
compatível retorna sem dereferenciar ponteiros nulos.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| FieldScene compatível criada | `FieldScene2.bin` carregado | `InitializeCompatFieldScene` | root 640 ligado, modal e oculto | bindings nativos e posição central | controle ausente mantém menu fechado |
| Ctrl+clique direito | alvo humano sob o cursor | `FUN_0055DAB8` / `PGTVisible` | root 640 visível e alvo salvo | Party/Guild/Trade/Challenge visíveis | alvo bloqueado ou AutoTrade: sem abertura |
| clique em ação | root visível e alvo salvo | `FUN_004662C5` | ação enviada e menu fechado | callback de Party/Guild/Trade/Challenge | botão ausente: callback não deve prosseguir |
| `Esc` ou UI concorrente | root visível | `FUN_0044DF53` | root oculto | modal liberado | cena já destruída: não reutilizar ponteiros |
| troca de cena/logout | FieldScene ativa | teardown da cena | grafo destruído | estado contextual descartado | relogin cria novos bindings |

### Vtables, vptrs e receptores

O receptor de mouse é o slot de input da FieldScene que materializa
`FUN_0055DAB8`. O callback de controles é o slot `+0x58`, identificado como
`FUN_004662C5`. Na source, os equivalentes são `OnMouseEventCompat` e
`OnControlEvent`; não há uma vtable nova ou um callback server-side para esta
UI.

### Ownership

O `SControlContainer` da FieldScene é owner do root 640 e de seus filhos. O
slot modal `m_pModalControl[2]` é apenas uma referência de despacho, não um
owner adicional. O alvo `m_pPGTOver` é ponteiro transitório para a entidade
atual e é descartado no teardown da cena.

### Falha parcial

O recurso 7.48 contém os controles documentados. Ainda assim, a source verifica
a árvore completa antes de `PGTVisible`; se um controle obrigatório faltar, o
menu não abre e não há crash por acesso nulo.

### Cleanup e teardown

O fechamento esconde o root e libera a condição modal. O destrutor comum da
FieldScene destrói o grafo de controles e invalida `m_pPGTOver`; nenhuma ação
do menu pode ser mantida depois dessa transição.

### Shutdown

O shutdown global segue o teardown comum da FieldScene. O menu não possui
thread, timer, socket ou owner global separado.

### Logout e relogin

Logout/troca de personagem destrói a FieldScene. No retorno ao mundo,
`InitializeCompatFieldScene` refaz os bindings e o menu retorna oculto; o
alvo anterior não é restaurado.

## Wire, ABI e recursos

Esta correção não altera wire, packing, structs ou opcodes. Os IDs são
propriedade do recurso nativo `FieldScene2.bin`; os handlers existentes
continuam enviando as intenções já suportadas por Party, Guild, Trade e
Challenge. A classificação é `PARIDADE_NATIVA`.

## Mapeamento atual

### Source recompilável

`InitializeCompatFieldScene` resolve os IDs `640/645`, `641..643`,
`620..623`, `639`, `816..817`, `862..863` e `912..915`; posiciona o root,
configura o modal slot 2 e mantém o root oculto. `OnMouseEventCompat` trata
`517 + MK_CONTROL`; `PGTVisible` realiza a abertura protegida.

### WYD-Go

Não há alteração no servidor. A autoridade das ações continua nos handlers e
na validação server-authoritative já existentes.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Evento de abertura | `517 + MK_CONTROL` | tratado em `OnMouseEventCompat` | caminho moderno não alcançava compat | N/A | `PARIDADE_NATIVA` |
| Controles | IDs `640` e filhos nativos | bindings no compat initializer | bindings só no initializer completo | N/A | `PARIDADE_NATIVA` |
| Posição/modal | centro do viewport, slot 2 | reaplicados no bootstrap | dependia do caminho moderno | N/A | `PARIDADE_NATIVA` |
| Ações | callback da FieldScene | `OnControlEvent` existente | preservado | revalidação existente | `PARIDADE_NATIVA` |

## Decisões

- Reutilizar os controles materializados no recurso 7.48.
- Corrigir a entrada no adaptador compatível, preservando a ordem do evento
  nativo e as guardas de alvo/AutoTrade.
- Não criar controles 7.69 nem alterar o wire para resolver uma falha de
  binding/input.
- Manter a proteção de falha parcial para impedir crash caso o recurso esteja
  incompleto.

## Lacunas

- Testar em jogo com dois clients Party, Guild, Trade e Challenge, incluindo
  Cancel/Esc e os callbacks de cada ação.
- Repetir após troca de cena e logout/relogin em `800x600`, `1024x768` e
  `1280x960`.
- Confirmar no runtime que o filtro de distância/cor de borda coincide com a
  experiência visual esperada.

## Validação

- Pesquisa: evento, IDs, callers/callees, vtable/receptores, ownership,
  falha parcial, teardown, shutdown e relogin registrados; estado `TRACED`.
- Source: implementação aplicada e compilada em Release|Win32.
- Automação: `go test -count=1 ./...`, `git diff --check` e
  `validate_research.py` devem passar.
- Client real: pendente; não alegar `CLIENT-TESTED` antes do teste manual.

### Fechamento

- `FUN_0044DF53` fecha o root modal na cascata de `Esc`.
- O teardown da FieldScene destrói o grafo de controles; não há ownership,
  thread ou timer global separado para o menu.
- Troca de cena, logout e relogin devem recriar os bindings a partir do
  recurso; nenhum ponteiro da instância anterior pode ser reutilizado.

### Correção de despacho do Ctrl+clique direito — 2026-09-04

- O primeiro ajuste de bindings e do ramo `517 + MK_CONTROL` não era
  suficiente: `OnMouseEventCompat` ainda chamava `TMScene::OnMouseEvent`
  antes do ramo PGT. Como o root 640 fica registrado como modal, o
  `SControlContainer` podia consumir o `WM_RBUTTONUP` e impedir que
  `PGTVisible` fosse chamado.
- A source agora dá prioridade somente ao pedido de abertura
  (`WM_RBUTTONUP`/`MK_CONTROL`) quando o root PGT está oculto. Para todos os
  demais eventos, e quando o menu já está visível, o despacho original da UI
  continua sendo executado. As guardas de personagem, viewport, AutoTrade,
  alvo humano e cor de borda permanecem ativas.
- Classificação: `PARIDADE_NATIVA`; não houve alteração de wire, ABI,
  servidor ou IDs de recurso.
- Build oficial `Build-Client.ps1`: Release|Win32 concluído com 0 erros e 13
  warnings C4018 preexistentes. O executável instalado
  `client748/project.exe` recebeu SHA-256
  `FA90D368E8EDE5F9D9E986263F13FAA0163A10DE8190C7B54C210AC59245B9C8`.
- Estado: `IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED BUILD`; permanece
  sem `CLIENT-TESTED` até a confirmação no jogo com dois clients e os quatro
  callbacks.
