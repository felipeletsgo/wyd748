# Catálogo de opcodes e contratos

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
`MSG_STANDARD` e encaminhado ao `ObjectManager`/cena. O gate global
`ReceivedPacketDispatch.h` valida tamanho real, tamanho declarado e opcode
dos contratos já extraídos antes dos callbacks legados; a lista atual de
opcodes cobertos está em `ExpectedSize`, não neste resumo.

Primeiro contrato de recepcao isolado: `MSG_ReqTransper_Opcode` (`0xFAA`),
52 bytes, em `CharacterTransferPacket.h`. `ReceivedPacketDispatch.h` valida
o comprimento real/declarado e o opcode antes do percurso no ObjectManager.
Os opcodes sem tamanho registrado conservam o fallback anterior. A ficha de transferencia
registra separadamente os claims nativos e o endurecimento local.
O WYD-Go aceita `0xFAA` apenas na selecao, com 52 bytes exatos, e responde
`Result=4` (erro generico do client) sem alterar conta/slot. Isto encerra a
espera do client, mas nao implementa a transferencia para o Integrated server.

Exemplos de contratos adicionais na mesma entrada: `0x182` (SendItem, 24 bytes), `0x101`
(MessagePanel, 108 bytes), `0x102/0x104` (mensagens opacas, 116/152 bytes) e
`0x333` (chat local, 108 bytes). Esses contratos possuem structs/asserts em
headers proprios de `internal/wire`, reexportados por Basedef. A validacao usa
o comprimento real da view, Size e Type do header; os callbacks mantem o
buffer original emprestado. Demais opcodes ainda dependem de seus consumidores.

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

`0x3AE` usa DelayStartPacket.h para o ABI compartilhado de 16 bytes. SysQuit
nomeia `Parm=0` nos caminhos de System/fechamento; DelayStart conserva os
valores `1/2` das transicoes de portal/teleporte e retorno. O gate nativo nao
enumera esse opcode, portanto a
validacao S->C de 16 bytes documenta a resposta coordenada do WYD-Go e preserva
o callback existente, sem ampliar o claim nativo.

`0x194` usa BillingNoticePacket.h: frame S->C de 16 bytes, com quatro bytes de
payload opaco. O client marca billing e mostra a mensagem localizada 132; não
há emissor ou regra de cobrança no WYD-Go.

`0x37D` usa PartyAddPacket.h: frame S->C de 40 bytes, `PARTY` em `+12` e
reserved final em `+38`. O gate preserva o callback existente de inclusão no
painel Party; convite, remoção e confirmação permanecem contratos separados.

`0x37E` usa PartyRemovePacket.h: frame C<->S de 16 bytes e `Parm` em `+12`.
Zero limpa/dissolve a lista; outro valor identifica o membro removido. O gate
preserva o callback existente e não altera a autoridade Party do servidor.

`0x37F` usa PartyRequestPacket.h: frame C<->S de 44 bytes, snapshot `PARTY` do
líder em `+12` e TargetID int32 em `+40`. O client e o servidor continuam
revalidando convite, destino, alcance e estado do grupo nos handlers existentes.

`0x3AB` usa PartyAcceptPacket.h: intenção C->S de 32 bytes, `LeaderID` short em
`+12`, `LeaderName[16]` em `+14` e WORD reservado em `+30`. Os dois emissores
do client preservam o fluxo existente; não há rota S->C para este opcode.

`0x36A` usa MotionPacket.h: frame C<->S de 20 bytes, `Motion` short em `+12`,
`Parm` short em `+14` e `Direction` float em `+16`. O servidor reconstrói
emotes do jogador com ID autoritativo e mantém efeitos especiais no sentido S->C.

`0x369` usa MissingEntityRequestPacket.h: intenção C->S de 16 bytes, `MobID`
short em `+12` e WORD reservado em `+14`. Action e Attack usam o mesmo tipo;
o servidor limita a resposta à entidade visível, próxima e autorizada.

`0x289` usa RestartRecallPacket.h: pedido C->S de 12 bytes formado somente por
`MSG_STANDARD`. Os caminhos de morte e recall preservam seus timers; HP,
posição, score e cooldown continuam decididos pelo servidor.

`0x3A0` usa KeepalivePingPacket.h: keepalive C->S de 12 bytes, somente
`MSG_STANDARD`. Field envia o ID local, SelectChar usa zero, e o servidor
reconhece atividade sem produzir resposta.

`0x291` usa ChangeCityPacket.h: intenção C->S de 16 bytes, com o índice de
vila em `+12`. Os dois caminhos de `TMFieldScene` preservam a condição nativa
`Village < 4`, atualizam HomeTown no mesmo fluxo e não enviam resposta; a
posição e qualquer regra de cidade continuam autoritativas no servidor.

`0x290` usa ReqTeleportPacket.h: intenção C->S de 16 bytes, com o DWORD
reservado em `+12` zerado. O `case 16` da confirmação de portal mantém a gate
de atributo `0x10`; destino, preço, gold e persistência continuam no servidor.

`0xAD9` usa AirMoveContract.h e `MSG_STANDARDPARM2`: intenção C->S de 20
bytes, `Header.ID` do personagem, rota `0..4` em `Parm1/+12` e modo de início
`1` ou fim `2` em `Parm2/+16`. Não transporta NPC ID ou destino. O servidor
valida o NPC visível e o chunk de origem, guarda o voo e publica somente o
ponto final da rota nativa após o fim correspondente. Ver
`../.agents/research/client748/flows/transport/airmove-contract.md`.

`0x28B` usa UseNPCPacket.h: intenção C->S de 20 bytes, com `TargetID` em
`+12` e `ClickOk` em `+16`. Os cliques e confirmações preservam os valores
`0/1`; o servidor continua validando o NPC e o contexto antes de abrir qualquer
feature.

`0x28C` usa GuildDeprivatePacket.h: intenção C->S de 16 bytes, com o `TargetID`
do membro em `+12`. O caso de confirmação da expulsão preserva o contrato e o
servidor continua autoritativo para cargo, alvo e persistência.

`0xE0E/0xE12` usam GuildRelationPacket.h: intenções C->S de 20 bytes, com a
guild local em `+12` e a guild alvo em `+16`. Os casos de confirmação preservam
os valores nativos; guerra, aliança, liderança e persistência continuam no
servidor.

`0xED7/0xED8` usam `ServerWarLetterContract.h`: intenções C->S de 16 bytes,
com o canal alvo inteiro em `Parm/+12`. No client nativo, os itens 4030/4031
abrem primeiro o modal nos modos 9/10; o packet só é enviado após confirmação.
O WYD-Go ainda não possui handler nem coordenação autoritativa entre instâncias,
portanto o client restaura o contrato sem alegar suporte server-side à guerra
entre canais.

`0x28F` usa ChallengeConfirmPacket.h: confirmação C->S de 20 bytes, com
`Parm1` em `+12` e `Parm2` em `+16`. O caso nativo de confirmação preserva
`Parm1=m_dwTID` e `Parm2=0`; a regra da disputa continua no servidor.

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

O atalho F também usa `UseItemPacket.h`/`0x373/36`: SourType=1, posição
normalizada, destinos zerados, GridX/GridY atuais e ItemID=0. A seleção do
consumível compatível com Equip[12] e o cooldown pertencem à Field; o wire é
compartilhado com a poção E e não ganha uma segunda struct.

`0x399` (PK Mode, 16 bytes) usa PKModePacket.h: `Parm` em +12, domínio
válido `0/1`. O servidor aplica o estado antes de PvP e confirma por
`MessagePanel` (`0x101`); o client 7.48 não despacha um retorno `0x166`.
Controle opcional ausente não altera o wire nem interrompe o toggle.

`0x378` (SetShortSkill C<->S, 32 bytes) usa
`ShortSkillSnapshotContract.h`: os vinte atalhos ocupam `[12:32]`. O client
envia o array integral; o servidor remove skills não aprendidas e devolve o
snapshot autoritativo. O gate de recepção exige tamanho real/declarado e os
dois opcodes antes do `memcpy` e do rebuild das duas páginas.

`0x366/0x367/0x368` compartilham `ActionFrameContract.h` e um envelope de 52
bytes: PosXY em +12, Speed em +16, Effect em +20, TargetXY em +24 e Route[24]
em +28. O gate de recepcao exige o frame integral antes de `TMHuman` escolher
movimento, parada ou Illusion; semantica e autoridade permanecem no WYD-Go.

`0x39D/0x39E/0x36C` usam `AttackFrameContract.h`: prefixos nativos de
48/52/96 bytes para um, dois e treze alvos e lista de dano em +44. O par ativo
também aceita as extensões coordenadas de dano amplo: `0x39D/52` físico,
`0x39D/60`, `0x39E/64|68` e `0x36C/108..156` em passo 4 com `DMGX`. O gate
exige que tamanho real, `Header.Size` e o conjunto permitido para o opcode
coincidam antes de `OnPacketAttack`; cálculo e publicação seguem autoritativos
no WYD-Go.

`0x376/20` e `0x379/24` usam `InventoryTransactionContract.h`. SwapItem
carrega origem/destino em +12..+15 e TargetID em +16; Buy carrega mercador em
+12, célula esparsa da loja em +14, Carry em +16 e Coin em +20. Além do gate
de envelope, o client exige os mesmos domínios do WYD-Go: Equip `0..15` exceto
9, Carry `0..62`, Cargo `0..119` e células de loja `0..8`, `27..35` ou
`54..62`. Sucesso de swap é concluído por `0x376`; `0x182` ressincroniza
células em rejeições. Compra bem-sucedida é confirmada pelo próprio `0x379`.

`0x387/0x388` (saque/depósito de gold do Cargo, C<->S, 16 bytes) usam
`CargoGoldTransferContract.h`: a quantidade `uint32` ocupa `+12`. O gate exige
o envelope completo antes dos casts de `OnPacketWithdraw`/`OnPacketDeposit`.
O WYD-Go valida e persiste a transferência, devolve o mesmo opcode/quantidade
e em seguida reconcilia os saldos com `0x339` e `0x337` autoritativos.

`0x39F` (PlayerChallenge C<->S, 20 bytes) usa
`PlayerChallengeContract.h`: o outro jogador ocupa `Parm1/+12` e o modo ocupa
`Parm2/+16`. O mesmo envelope carrega a intenção inicial (`0..3`), o convite
publicado ao alvo e a aceitação (`4`). O gate exige o frame integral antes de
`TMHuman::OnPacketReqRanking`; lifecycle, alcance, expiração e consumo único
continuam autoritativos no WYD-Go.

`0x397` (AutoTrade C<->S, 196 bytes) usa `AutoTradeContract.h`: descrição em
+12, doze itens em +36, posições em +132, preços em +144, taxa em +192 e alvo
em +194. O gate exige o envelope exato antes de a Field terminar a descrição,
copiar o snapshot e materializar as ofertas; validação e persistência continuam
autoritativas no WYD-Go.

`0x2CD` (consulta C->S, 16 bytes) e `0xDC3` (CapsuleInfo S->C, 52 bytes)
formam o roundtrip da Cápsula Celestial. `CapsuleInfoContract.h` fixa `CIndex`
em +12, classe/nível em +16/+18, atributos em +20..+26, duas masteries em +28,
nove skills em +32 e quest em +50. A antiga fachada herdada do 7.69 tinha
quatro masteries e 56 bytes; agora o ABI e o gate coincidem com o builder 7.48
do WYD-Go, que valida conta e ownership do selo antes da resposta.

`0x1BF` (resultado Gamble S→C, 36 bytes) e `0x2BE` (aposta C→S, 20 bytes)
usam GamblePacket.h. Resultado, prêmio e jackpot são copiados para a UI;
saldo, pool e RNG permanecem autoridade do servidor.

`0x338` (CNFMobKill S→C, 24 bytes) usa MobKillConfirmPacket.h: Hold/FakeExp
em +12, vítima +16, killer +18 e EXP uint32 em +20. O gate valida o frame
antes de a Field aplicar EXP/Hold e morte visual.

`0x337` (UpdateEtc S→C, 36 bytes) usa UpdateEtcPacket.h: Hold +12, EXP +16,
LearnedSkill +20, pontos WORD em +24..30 e gold +32. Score completo permanece
em `0x336`; CP/Chaos não ocupa este snapshot compacto.

`0x3CA` (Premium Firework S→C, 36 bytes) usa PremiumFireworkPacket.h:
reservado em +12 (8 bytes) e bitmap 10x10 LSB-first em +20 (16 bytes). O
servidor publica somente após validar/persistir o consumo; o efeito visual
continua dono da cena e do container de efeitos.

O request `0x3C9` usa PremiumFireworkUsePacket.h: 52 bytes, origem/destino
equivalentes a UseItem, ItemID em +32 e bitmap em +34. Os dois bytes finais,
bits fora da grade, item, posição e cooldown são validados pelo servidor.

## Regras de migração

- reexportar o opcode pela fachada enquanto houver consumidores legados;
- validar tamanho antes de reinterpretar o buffer;
- não inferir semântica apenas pelo nome do opcode;
- registrar caller, consumidor e fallback para cada entrada migrada;
- adicionar teste de pacote truncado antes de mover o handler.
