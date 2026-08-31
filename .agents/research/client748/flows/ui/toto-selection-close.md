---
id: toto-selection-close
title: Seleção, navegação e fechamento do painel TOTO
subsystem: ui-input
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Seleção, navegação e fechamento do painel TOTO

## Pergunta

Como o client nativo 7.48 seleciona uma partida no painel TOTO, percorre os
três campos editáveis por teclado e fecha o painel sem conservar seleção ou
foco obsoletos?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; exports
  `00472651_FUN_00472651.c`, `00472966_FUN_00472966.c`,
  `00455b5b_FUN_00455b5b.c`, `00455c38_FUN_00455c38.c`,
  `004662c5_FUN_004662c5.c`, `00410a91_FUN_00410a91.c` e
  `004727cc_FUN_004727cc.c`.
- Source: `TMFieldScene.cpp/.h`, abertura no uso do item em `SGrid.cpp` e
  tabela descrita em `toto-list-loader.md`.
- O envio de aposta de `FUN_004727CC`, o opcode `0x3CE` e sua validação no
  servidor não pertencem a esta ficha.

## Fluxo nativo 7.48

### Entrada observável

O fluxo existente abre o painel ao usar o item `4147` (`0x1033`), conserva a
mensagem-base com tipo TOTO `0x3CE`, oculta shop/descrição, limpa os três
edits e entrega foco ao número da partida. Esta abertura aparece em
`FUN_00410A91` e já existia na source; citar o tipo da mensagem aqui apenas
delimita o contexto e não promove o wire a `CONTRACT`.

Com o painel aberto, as entradas deste recorte são:

- botão `8964` ou Enter no campo `8963`: selecionar partida;
- Tab: `8963 -> 8973 -> 8974 -> 8963`;
- Enter em `8973`: focar `8974`; Enter em `8974`: focar `8963`;
- botão `8966` ou Esc: fechar o painel.

### Callers

- `FUN_00472651` tem callers diretos `FUN_00455C38` e `FUN_004662C5`.
- `FUN_00472966` tem callers diretos `FUN_004662C5` e `FUN_004727CC`.
- `FUN_00455B5B` e `FUN_00455C38` são chamados pelo dispatcher de caracteres
  `FUN_00453C59`.
- Em `FUN_004662C5`, os controles `0x2304/8964`, `0x2306/8966` e
  `0x2312/8978` chamam, respectivamente, selecionar, fechar e comprar.

O caller de compra fecha o painel somente depois de validar os dois placares,
copiar partida/placares para a mensagem e aceitar partida em `1..80`. Essa
transição permanece separada porque envolve wire ainda não comprovado.

### Seleção

`FUN_00472651 @ 0x00472651` lê o texto do controle `+0x28B68`, converte o
número e limpa número e placares. Valor menor que 1 ou maior que a quantidade
declarada em `TOTOGame.csv` grava seleção zero. Valor válido publica time,
equipe A e equipe B da entrada de 96 bytes e grava o número em `+0x28B40`.

Os três setters de texto são chamadas virtuais nos controles. A conversão
numérica usa o helper `FUN_005909D2`; a source expressa a mesma entrada decimal
com `atoi`.

### Callees

- `FUN_00472651` chama `FUN_005909D2` para conversão decimal e os slots
  virtuais de obtenção/atribuição de texto dos quatro controles envolvidos.
- `FUN_00472966` consulta visibilidade por `FUN_0040C0F0`, usa as virtuais
  `+0x40` do container para foco e `+0x60` do painel para visibilidade, e chama
  `FUN_004481C5(1)` depois do teardown local.
- `FUN_00455B5B` e `FUN_00455C38` consultam foco por `FUN_00493C10`; o segundo
  também chama `FUN_00472651` quando o número está focado.

Não há alocação, timer, callback ou envio de rede entre esses callees. O
fluxo de `FUN_004727CC` não é callee da seleção; ele é uma transição de
compra separada que pode chamar o fechamento após montar a mensagem.

### Tab e Enter

`FUN_00455B5B @ 0x00455B5B` consome apenas Tab quando um dos três edits é o
controle focado e move o foco circularmente. `FUN_00455C38 @ 0x00455C38`
consome apenas Enter: no número chama a seleção; no placar A avança para B; no
placar B retorna ao número. Tecla ou foco alheios retornam zero e permanecem
disponíveis ao restante do dispatcher.

### Fechamento

`FUN_00472966 @ 0x00472966` verifica o painel, remove o foco, torna-o invisível
e zera `+0x28B40`. Depois chama `FUN_004481C5(1)`, uma rotina ampla de estado
de UI com nove callers e efeitos que ultrapassam o painel TOTO. O lote atual
porta o núcleo local comprovado; não atribui uma função source única a essa
rotina global sem correlação própria.

Na source, tanto `OnESC` quanto o caminho de Esc em `AirMove_ShowUI` passam por
`TotoClose`, evitando que um fechamento por teclado deixe foco ou seleção
obsoletos. O botão `8966` chega ao mesmo método por `OnControlEvent`.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| abrir TOTO | item 4147 e controles materializados | fluxo de `FUN_00410A91` | painel visível, seleção 0, foco no número | limpa três edits | abertura já existente |
| selecionar válido | número em `1..count` | `FUN_00472651` | `m_nTotoNum=n` | publica horário/equipes; limpa edits | consome clique/Enter |
| selecionar inválido | vazio, zero ou fora da quantidade | `FUN_00472651` | seleção 0 | limpa edits; não indexa tabela | sem packet |
| Tab | painel visível e edit focado | `FUN_00455B5B` | próximo edit focado | ciclo circular | foco alheio retorna 0 |
| Enter no número | painel visível | `FUN_00455C38` | partida selecionada ou zero | executa seleção | sem wire |
| Enter no placar | A ou B focado | `FUN_00455C38` | B ou número focado | nenhum dado alterado | foco alheio retorna 0 |
| fechar | painel visível | `FUN_00472966` | painel oculto e seleção 0 | foco removido | efeito global excluído do lote |

### Vtables, vptrs e receptores

O receptor é a instância Field. Seu bloco TOTO começa em `+0x28B40`:
seleção em `+0x28B40`, painel em `+0x28B44`, textos em
`+0x28B54..+0x28B5C` e edits de placar/número em
`+0x28B60/+0x28B64/+0x28B68`. Visibilidade, obtenção/atribuição de texto e
foco usam slots virtuais dos controles/container. A adaptação usa os tipos
`SPanel`, `SText`, `SEditableText` e `SControlContainer`, sem transportar
vptrs ou offsets nativos.

### Ownership

Os controles são localizados no `m_pControlContainer` durante
`InitializeScene`; os membros TOTO são ponteiros emprestados da árvore do
layout, não alocações do método de seleção. `g_pTOTOList` é tabela global
estática. Seleção e foco pertencem à Field ativa.

### Falha parcial

Controles ausentes fazem a source sair sem dereferência; na seleção também
zeram `m_nTotoNum`. Um número inválido nunca indexa a tabela e nunca preserva
placares antigos. Fechar painel ausente não altera outros controles.

### Cleanup e teardown

Fechar remove o foco antes de ocultar e descarta a seleção. Não há heap,
handle, callback, timer ou pedido de rede criado por estas funções. A árvore
de controles é desmontada com a Field pelo ownership existente do container;
os ponteiros TOTO não são destruídos individualmente.

### Shutdown

Os métodos são síncronos e não sobrevivem à Field. No shutdown, o teardown
normal da cena elimina painel/foco; não existe trabalho TOTO pendente neste
recorte. A rotina ampla `FUN_004481C5` continua explicitamente fora do claim.

### Logout e relogin

Uma nova `TMFieldScene` inicializa `m_nTotoNum` e todos os ponteiros TOTO em
zero, resolve novamente os controles, oculta o painel e recarrega
`TOTOGame.csv`. Assim, seleção e foco não atravessam logout, troca de servidor
ou relogin. Nenhum estado server-side é criado antes de `TotoBuy`.

## Wire, ABI e recursos

- Controles: painel `8961`, número `8963`, selecionar `8964`, fechar `8966`,
  horário `8968`, equipes `8971/8972`, placares `8973/8974` e comprar `8978`.
- Recurso de dados: `UI\TOTOGame.csv`, já fechado em ficha `CONTRACT`.
- O layout nativo foi usado apenas para correlacionar estado e receptores; a
  source preserva sua própria ABI recompilável.
- Este recorte não envia packet. `0x3CE` permanece somente como pista do fluxo
  de compra e exige ficha `CONTRACT` client/server antes de implementação.

## Mapeamento atual

### Source recompilável

Inicialização, controles, abertura pelo item e dispatcher de botões já
existiam. Os stubs `TotoSelect`, `TotoClose`, `OnKeyTotoTab` e
`OnKeyTotoEnter` foram preenchidos. Os dois fechamentos por Esc agora usam o
mesmo teardown local.

### WYD-Go

Não participa de seleção, foco ou fechamento. Nenhum handler foi adicionado;
compra/aposta depende de pesquisa separada do wire e de sua autoridade.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- |
| selecionar partida | valida quantidade, publica três textos | stub | N/A | `PARIDADE_NATIVA` |
| Tab/Enter | ciclo entre três edits e seleção | stubs | N/A | `PARIDADE_NATIVA` |
| fechar | remove foco, oculta e zera seleção | ocultava diretamente em Esc; stub no botão | N/A | `PARIDADE_NATIVA` do núcleo local |
| estado global após fechar | `FUN_004481C5(1)` | sem correlação única | N/A | não portar neste lote |
| compra/aposta | valida placares e envia `0x3CE` | stub e mensagem-base existente | não comprovado | pesquisar como `CONTRACT` |

## Decisões

- Classificar o lote como `PARIDADE_NATIVA` e manter a ficha em `TRACED`, pois
  não altera nem comprova wire/ABI.
- Preservar a estrutura atual de controles tipados e guards de ponteiro, que é
  compatível e mais segura que reproduzir dereferências implícitas.
- Não ativar `TotoBuy`, não inventar handler server-side e não portar
  `FUN_004481C5(1)` por semelhança.

## Lacunas

- Executar no `client748/project.exe`: abrir com item 4147, testar partida 1,
  limite carregado, zero e acima do limite, e confirmar textos/limpeza.
- Confirmar Tab, Enter, botão fechar e os dois caminhos de Esc em jogo.
- Correlacionar `FUN_004481C5(1)` somente se surgir efeito observável ausente.
- Fechar `TotoBuy`/`0x3CE`, servidor, rejeição, rollback e relogin em ficha
  `CONTRACT` separada.

## Validação

- Pesquisa: callers, callees, IDs, estado, foco, ownership, falha parcial,
  teardown, shutdown e relogin fechados para seleção/fechamento no corpus do
  hash registrado.
- Automação: `validate_research.py` e `git diff --check` passaram.
- Build: `Build-Client.ps1` passou em Release/Win32 com 13 warnings
  preexistentes de signed/unsigned e zero erros; instalou o candidato em
  `client748/project.exe`.
- SHA-256 do candidato compilado:
  `90D7B460A2D6B0E1072A8BA992A4911535E789ECEB10DC80CE756D6ED41F01F9`.
- Estado máximo alegado: `STATICALLY VERIFIED`; build não testa a interação.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
