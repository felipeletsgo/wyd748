# Continuidade: arquitetura TMProject748

## Estado em 2026-09-05

Objetivo: executar `client-source/tmproject/docs/implementation-plan.md` sem
alterar wire/ABI/lifecycle. Modo do lote: MODERNIZACAO_COMPATIVEL, origem local.
Worktree contem numerosas alteracoes anteriores e exclusoes do usuario;
preserva-las. Nenhum commit/push realizado neste lote.

Implementado: `PacketView::HasSizeBetween`; guards antes dos casts em
TMUtil/CPSock/ITransport; `MutablePacketView` para envio (140 pontos migrados);
`PacketSendBoundary.h` compartilhado pelas duas fachadas, sem const_cast;
comentarios de ownership; testes C++ puros em
`TMProject748/tests`, executados por `Build-Client.ps1` antes da instalacao.
Documentacao detalhada: `docs/target-architecture.md`, secao de transporte.

Novo lote: ITransport nao depende de Basedef; SocketTransport<CPSock> tem
consumidor vivo em TMSelectCharScene. RequestCharacterLogin valida a intencao
usando ICharacterLoginSender; CharacterLoginSender codifica 0x213 por ITransport.
MessageHeader/CharacterLoginPacket sao os donos dos structs, reexportados pelos
headers antigos. Layout, bytes, tipos, ordem de UI/timers e ownership preservados.
PacketView pertence a application/ports; os quatro consumidores migraram e a
fachada wire redundante foi removida. Corrigido caminho obsoleto de
SharedStructs no projeto/filtros para CommonFiles local.

Lotes seguintes implementados: SendBuffer (limites, compactacao e propagacao
de falha de enqueue), ReceiveBuffer (janela e frame), guards em CPSock e
desconexao ordenada. NewApp entrega PacketView a ObjectManager::OnPacketView,
nao virtual, que valida envelope e chama o virtual legado. Relogio/dump leem
cabecalho const antes do callback. Ordem/percurso e eventos locais preservados.
O buffer deve ser originalmente gravavel; const_cast fica no ObjectManager.
Ainda nao migrados os callbacks de cena char* nem validacao por opcode.

Novo corte: `ObjectManager::HandleSelectCharacterItem` extraiu o consumo
antecipado de `MSG_SendItem` na selecao de personagem. A ordem anterior,
condicao de cena, destino 2 e retorno imediato foram preservados. Nenhum novo
claim nativo ou contrato wire foi criado.

Novo corte seguinte: `TMSelectCharScene::HandleJudgementEffect` extraiu o caso
`0x3B4` do dispatcher. Geometria, limites e lifecycle do efeito foram
preservados; nenhuma fronteira wire/ABI mudou. Debug e Release recompilados,
com 118 checks PASS; Release instalado com SHA-256
`0721B29CE9A4730CF54B5641E77E8B711162A0E87C66FA81513FF2AEAA46F8E7`.

Validacao atual: Build-Client.ps1 Debug e Release passaram com 118 checks e
asserts em cada configuracao, incluindo entrega unica e rejeicoes do dispatcher.
Debug SHA-256: A395C7FD2777C343BAB4AFC8D4B5930421B8C03BB8F017089A708DD65FA0BC34.
Release instalado em `client748/project.exe`, SHA-256
`08A10A1C02DA956A047C4B2274FB3E14A2D220BD6629BA7EB636410F7031280A`.
XML de projeto/filtros e registro unico dos headers novos PASS; diff --check PASS.
Instalacoes passaram sem encerrar processos. Nenhum build ficou executando.
Warnings legados C4018/C4305/C4309 permanecem. Nao CLIENT_TESTED.

## Proximo passo executavel

Proximo gate: executar no candidato login aceito/recusado, selecao, mundo,
retorno e logout/relogin. Nao confundir testes puros com teste de socket/jogo.
Depois, migrar um handler pequeno de recepcao mantendo tamanho ate a cena;
partir de ObjectManager::OnPacketEvent e TMSelectCharScene::OnPacketEvent.
O primeiro intercepta MSG_SendItem na selecao antes do percurso TreeNode;
preservar essa ordem. Eventos locais sem frame devem continuar separados.
Nao adicionar slots virtuais nem converter PacketView const em mutavel de
forma generica. Demais casos de uso e decomposicao dos arquivos extensos pendem.
Evidencia reutilizada para login: scene-transition.md, evento 0x1204,
FUN_004A32DD/callsite 0x004A3422. Sem promocao de maturidade.
Nao alegar que todos os const_cast da aplicacao foram eliminados.

Fases 1–7 continuam com gates abertos, incluindo casos de uso, fragmentacao,
testes de dispatch e smoke in-game. Nao reabrir triagem global ou chats.

## Corte 2026-09-05 — mutacoes da lista de personagens

`TMSelectCharScene::OnPacketEvent` agora delega `MSG_CNFNewCharacter_Opcode`
e `MSG_CNFDeleteCharacter_Opcode` para `HandleCharacterCreated` e
`HandleCharacterDeleted`. As duas funcoes preservam ocultacao do painel,
copia de `STRUCT_SELCHAR`, reset do controle 4616 na exclusao e a ordem de
`ReloadCharList`. Trata-se de `MODERNIZACAO_COMPATIVEL`, sem alteracao de
wire/ABI/vtable.

Debug compilou e instalou com 118 checks PASS, SHA-256
`ECFAC55A2E0BDC16E3E218489082B819D2289DBAD13CC0BCD7D7DF4924F96BA4`.
Release compilou com 118 checks PASS, mas a copia para `client748/project.exe`
foi bloqueada pelo Windows porque o arquivo esta aberto; nao foi encerrado
nenhum processo. `git diff --check` deve ser executado apos liberar o arquivo.

Proximo corte: extrair o proximo bloco autocontido de `OnPacketEvent`, depois
repetir Debug/Release e atualizar este handoff. O teste in-game continua
pendente e o objetivo global permanece incompleto.

## Corte seguinte - mensagens de operacao

`TMSelectCharScene::ShowCharacterOperationMessage` consolidou a exibicao das
mensagens dos casos `0x11A`, `0x11B` e `0x119`, preservando indices 19, 20 e 21,
duracao 2000 e visibilidade do painel. Nenhum contrato wire/ABI foi alterado.
Release recompilado e instalado em `client748/project.exe`, com 118 checks e
asserts de arquitetura PASS; `git diff --check` PASS. SHA-256 atual:
`C4C65B1992ACBFC99CC39059869AA38E1849D2426AC06DC76E390FA20C8D13C7`.
Teste in-game e compatibilidade total continuam pendentes.

Debug posterior tambem foi recompilado e instalado com 118 checks e asserts de
arquitetura PASS; `git diff --check` PASS. SHA-256 do candidato Debug:
`5587F7FF2ABBDBD66EAC7DCECF857313C9F933C6622E8A7193E001C4050AC22D`.
`MSG_ReqTransper` permanece sem alteracao por combinar protocolo, estado, UI e
lifecycle; requer rastreamento nativo antes de novo corte.

## Pesquisa seguinte — contrato de transferencia 0xFAA

Release atual compilado e instalado via Build-Client.ps1: 118 checks PASS,
SHA-256 `89359107F67A84164D408930B50C5C78B55ADFA16087CC3B8318BB250F757FE1`.
Diff inicial contem tambem HandleMoveServerNotification e reposicionamento do
cast apos guard nulo; preservados. Nenhuma alteracao funcional neste ciclo.
Nova evidencia em `flows/ui/select-character-transfer.md` (LOCATED) e
`exports/select-character-transfer-focused.tsv`: Ghidra read-only confirma
callback 004a626e via slot 005a44b8, emissor 004a32dd e gate 0055890a/52 bytes.
Encontrados deltas de duracao de mensagens, foco e transicoes de paineis;
nao substituir automaticamente pelo nativo. Fonte atual e nativo so rejeitam
Slot negativo; falta decidir limite superior e humano opcional.
Proximo passo: initializer 0049f0e7/bindings rename, cancelamento e timeout em
004a32dd/FrameMove; fechar receptores de cleanup antes de adaptar 0xFAA.
Nao repetir pesquisa de 0x52A: migracao de servidor e outro contrato.

## Corte seguinte — limites e controles de transferencia

Implementado endurecimento local (MODERNIZACAO_COMPATIVEL) na source:
slots fora de [0,4) rejeitados nos dois emissores, no retorno 0xFAA e na
confirmacao por mouse; humano ausente bloqueia a confirmacao por mouse.
Resposta de sucesso sem edit/humano usa nome do slot terminado e preserva
limpeza. Result 1 protege painel opcional. Sem mudar duracoes ou UI valida.
Assets confirmados por parser ate EOF: 1568/1569/628/629 em SelCharScene2.bin.
Ghidra confirma initializer/construtor/vtable; FrameMove nativo nao tem timeout
de transferencia, apenas debounces independentes. Ficha continua LOCATED.
Debug/Release: 118 checks existentes PASS, ambos instalados; Release atual
`08805042EA57594405431136EB00A9528ED16EC8B45C862487508313234186AD`.
Esses checks nao exercitam os guards de UI: teste dinamico ainda pendente.
Proximo passo: preservar comprimento do frame ate handler selecionado e
validar 0xFAA antes de ler Result/Slot, sem mudar slots virtuais; estudar
ObjectManager/TreeNode para manter percurso e eventos locais separados.

## Corte seguinte — gate de frame 0xFAA

Implementado `wire/ReceivedPacketDispatch.h` em ObjectManager::OnPacketView:
0xFAA exige tamanho real/declarado 52 e opcode coerente antes do callback;
rejeicao nao executa handler. Outros opcodes e eventos locais preservados.
`CharacterTransferPacket.h` possui struct/opcode unicos, reexportados por
Basedef, com asserts de largura/tamanho/offsets. Nenhuma vtable alterada.
185 checks PASS Debug/Release, incluindo testes do helper vivo com bytes
conhecidos, truncamento, divergencias, nao mutacao e entrega unica/fallback.
Release instalado: `A256F702F9487AAD9E38DA2A0DA9EFBC57CB89C532CBC6B556A1ABFABE5DFC54`.
XML/paths PASS. Nao CLIENT-TESTED; maturidade nativa integral continua LOCATED.
Push do corte anterior 7536efb1 falhou: GCM wincredman indisponivel e token gh
invalido. Nao repetir autenticacao sem mudanca desse input; trabalho local segue.
Proximo contrato: MSG_SendItem interceptado antes da arvore em
HandleSelectCharacterItem. Conferir tamanho nativo/offsets e DestPos antes de
ampliar o gate; nao inferir seguranca dos demais payloads pela validacao 0xFAA.

## Corte seguinte — contrato SendItem no baú da seleção

Ghidra confirmou `FUN_004B263E` como dispatcher da árvore e o ramo especial
`0x182`: com cena/estado compatíveis e `DestType==2`, copia exatamente 8 bytes
do item em `param+0x10` para `cargo+0x2EC + DestPos*8`; gate nativo
`FUN_0055890A` confirma 0x182/0x18 (24 bytes). O export está em
`exports/select-character-cargo-update.tsv`, hash nativo mantido.

Implementado `wire/SendItemContract.h` (opcode/tamanho), asserts na struct
legada, `wire/ReceivedPacketDispatch.h` agora valida também `0x182` antes do
callback e `application/ApplyCargoSlot.h` limita o índice à capacidade física
128 sem alterar o limite de uso 120. `HandleSelectCharacterItem` usa a função
pura; destino inválido é consumido sem escrita nem segundo dispatch. Go mantém
o mesmo builder/contrato e recebeu teste round-trip com `FinishPacket`/`ReadPacket`.
Testes C++: 232 checks PASS Debug/Release; Go `go test ./internal/wire`
selecionado e suíte wire PASS; `go vet ./internal/wire` PASS; PROJECT_PATHS_OK
e `git diff --check` PASS. Release candidato atual:
`377E573AD765DCD56EF1A7058A055EF2A27A9CFDB9C0B68EF9A6ED9D08B765F`.
Não CLIENT-TESTED; nenhum callback legado além dos contratos migrados foi
alterado. Próximo passo: revisar o handler `MSG_SendItem` na cena e o limite
de `DestPos` observado no fluxo real antes de migrar outro opcode.

## Corte seguinte — destinos de SendItem no mundo

Consumidor localizado em TMHuman::OnPacketSendItem (nao TMFieldScene).
Ghidra 0052EAA9 -> 0052A737 confirma filtro de ID e atualizacao local.
Implementados guards de destino Equip/Carry/Cargo pela capacidade real antes
das copias/efeitos e guard do characterSlot ao atualizar cache de selecao.
Sem remover slots extras ou mudar UI valida. Ficha propria agora em
`flows/ui/send-item-local-update.md`; exports de selecao e campo versionados.
Debug/Release passaram, 232 checks existentes; Release instalado
`5D1743014D80B9E34FEBE61CA8A3ADC34327363388A6841DCFA25C5C88A830AE`.
Nao CLIENT-TESTED; teste runtime dos guards de TMHuman continua pendente.
Proximo ponto concreto: ramo cargo em OnPacketSendItem ignora retorno de
PickupAtItem, diferente de Carry/Equip. Seguir ownership/cursor no nativo
0052A737 e SGrid antes de decidir descarte do item visual.

## Evidencia adicional — ownership visual SendItem

O export `field-send-item.tsv` confirma `0052EAA9 -> 0052A737` e o ramo nativo
de cargo: copia oito bytes para `cargo+0x2EC + DestPos*8`, resolve a grid por
`DestPos%9, DestPos/9`, remove o visual antigo e cria o novo somente se
`sIndex>0`; o cursor e limpo quando aponta para o item retirado. Carry/Equip
seguem a mesma ordem. A source preserva essa ordem e deixa `PickupAtItem`
sem retorno usado, aguardando ownership dos helpers atuais.
Nenhum novo patch funcional neste ciclo; ficha `send-item-local-update.md`
atualizada. Guards de capacidade permanecem STATICALLY VERIFIED pelos builds
anteriores; teste visual, cursor, grid ausente e relogin ainda pendentes.

Reaberta a descompilacao 0052A737: o ramo nativo Cargo tambem ignora o retorno
da retirada visual, cria o item somente para sIndex>0 e preserva a copia de
estado antes da consulta de grid. A source coincide nessa ordem; Cargo ainda
nao limpa explicitamente o cursor porque `PickupAtItem`/ownership local não
estão comprovados. Nenhuma alteração funcional neste ciclo. Próximo passo
continua revisar esse helper no SGrid e testar item/cursor no candidato antes
de alterar o fluxo.

Implementado: Cargo captura e libera o retorno de `PickupAtItem` e limpa o
cursor quando necessário, alinhado ao nativo 0052A737. Debug/Release passaram
com 232 checks; Release instalado com SHA-256
`87F0FFCCFAC29E3950979515B864D6F9B77D7F4E930C894128E7DA76EFD323C6`.
Teste visual/runtime ainda pendente.

Validacao de fechamento do ciclo SendItem: `TMHuman::OnPacketEvent` encaminha
0x182 para OnPacketSendItem, e a entrada de rede observada e exclusivamente
NewApp -> ObjectManager::OnPacketView -> OnPacketEvent. Eventos locais mantem
seus callbacks diretos; nao foi encontrada segunda entrada de rede para esse
contrato. `go test -count=1 ./...` PASS em todos os pacotes em 2026-09-05;
`git diff --check` PASS. Mantido Release
`87F0FFCCFAC29E3950979515B864D6F9B77D7F4E930C894128E7DA76EFD323C6`.
Proxima prioridade: executar o fluxo real SendItem/cursor/relogin; antes de
outro corte funcional, escolher um novo contrato de recepcao com ficha nativa.

## Correcao de evidencia e aliases Cargo

Os registros anteriores que atribuiam a liberacao do visual Cargo ao nativo
estavam incorretos: 0052A737 nao destroi local_94 explicitamente. A chamada
final do cursor +0x98 ainda exige interpretacao. A liberacao implementada e
correcao local baseada em PickupAtItem, que remove da lista e devolve ownership.
Destructor de SGridControlItem nao limpa referencias de interacao. Cargo agora
limpa tambem m_pLastMouseOverItem/m_pLastAttachedItem/m_pSellItem quando iguais
ao removido, alem do cursor; politica igual a SGridControl::Empty.
Debug/Release passaram, 232 checks existentes (nao teste de UI). Release:
`29485CB8C6570801C72C3F82D0B9BA944D16B09D74AF40638A2CEC6750353067`.
Proximo: testar confirmacao de venda/hover/cursor apos 0x182 no jogo. Claims
nativos corrigidos na ficha send-item-local-update; nao repetir a inferencia
de paridade. Ownership de Carry/Equip pode ter os mesmos aliases pendentes.

## Corte Carry/Equip — cleanup unico no SendItem

Carry/Equip tinham a mesma lacuna confirmada por source: liberavam o visual
mas so limpavam o cursor, sem hover/venda/ultimo attachment. Lambda local
releaseReplacedItem agora atende os tres ramos; nao muda Pickup, coordenadas,
ordem de copias ou ownership da grid. nullptr e alias de outro objeto ficam
intactos. Delta MODERNIZACAO_COMPATIVEL local, sem claim de paridade.
Debug/Release passaram com 232 checks existentes; Release instalado:
`79ED78DEE7826F262C017096E006762B4755A16AA9C9FA41BD699130AE5B3C10`.
Nao CLIENT-TESTED. Proximo ponto: criacao visual no ramo Equip ocorre mesmo
sem grid de destino; verificar AddItem e ownership de falha antes de corrigir
alocacoes sem owner. Teste in-game de hover/venda/relogin segue pendente.

## Guard da capacidade de insercao

AddItem/AddSkillItem/SetItem rejeitam ponteiro nulo, contador negativo e
contador >= capacidade do array antes de qualquer escrita. Retorno 0 mantem
ownership no caller. Ocupacao/coordenadas/recorte legados permanecem iguais;
a tentativa inicial de chamar CanItAdd foi retirada antes da entrega.
SendItem agora libera visuais rejeitados por AddItem e nao aloca visual de
Equip quando sua grid nao existe. Origem local, MODERNIZACAO_COMPATIVEL;
evidencia nativa reutilizada 0040E6AA (copia/append/retorno 1, sem teste de
ocupacao) e source SGrid/Empty/destructor ja lidos. Nao novo claim nativo.
Debug/Release passaram com 232 checks existentes, diff --check PASS. Estes
checks nao exercitam SGrid cheio. Proximo passo obrigatorio: teste focado da
rejeicao sem mutacao/ownership e revisar callers que ignoram retorno 0.
Coordenadas negativas/overflow e AddItemInEmpty nulo nao foram corrigidos.

## Teste do gate de insercao e busca vazia

GridInsertion.h concentra CanAppend/Execute; AddItem/AddSkillItem/SetItem
executam seus corpos legados dentro do callback, somente depois do guard.
GridInsertionTests usa o mesmo gate com lista/ocupacao/ownership observaveis:
preenche 128, rejeita 129o, valida todos os slots preservados, nullptr,
INT_MIN/-1/129/INT_MAX, capacidade derivada de outro array e falha sem retry.
AddItemInEmpty agora testa o mesmo CanAppend antes de ler dimensoes ou buscar;
lista cheia e nullptr retornam {-1,-1}. Nao houve mudanca de ocupacao/geometria.
Debug/Release passaram com 496 checks e instalacao. Release:
`A5C6BA276A332801A760E5CA6CD311FF9B6DB66E4C3C6E756E0A122EACBBB970`.
Teste cobre gate/callback, nao render real ou todos os callers. Proximo ponto:
revisar SetItemOnGrid e callers de AddItemInEmpty para ownership na rejeicao;
coordenadas negativas/overflow continuam pendentes. Push segue sem credencial.

## Retomada — rejeicao nas duas barras de skills (2026-09-05)

O commit 691f0763 ja fechou SetItemOnGrid. A worktree continha o corte
seguinte em TMFieldScene::UpdateScoreUI: os dois callers de AddItemInEmpty
liberam o visual quando o retorno tem x < 0. Revisao da source confirmou
que o helper retorna {-1,-1} sem transferir ownership na rejeicao; no sucesso
retorna coordenadas nao negativas e a grid retém o item. Busca scoped confirma
que estes sao os dois callers dessa API (CanAddItemInEmpty e outra API).
Delta local MODERNIZACAO_COMPATIVEL; source e testes UTILIZADOS. Evidencia
nativa anterior reutilizada apenas como contexto de grid; nenhum novo claim
de paridade. Assets, wire e servidor NAO APLICAVEIS: sem alteracao nesse corte.

Build-Client.ps1 Debug e Release PASS, 496 checks e static assertions PASS
em cada configuracao; instalacao e hash conferidos pelo script. Release:
`306DC514287FC47C6AA68A3ABFE71772B4959B9297D8D25E87E78814411F5A0B`.
git diff --check PASS antes do registro. Os checks exercitam o gate puro,
nao UpdateScoreUI/render real; nao CLIENT_TESTED. Mudancas preexistentes em
AGENTS.md e cinco arquivos de skills preservadas fora deste corte.

Proximo passo: validar barras de skills, rejeicao visual e relogin no candidato;
seguir a revisao dos callers diretos de AddItem/SetItem que ignoram rejeicao.
Coordenadas negativas/overflow continuam uma frente separada pendente.

## Retomada — falha de alocacao no SendItem (2026-09-05)

Revisados os tres callers ativos de `SGridControl::AddItem` em
`TMHuman::OnPacketSendItem` (Equip, Carry e Cargo). Quando a alocacao do
`SGridControlItem` falhava depois de `STRUCT_ITEM`, faltava liberar o estado
intermediario. O corte agora usa `SAFE_DELETE(pstItem)` somente nesse caminho;
falha de AddItem continua usando `releaseReplacedItem`, e sucesso preserva o
ownership da grid. Delta local MODERNIZACAO_COMPATIVEL, sem mudanca wire,
ABI, recurso ou ordem do handler.

Build-Client.ps1 Debug e Release PASS e instalaram o candidato. Release:
`7C336D49F07491BA7AEC3A17E750234C22B68807E6190B52093790FF03102A8B`.
`git diff --check` PASS. Nao CLIENT_TESTED; falha de alocacao e fluxo visual
real ainda nao sao exercitados pelos checks puros. Proximo: revisar somente
os callers restantes de AddItem que alocam visuais dinamicamente, sem alterar
listas de texto/controlos nem inventar nova politica de ownership.

## Retomada — visuais de skill belt e pickup (2026-09-05)

Dois callers adicionais com alocacao dinamica foram corrigidos em
`TMFieldScene`: o skill belt agora libera `STRUCT_ITEM` se a criacao do
controle falhar ou libera o controle se `AddItem` rejeitar; `OnPacketCNFGetItem`
aplica a mesma politica para a grade de carry. Sucesso continua transferindo o
ownership para a grid, sem alterar wire, coordenadas ou ordem de efeitos.
Delta local MODERNIZACAO_COMPATIVEL.

O primeiro build revelou apenas um erro de associacao de `else`, corrigido no
mesmo ciclo. Release final passou pelo `Build-Client.ps1`, 496 checks e asserts
PASS, candidato instalado com SHA-256
`B43CFECCCBD8C5E14D419956BD8E7D7EA7E00325B7EFDA57823229CA524B2603`.
`git diff --check` PASS. Nao CLIENT_TESTED. Restam callers dinamicos em
inventario/loja/mixagem, a revisar individualmente.

## Retomada — resultado e necessidade em MrItemMix (2026-09-05)

Em `CItemMix::ResultItemListSet` e `Set_NeedItemList`, os callers alocavam
`STRUCT_ITEM` e passavam um `SGridControlItem` diretamente para `AddItem`,
sem tratar grid nula, falha de construcao ou rejeicao da insercao. Foi aplicado
um helper local em cada fluxo: sucesso transfere ownership para a grid; falha
libera o controle ou o `STRUCT_ITEM` conforme o ponto de falha. Nenhum wire,
regra de combinacao ou ordem visual foi alterado. Delta local
MODERNIZACAO_COMPATIVEL.

O primeiro build encontrou apenas dangling-else causado pelo macro legado
`SAFE_DELETE`; os dois helpers foram ajustados com blocos explicitos. Release
passou com 496 checks/asserts e candidato instalado. SHA-256:
`5ED255E897D8C0DABA6589A68B1E16FD209479C281C776DF00961EF01E0DEF39`.
`git diff --check` PASS; nao CLIENT_TESTED. Os fluxos de `ClickInvItem` ainda
possuem varios callers repetidos e ficam para o proximo corte isolado.

## Retomada — callers repetidos de ClickInvItem (2026-09-05)

Os cinco callers de `ClickInvItem` que criavam `STRUCT_ITEM` e inseriam
diretamente em `m_pGridNeedItem[i]` foram concentrados em `AddOwnedMixItem`.
O helper preserva a grid como owner em sucesso e libera corretamente o
controle ou o item em grid nula, falha de construcao ou rejeicao de `AddItem`.
Nao houve mudanca na selecao de receita, efeitos ou contrato de inventario;
delta local MODERNIZACAO_COMPATIVEL.

Release passou pelo `Build-Client.ps1`, 496 checks/asserts PASS e candidato
instalado. SHA-256: `9ECB13AAB6FF8DA45E9B5C98124CC6D6257C507EB623ED1228FC636EBD7F784D`.
`git diff --check` PASS; nao CLIENT_TESTED. Permanecem apenas os callers
legados de MrItemMix que criam visuais em outras rotinas de inicializacao,
fora do fluxo ClickInvItem.

## Retomada — inicializacao de Carry e Cargo (2026-09-05)

Na inicializacao de `TMFieldScene`, os dois loops que materializam Carry e
Cargo criavam `SGridControlItem` diretamente e ignoravam a rejeicao da grid.
Agora cada loop libera o controle quando `AddItem` falha e libera o
`STRUCT_ITEM` quando a criacao do controle falha; o ramo de pagina invalida
continua liberando o item como antes. Nao houve alteracao de indices,
coordenadas, capacidade ou contrato. Delta local MODERNIZACAO_COMPATIVEL.

Release passou pelo `Build-Client.ps1`, 496 checks/asserts PASS e candidato
instalado. SHA-256: `CA455069E7CF35CE81390087504800FE125E6F17CB3136A7B2DCD81F5F4DBC2D`.
`git diff --check` PASS; nao CLIENT_TESTED. Os grids de equipamento da mesma
inicializacao ainda usam insercoes diretas e serao tratados em corte separado.

## Retomada — ownership dos grids de equipamento (2026-09-05)

As quinze insercoes diretas de equipamento na inicializacao de
`TMFieldScene` foram concentradas em `WYD748_AddOwnedGridItem`. O helper
protege grid nula, falha de criacao e rejeicao de `AddItem`, mantendo a grid
como owner no sucesso. Indices, slots, coordenadas e ordem da inicializacao
foram preservados; delta local MODERNIZACAO_COMPATIVEL.

Release passou pelo `Build-Client.ps1`, com 496 checks/asserts PASS e candidato
instalado. SHA-256: `AF66062D737F6906DEEC0062B327F8EAEC37AED58C7F4600349CD9399408989D`.
`git diff --check` PASS. Nao CLIENT_TESTED. A fila de ownership continua em
callers de loja/drop/mixagem fora desta inicializacao.

## Correcao de escopo — populacao do ramo 7.48 (2026-09-05)

InitializeScene seleciona InitializeCompatFieldScene e retorna antecipadamente
quando o recurso nao possui o controle 66817. Portanto os cortes de inicializacao
Carry/Cargo/equipamento acima cobrem o ramo do recurso moderno; nao comprovam
correcao da populacao no ramo 7.48. Nao repetir essa fila como se fosse o fluxo
ativo observado. A execucao runtime de cada ramo ainda nao foi verificada.

O lambda populateGridItem do ramo 7.48 atende Carry, quinze equipamentos e
Cargo. Agora destroi gridItem quando AddItem retorna 0; o destructor libera
itemCopy. Sucesso, coordenadas, ordem e estado autoritativo permanecem iguais.
MODERNIZACAO_COMPATIVEL local. Source e testes UTILIZADOS; evidencia nativa
anterior de binding/ownership reutilizada sem novo claim. Wire, assets e Go
NAO APLICAVEIS ao delta. Nao acrescenta tratamento de excecao de alocacao.

Debug e Release via Build-Client.ps1 PASS, 496 checks/asserts cada; esses checks
cobrem gate puro, nao o lambda nem render. Release instalado/hasheado:
`A87D7F1BAF3B353F096FE044C71B3BCB063C90DA0B4DDE648CFCD971F5C1599E`.
Nao CLIENT_TESTED. Proximo: retomar fase 2 do implementation-plan pelo proximo
contrato de recepcao com evidencia nativa; nao prolongar revisao de ownership
por buscas de nomes sem confirmar callers e alcance do ramo.

## Fase 2 — mensagens opacas 0x102/0x104

Extraidos opcodes, structs e asserts de Basedef para wire/LegacySceneMessagePacket.h;
Basedef reexporta o contrato. Reutilizada ficha CONTRACT
ui/legacy-scene-message-102-104-consume.md: gate nativo 0055890A fixa 116/152
bytes; 0049889A consome payload opaco. ReceivedPacketDispatch agora confere
comprimento real/declarado e Type/opcode antes do ObjectManager encaminhar
para callbacks. Handler de cena e fallback dos demais opcodes preservados.
MODERNIZACAO_COMPATIVEL local, sem promover claim nativo ou CLIENT_TESTED.
Source/testes e ficha nativa UTILIZADOS; assets e Go NAO APLICAVEIS (sem
payload interpretado nem emissor Go existente). Nenhum builder inventado.

Debug/Release Build-Client.ps1 PASS, 782 checks/asserts cada. Fixtures independentes
116/152 cobrem todos os prefixos, excesso, nulo, Size/Type/opcode divergentes,
entrega unica, identidade do buffer e bytes preservados. PROJECT_PATHS_OK e
diff --check PASS. Teste do consumo real no jogo continua pendente.
Proximo: migrar outro contrato fechado para wire; fases 1-7 e seus gates
runtime permanecem incompletos conforme implementation-plan.md.
Release instalado com hash identico ao artefato:
`D2ABA7BA33609FD92FF578F36A9BA5B186FE2671ACCA2D605795F8C706C97971`.

## Fase 2 — chat local 0x333

ChatMessagePacket.h passa a possuir opcode, struct e asserts (108 bytes,
String em offset 12); Basedef apenas reexporta. ReceivedPacketDispatch valida
frame real/declarado e ambos os discriminantes antes dos handlers existentes
TMFieldScene::OnPacketMessageChat e TMHuman::OnPacketMessageChat. Buffer segue
emprestado e gravavel; nao altera terminacao, texto, roteamento ou baloes.
0x105/0x106 continuam extensoes separadas, sem novo gate neste corte.

MODERNIZACAO_COMPATIVEL local. Ficha CONTRACT ui/local-chat-message.md
reutilizada: 0055890A fixa 0x333/108; 0052EAA9 -> 0052D060 e 00481DD6
decidem os consumidores. Source, evidencia nativa documentada e Go/testes
UTILIZADOS; assets NAO APLICAVEIS (sem alteracao). Nenhum claim nativo novo.

Debug/Release Build-Client.ps1 PASS, 900 checks/asserts cada. Novos testes
cobrem fixture ID/texto, todos os prefixos, excesso, nulo, divergencias,
entrega unica e bytes preservados. Go: go test ./internal/wire -run
'Chat|Message' -count=1 PASS. PROJECT_PATHS_OK e diff --check PASS.
Release instalado/artefato com SHA-256 identico:
`172D9879E633728323B74073ED36174764EF92B229EDD11CF39AD10C43DAB528`.
Nao CLIENT_TESTED; chat entre dois clientes e fallback de party sem humano
materializado continuam pendentes. Proximo contrato: avaliar as extensoes
0x105/0x106 usando sua ficha coordenada, antes de ampliar o gate.

## Fase 2 — avisos coordenados 0x105/0x106

IndexedMessageContract.h possui os opcodes e o comprimento de 108 bytes,
reutilizando MSG_MessageChat sem nova struct. Basedef reexporta; o gate
ReceivedPacketDispatch agora rejeita comprimento real/declarado ou Type/opcode
divergentes antes de TMScene. ID/seletor/indice/CSV continuam no parser existente;
nao foi introduzida validacao semantica global nem alterada a UI.

Modo MODERNIZACAO_COMPATIVEL de contrato EXTENSAO_COORDENADA existente.
Ficha ui/indexed-parameterized-message-extension.md reutilizada sem promover
UNMAPPED: nao existe claim de equivalente nativo. Source, builders Go e testes
UTILIZADOS; fronteira nativa documentada reutilizada; assets NAO APLICAVEIS
ao delta. A compilacao nao comprova exibicao dos avisos ou fallback.

Debug/Release Build-Client.ps1 PASS com 1134 checks/asserts cada. Fixtures
108 bytes com -938/CSV cobrem prefixos, excesso, nulo, discriminantes, entrega
unica e preservacao de bytes. Go: go test ./internal/wire -run
'Indexed|Parameterized' -count=1 PASS. PROJECT_PATHS_OK e diff --check PASS.
Release instalado/artefato conferidos com hash identico:
`98AE6DB6267F87157E31457FE06928F3C870F6A980707B139790FF6C96260428`.
Nao CLIENT_TESTED. Restam convite de grupo, compra sem gold, indices vazios,
zero/seis parametros e relogin no jogo. Proximo corte da fase 2: localizar
contrato fechado para outro opcode; nao presumir todos os envelopes de 108
bytes equivalentes ao chat.

## Fase 2 — migracao de servidor 0x52A

ServerMigrationPacket.h possui MSG_CNFRemoveServer e asserts 80/12/28;
Basedef reexporta. Opcode nomeado usado pelo dispatch Field e replay local.
ReceivedPacketDispatch valida tamanho real/declarado e Type/opcode antes de
OnPacketCNFRemoveServer copiar a imagem completa. Replay, janela de 15s,
ID do personagem e reconexao permanecem no handler existente.

MODERNIZACAO_COMPATIVEL local. Ficha CONTRACT
lifecycle/field-scene-rebuild-after-server-move.md reutilizada; source e
testes UTILIZADOS, assets NAO APLICAVEIS. Busca atual nao encontrou emissor
0x52A/RemoveServer em internal/wire ou internal/game; nenhum builder criado.
O gate nao valida TID, sua terminacao ou indice de servidor usado por sscanf:
esses limites precisam de corte proprio antes de alegar seguranca do handler.

Debug/Release Build-Client.ps1 PASS, 1224 checks/asserts cada. Fixtures cobrem
todos os prefixos, excesso, nulo, divergencias, entrega unica e copia integral
80 bytes (conta 16/ticket 52). Nao executam socket/replay real. XML/caminhos e
diff --check PASS. Release instalado com hash identico ao artefato:
`5EF55BE55059D2BAA9A85BA7BCF4D11B7731EC8CD38381FF82DA01F7CA23C714`.
Nao CLIENT_TESTED. Proximo: investigar parsing limitado de TID e capacidade
real de g_pServerList, preservando o contrato documentado de migracao.

## Parsing limitado do ticket de migracao

Source confirmou tabela MAX_SERVERGROUP x MAX_SERVERNUMBER x 64 e sscanf
sem retorno/limites no handler. Corpus 00484d44 reutilizado confirma leitura
em +0x1c e indexacao com strides 0x40/0x2c0. Delta local
MODERNIZACAO_COMPATIVEL: ParseMigrationServer copia no maximo 52 bytes para
buffer terminado, le prefixo decimal e rejeita conversao vazia, ERANGE,
negativo ou indice >= capacidade. Mantem sufixo opaco e sintaxe decimal
legada (espaco/sinal positivo). Saida fica intacta na rejeicao.
OnPacketCNFRemoveServer valida tambem grupo antes de efeitos/conexao no
ramo socket fechado. Replay no ramo socket aberto nao muda.

Source, corpus/ficha existente e testes UTILIZADOS; Go/asset NAO APLICAVEIS.
Sem claim novo de paridade. Testes puros cobrem limites, overflow, prefixo,
sufixo e ticket sem NUL. Nao exercitam socket, tabela real ou reconexao.
Ainda pendentes: validar terminacao do endereco carregado de g_pServerList,
fluxo de erro visual e migracao real antes de CLIENT_TESTED.
Debug/Release Build-Client.ps1 PASS (1242 checks/asserts); Release instalado
com hash identico ao artefato:
`EDDA60A71ACD1F0C0A0A2DB62A73CC2D210D12EF896C4BEDB991BCE2E8DDF34B`.

## Endereco limitado na reconexao

BASE_InitializeServerList decodifica todos os 64 bytes de cada entrada sem
garantir terminador. NewApp::m_szServerIP tem 128 bytes. O handler agora
procura NUL somente dentro da entrada selecionada e rejeita vazio/ausente
antes dos efeitos de reconexao. Copia apenas comprimento+NUL, com static_assert
de capacidade do destino. Nao trunca ou inventa endereco alternativo.
MODERNIZACAO_COMPATIVEL local; source UTILIZADA, ficha/corpus de migracao
reutilizados, assets/Go NAO APLICAVEIS (nenhum arquivo ou protocolo alterado).

Debug/Release Build-Client.ps1 PASS, 1242 checks existentes e asserts PASS;
esses checks nao exercitam o handler nem o loader. Diff --check PASS.
Release instalado e hash conferido com artefato:
`37890AC2ACA9B6D56677D9BAF6D994C719E02F87C9610C60DF9D10F7A705682F`.
Nao CLIENT_TESTED. Proximo: retomar contratos/casos de uso da fase 2; fluxo
runtime de migracao, erro visual e reconexao permanece pendente, sem emissor
Go conhecido. Nao tratar o gate de endereco como conclusao desse lifecycle.

## Fase 2 — whisper/canal/mail 0x334

WhisperMessagePacket.h agora possui MSG_MessageWhisper, opcode e asserts do
contrato 7.48: 128 bytes, MobName em +12, String[96] em +28 e Color em +124.
Basedef reexporta o header; ReceivedPacketDispatch valida tamanho real,
Size, Type e opcode antes de TMFieldScene::OnPacketMessageWhisper. Prefixos
`-`, `--`, `=` e `!`, filtros, party fallback e mail continuam nos handlers;
nenhuma semantica foi movida para o gate.

Modo MODERNIZACAO_COMPATIVEL, procedencia 7.48 para ABI e source local para
extracao. Ficha CONTRACT ui/local-chat-message.md e gate nativo 0055890A
reutilizados; source, teste Go e builders existentes UTILIZADOS. Assets
NAO APLICAVEIS; shout `0xD1D` permanece fora. Nao foi criado opcode nem
servidor novo.

Debug/Release Build-Client.ps1 PASS, 1380 checks/asserts em cada. Fixtures
cobrem prefixos truncados, excesso, nulo, Type/opcode/Size divergentes,
entrega unica, offsets nome/texto/cor e preservacao do buffer. `go test
./internal/wire -run MessageWhisper -count=1` PASS. PROJECT_PATHS_OK e
`git diff --check` PASS. Release instalado e hash identico ao artefato:
`DDCB0B67EFF3E3E9A4F99D896227A3417CF23AE3B238C0320EC60E496A88DDAA`.
Nao CLIENT_TESTED; filtros de dois clientes, whisper, canal, mail e relogin
continuam pendentes. Proximo contrato deve ser escolhido por ficha existente;
nao repetir extracoes ja registradas neste handoff.

## Fase 2 — confirmacao de logout 0x116

CharacterLogoutConfirmPacket.h nomeia o opcode e reexporta MSG_STANDARD com
assert de 12 bytes. ReceivedPacketDispatch valida frame real/declarado e
Type/opcode antes de TMFieldScene::OnPacketCNFCharacterLogout; o handler
continua validando o ID e copiando Score/Equip para o slot antes de mudar
para SelectChar. Nenhuma regra de logout, persistencia ou relogin foi criada.

Modo PARIDADE_NATIVA/CONTRACT para o envelope, modernizacao local da fachada.
Ficha lifecycle/character-logout-selectchar-relogin.md reutilizada: Ghidra
FUN_0055890A fixa 0x116/12 e FUN_00484C8A valida o personagem. Source e
testes UTILIZADOS; servidor/Go e assets NAO APLICAVEIS neste gate. O envio
0x3AE/0x215 e a confirmacao 0x114 continuam fora deste corte.

Debug/Release Build-Client.ps1 PASS, 1401 checks/asserts em cada. Fixtures
cobrem prefixos truncados, excesso, nulo, Type/opcode/Size divergentes,
entrega unica, preservacao do ID e bytes do header. PROJECT_PATHS_OK e
`git diff --check` PASS. Release instalado e hash identico ao artefato:
`D3B40B4231443BD8D001D0294659AD7D381855233378D0731E96635F9390E4EE`.
Nao CLIENT_TESTED; menu System, espera de cinco segundos, persistencia e
relogin real continuam pendentes. Proximo corte deve seguir a ficha de
logout para o envio `0x215` ou confirmar outro contrato ja pesquisado, sem
repetir o gate `0x116`.

## Fase 2 — pedido de logout 0x215

CharacterLogoutRequestPacket.h nomeia o contrato C->S de 12 bytes e opcode
`0x215`, com static_assert; Basedef reexporta. O timer existente de cinco
segundos em TMFieldScene::TimeDelay agora usa MSG_CharacterLogout em vez de
literal, preservando ID local, Size e ordem de envio. O request nao foi
adicionado ao gate S->C, por direcao e responsabilidade distintas.

Modo PARIDADE_NATIVA/CONTRACT para wire e timer; ficha
lifecycle/character-logout-selectchar-relogin.md reutilizada: FUN_004776C3
envia 0x215/12 apos a espera. Source e testes UTILIZADOS; Go e assets
NAO APLICAVEIS ao client-only gate (builder Go ja existia em opcodes/codec).
Nenhuma persistencia ou resposta foi inventada.

Debug/Release Build-Client.ps1 PASS, 1402 checks/asserts cada. Teste puro
confirma ID, opcode e Size do request; o gate de recepcao `0x116` continua
cobrindo truncamento/divergencia. PROJECT_PATHS_OK e `git diff --check` PASS.
Release instalado/artefato conferidos; SHA-256:
`A4AAA41B38C3D7930D996B7AFA21F51278E7BC08F05ACFE57FCCE7DA1C61AD5D`.
Nao CLIENT_TESTED; timer/menu, persistencia server-side, confirmacao e
relogin real continuam pendentes. Proximo corte deve seguir a ficha de
logout para `0x114`/relogin ou outro contrato já pesquisado, sem repetir
`0x215`/`0x116`.

## Fase 2 — confirmacao de relogin 0x114

CharacterLoginConfirmContract.h nomeia `0x114` e fixa 2.104 bytes; Basedef
reexporta o contrato e agora possui static_assert no struct completo.
ReceivedPacketDispatch valida tamanho real/declarado e Type/opcode antes de
TMSelectCharScene::HandleCharacterLogin. O handler continua interpretando
PosX/PosY, MOB, Slot, ClientID, Weather, ShortSkill, Ext1/Ext2 e mudando para
Field; o gate nao interpreta nem copia payload.

Modo PARIDADE_NATIVA/CONTRACT para ABI, modernizacao local da fachada. Ficha
lifecycle/character-logout-selectchar-relogin.md reutilizada: `0x213 ->
0x114` e estado SelectChar -> Field fechados; source, serializer Go e testes
UTILIZADOS; assets NAO APLICAVEIS. Nenhuma regra de relogin foi inventada.

Debug/Release Build-Client.ps1 PASS, 1535 checks/asserts cada. Fixtures
cobrem prefixos truncados, excesso, nulo, Type/opcode/Size divergentes,
entrega unica e preservacao do frame completo; static_assert confirma 2104.
PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado/artefato
conferidos com SHA-256:
`535AEED067F4C5B68F154685A51039B645315D75B2278F886E8004FD7FC86FA3`.
Nao CLIENT_TESTED; menu, persistencia server-side e relogin real continuam
pendentes. Proximo corte deve seguir outra transicao CONTRACT ja pesquisada,
sem repetir os envelopes de logout/relogin.

## Fase 2 — probe de array de animação 0x1C1/0x2C2

ClientIntegrityArrayContract.h agora possui os opcodes, MSG_REQArray e
asserts de 24 bytes/offsets 12,16,20; Basedef reexporta. ReceivedPacketDispatch
valida o request S->C `0x1C1` antes de TMFieldScene::OnPacketREQArray; `0x2C2`
fica fora do gate de recepcao por ser resposta C->S. O handler preserva campos
recebidos, aplica clamps locais de categoria/offset, le o byte como signed
char, promove para int32 e envia a resposta existente com ID do humano.

Modo PARIDADE_NATIVA/CONTRACT para wire/handler; MODERNIZACAO_COMPATIVEL para
limite bruto numBoneBytes já existente; EXTENSAO_COORDENADA fica no manifesto
e pending server-side já documentados. Ficha transport/bone-animation-array-
probe.md reutilizada com Ghidra/gate/loader fechados; source, assets Mesh,
Go e testes UTILIZADOS. Nao houve novo asset, opcode ou regra de sessão.

Debug/Release Build-Client.ps1 PASS, 1554 checks/asserts cada. Fixtures cobrem
truncamento, excesso, nulo, Type/opcode/Size divergentes, entrega única,
offset signed, eco e resposta 0x2C2. Go:
`go test ./internal/wire -run ClientIntegrity -count=1` PASS. PROJECT_PATHS_OK
e `git diff --check` PASS. Release instalado/artefato com SHA-256:
`33D2061D9AFB54A3FD71CACF4DAE80DD037776F86748D1E17A7E0202A033EF3F`.
Nao CLIENT_TESTED; probe com assets e pending real, timeout, logout/relogin
continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT sem
repetir os gates de logout, relogin ou integridade já registrados.

## Fase 2 — compra TOTO 0x3CE

TotoPurchasePacket.h agora possui MSG_BuyToto, opcode e asserts do wire nativo:
36 bytes, TargetID +12, TargetCarryPos +14, MyCarryPos +16, Coin +20,
Gindex +24, placares +28/+32. Basedef reexporta; TMFieldScene continua
preenchendo e enviando o mesmo struct no fluxo TotoBuy. Nenhuma validação local,
fechamento de painel ou campo foi reinterpretado.

Modo PARIDADE_NATIVA/CONTRACT para wire/UI e EXTENSAO_COORDENADA para a
autoridade server-side já existente. Ficha transport/toto-buy.md reutilizada:
Ghidra FUN_004727CC confirma 36 bytes e validação textual; source, Go,
data/itemlist.csv e testes UTILIZADOS; assets visuais NAO APLICAVEIS. `Coin`
continua intenção ignorada pelo servidor.

Debug/Release Build-Client.ps1 PASS, 1555 checks/asserts cada. Fixture C++
confirma opcode, Size, signedness e offsets; `go test ./internal/game -run
'TestBuyToto' -count=1` e `go test ./internal/wire -count=1` PASS.
PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado/artefato com
SHA-256 `9BE18F2D3532E0C652698CBFB1FBE176B2DAF48645F7372E14C17959B34FA967`.
Nao CLIENT_TESTED; compra valida/invalida, tooltip e relogin continuam
pendentes. Proximo corte deve seguir outra ficha CONTRACT sem repetir os
contratos de transporte já extraídos.

## Fase 2 — compra de skill no mestre 0x277

ApplyBonusPacket.h agora possui MSG_ApplyBonus, opcode e asserts do contrato
7.48: 20 bytes, BonusType +12, Detail +14 e TargetID +16. Basedef reexporta;
os callers existentes de TMFieldScene continuam enviando o mesmo packet, sem
alterar confirmação, UI ou ordem de efeitos. O servidor segue autoridade para
classe, pré-requisitos, pontos, custo, skill e persistência.

Modo PARIDADE_NATIVA/CONTRACT para wire e envio; EXTENSAO_COORDENADA para a
validação/persistência server-side já existente. Ficha ui/skill-master-
purchase.md reutilizada: Ghidra FUN_004640E5 confirma 0x277/20 e BonusType 2
na compra; source, Go, catálogo e testes UTILIZADOS; assets NAO APLICAVEIS
neste corte. Nenhuma regra otimista foi criada.

Debug/Release Build-Client.ps1 PASS, 1556 checks/asserts cada. Fixture C++
confirma tipo, detalhe, mestre, opcode e tamanho; `go test ./internal/game
-run 'TestApplyBonusSkillMasterPacketContract|TestSkillMaster' -count=1` e
`go test ./internal/wire -count=1` PASS. PROJECT_PATHS_OK e
`git diff --check` PASS. Release instalado com SHA-256:
`C0391DA3B28F0B8C218B93248AA8F5EBEAC6A6A1D3118EC2693505B3AF6F3301`.
Nao CLIENT_TESTED; abertura da lista, sprites, confirmação, rejeições e
relogin continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT
sem repetir TOTO, ApplyBonus ou os envelopes anteriores.

## Fase 2 — atalho de poção especial 0x373

UseItemPacket.h agora possui MSG_UseItem e asserts do wire nativo: 36 bytes,
SourType +12, SourPos +16, DestType +20, DestPos +24, GridX +28, GridY +30
e ItemID +32. Basedef reexporta; UsePPotion/OnKeyPPotion continuam com a ordem
de busca 9x7, exclusao 3378, cooldown, envio e consumo otimista já fechada.

Modo PARIDADE_NATIVA/CONTRACT para wire/atalho e EXTENSAO_COORDENADA para a
reconciliação autoritativa server-side. Ficha ui/special-potion-shortcut.md
reutilizada: Ghidra FUN_0044F88F confirma 0x373/36, signedness e ordem; source,
Go, catálogo e testes UTILIZADOS; assets NAO APLICAVEIS neste corte. Nenhuma
mutação nova ou mudança no cooldown foi criada.

Debug/Release Build-Client.ps1 PASS, 1557 checks/asserts cada. Fixture C++
confirma origem, célula, item, opcode e tamanho; `go test ./internal/game
-run 'TestOnUseItem.*|TestUseItem' -count=1` e `go test ./internal/wire
-count=1` PASS. PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado
com SHA-256 `23973BDDC1417D1A172812F4F8422DEFFE70E053AF9FAA45F356E909D79931B2`.
Nao CLIENT_TESTED; atalho E, cooldown, rejeição, SendItem e relogin real
continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT sem repetir
UseItem ou contratos já extraídos.

## Fase 2 — atalho F de consumível compatível 0x373

O atalho F não introduz packet novo: reutiliza UseItemPacket.h e o mesmo
`0x373/36`. A source preserva seleção pela primeira célula 9x7 com ability 38
igual a 17 e chaves 56/57 compatíveis com Equip[12], cooldown compartilhado,
posição normalizada/fallback e envio sem remoção otimista. O servidor segue
autoridade e reconcilia pela resposta normal de item.

Modo PARIDADE_NATIVA/CONTRACT para seleção/wire; ficha
ui/equipped-item-matched-consumable-shortcut.md reutilizada: Ghidra
FUN_0044FC4B confirma F/f, controles 0x9C99, abilities, cooldown e 0x373/36.
Source, contrato UseItem, Go e testes UTILIZADOS; assets NAO APLICAVEIS.
Nenhuma regra ou packet duplicado foi criado.

Validação documental: `git diff --check` PASS. O build deste corte não foi
reexecutado, pois não houve alteração de código, ABI, projeto ou artefato;
somente catálogo e handoff foram atualizados. Estado permanece o build Release
anterior, `23973BDDC1417D1A172812F4F8422DEFFE70E053AF9FAA45F356E909D79931B2`.
Nao CLIENT_TESTED; tecla F, match, cooldown, rejeição e relogin continuam
pendentes. Proximo corte deve escolher outra ficha que exija código novo.

## Fase 2 — alternância PK 0x399

PKModePacket.h agora possui MSG_SetPKMode e asserts do wire nativo: 16 bytes,
Parm +12, domínio 0/1. Basedef reexporta e `TMFieldScene::SetPK` continua
alternando estado, protegendo controles opcionais e enviando o mesmo packet.
O servidor mantém a aplicação autoritativa em Player.PKMode e os gates de
ataque, skill e summon; nenhum combate foi alterado neste corte.

Modo PARIDADE_NATIVA/CONTRACT para wire/toggle; EXTENSAO_COORDENADA para o
gate server-side já existente. Ficha ui/pk-mode-toggle-lifecycle.md reutilizada:
Ghidra FUN_0044ECAE confirma callers, controle opcional e 0x399/16. Source,
Go e testes UTILIZADOS; assets NAO APLICAVEIS. Nenhum controle ou exceção de
guerra novo foi criado.

Debug/Release Build-Client.ps1 PASS, 1558 checks/asserts cada. Fixture C++
confirma identidade, domínio, opcode e tamanho; `go test ./internal/game
-run 'PK|SummonRequiresPKMode|Attack.*PK' -count=1` e `go test ./internal/wire
-count=1` PASS. PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado
com SHA-256 `9BEC5B083E52865F799E39AFFAE3878D7209C67238B2F6219900D40E657F2DE3`.
Nao CLIENT_TESTED; toggle, ataques PvP/PvE, summon, desconexao e relogin real
continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT com codigo
novo, sem repetir PK ou UseItem.

## Fase 2 — Gamble/Jackpot 0x2BE/0x1BF

GamblePacket.h agora possui MSG_DoJackpotBet e MSG_ResultGamble com asserts
dos contratos nativos: aposta 20 bytes (tipo +12, valor +16) e resultado 36
bytes (Result +12, StopPosition +17, Prize +28, Jackpot +32). Basedef
reexporta; SReelPanel e TMFieldScene continuam donos da animação e apenas
aplicam resultado validado. Saldo, prêmio, jackpot, pool e RNG continuam no
servidor autoritativo.

Modo PARIDADE_NATIVA/CONTRACT para wire/UI e EXTENSAO_COORDENADA para a
transação server-side. Ficha ui/gamble-jackpot.md reutilizada: Ghidra
FUN_00489882 confirma dispatch e layout; source, Go, assets e testes
UTILIZADOS. Nenhum resultado otimista ou controle novo foi criado.

Debug/Release Build-Client.ps1 PASS, 1560 checks/asserts cada. Fixture C++
confirma aposta, resultado, prêmio, jackpot, opcode e tamanhos; `go test
./internal/game -run Gamble -count=1` e `go test ./internal/wire -run Gamble
-count=1` PASS. PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado
com SHA-256 `D3087AE530631F3E09AF0CD120CE935BC74206D97FB822AB51A1085CB5343D1A`.
Nao CLIENT_TESTED; painel, rolagem, rejeição, saldo, jackpot, logout e relogin
reais continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT sem
repetir Gamble, Firework ou PK.

## Fase 2 — confirmação de morte 0x338

MobKillConfirmPacket.h agora possui MSG_CNFMobKill, opcode e asserts do wire
7.48: 24 bytes, Hold/FakeExp +12, KilledMob +16, Killer +18 e EXP uint32 +20.
Basedef reexporta; ReceivedPacketDispatch valida o envelope antes de
TMFieldScene::OnPacketCNFMobKill. A Field continua atualizando apenas a cópia
visual; dívida, EXP e persistência permanecem no servidor.

Modo PARIDADE_NATIVA/CONTRACT para wire/client e adaptação server-authoritative
para Held EXP já existente. Ficha combat/pvp-death-held-exp-lifecycle.md
reutilizada: source, Ghidra/ficha, Go e testes UTILIZADOS; assets NAO
APLICAVEIS. CP/Chaos permanece separado de Hold e não houve regra nova.

Debug/Release Build-Client.ps1 PASS, 1574 checks/asserts cada. Fixtures cobrem
truncamento, excesso, nulo, Size divergente, entrega única e offsets/unsigned
de Hold, IDs e EXP. `go test ./internal/game -run 'HeldExperience|PvP|MobKill'
-count=1` e `go test ./internal/wire -run 'MobKill|UpdateEtc' -count=1` PASS.
PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado com SHA-256
`F6B35960151FEDA38A57B3D19DB03B8B71187EC15D4A7E98D86ED052DB2CDB70`.
Nao CLIENT_TESTED; morte PvP, pagamento de Hold, limiar de HP, logout e relogin
reais continuam pendentes. Proximo corte da mesma ficha pode extrair 0x337,
sem repetir 0x338 ou contratos anteriores.

## Fase 2 — snapshot compacto UpdateEtc 0x337

UpdateEtcPacket.h agora possui MSG_UpdateEtc, opcode e asserts do wire 7.48:
36 bytes, Hold +12, EXP +16, LearnedSkill +20, Status/Mastery/Skill/Magic
WORDs +24/+26/+28/+30 e Coin +32. Basedef reexporta; ReceivedPacketDispatch
valida o frame antes de TMHuman::OnPacketUpdateEtc. Score completo permanece
exclusivo de 0x336, e CP/Chaos não foi confundido com Hold.

Modo PARIDADE_NATIVA/CONTRACT para wire/client e adaptação server-authoritative
para Held EXP já existente. Ficha combat/pvp-death-held-exp-lifecycle.md
reutilizada; source, Go e testes UTILIZADOS; assets NAO APLICAVEIS. Nenhuma
regra de progressão, skill ou gold foi criada neste corte.

Debug/Release Build-Client.ps1 PASS, 1589 checks/asserts cada. Fixtures cobrem
truncamento, excesso, nulo, Size divergente, entrega única e todos os campos
do snapshot; `go test ./internal/game -run 'HeldExperience|UpdateEtc|SkillMaster'
-count=1` e `go test ./internal/wire -run 'UpdateEtc' -count=1` PASS.
PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado com SHA-256
`117C602B57DA956C0A2B4540091D3D903C05A89F5E697C97740B0C526D4B8E29`.
Nao CLIENT_TESTED; Hold, pontos, skills, gold, limiar de HP e relogin reais
continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT sem repetir
0x337/0x338 ou os contratos anteriores.

## Fase 2 — publicação Premium Firework 0x3CA

PremiumFireworkPacket.h agora possui MSG_PremiumFirework e asserts do wire
nativo: 36 bytes, reservado +12 e bitmap +20. Basedef reexporta; o efeito,
dispatcher e container existentes não foram reescritos. Os 100 bits válidos,
LSB-first, continuam responsabilidade do handler/efeito e do servidor que já
valida/persiste o consumo.

Modo PARIDADE_NATIVA/CONTRACT para publicação/layout; EXTENSAO_COORDENADA para
consumo, persistência e observers server-side. Ficha ui/premium-firework-
display.md reutilizada: Ghidra FUN_00529B07/FUN_004D0651 confirma receptor,
bitmap e lifecycle; source, assets, Go e testes UTILIZADOS. Nenhum recurso ou
efeito novo foi inventado.

Debug/Release Build-Client.ps1 PASS, 1559 checks/asserts cada. Fixture C++
confirma ID, reservado, bitmap, opcode e tamanho; `go test ./internal/game
-run PremiumFirework -count=1` e `go test ./internal/wire -run PremiumFirework
-count=1` PASS. PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado
com SHA-256 `96C580D06FE8AA9A777A39D72608F43FF9F329CE84D41CC5B54B194BCE7C4229`.
Nao CLIENT_TESTED; uso do item, observadores, animação, expiração, logout e
relogin reais continuam pendentes. Proximo corte deve escolher outra ficha
CONTRACT sem repetir Firework, PK ou UseItem.

## Fase 2 — request Premium Firework 0x3C9

PremiumFireworkUsePacket.h agora possui MSG_UseItem2, opcode e asserts do wire:
52 bytes, SourType/SourPos +12/+16, DestType/DestPos +20/+24, GridX/GridY
+28/+30, ItemID +32 e bitmap Parm[16] +34. Basedef reexporta; UseFireWork
continua construindo o mesmo packet e consumindo a UI local na ordem existente.
A resposta 0x3CA permanece no contrato separado já registrado.

Modo PARIDADE_NATIVA/CONTRACT para wire/client e EXTENSAO_COORDENADA para
validação, persistência, cooldown e observers server-side. Ficha
ui/premium-firework-display.md reutilizada; source, assets, Go e testes
UTILIZADOS. Nenhum campo, efeito ou fallback foi inventado.

Debug/Release Build-Client.ps1 PASS, 1590 checks/asserts cada. Fixture C++
confirma slot, posição, bitmap, opcode, offsets/padding e tamanho; `go test
./internal/game -run PremiumFirework -count=1` e `go test ./internal/wire
-run PremiumFirework -count=1` PASS. PROJECT_PATHS_OK e `git diff --check`
PASS. Release instalado com SHA-256
`6AE672FF27B9331577E8D4AD0B88AF1771CE798C4BD703DCCA594F7A571877B5`.
Nao CLIENT_TESTED; desenho, consumo/rejeição, observers, expiração e relogin
continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT sem repetir
0x3C9/0x3CA ou contratos anteriores.

## Fase 2 — aviso textual MessagePanel 0x101

MessagePanelPacket.h agora concentra MSG_MessagePanel, opcode e asserts do wire
nativo: 108 bytes, texto de 96 bytes em +12. Basedef reexporta o contrato;
ReceivedPacketDispatch valida comprimento real/declarado e Type/opcode antes de
TMScene::OnPacketEvent. O parser de prefixos, o painel e a ordem de efeitos
existentes nao foram alterados.

Modo PARIDADE_NATIVA/CONTRACT para wire e MODERNIZACAO_COMPATIVEL para o gate
local. A ficha ui/message-panel-text.md registra FUN_0055890A (0x101/108),
FUN_0049889A (ID zero e texto em +12), callers, ownership e lifecycle. Source,
Go e testes foram UTILIZADOS; assets foram NAO APLICAVEIS. Os contratos
0x102/0x104 e as extensoes 0x105/0x106 permanecem separados.

Debug/Release Build-Client.ps1 PASS, 1708 checks/asserts cada. A fixture C++
cobre truncamento, excesso, nulo, Type/Size divergentes, entrega unica e
preservacao do buffer; `go test ./internal/wire -run 'MessagePanel|DaySync'
-count=1` PASS. PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado
com SHA-256 `7277CC4AD3ED65493D9375F149CDBD843C6B7A60826FA9360E79E60E2EBC4D25`.
O validador reconhece a nova ficha como CONTRACT; permanece o erro preexistente
da ficha LOCATED ui/send-item-local-update.md, fora deste corte.

Nao CLIENT_TESTED; aviso comum, prefixos, troca de cena, logout e relogin reais
continuam pendentes. Proximo corte deve seguir outra ficha CONTRACT sem repetir
0x101, 0x102/0x104 ou contratos anteriores.

## Fase 2 — intenção sobrecarregada DelayStart/SysQuit 0x3AE

DelayStartPacket.h agora concentra o ABI compartilhado de 16 bytes: opcode
0x3AE, ID em +6 e Parm int32 em +12. MSG_SysQuit nomeia Parm=0 nos caminhos de
System/fechamento; MSG_DelayStart preserva Parm=1/2 nas transições locais. Basedef reexporta
o contrato e deixa de representar esses fluxos pelo alias de PK Mode.

Modo PARIDADE_NATIVA/CONTRACT para os envios documentados nas fichas
lifecycle/character-logout-selectchar-relogin.md e application-close-global-
shutdown.md. O gate S->C é EXTENSAO_COORDENADA: FUN_0055890A não enumera 0x3AE,
enquanto o WYD-Go emite a resposta de 16 bytes consumida pela source. Timers,
IDs, Parm, PostMessage, persistência e semântica sobrecarregada foram
preservados; o servidor não transforma 0x3AE em desconexão prematura.

Debug/Release Build-Client.ps1 PASS, 1735 checks/asserts cada. A fixture C++
cobre truncamento, excesso, nulo, Type/Size divergentes, entrega única, ID,
Parm 0/1 e preservação do buffer. `go test ./internal/wire -run SysQuit
-count=1` e `go test ./internal/game -run 'SysQuit|RestartAndSysQuit' -count=1`
PASS. PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado com SHA-256
`96646D17ED2F52F7CD4D87F5412BC995253DE14AF2C386B9C24D062590FB1800`.

Nao CLIENT_TESTED; System 633..635, portal, resposta, fechamento, falha de save
e relogin reais continuam pendentes. Proximo corte deve seguir outra ficha
CONTRACT sem repetir 0x3AE ou contratos anteriores.

## Fase 2 — notificação de billing 0x194

BillingNoticePacket.h agora concentra o contrato S->C de 16 bytes: opcode
0x194, MSG_STANDARD de 12 bytes e quatro bytes de payload opaco. Basedef
reexporta; ReceivedPacketDispatch valida frame real/declarado e Type/opcode;
TMScene::OnPacketEvent rejeita tamanho divergente antes de marcar billing e
preserva a mensagem localizada 132 para o frame válido.

Modo PARIDADE_NATIVA/CONTRACT para wire e consumo local. A ficha
ui/billing-notice.md reutiliza `FUN_0055890A` (0x194/16) e `FUN_0049889A`
(estado billing, MessageBox e payload não lido). WYD-Go e assets são
NAO APLICAVEIS: não foi criado emissor ou regra de cobrança.

Debug/Release Build-Client.ps1 PASS, 1761 checks/asserts cada. Fixture C++
cobre truncamento, excesso, nulo, Type/Size divergentes, entrega única,
payload opaco e preservação do buffer. PROJECT_PATHS_OK e `git diff --check`
PASS. Release instalado com SHA-256
`D7A487FD78CB838FA1867584A2C2C67476DAA3FF5927700D3D9CE1DBA28C2841`.

Nao CLIENT_TESTED; billing válido/recusado, MessageBox, troca de cena e
relogin reais continuam pendentes. Proximo corte deve seguir outra ficha
CONTRACT sem repetir 0x194, 0x3AE ou contratos anteriores.

## Fase 2 — inclusão de membro Party 0x37D

PartyAddPacket.h agora concentra `PARTY` (28 bytes) e `MSG_AddParty` (40 bytes),
com ID em `+20`, Name em `+22` e reserved em `+38`. Basedef reexporta e
ReceivedPacketDispatch valida o frame antes de `TMFieldScene::OnPacketAddParty`.
Convite `0x37F`, remoção `0x37E` e confirmação `0x3AB` continuam fora deste
corte para não misturar seus callbacks e contratos.

Modo PARIDADE_NATIVA/CONTRACT para wire e inclusão no painel. A ficha
ui/party-panel-layout-lifecycle.md foi promovida para CONTRACT no escopo do
`0x37D`; `FUN_00492E7D -> FUN_00488879` permanece o receptor nativo e
`FUN_0055890A` fixa 40 bytes. Source, assets, Go e testes foram avaliados;
WYD-Go já era autoridade do snapshot Party.

Debug/Release Build-Client.ps1 PASS, 1811 checks/asserts cada. Fixture C++
cobre truncamento, excesso, nulo, Type/Size divergentes, entrega única, ID,
nível, nome e preservação do buffer. PROJECT_PATHS_OK e `git diff --check`
PASS. Release instalado com SHA-256
`12C509EB7A08AE357F4DD68F6CB113DFC4671FDA4EEADF19916885D1FDDB5161`.

Nao CLIENT_TESTED; convite, inclusão visual, remoção, fechamento, troca de cena
e relogin reais continuam pendentes. Proximo corte deve seguir outro contrato
Party separado ou outra ficha CONTRACT sem repetir `0x37D`.

## Fase 2 — remoção de membro Party 0x37E

PartyRemovePacket.h agora concentra o contrato C<->S de 16 bytes, com Parm
int32 em +12: zero limpa/dissolve o grupo e outro valor identifica o membro.
Basedef reexporta; o emissor e `TMFieldScene::OnPacketRemoveParty` usam o tipo
dedicado; ReceivedPacketDispatch valida frame real/declarado e Type/opcode.

Modo PARIDADE_NATIVA/CONTRACT para wire e remoção no painel. A ficha
ui/party-panel-layout-lifecycle.md reutiliza `FUN_0055890A` (0x37E/16) e
`FUN_00492E7D -> FUN_00488B52`; source, Go e testes foram UTILIZADOS. Nenhuma
regra de composição, liderança, minimapa ou lifecycle foi alterada.

Debug/Release Build-Client.ps1 PASS, 1838 checks/asserts cada. Fixture C++
cobre truncamento, excesso, nulo, Type/Size divergentes, entrega única, Parm
zero/não zero e preservação do buffer. `go test ./internal/wire -run
'PartyMemberAndRemove' -count=1` e `go test ./internal/game -run Party -count=1`
PASS. PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado com SHA-256
`E3F9E1FED09924AC008600EE1F6BB5833035D22DF25A5FA9679BBE41B7FE8DF3`.

Nao CLIENT_TESTED; saída, expulsão, dissolução, liderança, minimapa, troca de
cena e relogin reais continuam pendentes. Proximo corte Party pode extrair
`0x37F` ou `0x3AB`, sem repetir `0x37D/0x37E`.

## Fase 2 — convite Party 0x37F

PartyRequestPacket.h agora concentra o contrato C<->S de 44 bytes, reutiliza
PARTY em +12 e fixa TargetID int32 em +40. Basedef reexporta; o emissor e
`TMFieldScene::OnPacketREQParty` preservam o mesmo struct; ReceivedPacketDispatch
valida frame real/declarado e Type/opcode antes do callback.

Modo PARIDADE_NATIVA/CONTRACT para wire e convite. A ficha
ui/party-panel-layout-lifecycle.md reutiliza `FUN_0055890A` (0x37F/44) e
`FUN_00492E7D -> FUN_004883BF`; source, Go e testes foram UTILIZADOS. Nenhuma
regra de alcance, convite, AutoParty, UI ou autoridade foi movida.

Debug/Release Build-Client.ps1 PASS, 1892 checks/asserts cada. Fixture C++
cobre truncamento, excesso, nulo, Type/Size divergentes, entrega única, líder,
HP, nome, destino e preservação do buffer. `go test ./internal/wire -run Party
-count=1` e `go test ./internal/game -run Party -count=1` PASS.
PROJECT_PATHS_OK e `git diff --check` PASS. Release instalado com SHA-256
`65CF0A3A2958238963F5FA0750CA75098BC016AA8E07D6289FE16BC92FABDD37`.

Nao CLIENT_TESTED; convite válido/recusado, alcance, aceite, troca de cena e
relogin reais continuam pendentes. Proximo corte Party pode extrair `0x3AB`,
sem repetir `0x37D/0x37E/0x37F`.

## Fase 2 — aceite Party 0x3AB

PartyAcceptPacket.h agora concentra a intenção C->S de 32 bytes, com LeaderID
short em +12, LeaderName[16] em +14 e WORD reservado em +30. Basedef reexporta
o tipo; os dois emissores de TMFieldScene preservam o mesmo fluxo e não foi
adicionada uma rota ao ReceivedPacketDispatch, pois não há evidência S->C.

Modo PARIDADE_NATIVA/CONTRACT para wire e aceite. A ficha
ui/party-panel-layout-lifecycle.md reutiliza `FUN_004662C5` como construtor e
emissor nativo; source, binário/Ghidra 7.48, WYD-Go e testes foram UTILIZADOS.
Assets não participam do frame e W2PP, Secrets e Micronics permaneceram
excluídos. O servidor continua revalidando líder, nome, convite e espaço.

Debug/Release Build-Client.ps1 PASS, 1895 checks/asserts cada. A fixture C++
cobre opcode, tamanho, LeaderID, LeaderName, terminador e WORD reservado sem
criar um receptor. `go test ./internal/wire -run Party -count=1` e `go test
./internal/game -run Party -count=1` PASS. PROJECT_XML_AND_PATHS_OK e
`git diff --check` PASS. Release instalado com SHA-256
`567F766C6A06E7A64C7AAB226079AEAAD1ABB98068697B0A32889995F8D92166`.
O validador da pesquisa aceitou esta ficha e manteve somente o erro preexistente
de `send-item-local-update.md`, fora deste corte.

Nao CLIENT_TESTED; aceite por click e automático, rejeição, convite expirado,
troca de cena e relogin reais continuam pendentes. Proximo corte deve seguir
outra ficha CONTRACT sem repetir `0x37D/0x37E/0x37F/0x3AB`.

## Fase 2 — roundtrip Motion/Emote 0x36A

A nova ficha transport/motion-emote-roundtrip.md fecha o contrato nativo C<->S
de 20 bytes. O export Ghidra read-only confirma `FUN_00455950` e
`FUN_004625DA` construindo o frame, `FUN_0055890A` exigindo 20 bytes e
`FUN_0052EAA9 -> FUN_005296E8` consumindo Motion short em +12, Parm short em
+14 e Direction float em +16.

MotionPacket.h agora possui o contrato e Basedef o reexporta. O gate recebido
valida frame real/declarado e Type/opcode antes de TMHuman; o literal do
dispatcher foi substituído pelo símbolo sem mudar o callback. A fixture C++
cobre truncamento, excesso, nulo, Type/Size divergentes, entrega única, campos e
preservação do buffer.

O WYD-Go deixou de descartar o emote válido: aceita somente Parm=0 e motions
produzidas pelo input 7.48 (`13`, `15..25`, `27`), ignora ID/Direction do client
e reconstrói o retorno com ID autoritativo e Direction zero para dono e
observers visíveis. Motion=100, motion 14 e Parm 1..3 permanecem exclusivos de
efeitos server-side. Testes cobrem fan-out, isolamento por visão e rejeições.

Modo PARIDADE_NATIVA/CONTRACT para wire e roundtrip, com revalidação
server-authoritative. `go test -count=1 ./...` PASS.
Debug/Release Build-Client.ps1 PASS, 1925 checks/asserts cada.
PROJECT_XML_AND_PATHS_OK, hash candidato/Release e `git diff --check` PASS.
Release instalado com SHA-256
`DB0BEE35327ED0E6DEBD987BB3F515554D4EF5E7FFF01D212593E5AA1D68DB3E`.

Nao CLIENT_TESTED; dez teclas, sentar/levantar, observer real, troca de cena e
relogin continuam pendentes. Proximo corte deve partir de outra ficha CONTRACT
ou abrir pesquisa focada; não repetir `0x36A`.

## Fase 2 — recuperação de entidade ausente 0x369

A ficha transport/missing-entity-request.md e o export Ghidra
`request-mob-by-id-flow.tsv` fecham `FUN_00482903` (Action) e
`FUN_00489A3E` (Attack): ambos consultam o ObjectManager, zeram um frame e
enviam `0x369/16`, com MobID WORD em +12 e reservado zero em +14. O gate nativo
`FUN_0055890A` confirma o tamanho. O export read-only usou o hash 7.48 esperado,
slots/callers e `instruction_search` sem SCRIPT ERROR.

MissingEntityRequestPacket.h agora é o dono único do contrato; Basedef reexporta
e os dois emissores usam o tipo dedicado. O dispatcher recebido não foi alterado
porque `0x369` é somente C->S. O servidor já revalidava tamanho, próprio ID,
visibilidade, alcance e instância; nenhum bypass foi criado.

`go test -count=1 ./...` PASS. Debug/Release Build-Client.ps1 PASS, 1927
checks/asserts cada. PROJECT_XML_AND_PATHS_OK, `git diff --check` e hash
candidato/Release PASS. Release instalado com SHA-256
`E6772EA2C4E2A5449AB0B8E9CC46735D12903B66D7CE2680A3F98C88876DCC7`.

Nao CLIENT_TESTED; perda real de Action/Attack, rematerialização de mob/player,
troca de cena e relogin continuam pendentes. Próximo corte deve escolher outra
ficha CONTRACT sem repetir `0x369`.

## Fase 2 — pedido de renascimento/recall 0x289

A ficha transport/restart-recall-request.md fecha o frame C->S de 12 bytes. O
export Ghidra `restart-recall-flow.tsv` confirma `FUN_00476006` e
`FUN_004776C3` enviando `MSG_STANDARD` zerado com `Type=0x289`, enquanto
`FUN_0055890A` exige tamanho 12. O hash 7.48 esperado e as relações de função
foram conferidos em modo read-only.

RestartRecallPacket.h agora concentra `MSG_Recall_Opcode` e a asserção do
header; Basedef reexporta e o literal restante do caminho de morte usa o símbolo.
Timers, flags, lifecycle e autoridade de renascimento não foram alterados. Não
foi adicionado dispatcher S->C.

`go test -count=1 ./...` PASS. Debug/Release Build-Client.ps1 PASS, 1928
checks/asserts cada. XML/caminhos, `git diff --check` e hash candidato/Release
PASS. Release instalado com SHA-256
`6EEB158C59829A8D65E41F8CE6962B62CBAB084B2AF56B9BAB32B1079D15F35E`.

Nao CLIENT_TESTED; morte, cooldown, recall, troca de cena e relogin reais
continuam pendentes. Proximo corte deve escolher outra ficha CONTRACT sem
repetir `0x289`.

## Fase 2 — keepalive periódico 0x3A0

A ficha transport/keepalive-ping.md e o export Ghidra
`keepalive-ping-flow.tsv` fecham os dois emissores nativos: `FUN_0047E9B6`
(Field, ID do personagem, intervalo de 250 s) e `FUN_004A2AEF`
(SelectChar, ID zero, intervalo de 300 s). Ambos zeram `MSG_STANDARD` de 12
bytes, gravam `Type=0x3A0` e enviam; o gate `FUN_0055890A` confirma o tamanho.

KeepalivePingPacket.h agora concentra o opcode e a asserção do header; Basedef
reexporta. Os timers, IDs, transporte e ausência de resposta foram preservados.
Não houve entrada no dispatcher recebido.

`go test -count=1 ./...` PASS. Debug/Release Build-Client.ps1 PASS, 1930
checks/asserts cada. XML/caminhos, `git diff --check` e hash candidato/Release
PASS. Release instalado com SHA-256
`F80936677E75630FE31EF47D799DC3672ACFF1C8F9531C24864B8934A9B67DBA`.

Nao CLIENT_TESTED; permanência real nas cenas, timeout, disconnect e relogin
continuam pendentes. Próximo corte deve escolher outra ficha CONTRACT sem repetir
`0x3A0`.


## Fase 2 - pedido de troca de cidade 0x291

`ChangeCityPacket.h` agora concentra `MSG_ChangeCity_Opcode` e o frame C->S de
16 bytes, com `Village` int32 em `+12`. `Basedef.h` reexporta o tipo; os dois
emissores de `TMFieldScene` preservam a condicao nativa `Village < 4`, o envio,
a atualizacao de `HomeTownX/Y` e o marcador de vila.

O export Ghidra focado `change-city-request-flow.tsv` (SHA-256
`6C48347D35B3F33AF678BD9525090BF99029525FA8FFADB68EA52F4A740D5B18`) confirma
`FUN_004776C3`, os writes de `0x291` em `0x00478B2B` e `0x00478F7A`, os envios
`FUN_0055F2DD` em `0x00478B8A`/`0x00478FC7`, o caller do tick e o gate
`FUN_0055890A` com tamanho `0x10`. O WYD-Go ja exigia 16 bytes e mantem o
servidor autoritativo; nenhum dispatcher S->C foi adicionado.

Fixture C++ cobre opcode, ID, tamanho, offset e bytes zerados. `go test
-count=1 ./...`, XML/caminhos e `git diff --check` passaram. O validador aceitou
a ficha `CONTRACT` e manteve somente o erro preexistente de
`send-item-local-update.md`, fora deste corte. Debug/Release via
`Build-Client.ps1 -Rebuild` passaram com 1932 checks/asserts; Debug instalou
`A079B795B122FF113C94E7974A0F202E4123FAACB6CBBB5767A856898DE33117` e Release
instalou `1D8C6077CA7B97D568FC998D8AF1F099EBA503C6695AC5F2AA4349060DA4FDA1`.
O fluxo real de entrada/troca de cidade, troca de cena e relogin permanece sem
`CLIENT_TESTED`.

## Fase 2 — pedido de portal 0x290

`ReqTeleportPacket.h` agora concentra `MSG_ReqTeleport_Opcode` e o frame C->S
de 16 bytes, com DWORD reservado zero em `+12`. `Basedef.h` reexporta o tipo;
`TMFieldScene::OnMsgBoxEvent` preserva o `case 16`, a condição de atributo
`0x10`, o aviso de erro, o fechamento da caixa e o envio único.

O export Ghidra focado `req-teleport-flow.tsv` confirma `FUN_004640E5`, o
branch de confirmação, `FUN_00433A60`, o write de `0x290`, reservado zero,
`FUN_0055F2DD` e o gate `FUN_0055890A` com tamanho `0x10`. O WYD-Go já exige o
mesmo frame e mantém rota, preço, gold e persistência autoritativos.

Fixture C++ cobre opcode, ID, tamanho, offset e reservado zero.

`go test -count=1 ./...`, XML/caminhos e `git diff --check` passaram. O
validador aceitou a ficha `CONTRACT` e manteve somente o erro preexistente de
`send-item-local-update.md`. Debug/Release via `Build-Client.ps1 -Rebuild`
passaram com 1934 checks/asserts; Debug instalou
`B05A69DFE6D87964BFC8DD50B9EBE0879E23ED9339EE6939E8E879852C654415` e Release
instalou `20E080DC44D40F5620B9A649D77976F29A85BD3357F7A9B459DC4FFBE33192BD`.
Confirmação real do portal, recusa e relogin continuam sem `CLIENT_TESTED`.

## Fase 2 — interação com NPC 0x28B

`UseNPCPacket.h` agora concentra `MSG_UseNPC_Opcode` e o frame C->S de 20
bytes, com `TargetID` em `+12` e `ClickOk` em `+16`. `Basedef.h` reexporta o
tipo; os seis emissores de `TMFieldScene` preservam os valores nativos `0` e
`1` para clique e confirmação.

O export Ghidra focado `use-npc-flow.tsv` confirma `FUN_004640E5`, os grupos de
eventos, os writes de `0x28B`, os campos, `FUN_0055F2DD` e o gate
`FUN_0055890A` com tamanho `0x14`. O WYD-Go já exige 20 bytes e revalida NPC,
alcance, visibilidade e allowlist.

Fixture C++ cobre opcode, tamanho, offsets e `ClickOk`.

`go test -count=1 ./...`, XML/caminhos e `git diff --check` passaram. O
validador aceitou a ficha `CONTRACT` e manteve somente o erro preexistente de
`send-item-local-update.md`. Debug/Release via `Build-Client.ps1 -Rebuild`
passaram com 1936 checks/asserts; Debug instalou
`A42850D9DEE871AAF6BED6268D7026EF58815DCE6EFACAED8BE734C36CB529DC` e Release
instalou `C7406A06EAA5C8CCA2FEC7BC8726DC646E11512957DEE4F431D56F4F2BE01FFD`.
Clique, confirmacao, recusa, troca de cena e relogin continuam sem
`CLIENT_TESTED`.


## Fase 2 — remoção de membro da guild 0x28C

`GuildDeprivatePacket.h` agora concentra `MSG_GuildDeprivate_Opcode` e o frame
C->S de 16 bytes, com `TargetID` em `+12`. `Basedef.h` reexporta o tipo; o
caso `816` de `TMFieldScene::OnMsgBoxEvent` preserva o alvo, a caixa e o envio,
substituindo somente o literal decimal `652` e a fachada `MSG_STANDARDPARM`.

O export Ghidra focado `guild-deprivate-flow.tsv` confirma `FUN_004640E5`, o
evento `0x330`, os writes de `0x28C`, o alvo, `FUN_0055F2DD` e o gate
`FUN_0055890A` com tamanho `0x10`. O WYD-Go já exige 16 bytes e mantém cargo,
alvo e persistência autoritativos.

Fixture C++ cobre opcode, tamanho, offset, alvo e bytes reservados.

`go test -count=1 ./...`, XML/caminhos e `git diff --check` passaram. O
validador aceitou a ficha `CONTRACT` e manteve somente o erro preexistente de
`send-item-local-update.md`. Debug/Release via `Build-Client.ps1 -Rebuild`
passaram com 1938 checks/asserts; Debug instalou
`1B0E1110AFFFE07170A9D5F5F10694700240C9981F18388F19723EF782E6CC4` e Release
instalou `AAF0E76E08254C980856BBFDE84EDFEAF6B43FBAA365EB4E065856B403E14CE2`.
Expuls?o real, rejei??o, troca de cena e relogin continuam sem `CLIENT_TESTED`.


## Fase 2 ? confirma??o de disputa de zona 0x28F

`ChallengeConfirmPacket.h` agora concentra `MSG_ChallengeConfirm_Opcode` e o
frame C->S de 20 bytes, com `Parm1` em `+12` e `Parm2` em `+16`. `Basedef.h`
reexporta o tipo; o caso `60` de `TMFieldScene::OnMsgBoxEvent` preserva
`Parm1=m_dwTID` e `Parm2=0`, substituindo somente o literal decimal `655` e a
fachada `MSG_STANDARDPARM2`.

O export Ghidra focado `challenge-confirm-flow.tsv` confirma o estado nativo
de confirma??o em `FUN_004662C5`, o buffer de 20 bytes, `Type=0x28F`, os dois
par?metros, `FUN_0055F2DD` e o gate `FUN_0055890A` com tamanho `0x14`. O
`0x28E` continua separado e pendente: sua emiss?o nativa completa ainda n?o
foi localizada, portanto n?o foi alterada.

Fixture C++ cobre opcode, tamanho, offsets e par?metros. `go test -count=1 ./...`,
XML/caminhos e `git diff --check` passaram. O validador aceitou a ficha
`CONTRACT` e manteve somente o erro preexistente de
`send-item-local-update.md`. Debug/Release via `Build-Client.ps1 -Rebuild`
passaram com 1940 checks/asserts; Debug instalou
`9261463481EDCBF2946E93440723D2837DF17B362933638D74C5EA1161C95746` e Release
instalou `0BC80774E9D9887796153B6A0383BC5966A59401253261FE254EFBAD9A2F2102`.
Confirma??o real e relogin continuam sem `CLIENT_TESTED`.


## Fase 2 — relações entre guilds 0xE0E/0xE12

`GuildRelationPacket.h` concentra os opcodes de guerra e aliança e o payload
C->S compartilhado de 20 bytes: `GuildID` `uint32` em `+12` e
`TargetGuildID` em `+16`. Os casos `817/862` de
`TMFieldScene::OnMsgBoxEvent` deixaram de usar `MSG_STANDARDPARM2` e os
literais `3598/3602`; os valores, callbacks e fechamento do painel foram
preservados.

Antes da edição, `git log --all`, o catálogo e as fichas confirmaram que esse
par ainda não possuía corte próprio. O Ghidra 12.1.3 read-only confirmou em
`FUN_004640E5` os eventos `0x331/0x35E`, os dois buffers zerados, campos e
calls de 20 bytes para `FUN_0055F2DD`; `FUN_0055890A` também confirma o gate
`0xE0E/0x14`. O servidor já revalida liderança, guild e alvo e mantém rollback
de persistência, sem mudança nesta unidade.

`go test -count=1 ./...`, validador de pesquisa (`CONTRACT=29`, `TRACED=18`,
`LOCATED=6`, `UNMAPPED=2`), XML/caminhos e `git diff --check` passaram.
Debug/Release via `Build-Client.ps1 -Rebuild` passaram com 1942 checks/asserts;
o Release foi instalado em `client748/project.exe` com SHA-256
`891463810FDDF12E959A6DE0C64DE0A751CF4E6D0F6FBE9EB6DAD1C2C05FDDC7`.
Guerra, aliança, rejeição e relogin reais continuam sem `CLIENT_TESTED`.

## Correção funcional — limites da consulta de ocupação (2026-09-06)

Source atual confirmou que `CanItAdd` ainda somava dimensões/coordenadas signed
e aceitava índices negativos antes de ler `m_pbFilled`. `GridInsertion.h`
agora contém a consulta pura `grid_occupancy::CanPlace`: valida retângulo e
produto da grade antes do acesso, mantendo exatamente a regra `ocupado == 1`.
`CanAddItemInEmpty` e `AddItemInEmpty` validam dimensões antes de subtrair nos
limites dos loops. Inserções diretas e sua política de sobreposição/recorte não
foram alteradas; não foi inserido `CanItAdd` dentro de `AddItem`.

Modo: `MODERNIZACAO_COMPATIVEL`, endurecimento local de entradas inválidas.
Fontes UTILIZADAS: source SGrid/constructor (buffer columns*rows), testes C++,
descompilação estudada `0040e604_FUN_0040e604.c` e
`0040e6aa_FUN_0040e6aa.c` do corpus Ghidra 20260821; identidade nativa
reutilizada da ficha `send-item-local-update.md`. A consulta nativa também
bloqueia somente células iguais a 1; não alegamos que suas entradas inválidas
eram seguras. Assets, wire e servidor NÃO APLICÁVEIS ao delta: inalterados.
TMProject posterior/guias NÃO APLICÁVEIS como prova nativa; W2PP, Secrets e
Micronics excluídos. Nenhuma promoção de maturidade da ficha SendItem.

Testes enumeram retângulos de uma grade 9x7 com ocupação em início/meio/fim,
comparando com uma enumeração independente; incluem INT_MIN/INT_MAX, dimensões
zero/negativas, buffer nulo, produto inválido e ausência de mutação.
`go test -count=1 ./...` passou na retomada. Build Release e gate C++ passaram
com 11762 checks, inclusive após a guarda complementar de AddItemInEmpty.
`git diff --check` passou. O build oficial instalou e conferiu
`client748/project.exe`, SHA-256
`BAFEB5162EF1C1B0DAAA4443159F898CFD3C7BC51B17F312193ADB9D85701BA6`.
Estado: `IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED TESTED`, não
`CLIENT-TESTED`. Dois warnings C4018 preexistentes em SGrid permanecem.

Lacunas confirmadas: inserções diretas AddItem/AddSkillItem/SetItem ainda exigem
revisão própria para coordenadas negativas/overflow; teste no cliente real
permanece pendente. `internal/game/teleports.go:onGuildChallenge` ainda é stub
de disputa de cidades (0x28E/0x28F), logo o objetivo global não está concluído.

## Correção funcional — escrita e retirada nas grades (2026-09-06)

`AddItem`, `AddSkillItem` e `SetItem` agora usam `FillClipped` antes de
modificar binding/lista/contador. Origem negativa/fora da grade, dimensões não
positivas, buffer nulo e overflow são rejeitados sem transferência de ownership.
O footprint pode ultrapassar o receptáculo: recorte direito/inferior e
sobreposição continuam permitidos, preservando equipamento e grids de mix.
As duas retiradas (`PickupItem` por coordenada e `PickupAtItem`) usam o mesmo
recorte para limpar ocupação; a remoção da lista e a devolução do item continuam
mesmo se sua geometria estiver inconsistente. Loops são limitados pela grade,
não por uma dimensão potencialmente enorme do item.

Modo `MODERNIZACAO_COMPATIVEL`, implementação local. Fontes UTILIZADAS:
source atual SGrid, GridInsertion e testes; descompilação Ghidra estudada de
`FUN_0040E6AA`/`FUN_0040E817` (append e recorte) e `FUN_0040E974` (busca),
com identidade histórica reutilizada do lote anterior. Não há claim novo de
paridade de entradas inválidas. Assets, wire e WYD-Go NÃO APLICÁVEIS ao delta,
sem mudança; TMProject posterior/guias NÃO APLICÁVEIS como prova nativa;
W2PP/Secrets/Micronics excluídos. A ficha SendItem permanece LOCATED.

Build oficial Release passou, instalou e conferiu `client748/project.exe`:
`27B25D80BCC96FACD81D0B68D70BDC56AFE43FB34656CEB5FCE1DD8FADF24C69`.
Gate C++: 24253 checks PASS, incluindo escrita/retirada com sentinelas em
6237 retângulos, overflow, footprint enorme, rejeição sem mutação/ownership.
Não foi repetida a suíte Go: nenhum consumidor Go mudou desde seu PASS no
lote anterior. Estado `IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED TESTED`;
warnings legados permanecem e não houve teste visual/in-game.

Revisão dos callers em `TradeItem`: oferta de trade e composição Item4 agora
liberam o visual rejeitado e retornam antes de alterar Item/CarryPos/cor. Falha
de alocação da composição também libera o payload. O destructor de
SGridControlItem é dono do payload; não há delete duplicado. Build e checks
acima incluem essas guardas; sua execução real ainda não foi testada.

Próximo passo: revisar callers restantes que criam visuais e ignoram rejeição
de AddItem/SetItem, começando por quickslots/skill belt em SGrid::SellItem;
testar arrastar/equipar/retirar e oferta de trade no candidato real.
Não repetir consulta/escrita pura já coberta. Disputa de cidades e demais
fluxos in-game continuam pendentes; o objetivo global permanece ativo.

## Correção funcional — drop na segunda página de skills (2026-09-06)

Ao revisar SellItem/SellItem2, foi confirmado que ambos testavam o ID moderno
65645 para selecionar +10. A árvore ativa vincula a segunda barra ao ID 586,
portanto um drop nela escrevia nos slots 0..9. Os dois consumidores agora
comparam a grade com `m_pGridSkillBelt3` da cena. Primeira página, wire, regras
de skill e servidor permanecem inalterados; sem nova tabela de IDs.

Ficha `ui/skill-belt-page-selection.md` TRACED, limitada à seleção da página;
export Ghidra read-only `skill-belt-page-flow.tsv`, identidade histórica
confirmada e execução sem SCRIPT ERROR. O branch `FUN_00416196` testa 0x24A
e soma 10; seu caller 004209FC e consumidor 004470B9 foram inspecionados.
Modo MODERNIZACAO_COMPATIVEL de binding local, sem claim de paridade completa.

Build oficial Release PASS, 24253 checks existentes e static assertions PASS;
estes checks não simulam o drag. `go test -count=1 ./internal/game
./internal/wire` PASS. Validador: CONTRACT=29, TRACED=19, LOCATED=6,
UNMAPPED=2. Instalado `client748/project.exe`, SHA-256
`2FFA834BF2941C80FA19840E397E543F9E275F74E74EFA473E25A5609E94E55A`.
Estado da correção: IMPLEMENTED / STATICALLY VERIFIED; não CLIENT-TESTED.

Próximo passo: concluir rollback/ownership dos callers de drop na barra
(SellItem/SellItem2 retiram o visual anterior antes de aceitar o substituto),
depois quickslots. Testar drag em slots 1 e 10 de ambas as páginas, troca por
Z e pelos botões 587/588, resposta do servidor e persistência após relogin.
O teste de ID 65645 restante no ramo CUBEBOX calcula um nSeg não utilizado e
não participa dessa gravação; não foi alterado por limpeza incidental.

## Correção funcional — substituição na barra de skills (2026-09-07)

Retomada conferiu HEAD 8bcc7a8f e worktree limpa: o binding da segunda pagina
ja estava corrigido; nao foi refeito. SellItem/SellItem2 ainda retiravam o
visual antes de AddItem e ignoravam rejeicao. Agora o helper local insere
primeiro e retira o primeiro visual anterior atingido somente apos sucesso,
restabelecendo a ocupacao que PickupItem limpa. O caller desanexa o cursor
antes de destruir o antigo, inclusive quando o cursor aponta para ele.
Rejeicao libera somente o novo visual/payload e retorna antes de gravar
ShortSkill/enviar packet. Slots fora de 0..9, linha diferente de zero e
cursor/payload ausentes sao rejeitados. Lista cheia rejeita conservando o
anterior; a barra normal tem dez slots e a lista tem capacidade 128.

Modo MODERNIZACAO_COMPATIVEL, implementacao local de ownership. UTILIZADAS:
source atual (AddItem/PickupItem/destructor), documentacao da ficha
skill-belt-page-selection e descompilacao estudada FUN_00416196 do corpus
20260821: retirada +0xA4, construcao, insercao +0x8C, detach, delete, envio.
SHA-256 nativo reconferido: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593.
Nenhum claim de rollback nativo novo. Assets, guia KR e servidor NAO
APLICAVEIS ao delta de ownership; wire, mecanicas e IDs inalterados.
Sources 7.54/W2PP/Secrets/Micronics nao consultadas.

Build oficial Release PASS; 24253 checks existentes/static assertions PASS,
sem alegar que simulam este drop. Candidato instalado e conferido:
1FD6A310D13DB90DEC0C9B5AC1B7FD6732B33F3A76EE6AB80D88647AC7C3321F.
Estado IMPLEMENTED / STATICALLY VERIFIED; nao CLIENT-TESTED. Warnings C4018
preexistentes permanecem. Nenhum consumidor Go mudou neste lote.

Proximo passo: testar substituicao/slot vazio/mesmo slot/rejeicao em ambas as
paginas e relogin no candidato; seguir para os callers de quickslot em
SellItem, que ainda apagam o anterior e ignoram AddItem. Nao refazer as
correcoes de pagina nem de limites da grade. Objetivo global permanece ativo.
