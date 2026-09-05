---
id: send-item-local-update
title: Atualizacao local de slot por SendItem
subsystem: ui
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-05
---

# Atualizacao local de slot por SendItem

## Pergunta

Onde 0x182 atualiza armazenamento e UI na selecao e no mundo, e quais indices
precisam de endurecimento antes da copia?

## Fronteira de evidência

- UTILIZADA: binario nativo com identidade confirmada nos ciclos anteriores e
  projeto Ghidra WYD748Native_20260821, read-only/noanalysis.
- UTILIZADA: descompilacao 004B263E, 0052EAA9 e 0052A737; exports focados
  `select-character-cargo-update.tsv` e `field-send-item.tsv`.
- UTILIZADA: source atual ObjectManager::HandleSelectCharacterItem,
  TMHuman::OnPacketSendItem e arrays em Basedef/ObjectManager.h.
- UTILIZADA: WYD-Go wire.SendItem, FinishPacket/ReadPacket e teste
  send_item_contract_test.go; nao foi alterado comportamento do servidor.
- NAO APLICAVEL neste endurecimento: assets/recursos nao mudam e nenhum novo
  controle e criado. Ownership completo de grids continua pendente.
- NAO APLICAVEL: TMProject posterior/guia como prova de offsets nativos.
  W2PP, Secrets e Micronics excluidos.

## Fluxo nativo 7.48

### Callers

004B263E ocupa slot +8 de ObjectManager (005A4604). Na selecao 0x7531,
intercepta 0x182 antes da arvore; em outras cenas percorre filhos ativos.
0052EAA9 filtra ID pelo humano receptor e chama 0052A737 para 0x182.

### Callees

004B263E chama 0058F220 para copiar oito bytes ao cargo. 0052A737 compara
o receptor com o humano local antes das copias de equipamento/inventario/cargo.
O ramo Equip atualiza o cache de selecao quando DestPos !=0, antes dos grids.
O export Ghidra confirma o caller direto 0052EAA9 -> 0052A737; receptores dos
metodos virtuais de grids e teardown completo ainda precisam ser fechados.

No ramo nativo de cargo, `0052A737` remove o controle visual da célula
`DestPos % 9, DestPos / 9`, limpa o cursor quando necessário e só cria/adiciona
um novo item quando `sIndex > 0`. A mesma ordem aparece nos ramos de Carry e
Equip: retirar o antigo, limpar cursor, criar o novo e inserir na grid.

## Estado e lifecycle

Na selecao, qualquer 0x182 e consumido; somente DestType=2 escreve no cargo.
No mundo, o fluxo atual atualiza armazenamento, controles e apresentacao do
humano. O estudo ainda nao fecha todos os caminhos de cursor, grid ausente,
destruicao de item visual e reentrada; nao promove paridade integral.

## Wire, ABI e recursos

S->C: 24 bytes, opcode 0x182, DestType short signed@12, DestPos short
signed@14, item de oito bytes@16. O Go emite o mesmo frame; Size e cifragem
sao preenchidos no transporte. A funcao nativa 0055890A contem o par
0x182/24, mas sua alcancabilidade continua pendente na ficha packet-size-gate.
Asserts da source protegem tamanho/offsets. Nao transportar offsets de objeto
nativo para arrays C++ atuais.

## Mapeamento atual

ReceivedPacketDispatch valida tamanho/opcode antes do percurso legado.
ObjectManager usa ApplyCargoSlot, testado com 128 posicoes de armazenamento.
TMHuman::OnPacketSendItem agora rejeita indices negativos ou acima da capacidade
real de Equip/Carry/Cargo antes de Bag_View/copias. O cache de selecao so e
escrito com characterSlot em [0,4); ausencia desse cache nao impede atualizar
equipamento do mundo. Slots extras locais de Equip sao mantidos.
O ramo Cargo ainda usa `PickupAtItem` sem testar o retorno, como o nativo faz
com seu receptor visual; essa decisao nao foi alterada sem prova do ownership
dos grids atuais. A copia autoritativa do slot ocorre antes da consulta visual,
preservando o estado mesmo quando a grid nao estiver materializada.

O estudo de `0052A737` confirmou que o nativo tambem nao examina o retorno da
remoção visual no ramo Cargo. Ele chama o slot de retirada, depois cria o item
quando `sIndex > 0`; a ausência da grid não desfaz a cópia de estado. Portanto,
o comportamento atual é uma modernização compatível deliberada, não uma falha
identificada. O cursor é explicitamente limpo no ramo nativo quando aponta para
o item removido; a source Carry/Equip já faz isso, enquanto Cargo ainda precisa
de uma confirmação do contrato de `PickupAtItem` antes de receber mudança.

## Matriz de delta

| Fronteira | Evidencia | Decisao |
| --- | --- | --- |
| frame 24 bytes | nativo/source/Go concordam | validar antes do callback |
| cargo na selecao | copia antecipada e consumo | manter ordem, limitar a 128 |
| destinos no mundo | indices usados antes dos guards visuais | limitar pela capacidade atual |
| cache da selecao | indice de personagem separado do destino | proteger sentinela sem bloquear mundo |
| ownership visual | ainda incompleto | preservar codigo, nao alegar paridade |

## Decisões

MODERNIZACAO_COMPATIVEL de origem local: rejeitar indices impossiveis e manter
ordem/bytes dos caminhos validos. Nao copiar vulnerabilidade do pseudocodigo.
Nao reduzir armazenamento de cargo 128 para limite de uso 120, nem remover
Equip[16..17] por ausencia no nativo. Nenhuma mudanca wire, asset ou vtable.

## Lacunas

- Exercitar rejeicao e atualizacao de Equip/Carry/Cargo no candidato real.
- Fechar ownership do item removido por PickupAtItem no cargo e cursor.
- Verificar as chamadas de OnPacketEvent fora da entrada size-aware.
- Preservar e testar relogin, slots reservados e refresh visual.

## Validação

Exports Ghidra concluidos sem SCRIPT ERROR; hash program nativo correto,
slot_outgoing 005A4604 -> 004B263E e caller 0052EAA9 -> 0052A737 presentes.
232 checks C++ e asserts PASS em Debug/Release; suite wire Go e vet PASS no
corte anterior, sem alteracao posterior desses inputs. Esses testes cobrem
frame e ApplyCargoSlot; nao executam os novos guards dentro de TMHuman.
Debug/Release recompilados e instalados depois dos guards de TMHuman.
Release: `5D1743014D80B9E34FEBE61CA8A3ADC34327363388A6841DCFA25C5C88A830AE`.
STATICALLY VERIFIED para o novo corte. Nao CLIENT-TESTED.
