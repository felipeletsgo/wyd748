package wire

import (
	"encoding/binary"
	"time"

	"wydgo/internal/model"
)

// Build allocates a packet buffer with Type@4 and ID@6 filled in. Size@0 and
// Tick@8 are filled in on send (net.Session.Send -> FinishPacket). Every builder
// below returns the packet in this half-finished state.
func Build(typ, id uint16, size int) []byte {
	b := make([]byte, size)
	binary.LittleEndian.PutUint16(b[4:6], typ)
	binary.LittleEndian.PutUint16(b[6:8], id)
	return b
}

// ResultGamble keeps the exact layout consumed by the 0x1BF dispatcher of the
// 7.48 client. The eight bytes between the stops and the prize are reserved and
// stay zero.
func ResultGamble(id uint16, result [5]byte, stops [3]byte, prize int32, jackpot uint32) []byte {
	b := Build(OpResultGamble, id, 36)
	copy(b[12:17], result[:])
	copy(b[17:20], stops[:])
	putU32(b, 28, uint32(prize))
	putU32(b, 32, jackpot)
	return b
}

// MessageWhisper is also the native 7.48 mail: when message starts with '!',
// TMFieldScene::OnPacketMessageWhisper writes the body to the H panel instead of
// showing it in chat. The 7.48 client uses Name[16]@12, String[96]@28 and
// Color@124, EXACTLY 128 bytes in total (not the 160-byte 7.59 layout).
func MessageWhisper(senderID uint16, from, message string, color uint16) []byte {
	// In 7.48, ID=0 dispatches the ordinary whisper through TMFieldScene. A player
	// ID sends it to TMHuman; that path serves the special prefixes but leaves an
	// ordinary private message without text. The parameter stays explicit because
	// future letters/variants may choose the dispatcher target.
	b := Build(OpMessageWhisper, senderID, 128)
	copy(b[12:28], from)
	copy(b[28:123], message)
	putU32(b, 124, uint32(color))
	return b
}

// MessagePanel builds the native notice shown at the top of the screen for
// about four seconds. The client requires Header.ID=0 to dispatch this packet
// through TMScene instead of associating it with a character.
func MessagePanel(message string) []byte {
	b := Build(OpMessagePanel, 0, 108)
	copy(b[12:107], message)
	return b
}

// GuildChallengePrompt is the native 7.48 header-only request used to open
// the city-war confirmation dialog. The client supplies no fee or city in
// this packet; those values remain server-owned and are checked on 0x28F.
func GuildChallengePrompt() []byte {
	return Build(OpReqChallenge, SceneField, HeaderSize)
}

// DaySync answers the periodic MobName="day" request of the 7.48 client. The
// !# prefix is consumed by TMScene and is not shown as a notice: "11" becomes
// m_nYear and "  2" becomes m_nDays. The fixed value reproduces captured TMSrv 7.54.
func DaySync() []byte {
	return MessagePanel("!#11  2")
}

// Motion builds MSG_Motion (0x36A), used by the server to propagate emotes and
// one-off effects. Fireworks use Motion=100 and Parm=0..5 natively.
func Motion(id uint16, motion, parm uint16) []byte {
	b := Build(OpMotion, id, 20)
	putU16(b, 12, motion)
	putU16(b, 14, parm)
	return b
}

// PremiumFirework publishes the 100 points (10x10) drawn by the player. The
// WYD 7.48 handler hands packet+20 to TMEffectFireWork::SetCustomFireWork, so
// the bitmap must start exactly at that offset.
func PremiumFirework(id uint16, pattern [16]byte) []byte {
	b := Build(OpPremiumFirework, id, 36)
	copy(b[20:36], pattern[:])
	return b
}

// StandardParm builds the 16-byte WYD layout used by several confirmation
// signals: header followed by a DWORD at @12.
func StandardParm(opcode, id uint16, parm uint32) []byte {
	b := Build(opcode, id, 16)
	putU32(b, 12, parm)
	return b
}

// StandardParm2 builds the 20-byte layout used by commands that carry two
// consecutive DWORDs in Parm1@12 and Parm2@16.
func StandardParm2(opcode, id uint16, parm1, parm2 uint32) []byte {
	b := Build(opcode, id, 20)
	putU32(b, 12, parm1)
	putU32(b, 16, parm2)
	return b
}

// CapsuleInfoData holds the fields the 7.48 UI accepts in MSG_CAPSULEINFO. The
// executable copies exactly 13 DWORDs (52 bytes) into a 12-entry cache. This
// version has only two Mastery WORDs; Skill keeps the nine fields shown in
// three tooltip lines.
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

// CNFCapsuleInfo builds the 0xDC3 received directly by WYD.exe 7.48. The
// 76-byte 0xD1F is an internal DB/TMSrv packet of later sources and has no
// handler in this executable. The capsule ID is at @12.
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

// PartyRequest replicates MSG_REQParty of the 7.54 protocol. The old structure
// has 44 bytes and carries Target as a DWORD at @40. Later versions inserted
// another field and moved Target to @44, growing the packet to 48 bytes.
func PartyRequest(leaderID uint16, name string, class byte, level, hp, maxHP uint32, targetID uint16) []byte {
	b := Build(OpPartyRequest, SceneField, 44)
	hp, maxHP = partyDisplayHP(hp, maxHP)
	b[12] = class
	b[13] = 0 // PartyIndex 0 = leader/pending invite
	putU16(b, 14, packetU16(level))
	putU16(b, 16, packetU16(maxHP))
	putU16(b, 18, packetU16(hp))
	putU16(b, 20, leaderID)
	copy(b[22:38], name)
	putU32(b, 40, uint32(targetID))
	return b
}

// PartyMember builds MSG_CNFAddParty/MSG_AddParty (0x37D). PARTY takes 28 bytes
// from @12; the client uses PartyIndex 0 to highlight the leader.
func PartyMember(id uint16, name string, class, partyIndex byte, level, hp, maxHP uint32) []byte {
	b := Build(OpPartyAdd, SceneField, 40)
	hp, maxHP = partyDisplayHP(hp, maxHP)
	b[12] = class
	b[13] = partyIndex
	putU16(b, 14, packetU16(level))
	putU16(b, 16, packetU16(maxHP))
	putU16(b, 18, packetU16(hp))
	putU16(b, 20, id)
	copy(b[22:38], name)
	return b
}

// PartyRemove uses MSG_STANDARDPARM. memberID=0 clears the whole panel; a
// specific ID removes only that member.
func PartyRemove(memberID uint16) []byte {
	b := Build(OpPartyRemove, SceneField, 16)
	putU32(b, 12, uint32(memberID))
	return b
}

// The native PARTY HP fields are signed shorts. TMSrv reduces values above
// 32000 to a percentage/hundreds before building the panel.
func partyDisplayHP(hp, maxHP uint32) (uint32, uint32) {
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

// putU64 is used by the TMProject748 client ABI where EXP remains a native uint64;
// stock 7.48 builders keep their established 32-bit projection.
func putU64(b []byte, off int, v uint64) { binary.LittleEndian.PutUint64(b[off:off+8], v) }

// The CreateMob byte holds the raw native PK, CP+75. The model domain stays
// signed (-75..+75), so a neutral character (CP 0) sends 75. 150 is reserved
// for the positive extreme (+75), not for the normal state.
const NormalNameChaos = 75

func CPNameByte(cp int16) byte {
	return byte(model.ClampCP(int(cp)) + 75)
}

// PutItem writes a STRUCT_ITEM (8B) at b@off: sIndex + 3 effect pairs.
func PutItem(b []byte, off int, it model.Item) {
	putU16(b, off, it.Index)
	// Timed items carry a presentation-only expiry date for the native tooltip.
	effects := model.ExpiryDisplayEffects(it, time.Now())
	copy(b[off+2:off+8], effects[:])
}

// Trade builds MSG_Trade 0x383 (156B) in the 7.48 client layout:
// Item[15]@12, CarryPos[15]@132, padding@147, TradeMoney@148,
// MyCheck@152, padding@153 and OpponentID@154.
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

func CNFTradeCheck(id uint16) []byte {
	return Build(OpCNFTradeCheck, id, 12)
}

// AutoTrade builds MSG_AutoTrade 0x397 (196B): Desc[24]@12, Item[12]@36,
// CarryPos[12]@132, TradeMoney[12]@144, Tax@192 and TargetID@194.
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

// ItemSold uses MSG_STANDARDPARM2: tradeEntityID@12 and listing position@16.
// In ghost shops, tradeEntityID is the clone's virtual ID.
func ItemSold(tradeEntityID uint16, pos uint32) []byte {
	b := Build(OpItemSold, SceneField, 20)
	putU32(b, 12, uint32(tradeEntityID))
	putU32(b, 16, pos)
	return b
}

// UpdateCargoGold synchronizes the account money vault (MSG_STANDARDPARM).
func UpdateCargoGold(id uint16, gold uint32) []byte {
	b := Build(OpUpdateCargoGold, id, 16)
	putU32(b, 12, gold)
	return b
}

// CargoGoldTransfer confirms a deposit/withdrawal. The client applies the same
// value to both counters and the following UpdateCargoGold reaffirms the balance.
func CargoGoldTransfer(op uint16, id uint16, amount uint32) []byte {
	b := Build(op, id, 16)
	putU32(b, 12, amount)
	return b
}
func NewCharacterFail() []byte { return Build(OpNewCharacterFail, 0, 12) }

// CNFCharacterLogout is only an MSG_STANDARD. The ID must be the ClientID of
// the character leaving the world; TMFieldScene switches to SelectChar only
// when it matches the local ID.
func CNFCharacterLogout(id uint16) []byte { return Build(OpCNFCharacterLogout, id, 12) }

// AlreadyPlaying is the native answer that TMSelectServerScene 7.48 handles
// still on the login screen, re-enabling the fields and showing the localized error.
func AlreadyPlaying() []byte { return Build(OpAlreadyPlaying, 0, 12) }
func clampByte(v int) byte {
	if v < 0 {
		return 0
	}
	if v > 255 {
		return 255
	}
	return byte(v)
}

// GuildWireID applies the native ABI range: four channel bits (currently a
// single channel = 0) and twelve index bits. Out-of-range IDs cannot reach the
// wire; the canonical registry already rejects them, and the mask keeps a
// legacy state from corrupting the part reserved for the channel.
func GuildWireID(id uint16) uint16 { return id & 0x0FFF }
func wireScore(ch model.Char) *model.Score {
	if ch.RuntimeScore != nil {
		return ch.RuntimeScore
	}
	if ch.Score != nil {
		return ch.Score
	}
	return &model.Score{Version: model.ScoreVersion}
}
func packetU16(value uint32) uint16 {
	if value > 65_535 {
		return 65_535
	}
	return uint16(value)
}

// Critical Armor keeps canonical affect 31 and its persisted discriminator 24.
// Native 7.48 uses visual 36 for Critical Armor; visual 24 belongs to Samaritan.
// Normalize only the wire projection, including raw legacy SkillData type 50.
// Ordinary affect 31 remains Coin Armor and must not be remapped.
func clientAffectType(affect model.Affect) byte {
	if affect.Type == 50 || (affect.Type == 31 && affect.ClientType == 24) {
		return 36
	}
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

// putAffectWords writes WORDs in the little-endian format observed by the
// client: low byte = remaining time in 8 s units; high byte = visual type.
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

// UpdateEtc builds the exact 36-byte 7.48 MSG_UpdateEtc: Hold@12, EXP@16,
// LearnedSkill@20, Status@24, Mastery@26, SkillPts@28, Magic@30 and Gold@32.
// LearnedSkill@20 is MANDATORY: it is how the client learns the known skills
// (TMHuman::OnPacketUpdateEtc copies LearnedSkill/bonus/coin from it). Removing
// this field erases the client skills and shifts the points (2026-07-14 regression).
func UpdateEtc(id uint16, ch model.Char) []byte {
	b := Build(OpUpdateEtc, id, 36)
	putU32(b, 12, ch.Hold)
	putU32(b, 16, uint32(ch.Exp))
	putU32(b, 20, ch.LearnedSkill)
	score := wireScore(ch)
	// The native packet has WORD counters.  Wide values continue travelling in
	// 0x336, while this incremental projection follows the executable ABI.
	putU16(b, 24, packetU16(score.StatusPts))
	putU16(b, 26, packetU16(score.MasterPts))
	putU16(b, 28, packetU16(score.SkillPts))
	putU16(b, 30, packetU16(score.MagicAmp))
	putU32(b, 32, ch.Gold)
	return b
}

// SetShortSkill confirms the four primary + 16 secondary shortcuts.
func SetShortSkill(id uint16, skills [20]byte) []byte {
	b := Build(OpSetShortSkill, id, 32)
	copy(b[12:32], skills[:])
	return b
}

// SysQuit confirms to the client that it may close the scene/connection.
func SysQuit(id uint16) []byte {
	return Build(OpSysQuit, id, 16)
}

// UpdateCarry builds 0x185 (528B): Item[64]@12 + Coin@524. It is the
// INTERACTIVE inventory (without it the client does not move/equip items).
func UpdateCarry(id uint16, inv []model.Item, coin uint32) []byte {
	b := Build(OpUpdateCarry, id, 528)
	itemCount := len(inv)
	// A 65th element would start exactly at the Coin field; invalid callers cannot
	// overwrite the tail or cause a slice-bounds panic in the encoder.
	if itemCount > model.MaxCarry {
		itemCount = model.MaxCarry
	}
	for i := 0; i < itemCount; i++ {
		PutItem(b, 12+i*8, inv[i])
	}
	putU32(b, 524, coin)
	return b
}

// SelfEquip builds 0x36B (60B): 16 visual WORDs @12 and 16 AnctCode @44. The
// second block is not optional in the 7.48 protocol: SetColorItem reads the old
// dye/color from it, while the WORDs carry mesh and visual level.
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

// VisualEquip updates the 16 models without changing authoritative equipment;
// BM transformations use a virtual face while Affect 16 is active.
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

// ActionStop builds an Action_Stop (0x366, 52B): it stands the mob still at the
// position. PosXY@12, F1@16=2, F2@20=1, TargetXY@24 = PosXY (still).
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

// ActionRouteCorrection is a paired-client extension. Effect 8 corrects only
// the owner's rejected route; unlike ActionStop it must not trigger a warp,
// revival, or teleport effects in the source client.
func ActionRouteCorrection(id, x, y uint16) []byte {
	b := Build(OpAction, id, 52)
	putU16(b, 12, x)
	putU16(b, 14, y)
	putU32(b, 20, 8)
	putU16(b, 24, x)
	putU16(b, 26, y)
	return b
}

// PlayerMove produces the 7.48 Action that animates another player. The route
// may only come after server validation: the remote client uses these steps
// to reproduce curves and slopes without computing a different path.
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
	// Normalize the route string: bytes after the first NUL were not part of the
	// validation and must not cross the server -> client boundary.
	for i, step := range route {
		if i >= 24 || step == 0 {
			break
		}
		b[28+i] = step
	}
	return b
}

// MobMove replicates TMSrv GetAction for normal walking: empty route, Effect=0
// and server-side speed. The client rebuilds and interpolates the segment up
// to TargetXY; one Action per tile would restart the animation clock.
func MobMove(id, fromX, fromY, toX, toY uint16, speed uint32) []byte {
	b := Build(OpAction, id, 52)
	putU16(b, 12, fromX)
	putU16(b, 14, fromY)
	if speed < 1 {
		speed = 1
	} else if speed > 4 {
		speed = 4
	}
	// 7.48 wire (confirmed in the dump): Speed@16, Effect@20. The 7.59 struct has
	// these two fields in the opposite order.
	putU32(b, 16, speed)
	putU32(b, 20, 0) // normal movement
	putU16(b, 24, toX)
	putU16(b, 26, toY)
	return b
}

// IllusionMove confirms the HT jump. The 7.48 client requires MSG_Action2
// (0x368) with Effect=6 to run the animation, camera and visual MP use.
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

// AttackHit reproduces the compact p39D ALREADY CONVERTED by PacketProtocolV754.
// The DLL moves the Target/Damage DWORD from @40 to @44 before it reaches the client.
func AttackHit(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32) []byte {
	b := SkillHit(attackerID, targetID, attackerX, attackerY, targetX, targetY,
		damage, targetMaxHP, currentExp, currentMP, -1, 5, 0)
	// Melee has no second locally generated animation packet. With FlagLocal=1
	// OnPacketAttack skips exactly the path that creates the floating damage.
	// Skills keep the flag in the main packet and use the additional wide 0x39D;
	// the physical attack must pass directly with zero.
	b[30] = 0
	return b
}

// SendItem builds 0x182 (24B): DestType@12, DestPos@14, Item(8)@16. It
// resynchronizes a single slot; the successful swap lifecycle uses 0x376.
func SendItem(id uint16, placeType, pos byte, it model.Item) []byte {
	b := Build(OpSendItem, id, 24)
	putU16(b, 12, uint16(placeType))
	putU16(b, 14, uint16(pos))
	PutItem(b, 16, it)
	return b
}

// SwapItem confirms the same 0x376 structure sent by the client. The native
// handler FUN_00486808 only releases the cursor item and swaps both controls
// when it receives SourType/SourPos/DestType/DestPos back in this packet.
func SwapItem(id uint16, sourType, sourPos, destType, destPos byte, targetID uint16) []byte {
	b := Build(OpSwapItem, id, 20)
	b[12], b[13], b[14], b[15] = sourType, sourPos, destType, destPos
	putU16(b, 16, targetID)
	return b
}

// BuyItem builds the native 0x379 confirmation (24B). It carries the shop and
// Carry positions and the final gold; the client copies the item from the shop grid.
func BuyItem(id, targetID, targetCarryPos, myCarryPos uint16, coin uint32) []byte {
	b := Build(OpBuyItem, id, 24)
	putU16(b, 12, targetID)
	putU16(b, 14, targetCarryPos)
	putU16(b, 16, myCarryPos)
	putU32(b, 20, coin)
	return b
}

// AttackHitWide keeps the 7.48 MSG_AttackOne in the first 48 bytes and appends
// the REAL damage as uint32 (the DMGX tail, which the rebuilt client reads for
// the floating text).
//
// The legacy WORD carries the PROJECTED damage in the target's scale, not the
// raw damage: the client subtracts that WORD from the native prefix CurHP,
// which is already scaled. The two values are deliberately different -- one
// feeds the on-screen number, the other the bar.
func AttackHitWide(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32) []byte {
	return AttackHitWideResult(attackerID, targetID, attackerX, attackerY, targetX, targetY,
		damage, targetMaxHP, currentExp, currentMP, 0, false)
}

// AttackHitWideResult adds the native Double/Critical visual bits and the
// signed -3 MISS sentinel without trusting any client-provided combat result.
func AttackHitWideResult(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32, doubleCritical byte, miss bool) []byte {
	b := AttackHit(attackerID, targetID, attackerX, attackerY, targetX, targetY,
		damage, targetMaxHP, currentExp, currentMP)
	b[31] = doubleCritical
	if miss {
		putU16(b, 46, 0xFFFD)
	}
	extended := make([]byte, 52)
	copy(extended, b)
	putU16(extended, 0, uint16(len(extended)))
	if !miss {
		putU32(extended, 48, damage)
	}
	return extended
}

// AttackHitWideFlankResult serializes the native Huntress Air Blade shape:
// MSG_AttackTwo, one real target in Dam[0], bonus-only Dam[1] with TargetID=0,
// and DoubleCritical bit 2. The DMGX tail preserves both uint32 values because
// STRUCT_DAM.Damage is a signed short in the 7.48 client.
func AttackHitWideFlankResult(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32, doubleCritical byte, flank uint32) []byte {
	if flank == 0 {
		return AttackHitWideResult(attackerID, targetID, attackerX, attackerY, targetX, targetY,
			damage, targetMaxHP, currentExp, currentMP, doubleCritical&^4, false)
	}
	base := AttackHit(attackerID, targetID, attackerX, attackerY, targetX, targetY,
		damage, targetMaxHP, currentExp, currentMP)
	native := make([]byte, 52)
	copy(native, base)
	putU16(native, 0, uint16(len(native)))
	putU16(native, 4, OpAttackTwo)
	native[31] = doubleCritical | 4
	putU16(native, 48, 0)
	putU16(native, 50, wireDamage(SkillTarget{Damage: flank}))

	extended := make([]byte, 68)
	copy(extended, native)
	putU16(extended, 0, uint16(len(extended)))
	putU32(extended, 52, 0x58474D44) // "DMGX"
	putU32(extended, 56, 2)
	putU32(extended, 60, damage)
	putU32(extended, 64, flank)
	return extended
}

// AttackHitsWideTwoResult serializes a real two-target physical MSG_AttackTwo.
// It reuses the same native layout/DMGX contract as multi-target skills, but
// keeps SkillIndex=-1 and the physical motion so OnPacketAttack renders one
// attack action with two authoritative damage slots.
func AttackHitsWideTwoResult(attackerID, attackerX, attackerY, targetX, targetY uint16,
	currentExp, currentMP uint32, doubleCritical byte, targets []SkillTarget) []byte {
	b := SkillHits(attackerID, attackerX, attackerY, targetX, targetY,
		currentExp, currentMP, -1, 5, 0, 2, targets)
	if len(b) > 31 {
		b[31] = doubleCritical
	}
	return b
}

// CNFGetItem builds 0x171 (28B): confirms picking up a ground item. Native
// TMSrv sends ID=SceneField, DestType@12, DestPos@16 and a zeroed Item@20; the
// real slot arrives right after through the 0x182 SendItem.
func CNFGetItem(destType, destPos uint32) []byte {
	b := Build(OpCNFGetItem, SceneField, 28)
	putU32(b, 12, destType)
	putU32(b, 16, destPos)
	return b
}

// WarInfo builds the complete native 0x3A8 form: three DWORDs in 24 bytes, with
// ID=SceneField. Zero means no war, clan or allied guild; the active peer always
// sends the complete snapshot to update all three states.
func WarInfo() []byte {
	return Build(OpWarInfo, SceneField, 24)
}

// GuildWarInfo uses the already verified complete native 0x3A8 envelope.
func GuildWarInfo(enemy uint16) []byte {
	pkt := WarInfo()
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(enemy))
	return pkt
}

// RemoveMob builds 0x165 (16B): removes/kills mob ID=id. RemoveType@12:
// 0 = vanishes without animation (left range), 1 = DIES (client sets Hp=0 +
// Die(), animates the death and removes it later). Header ID = mob to remove.
func RemoveMob(id uint16, removeType uint32) []byte {
	b := Build(OpRemoveMob, id, 16)
	putU32(b, 12, removeType)
	return b
}

// CNFMobKill uses the final 7.48 wire produced by FixSendExp_SERVER of
// PacketProtocolV754: Hold/FakeExp@12, Killed/Killer@16, Exp uint32@20.
func CNFMobKill(killedID, killerID uint16, hold, exp uint32) []byte {
	b := Build(OpCNFMobKill, SceneField, 24)
	putU32(b, 12, hold)
	putU16(b, 16, killedID)
	putU16(b, 18, killerID)
	putU32(b, 20, exp)
	return b
}

// ShopType of 0x17C (confirmed in the client, OnPacketShopList): 1=normal gold
// shop (SetVisibleShop), 3=skill master, 4=donate. Without the right value the
// client ignores the packet and the window does not open.
const ShopNormal = 1
const ShopSkill = 3

// SkillTarget carries the REAL damage and the target MaxHP. The WORD in the
// packet derives from both: the client subtracts it from the already projected
// CurHP, so sending raw damage made the bar drop in the wrong scale.
//
// Zero MaxHP means "no projection" (scale 1) -- used for targets whose MaxHP
// fits in the native prefix.
type SkillTarget struct {
	ID     uint16
	Damage uint32
	// Miss serializes the native signed short -3 visual. Damage remains zero
	// in the wide tail because MISS is an animation result, not HP damage.
	Miss bool
	// Heal is healing: the protocol represents it as a NEGATIVE short in the same
	// field as damage, which makes 7.48 raise HP and show "+ value". A separate
	// field keeps the sign explicit -- it used to be embedded in an already
	// converted uint16, and the scale projection would have destroyed it.
	Heal  uint32
	MaxHP uint32
}

// wireDamage is the only HP-change -> packet WORD conversion. All saturation
// happens here; melee used to saturate at 65535 and skills at 32767 in the
// SAME field.
//
// Healing leaves as a negative short with the SAME scale projection as damage:
// the client adds that WORD to the already projected CurHP, so both sides must
// speak the same unit.
func wireDamage(t SkillTarget) uint16 {
	if t.Miss {
		return 0xFFFD
	}
	if t.Heal > 0 {
		v := t.Heal
		if v > 32767 {
			v = 32767
		}
		return uint16(-int16(v))
	}
	v := t.Damage
	if v > 32767 {
		v = 32767
	}
	return uint16(v)
}

// SkillHits uses the compact 7.48 p39D/p39E/p367 family. Opcode and size depend
// on the skill's MaxTarget, even when only one living target remains.
func buildSkillHitsPacket(attackerID, attackerX, attackerY, targetX, targetY uint16,
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
	// 7.48 keeps CurrentMp as a WORD in the attack; the wide value belongs to the
	// extended UpdateScore/SetHpMp, never to this compact packet. Never let the
	// wide value wrap in this channel (saturates at 30000).
	wireMP := packetU16(currentMP)
	putU16(b, 26, wireMP)
	// FlagLocal=1 keeps TMFieldScene::OnPacketAttack from overwriting the attacker's
	// own MP from this legacy WORD. Observers stay on the normal path (the attacker
	// is not their m_pMyHuman) and receive the animation.
	b[28], b[29], b[30], b[31] = motion, mastery, 1, 0
	putU32(b, 32, currentExp)
	putU16(b, 36, wireMP)
	putU32(b, 40, 0)
	for i, target := range targets {
		putU16(b, 44+i*4, target.ID)
		// Each target has ITS OWN scale: in an area hit, two targets with different
		// MaxHP need different WORDs for the same damage.
		putU16(b, 46+i*4, wireDamage(target))
	}
	return b
}

// SkillHit is the shortcut for single-target skills.
func SkillHit(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32, skill int16, motion, mastery byte) []byte {
	return buildSkillHitsPacket(attackerID, attackerX, attackerY, targetX, targetY,
		currentExp, currentMP, skill, motion, mastery, 1,
		[]SkillTarget{{ID: targetID, Damage: damage, MaxHP: targetMaxHP}})
}

// AttackerHold writes the attacker's Hold into FakeExp (@40). The client reads
// FakeExp for its own attacks and replaces the displayed Hold with it.
func AttackerHold(packet []byte, hold uint32) []byte {
	if len(packet) >= 44 {
		putU32(packet, 40, hold)
	}
	return packet
}

// SpectralVisual sets DoubleCritical bit 3. The client uses that bit to render
// the Spectral Force effect on the weapon during the attack.
func SpectralVisual(packet []byte) []byte {
	if len(packet) > 31 {
		packet[31] |= 0x08
	}
	return packet
}

// SkillHitExtended is the single-target skill-wide adapter. Physical hits own
// the compact 0x39D/52 contract; skills use the self-describing 0x39D/60 DMGX
// tail so the source-built 7.48 client cannot confuse the two result families.
func SkillHitExtended(attackerID, targetID, attackerX, attackerY, targetX, targetY uint16,
	damage, targetMaxHP, currentExp, currentMP uint32, skill int16, motion, mastery byte) []byte {
	return SkillHits(attackerID, attackerX, attackerY, targetX, targetY,
		currentExp, currentMP, skill, motion, mastery, 1,
		[]SkillTarget{{ID: targetID, Damage: damage, MaxHP: targetMaxHP}})
}

// CNFDropItem builds 0x175 (28B): item drop confirmation (removes from inv).
// Native TMSrv leaves ID=0. SourType@12, SourPos@16, Rotate@20, GridX@24,
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

// CreateItem builds 0x26E (32B): an item appears on the ground (SceneField).
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

// UpdateItem builds 0x374 (20B): changes the state of a materialized item.
// 7.48 consumes ItemID i32@12, State i16@16, Height i8@18 and reserved@19.
func UpdateItem(itemID uint16, state uint16) []byte {
	b := Build(OpUpdateItem, SceneField, 20)
	putU32(b, 12, uint32(itemID))
	putU16(b, 16, state)
	return b
}

// RemoveItem builds 0x16F (16B): removes a ground item. MSG_STANDARDPARM.
func RemoveItem(id uint32) []byte {
	b := Build(OpRemoveItem, SceneField, 16)
	putU32(b, 12, id)
	return b
}
