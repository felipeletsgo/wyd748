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

Validacao: Build-Client.ps1 Debug e Release passaram com 98 checks e asserts
estaticos em cada configuracao. Testes incluem 36 bytes dos quatro slots,
adaptador emprestado, falha sem retry e caso de uso isolado sem wire.
Debug SHA-256: 2CE0810D03C07F636CAD8BCB3223FFB67518DC88B6C8F30DA3687EEB0A0EE826.
Release instalado em `client748/project.exe`, SHA-256
`2B03DE7585E48B9096D22A73D5F2A2F27F515343934A9B9F44EDDF616DAAC440`.
Uma copia Debug intermediaria encontrou user-mapped section open; instalacoes
finais passaram sem encerrar processos. Nenhum build ficou executando.
Warnings legados C4018/C4305/C4309 permanecem. Nao CLIENT_TESTED.

## Proximo passo executavel

Preparar teste focado do enfileiramento/envio parcial em CPSock. Evidencia atual:
AddMessage (linha ~493) soma Size+nSendPosition em int e preenche WORD Size;
SendOneMessage (~620) ignora a falha de AddMessage e retorna SendMessageA;
RefreshSendBuffer (~681) copia pRecvBuffer para pSendBuffer. Assim true do
backend NAO garante o aceite local desejado pela porta. Nao corrigido neste lote.
Antes de editar, reutilizar a ficha de transporte exigida pelas skills e
testar fila cheia, limite WORD, overflow, envio parcial e consumo de chave.
AddMessage escreve Size/KeyWord/CheckSum/Tick no original; nao introduzir copia
ou alterar sequencia de chaves silenciosamente. O caso de uso/cena atual ignora
retorno como antes; testes falsos provam propagacao, nao socket real.
Evidencia reutilizada para login: scene-transition.md, evento 0x1204,
FUN_004A32DD/callsite 0x004A3422. Sem promocao de maturidade.
NewApp ainda adapta recepcao const para callbacks legados char*; nao alegar
que todos os const_cast da aplicacao foram eliminados. diff --check passou.

Fases 1–7 continuam com gates abertos, incluindo casos de uso, fragmentacao,
testes de dispatch e smoke in-game. Nao reabrir triagem global ou chats.
