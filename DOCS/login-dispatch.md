# Dispatch de login e seleção de personagem

## Mapa atual

- `CPSock` valida o enquadramento e entrega `MSG_STANDARD`.
- `TMScene::OnPacketEvent` executa o comportamento comum e repassa à cena.
- `TMFieldScene::OnPacketEvent` trata `0x10A` (`MSG_CNFAccountLogin`) e
  `0x114` (`MSG_CNFCharacterLogin`) por meio de métodos próprios.
- `TMSelectServerScene` também trata `MSG_CNFAccountLogin_Opcode` durante a
  seleção de servidor.
- `TMSelectCharScene` trata `MSG_CNFCharacterLogin_Opcode` durante a seleção
  de personagem.

`CPSock::ReadPacketView` já entrega o tamanho enquadrado (`MSG_STANDARD::Size`)
a `NewApp`, que ainda descarta essa informação ao chamar `ObjectManager::OnPacketEvent`
e a cadeia virtual seguinte. Esta é a fronteira correta para uma futura API
size-aware; alterar somente uma cena quebraria os callbacks sintéticos que
entregam structs locais diretamente.

## Decisão de migração

Não extrair ainda os métodos diretamente: eles alteram estado de cena,
controles e ownership de objetos. A primeira extração segura será um tradutor
sem efeitos colaterais que valide `Type` e tamanho mínimo antes do cast. A
fachada atual continuará responsável pelo lifecycle e pela troca de cena.

## Contrato extraído

`internal/wire/LoginPacketContract.h` concentra a classificação pura dos
opcodes `0x10A` e `0x114` e os tamanhos mínimos dos structs. O helper não lê
estado de cena nem altera o dispatch. Como `OnPacketEvent` ainda recebe apenas
`(dwCode, buf)`, a validação de comprimento permanece preparada, mas não foi
acoplada silenciosamente aos handlers existentes.

`internal/application/ports/PacketView.h` fornece a base não proprietária para a próxima
etapa: opcode, ponteiro e tamanho enquadrado viajam juntos, sem mudar o layout
dos pacotes nem assumir ownership. A adoção deve começar no transporte e
seguir pela cadeia inteira; um overload isolado em uma cena não é suficiente.

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
