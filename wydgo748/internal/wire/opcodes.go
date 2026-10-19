package wire

// WYD 7.48 protocol opcodes (field Type@4 of the _MSG header).
// C->S = sent by the client; S->C = sent by the server.
const (
	OpMessageIndexed           = 0x105 // S->C coordinated extension: int16 index relative to 1000 in String[2:4]
	OpMessageParameterized     = 0x106 // S->C coordinated extension: 0x105 + up to six CSV parameters in String[4:]
	OpClientIntegrityChallenge = 0x1C1 // S->C native byte probe of the animation array
	OpClientIntegrityResponse  = 0x2C2 // C->S native reply to the integrity probe
)

const (
	OpMessagePanel       = 0x101 // S->C floating notice at the top (108B: Header + text[96], ID=0)
	OpConnectAccount     = 0x20D // C->S login (account@12, password@28, cliver@40)
	OpCharList           = 0x10A // S->C character list (2360B, Score uint32)
	OpCreateCharacter    = 0x20F // C->S create character (slot@12, name@16, class@32)
	OpCNFNewCharacter    = 0x110 // S->C confirms the creation and updates STRUCT_SELCHAR
	OpNewCharacterFail   = 0x11A // S->C creation refused (MSG_STANDARD)
	OpAlreadyPlaying     = 0x11C // S->C account already connected (MSG_STANDARD)
	OpCharacterLogin     = 0x213 // C->S enter the world (selected slot)
	OpCharacterLogout    = 0x215 // C->S back from the world to character selection
	OpEnterWorld         = 0x114 // S->C full STRUCT_MOB (2104B)
	OpCNFCharacterLogout = 0x116 // S->C confirms CharacterLogout (MSG_STANDARD)
	OpDeleteCharacter    = 0x211 // C->S deletes a character (slot/name/password)
	OpCNFDeleteCharacter = 0x112 // S->C updates the selection after a deletion
	OpCharacterTransfer  = 0xFAA // C<->S transfer/rename at selection (52B)
	OpDeclareServerWar   = 0xED7 // C->S war declaration letter 4030: target channel@12 (16B)
	OpRefuseServerWar    = 0xED8 // C->S war rejection letter 4031: target channel@12 (16B)
	OpBuyLotteryTicket   = 0x3B8 // C->S WYD Lottery: 0x379 body + six numbers@24 (32B)
	OpHellStoreBuy       = 0x2BD // C->S Hell Gate store: target@12 option@14 carry@16 (24B)
	OpStoreGoldRequest   = 0x3D1 // C->S native gold prompt mode 7 (merchant 9, head 51): amount@12 (16B)
	OpCreateMobTrade     = 0x363 // S->C coordinated extension: entity + title (352B)
	OpCreateMob          = 0x364 // S->C coordinated extension: entity/Score@140 (328B)
	OpSetHpMp            = 0x181 // S->C coordinated extension: HP/MP uint32 (28B)
	OpUpdateScore        = 0x336 // S->C coordinated extension: Score/affects (232B)
	OpUpdateAffect       = 0x3B9 // S->C the owner's 16 full affects (140B)
	OpUpdateEtc          = 0x337 // S->C Hold/exp/three WORD points/gold (native 36B; CP stays in CreateMob)
	OpCNFMobKill         = 0x338 // S->C confirms the death and updates the killer's EXP
	OpUpdateCarry        = 0x185 // S->C interactive inventory (528B)
	OpUpdateCargoGold    = 0x339 // S->C gold stored in the Cargo (16B)
	OpUpdateEquip        = 0x36B // S->C equipment visual (60B: ItemEff + AnctCode)
	OpAction             = 0x366 // C<->S walk / Action (52B)
	OpActionStop         = 0x367 // C->S stops the current action/movement (52B)
	OpIllusion           = 0x368 // C<->S Action2 of the Illusion skill (52B, Effect=6)
	OpREQMobByID         = 0x369 // C->S asks for an absent entity referenced by an Action
	OpMotion             = 0x36A // C->S one-off effect/motion (20B)
	OpInstanceTime       = 0x3A1 // S->C green time counter
	OpInstanceMobs       = 0x3B0 // S->C remaining monster counter
	OpClientUnknown2BC   = 0x2BC // C->S 7.48 client telemetry/state (108B)
	OpSwapItem           = 0x376 // C<->S move/equip an item and confirm the lifecycle (20B)
	OpWithdraw           = 0x387 // C<->S withdraw gold from the Cargo (16B)
	OpDeposit            = 0x388 // C<->S deposit gold into the Cargo (16B)
	OpUseItem            = 0x373 // C->S use a volatile item (36B)
	OpUsePremiumFirework = 0x3C9 // C->S drawn Premium FireCracker (52B, 10x10 bitmap@34)
	OpPremiumFirework    = 0x3CA // S->C publishes the drawing to the observers (36B, bitmap@20)
	OpCapsuleInfo        = 0x2CD // C->S queries a Spirit's Seal (16B)
	OpPutoutSeal         = 0x3CC // C->S extracts the character from a Spirit's Seal (52B)
	OpBuyToto            = 0x3CE // C->S buys a TOTO ticket (36B: shop/target/match/scores)
	OpDoJackpotBet       = 0x2BE // C->S 7.48 Gamble bet (20B: type@12, bet@16)
	OpResultGamble       = 0x1BF // S->C 7.48 Gamble result (36B)
	OpCNFCapsuleInfo     = 0xDC3 // S->C compact MSG_CAPSULEINFO of the 7.48 client (52B)
	OpSendItem           = 0x182 // S->C confirms ONE slot (24B)
	OpWarInfo            = 0x3A8 // S->C war info (24B)
	OpDropItem           = 0x272 // C->S drop an item (32B)
	OpCNFDropItem        = 0x175 // S->C confirms an item drop (28B)
	OpCreateItem         = 0x26E // S->C an item appears on the ground (32B)
	OpGetItem            = 0x270 // C->S pick an item up from the ground (28B)
	OpCNFGetItem         = 0x171 // S->C confirms the pickup (28B)
	OpRemoveItem         = 0x16F // S->C removes an item from the ground (16B)
	OpUpdateItem         = 0x374 // C<->S asks for/confirms a ground object state (20B)
	OpDeleteItem         = 0x2E4 // C->S deletes an item from the Carry (Slot@12, Index@16)
	OpSplitItem          = 0x2E5 // C->S splits a Carry stack (Slot@12, Index@16, Amount@20)
	OpMessageChat        = 0x333 // C<->S local chat/commands (String[96]@12, 108B)
	OpGuildDisable       = 0x3A4 // S->C native guild-affiliation signal (Parm@12, 16B)
	OpMessageWhisper     = 0x334 // C->S whisper/command/day (MobName[16]@12, String[96]@28, 128B observed)
	OpSetShortSkill      = 0x378 // C<->S bar of 20 skill shortcuts (32B)
	OpChangeCity         = 0x291 // C->S entered/changed city (village@12, 16B)
	OpMoveStop           = 0x2CB // C->S stops moving before attacking (36B)
	OpRestart            = 0x289 // C->S respawn after death (MSG_STANDARD, 12B)
	OpPing               = 0x3A0 // C->S keepalive (12B)
	OpSysQuit            = 0x3AE // C<->S asks for/confirms quitting (16B)
	OpUseNPC             = 0x28B // C->S clicked a generic NPC (npcid@12, 20B)
	OpGuildDeprivate     = 0x28C // C->S expels a guild member (MSG_STANDARDPARM: target client id@12)
	OpInviteGuild        = 0x3D5 // C->S recruits a player (GuildTarget@12, InviteType@16)
	OpGuildAlly          = 0xE12 // C->S alliance between guilds (Guild@12, Ally@16) -- master only
	OpGuildWar           = 0xE0E // C->S declares/cancels a war (Guild@12, Enemy@16)
	OpChallenge          = 0x28E // C->S guild zone interaction/challenge (Parm@12)
	OpChallengeConfirm   = 0x28F // C->S confirms a zone challenge (Parm1/Parm2)
	OpReqChallenge       = 0x18D // S->C opens the native application dialog (MSG_STANDARD)
	OpWeather            = 0x18B // S->C world weather (MSG_STANDARDPARM, 16B): 0 clear, 1 rain, 2 snow, 3 heavy snow
	OpReqTeleport        = 0x290 // C->S asks for the portal at the current coordinate (16B)
	OpAirMove            = 0xAD9 // C->S native AirMove: route@12, mode@16 (20B)
	OpPKMode             = 0x399 // C->S enables/disables PK mode (Parm@12)
	OpReqShopList        = 0x27B // C->S clicked a type-3 merchant (TargetID@12) -> asks for the shop
	OpShopList           = 0x17C // S->C shop list / opens the window (236B, 27 items)
	OpBuyItem            = 0x379 // C<->S buys/confirms a shop item (24B)
	OpSellItem           = 0x37A // C->S sells an item to the merchant (TargetID@12, MyType@14, MyPos@16)
	OpApplyBonus         = 0x277 // C->S stats/mastery/skill (BonusType@12, Detail@14, TargetID@16, 20B)
	OpPartyAdd           = 0x37D // S->C adds/updates a member in the party panel (40B)
	OpPartyRemove        = 0x37E // C<->S leave/expel a party member (16B)
	OpPartyRequest       = 0x37F // C<->S 7.54 party invitation (44B)
	OpPartyAccept        = 0x3AB // C->S accepts an invitation (32B)
	OpTrade              = 0x383 // C<->S trade invitation/offer/confirmation (156B)
	OpCloseTrade         = 0x384 // C<->S cancels and closes the trade window (12B)
	OpCNFTradeCheck      = 0x386 // S->C confirms the first trade check (12B)
	OpAutoTrade          = 0x397 // C<->S publishes/returns the auto-shop's 12 listings (196B)
	OpReqBuyAutoTrade    = 0x398 // C->S buys an auto-shop listing (36B)
	OpReqTradeList       = 0x39A // C->S asks for the list of a ghost shop (16B)
	OpItemSold           = 0x39B // S->C removes the sold listing from the window (20B)
	OpAttackMulti        = 0x36C // C<->S multi-target skill (96B in the real 7.48 client)
	OpAttackOne          = 0x39D // C->S single-target attack (the real 7.48 melee)
	OpAttackTwo          = 0x39E // C->S attack (2 hands)
	OpPlayerChallenge    = 0x39F // C<->S challenge between players (Parm1=TargetID, Parm2=mode)
	OpCombineComplete    = 0x3A7 // S->C common composition result (0/1/2)
	OpCombineLindy       = 0x2C3 // C->S Lindy composition
	OpCombineOdin        = 0x2D2 // C->S Alchemist Odin composition
	OpCombineEhre        = 0x2D3 // C->S Ehre composition
	OpCombineExtracao    = 0x2D4 // C->S Extraction skill (Parm2=inventory slot)
	OpCombineAlquimia    = 0x2E1 // C->S Alchemy skill (MSG_CombineItem)
	OpCombineCompositor  = 0x3A6 // C->S extreme item/jewel compositor
	OpCombineAylin       = 0x3B5 // C->S +10 compositor
	OpCombineAgatha      = 0x3BA // C->S Mortal -> Arch transfer
	OpCombineTiny        = 0x3C0 // C->S Tiny composition (Item[8]+CarryPos[8])
	OpRemoveMob          = 0x165 // S->C removes/kills (0=leaves view, 1=death, 3=immediate removal)
)

// InitCode -- 4-byte magic the 7.48 client sends on connect, before any
// packet (CPSock.h). The server validates and skips it.
const InitCode = 0x1F11F311

// "Scene/system" IDs used in field ID@6 of packets that belong to no specific
// player (the client matches these fixed values).
const (
	SceneField    = 0x7530 // CreateMob / WarInfo
	SceneEnter    = 0x7531 // EnterWorld
	SceneCharList = 0x7532 // CharList
)
