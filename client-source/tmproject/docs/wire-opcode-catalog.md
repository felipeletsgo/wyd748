# Catálogo inicial de opcodes

Fontes: `internal/core/Basedef.h` e headers proprietarios de `internal/wire/`,
reexportados pela fachada. Este catalogo e inventario, nao autorizacao para
alterar protocolo.

## Famílias identificadas

| Família | Exemplos |
|---|---|
| bootstrap/login | `MSG_CNFAccountLogin_Opcode`, `MSG_CNFCharacterLogin` |
| movimento | `MSG_Action_Opcode`, `MSG_Action_Stop_Opcode`, `MSG_Motion_Opcode`, `MSG_AirMove_Start_Opcode` |
| inventário | `MSG_DeleteItem_Opcode`, `MSG_SplitItem_Opcode`, `MSG_UseItem_Opcode`, `MSG_UpdateEquip` |
| trade/banco | `MSG_Trade_Opcode`, `MSG_CloseTrade_Opcode`, `MSG_Withdraw_Opcode`, `MSG_Deposit_Opcode` |
| combinação | `MSG_CombineItem_Opcode`, `MSG_CombineItemAylin_Opcode`, `MSG_CombineItemAgatha_Opcode`, `MSG_CombineItemTiny_Opcode` |
| quest/missão | `MSG_Quest_Opcode`, `MSG_Mission` |
| eventos | `MSG_Ping_Opcode`, `MSG_DelayStart_Opcode`, `MSG_SysQuit_Opcode` |

## Dispatch atual

Entrada de rede: `platform/windows/CPSock.cpp`. O pacote é enquadrado como
`MSG_STANDARD` e encaminhado ao `ObjectManager`/cena. O próximo passo da Fase
2 e ampliar a validacao e traducao sem duplicar os structs.

Primeiro contrato de recepcao isolado: `MSG_ReqTransper_Opcode` (`0xFAA`),
52 bytes, em `CharacterTransferPacket.h`. `ReceivedPacketDispatch.h` valida
o comprimento real/declarado e o opcode antes do percurso no ObjectManager.
Os outros opcodes conservam o fallback anterior. A ficha de transferencia
registra separadamente os claims nativos e o endurecimento local.

Contratos adicionais na mesma entrada: `0x182` (SendItem, 24 bytes),
`0x102/0x104` (mensagens opacas, 116/152 bytes) e `0x333` (chat local,
108 bytes). Chat e mensagens opacas possuem structs/asserts em headers
proprios de `internal/wire`, reexportados por Basedef. A validacao usa o
comprimento real da view, Size e Type do header; os callbacks mantem o buffer
original emprestado. Demais opcodes ainda dependem de seus consumidores.

As extensoes coordenadas `0x105/0x106` tambem validam 108 bytes nesta entrada.
`IndexedMessageContract.h` possui seus opcodes e referencia o envelope de
chat, sem duplicar a struct. ID, seletor, indice e CSV continuam no parser da
cena; nao constituem paridade com mensagens nativas.

`0x52A` (migracao de servidor/canal, 80 bytes) usa ServerMigrationPacket.h.
A entrada de rede valida o frame antes de a cena copiar a imagem para replay.
O replay local continua direto no handler, com o estado e a janela existentes;
esse gate nao valida o conteudo textual do ticket ou o indice de servidor.

O handler de reconexao valida separadamente o prefixo numerico do TID com
ParseMigrationServer (leitura limitada a 52 bytes, overflow e capacidade) e
o grupo local antes de indexar g_pServerList. Sufixo do ticket permanece opaco.

`0x334` (whisper/canal/mail, 128 bytes) usa WhisperMessagePacket.h, com
nome em offset 12, texto em 28 e cor em 124. ReceivedPacketDispatch valida o
envelope antes dos handlers existentes; prefixos e filtros continuam nesses
handlers. O opcode de shout `0xD1D` nao recebe novo gate por essa extracao.

`0x116` (confirmacao de logout de personagem, 12 bytes) usa
CharacterLogoutConfirmPacket.h. O contrato e somente o header; a cena valida
o ID do personagem e entao copia Score/Equip antes da transicao para
SelectChar. O gate de recepcao nao altera essa autoridade nem a transicao.

O pedido C->S `0x215` usa CharacterLogoutRequestPacket.h e o mesmo envelope
de 12 bytes, mas fica fora de ReceivedPacketDispatch por ser direcao de
saida. O timer de cinco segundos da FieldScene preenche o ID local e envia o
request; o servidor decide persistencia e somente entao emite `0x116`.

`0x114` (confirmacao de CharacterLogin, 2.104 bytes) usa
CharacterLoginConfirmContract.h. O gate valida somente o envelope; a selecao
continua dona da interpretacao de Pos/MOB/Slot/ClientID/Weather/ShortSkill e
sidecars antes de reconstruir a FieldScene.

O probe de integridade `0x1C1`/`0x2C2` usa ClientIntegrityArrayContract.h:
24 bytes, Category em +12, ByteOffset em +16 e Value em +20. A recepcao
valida apenas `0x1C1`; o handler cria a resposta `0x2C2` preservando categoria
e offset recebidos e promovendo o byte assinado para int32.

`0x3CE` (compra TOTO, 36 bytes) usa TotoPurchasePacket.h. O client valida
campos locais e fecha o painel após o envio; preço, loja, item 4147, gold,
UID, efeitos e rollback continuam autoridade exclusiva do WYD-Go.

`0x277` (ApplyBonus, 20 bytes) usa ApplyBonusPacket.h. `BonusType=2`,
`Detail` e `TargetID` são intenção de compra; skill, pontos, pré-requisitos,
custo e persistência são decididos pelo servidor.

`0x373` (UseItem, 36 bytes) usa UseItemPacket.h, com origem em +12/+16,
destino em +20/+24, célula em +28/+30 e ItemID em +32. O client pode consumir
visualmente antes da resposta; a reconciliação autoritativa ocorre por
SendItem no servidor.

## Regras de migração

- reexportar o opcode pela fachada enquanto houver consumidores legados;
- validar tamanho antes de reinterpretar o buffer;
- não inferir semântica apenas pelo nome do opcode;
- registrar caller, consumidor e fallback para cada entrada migrada;
- adicionar teste de pacote truncado antes de mover o handler.
