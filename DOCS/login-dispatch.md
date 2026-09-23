# Dispatch de login e seleção de personagem

## Mapa atual

- `CPSock::ReadPacketView` valida o enquadramento e preserva opcode, ponteiro e
  tamanho real em `PacketView`.
- `ObjectManager::OnPacketView` aplica `received_packet::Dispatch` antes de
  qualquer callback legado; frames conhecidos só seguem quando `Size`, `Type`
  e comprimento real coincidem com o contrato exato.
- `TMScene::OnPacketEvent` executa o comportamento comum e repassa à cena após
  esse gate.
- `TMFieldScene::OnPacketEvent` trata `0x10A` (`MSG_CNFAccountLogin`) e
  `0x114` (`MSG_CNFCharacterLogin`) por meio de métodos próprios.
- `TMSelectServerScene` também trata `MSG_CNFAccountLogin_Opcode` durante a
  seleção de servidor.
- `TMSelectCharScene` trata `MSG_CNFCharacterLogin_Opcode` durante a seleção
  de personagem.

O comprimento não é mais descartado na fronteira de `NewApp`. A API virtual
legada continua recebendo `(dwCode, buf)` para preservar os callbacks
sintéticos existentes, mas somente depois da validação size-aware global.

## Decisão de migração

Os métodos de cena não foram extraídos: eles alteram estado, controles e
ownership de objetos. A fachada atual continua responsável pelo lifecycle e
pela troca de cena. A proteção de memória foi isolada no gate global, sem
duplicar parsing nem alterar a ordem dos callbacks.

## Exclusão de personagem no RC 7.48

`SelCharScene2.bin` contém o painel modal 626 e o edit de senha 627. Após
confirmar o botão de exclusão 4615, o controle 1024 envia `0x211/44` com
slot, nome e senha; o controle 921 cancela sem envio. Esses IDs são do client
nativo 7.48, não do TMProject 7.69. O servidor continua autoritativo: valida
slot, nome e senha da conta antes de persistir e responder com `0x112/1288`.
O candidato recompilado liga esses controles e limpa o edit ao confirmar ou
cancelar. O fluxo completo ainda precisa de teste manual no client.

## Contrato extraído

`internal/wire/LoginPacketContract.h` concentra a classificação dos envelopes
de login/seleção emitidos pelo servidor:

- `0x10A` (`MSG_CNFAccountLogin`): 2360 bytes;
- `0x110` e `0x112` (`MSG_CNFNewCharacter` e
  `MSG_CNFDeleteCharacter`): 1288 bytes;
- `0x114` (`MSG_CNFCharacterLogin`): 2104 bytes;
- `0x11A` e `0x11C` (falha ao criar e conta já conectada): 12 bytes.

`received_packet::Dispatch` usa esses tamanhos como igualdade exata e rejeita
prefixos truncados, cauda excedente, buffer nulo e divergência entre opcode,
`Header.Type`, `Header.Size` e comprimento real antes dos casts das cenas.

`internal/application/ports/PacketView.h` fornece a vista não proprietária:
opcode, ponteiro e tamanho enquadrado viajam juntos sem mudar o wire nem assumir
ownership. O modo desta proteção é `MODERNIZACAO_COMPATIVEL`.

## Solicitacao de entrada extraida — 2026-09-05

`TMSelectCharScene` delega a montagem e o envio de `0x213` a
`RequestCharacterLogin(ICharacterLoginSender&, slot)`. O caso de uso valida
0..3 e chama a porta semantica uma vez, sem importar wire ou plataforma.
`CharacterLoginSender` codifica um unico `MSG_CharacterLogin` zerado usando
`ITransport`; `SocketTransport<CPSock>` empresta o
socket e encaminha uma vez a `CPSock::SendPacket`. A cena continua dona da
existencia do personagem, debounce, timestamp e desabilitacao dos botoes,
inclusive quando o envio falha. Nenhum handler de recepcao foi extraido.

Modo: `MODERNIZACAO_COMPATIVEL`; extracao local da source existente. Evidencia
reutilizada: `scene-transition.md`, secao "Evento de selecao e packet 0x213"
em `.agents/research/client748/flows/lifecycle/`: `FUN_004A32DD`, callsite
`0x004A3422`, 36 bytes zerados, opcode em +4 e slot em +12. Nao se promove a
maturidade da ficha nem se altera o contrato do servidor.

`MessageHeader.h` e `CharacterLoginPacket.h` sao os donos unicos dos dois
structs extraidos. SharedStructs/Basedef os reexportam para compatibilidade.
Asserts preservam tamanho, offsets e tipos subjacentes WORD/BYTE/DWORD; testes
comparam todos os 36 bytes antes do transporte para os quatro slots.
O header independente ainda exige o modelo de inteiros Windows (long de 32
bits); isto nao representa uma migracao de plataforma.

## Validacao manual pendente

Antes de mover qualquer handler, testar login aceito, login recusado, seleção
de personagem válida, seleção recusada e retorno à tela anterior. O build não
substitui esses testes de fluxo.
