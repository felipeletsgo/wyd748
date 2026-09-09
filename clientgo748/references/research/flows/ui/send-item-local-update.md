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
- UTILIZADA: descompilacao `FUN_004B263E`, `FUN_0052EAA9` e
  `FUN_0052A737`; exports focados
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

`FUN_004B263E` ocupa slot +8 de ObjectManager (`0x005A4604`). Na selecao 0x7531,
intercepta 0x182 antes da arvore; em outras cenas percorre filhos ativos.
`FUN_0052EAA9` filtra ID pelo humano receptor e chama `FUN_0052A737` para 0x182.

### Callees

`FUN_004B263E` chama `FUN_0058F220` para copiar oito bytes ao cargo.
`FUN_0052A737` compara
o receptor com o humano local antes das copias de equipamento/inventario/cargo.
O ramo Equip atualiza o cache de selecao quando DestPos !=0, antes dos grids.
O export Ghidra confirma o caller direto 0052EAA9 -> 0052A737; receptores dos
metodos virtuais de grids e teardown completo ainda precisam ser fechados.

No ramo nativo de cargo, `FUN_0052A737` chama a retirada na celula
`DestPos % 9, DestPos / 9`, guarda o retorno em local_94 e so cria/adiciona
um novo item quando `sIndex > 0`. Nao ha delete explicito de local_94 nesse
ramo, ao contrario de Carry/Equip. Ao fim do bloco do humano local ha uma
chamada virtual de cursor +0x98 se o attachment for nao nulo; sua semantica
nao foi fechada aqui. O registro anterior que igualava esses cleanups estava
incorreto: liberar o item retirado e uma correcao local, nao paridade provada.

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
Cargo captura e libera o retorno de PickupAtItem. A implementacao atual de
SGrid.cpp remove o ponteiro da lista, ajusta ocupacao/escala e retorna ownership
ao chamador; nao destroi o item nem limpa aliases de interacao. O destructor
SGridControlItem libera recursos visuais/item, mas nao limpa os aliases globais.
O caller Cargo agora zera hover, ultimo attachment, venda e cursor somente
quando apontam para o item retirado, usando a politica ja existente em Empty.
A copia de estado continua anterior a consulta visual; grid ausente nao a desfaz.

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
- Validar runtime os aliases de cursor/hover/venda apos substituicao no cargo.
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

Atualizacao: Cargo captura agora o retorno de `PickupAtItem`, limpa o cursor
quando ele aponta para o item removido e libera o objeto com `SAFE_DELETE`, em
correcao local baseada no ownership da source, sem claim de paridade. Release desse corte:
`87F0FFCCFAC29E3950979515B864D6F9B77D7F4E930C894128E7DA76EFD323C6`.

Complemento de aliases: Debug/Release passaram com os 232 checks existentes;
eles nao exercitam hover/venda no jogo. Release instalado:
`29485CB8C6570801C72C3F82D0B9BA944D16B09D74AF40638A2CEC6750353067`.
STATICALLY VERIFIED; teste in-game pendente.

Inspecao de `SGridControl::AddItem` confirmou que o helper incrementa
`m_nNumItem` sem validar capacidade interna. Essa frente permanece separada;
nao foi alterada sem contrato e teste especificos de grid.

Extensao do mesmo endurecimento: `releaseReplacedItem`, lambda local de
OnPacketSendItem, concentra a limpeza de aliases e a destruicao dos visuais
retirados de Equip/Carry/Cargo. Cada ramo conserva seu Pickup e coordenadas;
nullptr nao altera estado, e referencias a outros itens sao preservadas.
Procedencia local; reaproveita evidencia de Pickup/Empty/destructor acima,
sem novo claim nativo, ABI ou recurso. Debug/Release: 232 checks existentes
PASS; nao cobrem eventos UI. Release instalado:
`79ED78DEE7826F262C017096E006762B4755A16AA9C9FA41BD699130AE5B3C10`.
