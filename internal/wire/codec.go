package wire

import (
	"encoding/binary"
	"time"

	"wydgo/internal/model"
)

// Build aloca um buffer de pacote com Type@4 e ID@6 preenchidos. Size@0 e Tick@8
// sao preenchidos no envio (net.Session.Send -> FinishPacket). Todo builder
// abaixo devolve o pacote nesse estado "meio-pronto".
func Build(typ, id uint16, size int) []byte {
	b := make([]byte, size)
	binary.LittleEndian.PutUint16(b[4:6], typ)
	binary.LittleEndian.PutUint16(b[6:8], id)
	return b
}

// MessageChat monta o wire 7.48 de 0x333: header de 12 bytes seguido por
// String[96]. O 7.54 nativo rejeita qualquer tamanho diferente de 108 bytes.
func MessageChat(id uint16, message string) []byte {
	b := Build(OpMessageChat, id, 108)
	copy(b[12:107], message)
	return b
}

// MessageWhisper tambem e o correio nativo do 7.48: quando message inicia em
// '!', TMFieldScene::OnPacketMessageWhisper grava o corpo no painel H em vez
// de exibi-lo no chat. O cliente 7.48 usa Name[16]@12, String[96]@28 e
// Color@124, totalizando EXATOS 128 bytes (nao o layout 160B do 7.59/W2PP).
func MessageWhisper(senderID uint16, from, message string, color uint16) []byte {
	// No 7.48, ID=0 despacha o whisper comum pelo TMFieldScene. Um ID de jogador
	// o envia ao TMHuman; esse caminho serve aos prefixos especiais, mas deixa a
	// mensagem privada comum sem texto. O parametro permanece explicito porque
	// cartas/variantes futuras podem escolher o destino do dispatcher.
	b := Build(OpMessageWhisper, senderID, 128)
	copy(b[12:28], from)
	copy(b[28:123], message)
	putU32(b, 124, uint32(color))
	return b
}

// MessagePanel monta o aviso nativo exibido no topo da tela por cerca de
// quatro segundos. O client exige Header.ID=0 para despachar este pacote pelo
// TMScene em vez de tentar associa-lo a um personagem.
func MessagePanel(message string) []byte {
	b := Build(OpMessagePanel, 0, 108)
	copy(b[12:107], message)
	return b
}

// DaySync responde ao pedido periodico MobName="day" do client 7.48. O prefixo
// !# e consumido por TMScene e nao aparece como aviso: "11" vira m_nYear e
// "  2" vira m_nDays. O valor fixo reproduz o TMSrv 7.54 capturado e o W2PP.
func DaySync() []byte {
	return MessagePanel("!#11  2")
}

// Motion monta MSG_Motion (0x36A), usado pelo servidor para propagar emotes e
// efeitos pontuais. Fogos de artificio usam Motion=100 e Parm=0..5 no nativo.
func Motion(id uint16, motion, parm uint16) []byte {
	b := Build(OpMotion, id, 20)
	putU16(b, 12, motion)
	putU16(b, 14, parm)
	return b
}

// StandardParm monta o layout WYD de 16 bytes usado por PKInfo e diversos
// sinais de confirmacao: header seguido de um DWORD em @12.
func StandardParm(opcode, id uint16, parm uint32) []byte {
	b := Build(opcode, id, 16)
	putU32(b, 12, parm)
	return b
}

// CapsuleInfoData representa os campos que a UI 7.48 aceita em
// MSG_CAPSULEINFO. O executavel copia exatamente 13 DWORDs (52 bytes) para um
// cache de 12 entradas. Nesta versao existem somente dois WORDs de Mastery;
// Skill continua tendo os nove campos exibidos em tres linhas no tooltip.
type CapsuleInfoData struct {
	Class   uint16
	Level   uint16
	Str     uint16
	Int     uint16
	Dex     uint16
	Con     uint16
	Mastery [2]uint16
	Skill   [9]uint16
	Quest   uint16
}

// CNFCapsuleInfo monta o 0xDC3 recebido diretamente por WYD.exe 7.48. O 0xD1F
// de 76 bytes e um pacote interno DB/TMSrv de fontes posteriores e nao possui
// handler neste executavel. O ID da capsula fica em @12.
func CNFCapsuleInfo(sessionID, capsuleID uint16, info CapsuleInfoData) []byte {
	b := Build(OpCNFCapsuleInfo, sessionID, 52)
	putU32(b, 12, uint32(capsuleID))
	values := [...]uint16{
		info.Class, info.Level, info.Str, info.Int, info.Dex, info.Con,
		info.Mastery[0], info.Mastery[1],
		info.Skill[0], info.Skill[1], info.Skill[2], info.Skill[3], info.Skill[4],
		info.Skill[5], info.Skill[6], info.Skill[7], info.Skill[8], info.Quest,
	}
	for i, value := range values {
		putU16(b, 16+i*2, value)
	}
	return b
}

// PartyRequest replica MSG_REQParty do protocolo 7.54. A estrutura antiga tem
// 44 bytes e carrega o Target como DWORD em @40. Versoes posteriores inseriram
// outro campo e deslocaram o Target para @44, aumentando o pacote para 48 bytes.
func PartyRequest(leaderID uint16, name string, class byte, level, hp, maxHP uint16, targetID uint16) []byte {
	b := Build(OpPartyRequest, SceneField, 44)
	hp, maxHP = partyDisplayHP(hp, maxHP)
	b[12] = class
	b[13] = 0 // PartyIndex 0 = lider/convite pendente
	putU16(b, 14, level)
	putU16(b, 16, maxHP)
	putU16(b, 18, hp)
	putU16(b, 20, leaderID)
	copy(b[22:38], name)
	putU32(b, 40, uint32(targetID))
	return b
}

// PartyMember monta MSG_CNFAddParty/MSG_AddParty (0x37D). PARTY ocupa 28 bytes
// a partir de @12; o client usa PartyIndex 0 para destacar o lider.
func PartyMember(id uint16, name string, class, partyIndex byte, level, hp, maxHP uint16) []byte {
	b := Build(OpPartyAdd, SceneField, 40)
	hp, maxHP = partyDisplayHP(hp, maxHP)
	b[12] = class
	b[13] = partyIndex
	putU16(b, 14, level)
	putU16(b, 16, maxHP)
	putU16(b, 18, hp)
	putU16(b, 20, id)
	copy(b[22:38], name)
	return b
}

// PartyRemove usa MSG_STANDARDPARM. memberID=0 limpa todo o painel; um ID
// especifico remove apenas aquele membro.
func PartyRemove(memberID uint16) []byte {
	b := Build(OpPartyRemove, SceneField, 16)
	putU32(b, 12, uint32(memberID))
	return b
}

// Os campos de HP do PARTY nativo sao short com sinal. O TMSrv reduz valores
// acima de 32000 para percentual/centena antes de montar o painel.
func partyDisplayHP(hp, maxHP uint16) (uint16, uint16) {
	if hp > 32000 {
		hp = (hp + 1) / 100
	}
	if maxHP > 32000 {
		maxHP = (maxHP + 1) / 100
	}
	return hp, maxHP
}

func putU16(b []byte, off int, v uint16) { binary.LittleEndian.PutUint16(b[off:off+2], v) }
func putU32(b []byte, off int, v uint32) { binary.LittleEndian.PutUint32(b[off:off+4], v) }

// O byte de CreateMob guarda o PK bruto nativo, CP+75. O dominio do modelo
// permanece assinado (-75..+75), portanto o personagem neutro (CP 0) transmite
// 75. 150 e reservado ao extremo positivo (+75), nao ao estado normal.
const NormalNameChaos = 75

func CPNameByte(cp int16) byte {
	return byte(model.ClampCP(int(cp)) + 75)
}

// PutItem escreve um STRUCT_ITEM (8B) em b@off: sIndex + 3 pares de efeito.
func PutItem(b []byte, off int, it model.Item) {
	putU16(b, off, it.Index)
	copy(b[off+2:off+8], it.Eff[:])
}

// Trade monta MSG_Trade 0x383 (156B) no layout do client 7.48:
// Item[15]@12, CarryPos[15]@132, padding@147, TradeMoney@148,
// MyCheck@152, padding@153 e OpponentID@154.
func Trade(id uint16, items [15]model.Item, carryPos [15]int8, gold uint32, checked bool, opponentID uint16) []byte {
	b := Build(OpTrade, id, 156)
	for i := range items {
		PutItem(b, 12+i*8, items[i])
		b[132+i] = byte(carryPos[i])
	}
	putU32(b, 148, gold)
	if checked {
		b[152] = 1
	}
	putU16(b, 154, opponentID)
	return b
}

func CloseTrade(id uint16) []byte {
	return Build(OpCloseTrade, id, 12)
}

// RepurchaseEntry e a estrutura que o client 7.48 desenha na janela de
// recompra: Order@0, STRUCT_ITEM@4 e Price@12. O servidor nao envia UID;
// esse campo permanece exclusivamente no estado autoritativo.
type RepurchaseEntry struct {
	Order uint32
	Item  model.Item
	Price uint32
}

// RepurchaseList monta MSG_RepurchaseItems do client 7.48 (176 bytes):
// target@12 seguido por dez entradas de 16 bytes. O mesmo opcode e usado para
// o pedido e para a resposta; os campos do pedido sao ignorados depois de
// validar Header.ID e o contexto de loja.
func RepurchaseList(id uint16, target uint32, entries [10]RepurchaseEntry) []byte {
	b := Build(OpRebuy, id, 176)
	putU32(b, 12, target)
	for i, entry := range entries {
		off := 16 + i*16
		putU32(b, off, entry.Order)
		PutItem(b, off+4, entry.Item)
		putU32(b, off+12, entry.Price)
	}
	return b
}

func CNFTradeCheck(id uint16) []byte {
	return Build(OpCNFTradeCheck, id, 12)
}

// AutoTrade monta MSG_AutoTrade 0x397 (196B): Desc[24]@12, Item[12]@36,
// CarryPos[12]@132, TradeMoney[12]@144, Tax@192 e TargetID@194.
func AutoTrade(title string, items [12]model.Item, carryPos [12]int8, prices [12]uint32, tax, targetID uint16) []byte {
	b := Build(OpAutoTrade, SceneField, 196)
	copy(b[12:35], title)
	for i := range items {
		PutItem(b, 36+i*8, items[i])
		b[132+i] = byte(carryPos[i])
		putU32(b, 144+i*4, prices[i])
	}
	putU16(b, 192, tax)
	putU16(b, 194, targetID)
	return b
}

// CreateMobTrade segue a conversao exata do PacketProtocolV754: o prefixo
// visual/status coincide com CreateMob ate @171 e StoreName[27] comeca em @172.
// O MobID e o ID virtual da loja fantasma, nao o ID do personagem que a criou.
func CreateMobTrade(id uint16, name string, x, y uint16, mesh []uint16, sc model.WireScore, title string) []byte {
	// FixGetCreateMobTrade_SERVER zera @66..99: clones de loja nao carregam
	// affects nem guild do personagem real.
	normal := CreateMob(id, name, x, y, mesh, sc, nil, 2)
	b := Build(OpCreateMobTrade, SceneField, 200)
	copy(b[12:172], normal[12:172])
	copy(b[172:199], title)
	return b
}

// CreateMobTradeExtended mantem a projecao STRUCT_SCORE confinada ao wire.
func CreateMobTradeExtended(id uint16, name string, x, y uint16, mesh []uint16, ext *model.ExtendedScore, title string) []byte {
	return CreateMobTrade(id, name, x, y, mesh, compatibilityScore(ext), title)
}

// ItemSold usa MSG_STANDARDPARM2: tradeEntityID@12 e posicao do anuncio@16.
// Em lojas fantasma, tradeEntityID e o ID virtual do clone.
func ItemSold(tradeEntityID uint16, pos uint32) []byte {
	b := Build(OpItemSold, SceneField, 20)
	putU32(b, 12, uint32(tradeEntityID))
	putU32(b, 16, pos)
	return b
}

// PutScore escreve um STRUCT_SCORE 7.48 (28B) em b@off. Merchant@+6 e AttackRun@+7
// sao bytes CHEIOS (nao nibbles) -- foi o erro que quebrou o engine no C++.
// Este e o layout do Status@100 do CreateMob, do enter-world e do 0x336.
func PutScore(b []byte, off int, sc model.WireScore) {
	putU16(b, off+0, sc.Level)
	putU16(b, off+2, sc.Defense)
	putU16(b, off+4, sc.Attack)
	b[off+6] = sc.Merchant
	b[off+7] = sc.AttackRun
	putU16(b, off+8, sc.MaxHP)
	putU16(b, off+10, sc.MaxMP)
	putU16(b, off+12, sc.CurHP)
	putU16(b, off+14, sc.CurMP)
	putU16(b, off+16, sc.Str)
	putU16(b, off+18, sc.Int)
	putU16(b, off+20, sc.Dex)
	putU16(b, off+22, sc.Con)
	copy(b[off+24:off+28], sc.Mastery[:])
}

// putSelChar escreve o STRUCT_SELCHAR 7.48 (744B): HomeTownX[4]@0,
// HomeTownY[4]@8, nomes@16, scores@80, equips@192, guild@704,
// coin@712 e EXP uint32@728.
func putSelChar(b []byte, sel int, chars []model.Char) {
	for slot := 0; slot < 4 && slot < len(chars); slot++ {
		ch := chars[slot]
		if ch.Name == "" {
			continue
		}
		putU16(b, sel+slot*2, ch.X)
		putU16(b, sel+8+slot*2, ch.Y)
		copy(b[sel+16+slot*16:sel+16+(slot+1)*16], ch.Name)
		PutScore(b, sel+80+slot*28, clientCompatibilityScore(ch))
		for i, it := range ch.Equip {
			PutItem(b, sel+192+(slot*16+i)*8, it)
		}
		// STRUCT_SELCHAR.Guild e a fonte que o client usa no SelectChar e na
		// primeira entrada do FieldScene para reconstruir a guildmark do proprio
		// personagem. O rank chega no EnterWorld/UpdateScore; o indice precisa
		// estar presente aqui tambem.
		putU16(b, sel+704+slot*2, GuildWireID(ch.GuildID))
		putU32(b, sel+712+slot*4, ch.Gold)
		putU32(b, sel+728+slot*4, ch.Exp)
	}
}

// CharList monta o login 0x10E (1800B): SelChar(744)@12 + Cargo(1024) +
// Coin + AccountName@1784.
func CharList(accName string, chars []model.Char, cargo []model.Item, cargoGold uint32) []byte {
	b := Build(OpCharList, SceneCharList, 1800)
	putSelChar(b, 12, chars)
	for i := 0; i < len(cargo) && i < model.MaxCargo; i++ {
		PutItem(b, 756+i*8, cargo[i])
	}
	putU32(b, 1780, cargoGold)
	copy(b[1784:1784+16], accName)
	return b
}

// UpdateCargoGold sincroniza o cofre monetario da conta (MSG_STANDARDPARM).
func UpdateCargoGold(id uint16, gold uint32) []byte {
	b := Build(OpUpdateCargoGold, id, 16)
	putU32(b, 12, gold)
	return b
}

// CargoGoldTransfer confirma deposito/saque. O client aplica o mesmo valor aos
// dois contadores e o UpdateCargoGold subsequente reafirma o saldo autoritativo.
func CargoGoldTransfer(op uint16, id uint16, amount uint32) []byte {
	b := Build(op, id, 16)
	putU32(b, 12, amount)
	return b
}

// CNFNewCharacter monta o 0x110 que faz a tela de selecao recarregar os
// quatro slots depois de uma criacao bem-sucedida.
func CNFNewCharacter(id uint16, chars []model.Char) []byte {
	b := Build(OpCNFNewCharacter, id, 756)
	putSelChar(b, 12, chars)
	return b
}

// CNFDeleteCharacter usa o mesmo STRUCT_SELCHAR da confirmacao de criacao,
// mas o client 7.48 o despacha pelo opcode 0x112 e redesenha a selecao.
func CNFDeleteCharacter(id uint16, chars []model.Char) []byte {
	b := Build(OpCNFDeleteCharacter, id, 756)
	putSelChar(b, 12, chars)
	return b
}

func NewCharacterFail() []byte { return Build(OpNewCharacterFail, 0, 12) }

// CNFCharacterLogout e somente um MSG_STANDARD. O ID deve ser o ClientID do
// personagem que esta deixando o mundo; o TMFieldScene troca para SelectChar
// somente quando ele coincide com o ID local.
func CNFCharacterLogout(id uint16) []byte { return Build(OpCNFCharacterLogout, id, 12) }

// AlreadyPlaying e a resposta nativa que o TMSelectServerScene 7.48 trata
// ainda na tela de login, reabilitando os campos e exibindo o erro localizado.
func AlreadyPlaying() []byte { return Build(OpAlreadyPlaying, 0, 12) }

// EnterWorld monta o 0x114 (788B): PosXY@12 + STRUCT_MOB(756)@16 + trailing@772.
// Mob (base @16): Name@0 Class@20 Coin@24 Exp@28 SPX@32 SPY@34 BaseScore@36,
// CurrentScore@64, Equip@92, Inventory@220 e a cauda autoritativa @732..755.
// Essa cauda termina exatamente no byte 755 do MOB truncado usado pelo 7.48;
// omiti-la inicializa LearnedSkill/pontos/regen/resists como lixo ou zero.
func EnterWorld(id uint16, ch model.Char) []byte {
	b := Build(OpEnterWorld, id, 788)
	putU16(b, 12, ch.X) // PosX (antes do mob)
	putU16(b, 14, ch.Y) // PosY
	const m = 16
	copy(b[m+0:m+16], ch.Name)
	// MobName[12]=@28 carries the same native CP+75 projection used by
	// CreateMob. Keeping this neutral would make a relogged PK character look
	// neutral until the later visibility refresh arrives.
	b[m+12] = CPNameByte(ch.CP)
	b[m+20] = ch.Class
	putU32(b, m+24, ch.Gold) // Coin (coerente com o 0x337; nao alimenta o display)
	putU32(b, m+28, ch.Exp)  // Exp
	putU16(b, m+32, ch.X)    // SPX
	putU16(b, m+34, ch.Y)    // SPY
	wireScore := clientCompatibilityScore(ch)
	e := scoreWireExtension(ch)
	PutScore(b, m+36, wireScore)
	PutScore(b, m+64, wireScore)
	for i, it := range ch.Equip {
		PutItem(b, m+92+i*8, it)
	}
	for i, it := range ch.Inv {
		PutItem(b, m+220+i*8, it)
	}
	putU32(b, m+732, ch.LearnedSkill)
	putU16(b, m+736, compatibilityU16(e.StatusPts))
	putU16(b, m+738, compatibilityU16(e.MasterPts))
	putU16(b, m+740, compatibilityU16(e.SkillPts))
	b[m+742] = clampByte(int(e.Critical))
	b[m+743] = clampByte(int(e.SaveMana))
	copy(b[m+744:m+748], ch.ShortSkill[:4])
	// m+748 = GuildMemberType/GuildLevel. O client usa esse byte no proprio
	// login para selecionar a moldura da guildmark antes do primeiro 0x364.
	b[m+748] = ch.GuildRank
	b[m+749] = clampByte(int(e.MagicAmp))
	b[m+750] = clampByte(int(e.RegenHP))
	b[m+751] = clampByte(int(e.RegenMP))
	b[m+752] = clampByte(int(e.ResistFire))
	b[m+753] = clampByte(int(e.ResistIce))
	b[m+754] = clampByte(int(e.ResistHoly))
	b[m+755] = clampByte(int(e.ResistThunder))
	// Cauda decifrada (era o "trailing magico" 02 00 01 00 da captura): Slot@772 e
	// **ClientID@774** -- e DAQUI que o client aprende o ID do proprio personagem
	// (TMSelectCharScene: m_dwCharID = pCharLogin->ClientID). Hardcodar 1 aqui foi a
	// causa-raiz do bug do relog (id=2 + ClientID=1 -> self dessincronizado: CP lixo,
	// HP/MP travados). Weather@776 fica 0.
	putU16(b, 772, 2)  // Slot (valor da captura; char unico por ora)
	putU16(b, 774, id) // ClientID = o id REAL do player
	return b
}

func clampByte(v int) byte {
	if v < 0 {
		return 0
	}
	if v > 255 {
		return 255
	}
	return byte(v)
}

// CreateMob monta o 0x364 (176B) no layout REAL do client 7.48 (= Secrets p754,
// PROVADO in-game): @12 PosXY, @16 ClientId, @18 Name[12], @34 ItemEff[16],
// @66 Affect[16], @98 Guild, @100 Status(28), @128 Spawn. spawn: 0=NPC, 2=player.
func CreateMob(id uint16, name string, x, y uint16, mesh []uint16, sc model.WireScore, affects []model.Affect, spawn uint16) []byte {
	return CreateMobVisual(id, name, x, y, mesh, nil, sc, affects, spawn)
}

// CreateMobVisual inclui AnctCode@130, necessario para cores/refinos antigos.
func CreateMobVisual(id uint16, name string, x, y uint16, mesh []uint16, anct []byte, sc model.WireScore, affects []model.Affect, spawn uint16) []byte {
	b := Build(OpCreateMob, SceneField, 176)
	putU16(b, 12, x)
	putU16(b, 14, y)
	putU16(b, 16, id)
	copy(b[18:18+12], name)
	if spawn == 2 { // self: MobName[12]=@30 colore o nome (75 = neutro)
		b[30] = NormalNameChaos
	}
	for i, v := range mesh {
		putU16(b, 34+i*2, v)
	}
	putAffectWords(b, 66, affects, time.Now())
	PutScore(b, 100, sc) // Status @100 (curHP@112 = a vida real do mob)
	putU16(b, 128, spawn)
	copy(b[130:146], anct)
	return b
}

// CreateMobExtendedWithGuildRank monta o CreateMob 7.54 com a identidade
// completa da guild. No protocolo p754 o WORD em @98 guarda o id da guild
// (12 bits) e o canal (4 bits); o byte alto de Spawn em @129 e o
// GuildMemberType/GuildLevel. Sem esse segundo byte o client desenha o nome,
// mas nao consegue escolher o layout da guildmark (membro, sublider ou lider).
func CreateMobExtendedWithGuildRank(id uint16, name string, x, y uint16, mesh []uint16, anct []byte, ext *model.ExtendedScore, affects []model.Affect, spawn uint16, guild uint16, guildRank byte, cp int16) []byte {
	b := CreateMobVisual(id, name, x, y, mesh, anct, compatibilityScore(ext), affects, spawn)
	putU16(b, 98, GuildWireID(guild))
	// CreateMobVisual escreve o tipo de spawn no byte baixo. Preserve-o e
	// materialize o GuildLevel no byte alto, como STRUCT_MOB.Spawn do client.
	b[128] = byte(spawn)
	b[129] = guildRank
	// O byte de chaos/PK @30 colore o nome; para players ele e a projecao
	// CP+75, tanto no self quanto para observadores. Mobs/NPCs usam
	// CreateMobVisualExtended e nao passam por aqui.
	b[30] = CPNameByte(cp)
	return b
}

// GuildWireID aplica a faixa da ABI nativa: quatro bits de canal (atualmente
// canal unico = 0) e doze bits para o indice. IDs fora da faixa nao podem
// aparecer no wire; o registro canonico ja os rejeita, e a mascara evita que
// um estado legado corrompa a parte reservada ao canal.
func GuildWireID(id uint16) uint16 { return id & 0x0FFF }

// CreateMobVisualExtended e a variante com AnctCode para NPCs/monstros.
func CreateMobVisualExtended(id uint16, name string, x, y uint16, mesh []uint16, anct []byte, ext *model.ExtendedScore, affects []model.Affect, spawn uint16) []byte {
	return CreateMobVisual(id, name, x, y, mesh, anct, compatibilityScore(ext), affects, spawn)
}

// SetMobHpMp projeta os recursos uint32 do mob no 0x181 legado. A barra recebe
// a mesma escala proporcional usada para personagens; o estado real permanece
// exclusivamente no servidor.
func SetMobHpMp(id uint16, currentHP, maxHP, currentMP, maxMP uint32) []byte {
	ext := model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		CurHP:   currentHP, MaxHP: maxHP, CurMP: currentMP, MaxMP: maxMP,
	}
	sc := ext.CompatibilityScore()
	b := Build(OpSetHpMp, id, 20)
	putU16(b, 12, sc.CurHP)
	putU16(b, 14, sc.CurMP)
	putU16(b, 16, sc.MaxHP)
	putU16(b, 18, sc.MaxMP)
	return b
}

// SetHpMpExtended preserva o prefixo 0x181 de 20 bytes e acrescenta os quatro
// valores reais em uint32. O client sem patch ignora a cauda; o WYD748 patched
// alimenta BigHP/BigMP e os textos a partir dela.
func SetHpMpExtended(id uint16, ext *model.ExtendedScore) []byte {
	if ext == nil {
		ext = &model.ExtendedScore{}
	}
	sc := ext.CompatibilityScore()
	// Score ja contem a projecao proporcional e signed-safe calculada pelo
	// servidor. Ela alimenta somente barras/animacoes do engine legado; a cauda
	// abaixo continua sendo a fonte numerica autoritativa uint32.
	b := Build(OpSetHpMp, id, 36)
	putU16(b, 12, sc.CurHP)
	putU16(b, 14, sc.CurMP)
	putU16(b, 16, sc.MaxHP)
	putU16(b, 18, sc.MaxMP)
	putU32(b, 20, ext.CurHP)
	putU32(b, 24, ext.CurMP)
	putU32(b, 28, ext.MaxHP)
	putU32(b, 32, ext.MaxMP)
	return b
}

// UpdateScore monta o p754_SendScore (92B) no layout que o PacketProtocolV754
// envia ao client 7.48: Status@12, Critical@40, SaveMana@41, Affect[16]@42,
// Guild@74 (WORD), GuildLevel@76 (WORD), Resist@78, CurrHP@82, CurrMP@84,
// MagicIncrement@86, Unk[5]@87.
//
// O @74 e UM campo de 16 bits, nao dois bytes separados: o client le
// `unsigned short Guild` (MSG_UpdateScore em Basedef.h do client) e o decompoe
// como (canal << 12) | guildID -- por isso o id util tem 12 bits (1..4095).
// O prefixo nativo permanece byte-compativel justamente para o client conseguir
// ler campos como este; o ExtendedScore vive na cauda XSC2 e nao o substitui.
func UpdateScore(id uint16, ch model.Char) []byte {
	// A cauda wide e sempre enviada para jogadores e carrega o ExtendedRuntime.
	// O prefixo STRUCT_SCORE existe somente para manter a ABI do handler 7.48.
	e := scoreWireExtension(ch)
	wireScore := clientCompatibilityScore(ch)
	b := Build(OpUpdateScore, id, 236)
	PutScore(b, 12, wireScore)
	b[40] = clampByte(int(e.Critical))
	b[41] = clampByte(int(e.SaveMana))
	putAffectWords(b, 42, ch.Affects[:], time.Now())
	putU16(b, 74, GuildWireID(ch.GuildID))
	// O client 7.48 lê GuildLevel como WORD, embora o servidor nativo use
	// somente o byte baixo. RegenHP/MP pertencem ao XSC2 da cauda; escrever
	// esses valores em @76/@77 apagava o rank e impedia a guildmark visível.
	putU16(b, 76, uint16(ch.GuildRank))
	b[78] = clampByte(int(e.ResistFire))
	b[79] = clampByte(int(e.ResistIce))
	b[80] = clampByte(int(e.ResistHoly))
	b[81] = clampByte(int(e.ResistThunder))
	putU16(b, 82, wireScore.CurHP)
	putU16(b, 84, wireScore.CurMP)
	b[86] = clampByte(int(e.MagicAmp))
	putU32(b, 92, e.MagicAttack)
	putU32(b, 96, e.Attack)
	putU32(b, 100, e.Defense)
	putU32(b, 104, e.MaxHP)
	putU32(b, 108, e.MaxMP)
	putU32(b, 112, e.CurHP)
	putU32(b, 116, e.CurMP)
	putU32(b, 120, e.Str)
	putU32(b, 124, e.Int)
	putU32(b, 128, e.Dex)
	putU32(b, 132, e.Con)
	putU32(b, 136, e.Accuracy)
	putU32(b, 140, e.Evasion)
	putU32(b, 144, e.StatusPts)
	putU32(b, 148, e.MasterPts)
	putU32(b, 152, e.SkillPts)
	putU32(b, 156, e.Level)
	for i := range e.Mastery {
		putU32(b, 160+i*4, e.Mastery[i])
	}
	putU32(b, 176, e.Critical)
	putU32(b, 180, e.Parry)
	putU32(b, 184, e.Range)
	putU32(b, 188, e.ResistFire)
	putU32(b, 192, e.ResistIce)
	putU32(b, 196, e.ResistHoly)
	putU32(b, 200, e.ResistThunder)
	putU32(b, 204, e.SaveMana)
	putU32(b, 208, e.MagicAmp)
	putU32(b, 212, e.RegenHP)
	putU32(b, 216, e.RegenMP)
	putU32(b, 220, uint32(e.AttackRun))
	putU32(b, 224, uint32(e.Merchant))
	putU32(b, 228, model.ExtendedScoreVersion)
	putU32(b, 232, 0x32435358) // "XSC2"
	return b
}

func scoreWireExtension(ch model.Char) *model.ExtendedScore {
	if ch.ExtendedRuntime != nil {
		return ch.ExtendedRuntime
	}
	if ch.Extended != nil {
		return ch.Extended
	}
	return &model.ExtendedScore{Version: model.ExtendedScoreVersion}
}

// clientCompatibilityScore e somente a representacao estreita consumida pela
// logica legada do client. O estado autoritativo nunca e reduzido: os valores
// reais seguem na extensao uint32 de UpdateScore/SetHpMpExtended.
func clientCompatibilityScore(ch model.Char) model.WireScore {
	// Para chars extended, Score e deliberadamente uma projecao proporcional
	// <= 30000. Saturar atual e maximo separadamente em 32767 transforma toda
	// barra em 100% e impede o client de mostrar dano/gasto de mana.
	return scoreWireExtension(ch).CompatibilityScore()
}

func compatibilityScore(ext *model.ExtendedScore) model.WireScore {
	if ext == nil {
		return model.WireScore{}
	}
	return ext.CompatibilityScore()
}

// CompatibilityVitals devolve apenas os campos estreitos exigidos pelo painel
// de party. A estrutura Score nao atravessa a fronteira do wire.
func CompatibilityVitals(ext *model.ExtendedScore) (level, currentHP, maximumHP uint16) {
	score := compatibilityScore(ext)
	return score.Level, score.CurHP, score.MaxHP
}

// CompatibilityCombatMP projeta a mana para o WORD dos pacotes de ataque.
func CompatibilityCombatMP(ext *model.ExtendedScore) uint32 {
	return uint32(compatibilityScore(ext).CurMP)
}

func compatibilityU16(value uint32) uint16 {
	if value > 30_000 {
		return 30_000
	}
	return uint16(value)
}

// MobScore monta o mesmo 0x336 usado por jogadores, mas sem exigir model.Char.
// O client usa Affect[16] para ativar CheckAffect também em NPCs/monstros.
func MobScore(id uint16, sc model.WireScore, affects []model.Affect, resist model.ElementalResists) []byte {
	b := Build(OpUpdateScore, id, 92)
	PutScore(b, 12, sc)
	putAffectWords(b, 42, affects, time.Now())
	b[78] = clampByte(int(resist.Fire))
	b[79] = clampByte(int(resist.Ice))
	b[80] = clampByte(int(resist.Sacred))
	b[81] = clampByte(int(resist.Thunder))
	putU16(b, 82, sc.CurHP)
	putU16(b, 84, sc.CurMP)
	return b
}

// MobScoreExtended recebe o estado wide; Score so existe dentro do builder.
func MobScoreExtended(id uint16, ext *model.ExtendedScore, affects []model.Affect, resist model.ElementalResists) []byte {
	return MobScore(id, compatibilityScore(ext), affects, resist)
}

// O servidor usa a semantica 7.59 para Armadura Critica (affect 31), mas o
// client 7.48 chama esse slot de Coin Armor. Nesse client o visual/nome de
// Armadura Critica pertence ao slot 24; somente a representacao wire muda.
func clientAffectType(affect model.Affect) byte {
	if affect.ClientType != 0 {
		return affect.ClientType
	}
	return affect.Type
}

const affectWireUnit = 8 * time.Second

func affectTimeUnits(expiresAt, now time.Time) uint32 {
	remaining := expiresAt.Sub(now)
	if remaining <= 0 {
		return 0
	}
	return uint32((remaining + affectWireUnit - 1) / affectWireUnit)
}

// putAffectWords escreve WORDs no formato little-endian observado pelo client:
// byte baixo = tempo restante em unidades de 8s; byte alto = tipo visual.
func putAffectWords(b []byte, offset int, affects []model.Affect, now time.Time) {
	for i, affect := range affects {
		if i >= 16 || offset+i*2+1 >= len(b) {
			break
		}
		units := affectTimeUnits(affect.ExpiresAt, now)
		if affect.Type == 0 || units == 0 {
			continue
		}
		b[offset+i*2] = clampByte(int(units))
		b[offset+i*2+1] = clientAffectType(affect)
	}
}

// UpdateAffects monta o 0x3B9 p754/7.48: header + 16 STRUCT_AFFECT de 8B,
// cada um {Type, Value, Level, Time}. Time e contado em unidades de 8 segundos.
func UpdateAffects(id uint16, ch model.Char) []byte {
	b := Build(OpUpdateAffect, id, 140)
	now := time.Now()
	for i, affect := range ch.Affects {
		if i >= 16 {
			break
		}
		units := affectTimeUnits(affect.ExpiresAt, now)
		if affect.Type == 0 || units == 0 {
			continue
		}
		b[12+i*8] = clientAffectType(affect)
		b[13+i*8] = clampByte(affect.Value)
		putU16(b, 14+i*8, uint16(affect.Level))
		putU32(b, 16+i*8, units)
	}
	return b
}

// UpdateEtc monta o p754_SendEtc final: Hold@12, EXP@16,
// LearnedSkill@20, Status@24, Mastery@26, SkillPts@28, Magic@30 e Gold@32.
// LearnedSkill@20 e OBRIGATORIO: e daqui que o client sabe as skills aprendidas
// (TMHuman::OnPacketUpdateEtc copia LearnedSkill/bonus/coin deste pacote). Remover
// esse campo apaga as skills do client e desloca os pontos (regressao 2026-07-14).
func UpdateEtc(id uint16, ch model.Char) []byte {
	e := scoreWireExtension(ch)
	b := Build(OpUpdateEtc, id, 48)
	// O 7.54 insere um DWORD Hold antes de EXP e o patch nativo zera esse
	// campo. CP nao faz parte do UpdateEtc: ele viaja no byte MobName[12] do
	// CreateMob. Colocar CP aqui fazia o client interpretar, por exemplo, 75
	// como EXP retida e contaminava a barra/progressao.
	putU32(b, 12, 0)
	putU32(b, 16, ch.Exp)
	putU32(b, 20, ch.LearnedSkill)
	putU16(b, 24, compatibilityU16(e.StatusPts))
	putU16(b, 26, compatibilityU16(e.MasterPts))
	putU16(b, 28, compatibilityU16(e.SkillPts))
	putU16(b, 30, compatibilityU16(e.MagicAmp))
	putU32(b, 32, ch.Gold)
	putU32(b, 36, e.StatusPts)
	putU32(b, 40, e.MasterPts)
	putU32(b, 44, e.SkillPts)
	return b
}

// SetShortSkill confirma os quatro atalhos primarios + 16 atalhos secundarios.
func SetShortSkill(id uint16, skills [20]byte) []byte {
	b := Build(OpSetShortSkill, id, 32)
	copy(b[12:32], skills[:])
	return b
}

// SysQuit confirma ao client que ele pode encerrar a cena/conexao.
func SysQuit(id uint16) []byte {
	return Build(OpSysQuit, id, 16)
}

// UpdateCarry monta o 0x185 (528B): Item[64]@12 + Coin@524. E o inventario
// INTERATIVO (sem ele o client nao deixa mover/equipar itens).
func UpdateCarry(id uint16, inv []model.Item, coin uint32) []byte {
	b := Build(OpUpdateCarry, id, 528)
	for i := range inv {
		PutItem(b, 12+i*8, inv[i])
	}
	putU32(b, 524, coin)
	return b
}

// SelfEquip monta o 0x36B (60B): 16 WORDs visuais @12 e 16 AnctCode @44.
// O segundo bloco nao e opcional no protocolo 7.48: SetColorItem le dele a
// tintura/cor antiga, enquanto os WORDs carregam mesh e nivel visual.
func SelfEquip(id uint16, equip []model.Item) []byte {
	mesh := make([]uint16, len(equip))
	anct := make([]byte, len(equip))
	for i := range equip {
		mesh[i] = model.VisualItemCode(equip[i], model.IsMount(equip[i].Index))
		if !model.IsMount(equip[i].Index) {
			anct[i] = model.AncientCode(equip[i])
		}
	}
	return VisualEquip(id, mesh, anct)
}

// VisualEquip atualiza os 16 modelos sem alterar o equipamento autoritativo;
// transformacoes BM usam um rosto virtual enquanto o Affect 16 estiver ativo.
func VisualEquip(id uint16, mesh []uint16, anct []byte) []byte {
	b := Build(OpUpdateEquip, id, 60)
	for i := 0; i < 16 && i < len(mesh); i++ {
		putU16(b, 12+i*2, mesh[i])
	}
	n := len(anct)
	if n > 16 {
		n = 16
	}
	copy(b[44:60], anct[:n])
	return b
}

// ActionStop monta um Action_Stop (0x366, 52B): coloca o mob parado em pe na
// posicao. PosXY@12, F1@16=2, F2@20=1, TargetXY@24 = PosXY (parado).
func ActionStop(id, x, y uint16) []byte {
	b := Build(OpAction, id, 52)
	putU16(b, 12, x)
	putU16(b, 14, y)
	putU32(b, 16, 2)
	putU32(b, 20, 1)
	putU16(b, 24, x)
	putU16(b, 26, y)
	return b
}

// PlayerMove produz o Action 7.48 que anima outro jogador. A rota so pode vir
// depois de validada pelo servidor: o client remoto usa esses passos para
// reproduzir curvas e desniveis sem recalcular um caminho diferente.
func PlayerMove(id, fromX, fromY, toX, toY uint16, speed uint32, route []byte) []byte {
	b := Build(OpAction, id, 52)
	if speed < 1 {
		speed = 1
	} else if speed > 6 {
		speed = 6
	}
	putU16(b, 12, fromX)
	putU16(b, 14, fromY)
	putU32(b, 16, speed)
	putU32(b, 20, 0)
	putU16(b, 24, toX)
	putU16(b, 26, toY)
	// Normalize a string de rota: bytes depois do primeiro NUL nao fizeram
	// parte da validacao e nao devem atravessar a fronteira server -> client.
	for i, step := range route {
		if i >= 24 || step == 0 {
			break
		}
		b[28+i] = step
	}
	return b
}

// MobMove replica GetAction do TMSrv para caminhada normal: rota vazia,
// Effect=0 e velocidade server-side. O client reconstrói e interpola o trecho
// ate TargetXY; um Action por tile reiniciaria o relogio da animacao.
func MobMove(id, fromX, fromY, toX, toY uint16, speed uint32) []byte {
	b := Build(OpAction, id, 52)
	putU16(b, 12, fromX)
	putU16(b, 14, fromY)
	if speed < 1 {
		speed = 1
	} else if speed > 4 {
		speed = 4
	}
	// Wire 7.48 (confirmado no dump): Speed@16, Effect@20. A struct 7.59 tem
	// estes dois campos na ordem oposta.
	putU32(b, 16, speed)
	putU32(b, 20, 0) // movimento normal
	putU16(b, 24, toX)
	putU16(b, 26, toY)
	return b
}

// IllusionMove confirma o salto da HT. O client 7.48 exige MSG_Action2
// (0x368) com Effect=6 para executar a animacao, camera e consumo visual de MP.
func IllusionMove(id, fromX, fromY, toX, toY uint16, speed uint32) []byte {
	b := Build(OpIllusion, id, 52)
	putU16(b, 12, fromX)
	putU16(b, 14, fromY)
	if speed < 1 {
		speed = 1
	} else if speed > 6 {
		speed = 6
	}
	putU32(b, 16, speed)
	putU32(b, 20, 6)
	putU16(b, 24, toX)
	putU16(b, 26, toY)
	return b
}

// AttackHit reproduz o p39D compacto JA CONVERTIDO pelo PacketProtocolV754.
// O DLL move o DWORD Target/Damage de @40 para @44 antes de chegar ao client.
func AttackHit(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32) []byte {
	b := SkillHit(attackerID, targetID, attackerX, attackerY, targetX, targetY,
		damage, targetMaxHP, currentExp, currentMP, -1, 5, 0)
	// Melee nao possui um segundo pacote de animacao gerado localmente. Com
	// FlagLocal=1 o OnPacketAttack ignora justamente o caminho que cria o dano
	// flutuante. Skills mantem o flag no pacote principal e usam o 0x39D wide
	// adicional; o ataque fisico precisa passar diretamente com zero.
	b[30] = 0
	return b
}

// SendItem monta o 0x182 (24B): DestType@12, DestPos@14, Item(8)@16. Confirma
// UM slot (o client so aplica o swap quando recebe este pacote).
func SendItem(id uint16, placeType, pos byte, it model.Item) []byte {
	b := Build(OpSendItem, id, 24)
	putU16(b, 12, uint16(placeType))
	putU16(b, 14, uint16(pos))
	PutItem(b, 16, it)
	return b
}

// AttackHitExtended conserva o MSG_AttackOne 7.48 nos primeiros 48 bytes e
// anexa o dano REAL em uint32 (a cauda DMGX, que o client patcheado le para o
// texto flutuante).
//
// O WORD legado leva o dano PROJETADO na escala do alvo, nao o dano cru: o
// client subtrai esse WORD do CurHP do prefixo nativo, que ja esta escalado.
// Os dois valores sao propositalmente diferentes -- um alimenta o numero na
// tela, o outro a barra.
func AttackHitExtended(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32) []byte {
	b := AttackHit(attackerID, targetID, attackerX, attackerY, targetX, targetY,
		damage, targetMaxHP, currentExp, currentMP)
	extended := make([]byte, 52)
	copy(extended, b)
	putU16(extended, 0, uint16(len(extended)))
	putU32(extended, 48, damage)
	return extended
}

// CNFGetItem monta o 0x171 (28B): confirma pegar item do chao.
// O TMSrv nativo envia ID=SceneField, DestType@12, DestPos@16 e Item@20 zerado;
// o slot real chega logo depois pelo 0x182 SendItem.
func CNFGetItem(destType, destPos uint32) []byte {
	b := Build(OpCNFGetItem, SceneField, 28)
	putU32(b, 12, destType)
	putU32(b, 16, destPos)
	return b
}

// WarInfo monta o 0x3A8 (24B, payload zerado, ID=SceneField).
func WarInfo() []byte {
	return Build(OpWarInfo, SceneField, 24)
}

// RemoveMob monta o 0x165 (16B): remove/mata o mob de ID=id. RemoveType@12:
// 0 = some sem animacao (saiu do alcance), 1 = MORRE (client faz Hp=0 + Die(),
// anima a morte e remove depois). ID do header = o mob a remover.
func RemoveMob(id uint16, removeType uint32) []byte {
	b := Build(OpRemoveMob, id, 16)
	putU32(b, 12, removeType)
	return b
}

// CNFMobKill usa o wire final 7.48 produzido por FixSendExp_SERVER do
// PacketProtocolV754: FakeExp@12, Killed/Killer@16, Exp uint32@20.
func CNFMobKill(killedID, killerID uint16, exp uint32) []byte {
	b := Build(OpCNFMobKill, SceneField, 24)
	putU32(b, 12, 0)
	putU16(b, 16, killedID)
	putU16(b, 18, killerID)
	putU32(b, 20, exp)
	return b
}

// ShopType do 0x17C (confirmado no client, OnPacketShopList): 1=loja normal de
// gold (SetVisibleShop), 3=mestre de skill, 4=donate. Sem o valor certo o client
// ignora o pacote e a janela nao abre.
const ShopNormal = 1
const ShopSkill = 3

// ShopList monta o 0x17C (532B) -- ABRE a loja no client. ShopType@12,
// List[64] (STRUCT_ITEM 8B)@16, Tax@528. ID do header = SceneField (0x7530),
// igual ao SendShopList nativo. O client le os 27 primeiros num grid de 5 colunas
// e o preco vem do itemlist DELE. A lista e LINEAR: o client compra mandando 0x379
// com sellSlot = o indice do item aqui, que o servidor le direto de Vende[sellSlot].
func ShopList(items []model.Item, tax, shopType uint32) []byte {
	b := Build(OpShopList, SceneField, 532)
	putU32(b, 12, shopType)
	for i := 0; i < 64 && i < len(items); i++ {
		PutItem(b, 16+i*8, items[i])
	}
	putU32(b, 528, tax)
	return b
}

// SkillTarget carrega o dano REAL e o MaxHP do alvo. O WORD que vai no pacote
// e derivado dos dois: o client subtrai esse WORD do CurHP ja projetado, entao
// mandar o dano cru fazia a barra cair na escala errada.
//
// MaxHP zero significa "sem projecao" (escala 1) -- e o que vale para alvo
// cujo MaxHP cabe no prefixo nativo.
type SkillTarget struct {
	ID     uint16
	Damage uint32
	// Heal e cura: o protocolo a representa como short NEGATIVO no mesmo campo
	// do dano, e e isso que faz o 7.48 subir o HP e mostrar "+ valor". Campo
	// proprio para o sinal ser explicito -- antes ele vinha embutido num uint16
	// ja convertido, e a projecao de escala teria destruido a representacao.
	Heal  uint32
	MaxHP uint32
}

// wireDamage e a unica conversao de variacao de HP -> WORD do pacote. Toda
// saturacao passa por aqui; antes o melee saturava em 65535 e a skill em 32767,
// no MESMO campo.
//
// Cura sai como short negativo, com a MESMA projecao de escala do dano: o
// client soma esse WORD ao CurHP ja projetado, entao os dois lados precisam
// falar na mesma unidade.
func wireDamage(t SkillTarget) uint16 {
	if t.Heal > 0 {
		return uint16(-int16(model.ProjectHPDelta(t.Heal, t.MaxHP)))
	}
	return model.ProjectHPDelta(t.Damage, t.MaxHP)
}

// SkillHits usa a familia compacta p39D/p39E/p367 do 7.48. O opcode e o
// tamanho dependem do MaxTarget da skill, mesmo quando ha somente um alvo vivo.
func SkillHits(attackerID, attackerX, attackerY, targetX, targetY uint16,
	currentExp, currentMP uint32, skill int16, motion, mastery byte,
	maxTargets int, targets []SkillTarget) []byte {
	opcode, size, capacity := uint16(OpAttackOne), 48, 1
	if maxTargets == 2 {
		opcode, size, capacity = OpAttackTwo, 52, 2
	} else if maxTargets > 2 {
		opcode, size, capacity = OpAttackMulti, 96, 13
	}
	if len(targets) > capacity {
		targets = targets[:capacity]
	}
	b := Build(opcode, SceneField, size)
	putU16(b, 12, attackerID)
	putU16(b, 14, uint16(len(targets)))
	putU16(b, 16, attackerX)
	putU16(b, 18, attackerY)
	putU16(b, 20, targetX)
	putU16(b, 22, targetY)
	putU16(b, 24, uint16(skill))
	// O 7.48 guarda CurrentMp como WORD no ataque; o valor wide pertence ao
	// UpdateScore/SetHpMp estendido, nunca a este pacote compacto. Nunca deixa o
	// valor wide dar wrap nesse canal (satura em 30000).
	wireMP := compatibilityU16(currentMP)
	putU16(b, 26, wireMP)
	// FlagLocal=1 impede TMFieldScene::OnPacketAttack de sobrescrever o MP do
	// proprio atacante a partir desse WORD legado. Observadores continuam no
	// caminho normal (o atacante nao e seu m_pMyHuman) e recebem a animacao.
	b[28], b[29], b[30], b[31] = motion, mastery, 1, 0
	putU32(b, 32, currentExp)
	putU16(b, 36, wireMP)
	putU32(b, 40, 0)
	for i, target := range targets {
		putU16(b, 44+i*4, target.ID)
		// Cada alvo tem a SUA escala: num golpe em area, dois alvos com MaxHP
		// diferente precisam de WORDs diferentes para o mesmo dano.
		putU16(b, 46+i*4, wireDamage(target))
	}
	return b
}

// SkillHit e o atalho para skills de alvo unico.
func SkillHit(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32, skill int16, motion, mastery byte) []byte {
	return SkillHits(attackerID, attackerX, attackerY, targetX, targetY,
		currentExp, currentMP, skill, motion, mastery, 1,
		[]SkillTarget{{ID: targetID, Damage: damage, MaxHP: targetMaxHP}})
}

// SpectralVisual marca DoubleCritical bit 3. O client usa esse bit para
// renderizar o efeito da Forca Espectral na arma durante o ataque.
func SpectralVisual(packet []byte) []byte {
	if len(packet) > 31 {
		packet[31] |= 0x08
	}
	return packet
}

// SkillHitExtended usa o mesmo 0x39D estendido ja comprovado pelo melee, mas
// conserva Skill/Motion/Mastery. Assim o client escolhe cor/efeito magico e le
// o numero uint32 @48 sem alterar o tamanho fixo do 0x36C multi-alvo.
func SkillHitExtended(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32, skill int16, motion, mastery byte) []byte {
	b := SkillHit(attackerID, targetID, attackerX, attackerY, targetX, targetY,
		damage, targetMaxHP, currentExp, currentMP, skill, motion, mastery)
	extended := make([]byte, 52)
	copy(extended, b)
	putU16(extended, 0, uint16(len(extended)))
	putU32(extended, 48, damage)
	// Este 0x39D adicional existe para exibir o dano uint32. Ao contrario do
	// 0x36C principal, ele precisa passar pelo OnPacketAttack local para o
	// client criar o numero flutuante; FlagLocal=1 o suprime por completo.
	extended[30] = 0
	return extended
}

// CNFDropItem monta o 0x175 (28B): confirmacao de drop de item (remove do inv).
// O TMSrv nativo deixa ID=0. SourType@12, SourPos@16, Rotate@20, GridX@24,
// GridY@26.
func CNFDropItem(srcType, srcPos uint32, rotate uint32, x, y uint16) []byte {
	b := Build(OpCNFDropItem, 0, 28)
	putU32(b, 12, srcType)
	putU32(b, 16, srcPos)
	putU32(b, 20, rotate)
	putU16(b, 24, x)
	putU16(b, 26, y)
	return b
}

// CreateItem monta o 0x26E (32B): aparece um item no chao (SceneField).
// GridX@12, GridY@14, ItemID@16, Item@18, Rotate@26, State@27, Height@28, Create@29, Owner@30.
func CreateItem(gridX, gridY, itemID uint16, it model.Item, rotate, state, height, create byte, owner uint16) []byte {
	b := Build(OpCreateItem, SceneField, 32)
	putU16(b, 12, gridX)
	putU16(b, 14, gridY)
	putU16(b, 16, itemID)
	PutItem(b, 18, it)
	b[26] = rotate
	b[27] = state
	b[28] = height
	b[29] = create
	putU16(b, 30, owner)
	return b
}

// UpdateItem monta o 0x374 (20B): troca o ESTADO de um item ja materializado no
// chao, sem recria-lo. E como o nativo abre um portao -- MSG_UpdateItem tem
// ItemID@12 e State@16, os dois int (Basedef.h:3015).
func UpdateItem(itemID uint16, state uint32) []byte {
	b := Build(OpUpdateItem, SceneField, 20)
	putU32(b, 12, uint32(itemID))
	putU32(b, 16, state)
	return b
}

// RemoveItem monta o 0x16F (16B): remove item do chao. MSG_STANDARDPARM.
func RemoveItem(id uint32) []byte {
	b := Build(OpRemoveItem, SceneField, 16)
	putU32(b, 12, id)
	return b
}
