---
id: local-chat-message
title: Chat local bidirecional
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Chat local bidirecional

## Pergunta

Qual é o tamanho e o layout nativos do opcode `0x333`, como o client 7.48
constrói mensagens locais, como uma mensagem recebida é roteada entre a cena e
o humano materializado e quem possui as linhas adicionadas ao chat?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter; referência histórica somente leitura.
- Projeto Ghidra: `C:\Users\felipe\Tools\GhidraProjects\WYD748Native_20260821`,
  programa `/WYD.exe`, Ghidra 12.1.3, aberto read-only.
- Export focado: `%TEMP%\codex-wyd748-lifecycle-149205b7\scene-control-event-handlers-focused.tsv`;
  log sem `SCRIPT ERROR` e com o mesmo hash do frontmatter.
- Pseudocódigo reutilizado: `FUN_004662C5`, `FUN_0055890A`,
  `FUN_0052EAA9`, `FUN_0052D060`, `FUN_00481DD6`, `FUN_00407203`,
  `FUN_00408D33` e `FUN_0055F2DD` no corpus decompilado de 2026-08-21.
- Source atual: `Basedef.h`, `TMFieldScene.cpp`, `TMHuman.cpp`,
  `WYD748Compat.cpp` e o transporte em `client-source/tmproject`.
- Servidor atual: builder em `internal/wire/source_client.go`, gate em
  `internal/game/security.go` e dispatcher de chat do WYD-Go.
- Assets: nenhum asset é carregado ou alterado por esta transição.

## Fluxo nativo 7.48

### Entrada observável

Há duas entradas observáveis para o mesmo contrato:

- C->S: o usuário confirma texto ou aciona um comando de chat reconhecido no
  handler de controles `FUN_004662C5`; o client envia `0x333`.
- S->C: o transporte recebe `0x333/108`, o dispatcher humano encaminha o
  packet e a cena ou o `TMHuman` publica a linha/balão adequado.

Em ambos os sentidos o frame tem 108 bytes: header de 12 bytes seguido por 96
bytes de texto. O texto é limitado e terminado no próprio frame; não existe
ponteiro, tamanho variável ou payload posterior.

### Callers

- `FUN_004662C5` é o handler de eventos dos controles da Field. No caminho de
  envio em `0x00468723`, faz `PUSH 0x6C` antes de `_memset`; em `0x00468749`
  grava o opcode `0x333`; em `0x004687A3` faz outro `PUSH 0x6C`, em
  `0x004687A5` passa o endereço do frame e em `0x004687AC` chama
  `FUN_0055F2DD`. Outros comandos desse mesmo handler repetem a construção
  `_memset(..., 0x6C)` de `0x333`.
- `FUN_0055890A` é o size gate S->C. O `case 0x333` aceita somente `0x6C`.
- `FUN_0052EAA9` é o dispatcher do humano e encaminha `0x333` para
  `FUN_0052D060` quando o personagem remetente está materializado.
- O dispatcher da Field encaminha o mesmo opcode para `FUN_00481DD6`, que
  cobre o membro de party sem objeto humano materializado.

### Função principal

`FUN_0052D060` força NUL nos dois últimos bytes do texto do frame, escolhe
delay de três segundos ou dez segundos para a forma prefixada, constrói uma ou
duas linhas, cria `SListBoxItem` por `FUN_00407203` e o entrega à lista via
`FUN_00408D33`. Depois atualiza o horário do chat e o balão do humano. Retorna
`1`, indicando que o humano consumiu a mensagem.

`FUN_00481DD6` é o fallback da cena. Ele exige os controles de chat e party,
procura o ID do remetente na party e só atua quando não existe humano
materializado para esse ID. Também termina os dois últimos bytes do texto,
cria a linha com o nome do item de party, entrega-a à lista e atualiza o
horário. Retorna `0`, permitindo que o roteamento para filhos continue; se o
humano aparecer, `FUN_0052D060` registra a mensagem uma única vez.

### Callees

- `FUN_0055F2DD`: publica exatamente os 108 bytes construídos pelo client.
- `_memset`: zera o frame C->S antes de preencher header e texto.
- `FUN_00407203`: constrói um `SListBoxItem` para a linha renderizada.
- `FUN_00408D33`: adiciona o item à lista; quando ela está cheia, destrói
  virtualmente o item mais antigo e desloca os restantes.
- Rotinas de string/font usadas por `FUN_0052D060`: quebram texto longo,
  escolhem cor e atualizam a apresentação; não alteram o wire.
- Timer da cena: atualiza o instante da última linha/balão.

### Saídas e erros

- Tamanho diferente de 108 é rejeitado no gate antes do handler S->C.
- Controle de entrada ou texto ausente não constrói packet C->S.
- Chat list ausente faz o handler sair sem criar item.
- Party list ausente, vazia ou sem o ID deixa o fallback da cena sem efeito.
- Remetente materializado é ignorado pela cena e tratado pelo humano.
- O limite de 96 bytes não é negociável; os dois bytes finais são zerados para
  preservar terminação inclusive em texto multibyte truncado.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| confirmar texto | Field e transporte ativos | `FUN_004662C5 -> FUN_0055F2DD` | intenção publicada | envia `0x333/108` | sem controle/texto, não envia |
| receber tamanho inválido | socket ativo | `FUN_0055890A` | estado preservado | nenhum dispatch | rejeita antes da UI |
| receber de humano materializado | humano presente | `FUN_0052EAA9 -> FUN_0052D060` | linha e balão atualizados | lista assume item; delay 3/10 s | chat ausente, não publica |
| receber de party não materializada | party contém ID | dispatcher Field -> `FUN_00481DD6` | linha da party atualizada | lista assume item | controles/ID ausentes, sem efeito |
| lista cheia | nova linha válida | `FUN_00408D33` | janela conserva sua capacidade | destrói item mais antigo | nenhum item fica sem owner |
| troca/saída da Field | cena ativa | teardown da Field/control container | controles e itens destruídos | foco/balões deixam a cena | nenhum callback pendente |

### Vtables, vptrs e receptores

O dispatcher da cena recebe primeiro o packet e percorre os filhos
materializados; `FUN_0052EAA9` é o dispatcher virtual do humano. A lista de
chat é um controle pertencente à árvore da Field e `FUN_00408D33` chama o
destrutor virtual do item removido. O fluxo não importa vptrs ou offsets
nativos para a source: reutiliza `TMFieldScene`, `TMHuman`, `SListBox` e
`SListBoxItem` já vivos.

### Ownership

O frame é emprestado pelo dispatcher durante o tratamento. Cada item novo é
alocado pelo handler e transferido para `SListBox::AddItem`; a lista passa a
ser a única owner e destrói itens descartados ou desmontados com o container.
O humano observa o texto somente durante a chamada; o balão mantém sua própria
cópia conforme o lifecycle existente.

### Falha parcial

Alocação/controle ausente não altera party, personagem ou estado autoritativo.
Quando uma primeira linha já foi entregue e o texto exige quebra, a segunda
alocação pode falhar sem invalidar a primeira. Não existe persistência ou ACK
local para desfazer. O servidor continua autoritativo para comandos que usam o
mesmo opcode.

### Cleanup e teardown

`FUN_00408D33` fecha o ownership imediato dos itens. A chat list pertence ao
control container da Field e é eliminada pelo teardown geral da cena ligado a
`FUN_004358DA`; esse caminho já está fechado em
[`../lifecycle/scene-transition.md`](../lifecycle/scene-transition.md) e
[`../lifecycle/application-close-global-shutdown.md`](../lifecycle/application-close-global-shutdown.md).
Não há thread, handle, inscrição ou buffer externo específico de `0x333`.

### Shutdown

No shutdown, o dispatcher para com o transporte e a Field/control container
destrói lista e itens pelo caminho normal. Como envio e recepção são
síncronos, não há fila de UI ou callback de chat que sobreviva à cena.

### Logout e relogin

Logout elimina a Field atual, seus controles, itens e balões. O fluxo fechado
em [`../lifecycle/character-logout-selectchar-relogin.md`](../lifecycle/character-logout-selectchar-relogin.md)
materializa outra Field e outros humanos no relogin; nenhum ponteiro de item,
texto ou owner é reutilizado. Packets que chegam quando controles ainda não
existem são descartados com segurança.

## Wire, ABI e recursos

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD` |
| `0x0C` | 96 | texto local/comando, terminado nos bytes `0x6A/0x6B` |
| total | 108 (`0x6C`) | frame bidirecional `0x333` |

O contrato é comprovado nos dois sentidos: o gate S->C exige `0x6C` e o
construtor C->S zera, preenche e envia `0x6C`. Signedness, packing adicional e
padding não se aplicam ao corpo, que é um array de bytes. Não há recurso visual
novo nem dependência de layout 7.69.

## Mapeamento atual

### Source recompilável

Antes desta ficha, `MSG_MessageChat` tinha `String[128]` e media 140 bytes. Os
handlers terminavam índices `126/127`, e o envio de texto livre usava
`sprintf`. Isso divergia do gate e do construtor nativos. A adaptação reduz o
corpo a 96 bytes, limita cópias a 95 bytes, preserva dois NUL finais nos
handlers e mantém o roteamento cena -> humano e o ownership da lista.

### WYD-Go

O builder, comentário e security gate também exigiam/produziam 140 bytes. Eles
passam a usar 108, com no máximo 95 bytes de texto e terminação garantida. O
dispatcher e a autoridade dos comandos permanecem inalterados.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual antes | WYD-Go antes | Decisão |
| --- | --- | --- | --- | --- |
| frame `0x333` | 108 em C->S e S->C | 140 | 140 | restaurar `PARIDADE_NATIVA` |
| texto | 96 bytes, dois NUL finais | 128 e índices 126/127 | cópia até byte 138 | limitar a 96/95 |
| roteamento | cena cobre party; humano cobre materializado | fluxo já presente | publica por ID | preservar estrutura viva |
| ownership | lista assume e destrói item | `AddItem` já é owner | N/A | preservar owner único |
| recursos modernos | não participam do wire | controles opcionais | N/A | manter quando compatíveis |

## Decisões

- Classificar o delta como `PARIDADE_NATIVA` e promover a ficha a `CONTRACT`:
  tamanho, direção, layout, callers, callees, ownership e teardown estão
  fechados para esta transição.
- Corrigir client e servidor como uma unidade para evitar aceitar um frame que
  o client nativo rejeita ou emitir um frame que a source recompilada não deve
  produzir.
- Preservar o fallback de party e o dispatch para o humano; eles são partes
  complementares do mesmo roteamento, não duplicação acidental.
- Preservar controles modernos opcionais quando não alteram wire, ownership ou
  lifecycle.

## Lacunas

- Executar chat curto, texto de 95+ bytes, party com membro fora da tela,
  remetente materializado, logout durante chat e relogin no
  `client748/project.exe` recompilado.
- Capturar C->S e S->C em runtime para confirmar a integração completa com o
  servidor atual. Essas lacunas bloqueiam somente `CLIENT_TESTED`.

## Validação

- Pesquisa: Ghidra read-only confirmou gate, construtor, callsites, tamanho,
  handlers, roteamento, ownership e teardown no hash registrado.
- Pesquisa automatizada: `validate_research.py` passou com `CONTRACT=11`,
  `TRACED=6`, `LOCATED=4` e `UNMAPPED=1`.
- Testes focados: `go test -count=1 ./internal/wire ./internal/game` passou,
  incluindo frame de 108 bytes, truncamento em 95 bytes e NUL final.
- Suíte completa: `go test -count=1 ./...` passou.
- Build: `Build-Client.ps1` passou com zero erros e 31 warnings preexistentes;
  instalou `client748/project.exe` com SHA-256
  `E0B3D433B8C0CAD7E9AD015101A4BA0214E6E3D6D38E9EACFF264109234598C5`.
- Higiene: `git diff --check` passou; não restou referência ativa que trate
  `0x333`/`MSG_MessageChat` como frame de 140 bytes.
- Client real: não executado; nenhuma alegação `CLIENT_TESTED` é feita.
