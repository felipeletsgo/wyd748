package wire

// Opcodes do protocolo WYD 7.48 (campo Type@4 do header _MSG).
// C->S = client manda; S->C = server manda.
const (
	OpMessagePanel       = 0x101 // S->C aviso flutuante no topo (140B, ID=0)
	OpConnectAccount     = 0x20D // C->S login (conta@12, senha@28, cliver@40)
	OpCharList           = 0x10E // S->C lista de chars (1800B)
	OpCreateCharacter    = 0x20F // C->S criar char (slot@12, nome@16, classe@32)
	OpCNFNewCharacter    = 0x110 // S->C confirma criacao e atualiza STRUCT_SELCHAR
	OpNewCharacterFail   = 0x11A // S->C criacao recusada (MSG_STANDARD)
	OpAlreadyPlaying     = 0x11C // S->C conta ja conectada (MSG_STANDARD)
	OpCharacterLogin     = 0x213 // C->S entrar no mundo (slot selecionado)
	OpCharacterLogout    = 0x215 // C->S volta do mundo para a selecao de personagem
	OpEnterWorld         = 0x114 // S->C STRUCT_MOB completo (788B)
	OpCNFCharacterLogout = 0x116 // S->C confirma CharacterLogout (MSG_STANDARD)
	OpDeleteCharacter    = 0x211 // C->S apaga personagem (slot/nome/senha)
	OpCNFDeleteCharacter = 0x112 // S->C atualiza a selecao apos exclusao
	OpCreateMobTrade     = 0x363 // S->C cria clone de auto-loja com titulo (200B)
	OpCreateMob          = 0x364 // S->C cria mob/player (176B, Status@100)
	OpSetHpMp            = 0x181 // S->C HP/MP do mob (20B)
	OpUpdateScore        = 0x336 // S->C score/affects publicos de player ou mob (92B)
	OpUpdateAffect       = 0x3B9 // S->C 16 affects completos (140B no client 7.48)
	OpUpdateEtc          = 0x337 // S->C CP/exp/pontos/gold (48B com cauda wide)
	OpCNFMobKill         = 0x338 // S->C confirma morte e atualiza EXP do killer
	OpUpdateCarry        = 0x185 // S->C inventario interativo (528B)
	OpUpdateCargoGold    = 0x339 // S->C gold armazenado no Cargo (16B)
	OpUpdateEquip        = 0x36B // S->C visual do equip (60B: ItemEff + AnctCode)
	OpAction             = 0x366 // C<->S andar / Action (52B)
	OpActionStop         = 0x367 // C->S para a acao/movimento atual (52B)
	OpIllusion           = 0x368 // C<->S Action2 da skill Illusion (52B, Effect=6)
	OpREQMobByID         = 0x369 // C->S pede entidade ausente referenciada por um Action
	OpMotion             = 0x36A // C->S efeito/motion pontual (20B)
	OpInstanceTime       = 0x3A1 // S->C contador verde de tempo
	OpInstanceMobs       = 0x3B0 // S->C contador de monstros restantes
	OpClientUnknown2BC   = 0x2BC // C->S telemetria/estado do client 7.48 (108B)
	OpSwapItem           = 0x376 // C->S mover/equipar item (20B)
	OpWithdraw           = 0x387 // C<->S retirar gold do Cargo (16B)
	OpDeposit            = 0x388 // C<->S depositar gold no Cargo (16B)
	OpUseItem            = 0x373 // C->S usar item volatil (36B)
	OpSendItem           = 0x182 // S->C confirma UM slot (24B)
	OpWarInfo            = 0x3A8 // S->C info de guerra (24B)
	OpDropItem           = 0x272 // C->S dropar item (32B)
	OpCNFDropItem        = 0x175 // S->C confirma drop de item (28B)
	OpCreateItem         = 0x26E // S->C aparece item no chao (32B)
	OpGetItem            = 0x270 // C->S pegar item do chao (28B)
	OpCNFGetItem         = 0x171 // S->C confirma pegar item (28B)
	OpRemoveItem         = 0x16F // S->C remove item do chao (16B)
	OpUpdateItem         = 0x374 // C<->S solicita/confirma estado de objeto do chao (20B)
	OpDeleteItem         = 0x2E4 // C->S apaga item do Carry (Slot@12, Index@16)
	OpSplitItem          = 0x2E5 // C->S separa pilha do Carry (Slot@12, Index@16, Amount@20)
	OpMessageChat        = 0x333 // C<->S chat local/comandos (140B, texto@12)
	OpMessageWhisper     = 0x334 // C->S whisper/comando/day (MobName[16]@12, String[96]@28, 128B observado)
	OpSetShortSkill      = 0x378 // C<->S barra de 20 atalhos de skill (32B)
	OpChangeCity         = 0x291 // C->S entrou/trocou de cidade (village@12, 16B)
	OpMoveStop           = 0x2CB // C->S interrompe movimento antes de atacar (36B)
	OpRestart            = 0x289 // C->S renascer depois da morte (MSG_STANDARD, 12B)
	OpPing               = 0x3A0 // C->S keepalive (12B)
	OpSysQuit            = 0x3AE // C<->S solicita/confirma saida (16B)
	OpUseNPC             = 0x28B // C->S clicou num NPC generico (npcid@12, 20B)
	OpRequestInfoPlay    = 0x324 // S->C janela de inspecionar jogador (212B) -- suporte no 7.48 A CONFIRMAR
	OpGuildDeprivate     = 0x28C // C->S expulsa membro da guild (MSG_STANDARDPARM: client id do alvo@12)
	OpGuildAlly          = 0xE12 // C->S alianca entre guilds (Guild@12, Ally@16) -- so o mestre
	OpGuildWar           = 0xE0E // C->S declara guerra (Guild@12, Enemy@16) -- sem sistema de guerra
	OpChallenge          = 0x28E // C->S interacao/desafio de zona de guild (Parm@12)
	OpChallengeConfirm   = 0x28F // C->S confirma desafio de zona (Parm1/Parm2)
	OpReqTeleport        = 0x290 // C->S solicita portal na coordenada atual (16B)
	OpPKMode             = 0x399 // C->S ativa/desativa modo PK (Parm@12)
	OpPKInfo             = 0x166 // S->C publica estado PK (MSG_STANDARDPARM)
	OpReqShopList        = 0x27B // C->S clicou num mercador tipo 3 (TargetID@12) -> pede loja
	OpShopList           = 0x17C // S->C lista da loja / abre a janela (532B, ShopType@12=1)
	OpBuyItem            = 0x379 // C->S compra item da loja (TargetID@12, sellSlot@14)
	OpSellItem           = 0x37A // C->S vende item pro mercador (TargetID@12, MyType@14, MyPos@16)
	OpApplyBonus         = 0x277 // C->S stats/mastery/skill (BonusType@12, Detail@14, TargetID@16, 20B)
	OpPartyAdd           = 0x37D // S->C adiciona/atualiza membro no painel do grupo (40B)
	OpPartyRemove        = 0x37E // C<->S sair/expulsar membro do grupo (16B)
	OpPartyRequest       = 0x37F // C<->S convite de grupo 7.54 (44B)
	OpPartyAccept        = 0x3AB // C->S aceita convite (32B)
	OpTrade              = 0x383 // C<->S convite/oferta/confirmacao de trade (156B)
	OpCloseTrade         = 0x384 // C<->S cancela e fecha a janela de trade (12B)
	OpCNFTradeCheck      = 0x386 // S->C confirma o primeiro check do trade (12B)
	OpAutoTrade          = 0x397 // C<->S publica/retorna os 12 anuncios da auto-loja (196B)
	OpReqBuyAutoTrade    = 0x398 // C->S compra um anuncio da auto-loja (36B)
	OpReqTradeList       = 0x39A // C->S solicita a lista de uma loja fantasma (16B)
	OpItemSold           = 0x39B // S->C remove da janela o anuncio que foi vendido (20B)
	OpAttackMulti        = 0x36C // C<->S skill multi-alvo (96B no client 7.48 real)
	OpAttackOne          = 0x39D // C->S ataca alvo unico (o melee real do 7.48)
	OpAttackTwo          = 0x39E // C->S ataca (2 maos)
	OpReqRanking         = 0x39F // C->S consulta informacoes de outro jogador (Parm1=TargetID)
	OpCombineComplete    = 0x3A7 // S->C resultado comum de composicao (0/1/2)
	OpCombineLindy       = 0x2C3 // C->S composicao Lindy
	OpCombineOdin        = 0x2D2 // C->S composicao Alquimista Odin
	OpCombineEhre        = 0x2D3 // C->S composicao Ehre
	OpCombineCompositor  = 0x3A6 // C->S compositor de item extremo/joia
	OpCombineAylin       = 0x3B5 // C->S compositor +10
	OpCombineAgatha      = 0x3BA // C->S transferencia Mortal -> Arch
	OpCombineTiny        = 0x3C0 // C->S composicao Tiny (Item[8]+CarryPos[8])
	OpRemoveMob          = 0x165 // S->C remove/mata (0=sai da visao, 1=morte, 3=exclusao imediata)
)

// InitCode -- magic de 4 bytes que o client 7.48 manda ao conectar, antes de
// qualquer pacote (CPSock.h). O servidor valida e pula.
const InitCode = 0x1F11F311

// IDs de "cena/sistema" usados no campo ID@6 de pacotes que nao pertencem a um
// player especifico (o client casa por esses valores fixos).
const (
	SceneField    = 0x7530 // CreateMob / WarInfo
	SceneEnter    = 0x7531 // EnterWorld
	SceneCharList = 0x7532 // CharList
)
