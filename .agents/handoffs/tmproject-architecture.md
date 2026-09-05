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
