---
id: equipment-slot-compatibility
title: Slots de equipamento aceitos pela projeção WYD-Go 7.48
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-07
---

## Pergunta

Quais slots de equipamento devem existir no client que conversa com o
WYD-Go 7.48, e como impedir que controles de versões posteriores reapareçam na
UI? O array `Equip` e seu wire permanecem com o tamanho legado; o escopo desta
ficha é a projeção visual e o despacho de posições sem controle materializado.

## Fronteira de evidência

- UTILIZADA: contrato explícito do emulador WYD-Go 7.48 informado nesta tarefa:
  não há item/equipamento Necklace nem Belt/NewSlot, e não há slot visual para
  equipá-los.
- UTILIZADA: source atual `TMFieldScene::InitializeCompatInventory`,
  `SetEquipGridState`, `OnPacketSwapItem`, `OnPacketSell` e
  `TMHuman::OnPacketSendItem`.
- CONTRADITÓRIA para esta projeção: descompilação nativa `FUN_00435B13` do
  executável 7.48 (hash acima) materializa IDs `0x160..0x168`, e a source
  histórica possui `ITEMTYPE_NECKLACE`; isso documenta o binário estudado, mas
  não autoriza expor esses controles no contrato do emulador solicitado.
- CONTRADITÓRIA como mecânica histórica, sem força para substituir o contrato
  do emulador: guia KR scraped menciona Necklace em regras de selamento.
- NÃO APLICÁVEIS: sources 7.54, W2PP, Secrets e Micronics; não foram consultadas.

## Fluxo nativo 7.48

`FUN_00435B13` resolve a árvore de equipamento por controles do root 257 e
atribui slots virtuais. A ficha não altera essa leitura histórica; registra a
decisão de adaptar a projeção para um peer WYD-Go que deliberadamente não
possui Necklace/Belt.

## Estado e lifecycle

No bootstrap compatível, os controles encontrados para Necklace (`354`) e
NewSlot (`1048976`/`1048977`) são esvaziados, desabilitados e ocultados. Os
membros da cena tornam-se nulos e não entram na lista de grids populados. A
troca de cena/relogin recria a árvore e reaplica a mesma política; nenhum
ponteiro desses controles atravessa o teardown.

## Wire, ABI e recursos

O array `Equip[16]`, structs e offsets de packets não foram redimensionados.
Posições 9, 16 e 17 são mantidas apenas como valores de fronteira para não
deslocar o wire; no modo compatível, `MSG_SwapItem` e `MSG_Sell` que tentem
endereçá-las são rejeitados antes de acessar um grid nulo. `SendItem` conserva
o cache recebido, mas não materializa visual para esses slots.

## Mapeamento atual

Slots visuais ativos no modo compatível: rosto implícito, Helm, Coat, Pants,
Gloves, Boots, Left, Right, Ring, Orb, Cabuncle, Guild, Event, Mount/DRing e
Mantua/Cape. Necklace (9) e NewSlot/Belt (16/17) não têm grid ativo. As grades
de atalho de skills 571, 573 e 586 não são equipamento e permanecem ativas.

## Matriz de delta

| Claim | Source anterior | Source atual | Decisão |
| --- | --- | --- | --- |
| Necklace visual | bind/populate opcional | grid limpo, oculto e ponteiro nulo | não materializar |
| Belt/NewSlot visual | ponteiros modernos podiam sobreviver | 1048976/1048977 desabilitados | não materializar |
| Equip wire | array fixo de 16 | inalterado | preservar ABI |
| pacote para slot ausente | dereferência podia ocorrer em swap/venda | rejeição precoce no modo compatível | proteger lifecycle |

## Decisões

Aplicar `MODERNIZACAO_COMPATIVEL` na projeção client/server: manter o wire
legado intacto, mas fazer a UI WYD-Go 7.48 expor somente os slots que o
emulador suporta. Não remover enums ou campos compartilhados, pois isso mudaria
ABI e não é necessário para ocultar a feature.

## Lacunas

Ainda falta confirmar no `project.exe` que os controles permanecem ocultos ao
abrir Character, após resposta `SendItem`, troca de cena e relogin. O fluxo
visual não é `CLIENT_TESTED`.

## Validação

Build Release e ArchitectureTests devem ser registrados no handoff do lote.
Executar Character/equipamento em resolução de referência e verificar que não
há hitbox, ícone ou destino de drag para Necklace/Belt.
