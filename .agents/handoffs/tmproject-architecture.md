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
