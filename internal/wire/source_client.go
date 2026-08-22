package wire

import (
	"encoding/binary"
	"time"

	"wydgo/internal/model"
)

// ClientProtocol identifies only the serialization ABI spoken by a session.
// It never changes gameplay rules or makes client-provided score fields
// authoritative. Zero deliberately means the already deployed stock client.
type ClientProtocol uint32

const (
	ClientProtocolStock748 ClientProtocol = iota
	ClientProtocolSource748
)

const (
	// SourceClientProtocolMarker is carried at login offset 44, in the legacy
	// DBNeedSave field. "SRC2" is deliberately outside the stock zero/one
	// domain, so old WYD.exe builds keep selecting the stock packet family.
	SourceClientProtocolMarker uint32 = 0x32435253

	// SourceOpCharList is the opcode consumed by TMProject's source selection
	// scene. The stock binary continues to receive OpCharList (0x10E).
	SourceOpCharList uint16 = 0x10A
)

// ClientProtocolFromLogin detects the source-client marker without trusting
// any other login payload field. Invalid or short packets remain stock and are
// rejected by the normal exact-size/phase validation before authentication.
func ClientProtocolFromLogin(packet []byte) ClientProtocol {
	if len(packet) >= 48 && binary.LittleEndian.Uint32(packet[44:48]) == SourceClientProtocolMarker {
		return ClientProtocolSource748
	}
	return ClientProtocolStock748
}

// putSourceSelChar writes TMProject's 904-byte STRUCT_SELCHAR. Its 48-byte
// score and uint64 EXP are source-client presentation fields; ExtendedScore
// and the account model remain the only server-authoritative state.
func putSourceSelChar(dst []byte, offset int, chars []model.Char) {
	for slot := 0; slot < 4 && slot < len(chars); slot++ {
		ch := chars[slot]
		if ch.Name == "" {
			continue
		}
		putU16(dst, offset+slot*2, ch.X)
		putU16(dst, offset+8+slot*2, ch.Y)
		copy(dst[offset+16+slot*16:offset+32+slot*16], ch.Name)
		score := EncodeClientScore48(scoreWireExtension(ch))
		copy(dst[offset+80+slot*ClientScore48Size:], score[:])
		for equipSlot, item := range ch.Equip {
			// TMProject keeps 18 source-side equipment entries. The server model
			// has the canonical 16 slots, so the two trailing entries remain zero.
			PutItem(dst, offset+272+(slot*18+equipSlot)*8, item)
		}
		putU16(dst, offset+848+slot*2, GuildWireID(ch.GuildID))
		putU32(dst, offset+856+slot*4, ch.Gold)
		putU64(dst, offset+872+slot*8, uint64(ch.Exp))
	}
}

// SourceCharList builds the exact 1992-byte MSG_CNFAccountLogin expected by
// the source client: Header, alignment padding, SecretCode, 904-byte SelChar,
// Cargo, cargo coin and account identity. SecretCode stays zero because the Go
// transport authenticates each frame from its own header keyword/checksum.
func SourceCharList(accName string, chars []model.Char, cargo []model.Item, cargoGold uint32) []byte {
	const (
		packetSize = 1992
		selOffset  = 32
		cargoOff   = 936
	)
	b := Build(SourceOpCharList, SceneCharList, packetSize)
	putSourceSelChar(b, selOffset, chars)
	for i := 0; i < len(cargo) && i < model.MaxCargo; i++ {
		PutItem(b, cargoOff+i*8, cargo[i])
	}
	putU32(b, 1960, cargoGold)
	copy(b[1964:1980], accName)
	return b
}

// CharacterListForProtocol is the single selection boundary used by game
// handlers. Keeping the switch here prevents features from accidentally
// sending a stock score layout to the source client after a later mutation.
func CharacterListForProtocol(protocol ClientProtocol, accName string, chars []model.Char, cargo []model.Item, cargoGold uint32) []byte {
	if protocol == ClientProtocolSource748 {
		return SourceCharList(accName, chars, cargo, cargoGold)
	}
	return CharList(accName, chars, cargo, cargoGold)
}

// CharacterSelectionUpdateForProtocol rebuilds the entire selection aggregate
// after create/delete/evolution. TMProject aligns STRUCT_SELCHAR at byte 16;
// the stock client keeps its compact 744-byte value at byte 12.
func CharacterSelectionUpdateForProtocol(protocol ClientProtocol, opcode, id uint16, chars []model.Char) []byte {
	if protocol != ClientProtocolSource748 {
		if opcode == OpCNFDeleteCharacter {
			return CNFDeleteCharacter(id, chars)
		}
		return CNFNewCharacter(id, chars)
	}
	b := Build(opcode, id, 920)
	putSourceSelChar(b, 16, chars)
	return b
}

// putSourceAffects stores the full Type/Level/Value/Time representation used
// by TMProject. Only the 16 authoritative model slots are populated; the
// source-only trailing slots remain zero instead of inventing new affects.
func putSourceAffects(dst []byte, offset int, affects []model.Affect, now time.Time) {
	for i := 0; i < len(affects) && i < 32; i++ {
		affect := affects[i]
		units := affectTimeUnits(affect.ExpiresAt, now)
		if affect.Type == 0 || units == 0 {
			continue
		}
		// TMProject names this layout Type/Level/Value/Time, unlike the
		// stock 7.48 Type/Value/Level/Time order. Preserve that source ABI
		// here instead of changing the authoritative Affect model.
		dst[offset+i*8] = clientAffectType(affect)
		dst[offset+i*8+1] = clampByte(affect.Level)
		value := affect.Value
		// STRUCT_AFFECT carries Value as a signed 16-bit field in TMProject;
		// saturating avoids wrapping a valid server-side calculation on wire.
		if value < -32768 {
			value = -32768
		} else if value > 32767 {
			value = 32767
		}
		putU16(dst, offset+i*8+2, uint16(int16(value)))
		putU32(dst, offset+i*8+4, units)
	}
}

// SourceEnterWorld serializes the 1728-byte MSG_CNFCharacterLogin produced by
// the Win32 compiler. Padding and the two source-only equipment slots are
// deliberately zero. This is a projection from Char, never a model copy-back.
func SourceEnterWorld(id, slot uint16, ch model.Char) []byte {
	b := Build(OpEnterWorld, id, 1728)
	// MSG_CNFCharacterLogin places Ext1 immediately after ShortSkill[16].
	// Keeping these offsets named prevents the 32-byte Ext1.Data prefix from
	// being mistaken for padding and shifting every source affect by two bytes.
	const (
		sourceShortSkillOffset = 1062
		sourceExt1Offset       = sourceShortSkillOffset + 16
		sourceExt1AffectOffset = sourceExt1Offset + 8*4
	)
	putU16(b, 12, ch.X)
	putU16(b, 14, ch.Y)
	const mob = 16
	copy(b[mob:mob+16], ch.Name)
	b[mob+12] = CPNameByte(ch.CP)
	putU16(b, mob+18, GuildWireID(ch.GuildID))
	b[mob+20] = ch.Class
	putU32(b, mob+24, ch.Gold)
	putU64(b, mob+32, uint64(ch.Exp))
	putU16(b, mob+40, ch.X)
	putU16(b, mob+42, ch.Y)
	baseScore := EncodeClientScore48(ch.Extended)
	runtimeScore := EncodeClientScore48(scoreWireExtension(ch))
	copy(b[mob+44:mob+92], baseScore[:])
	copy(b[mob+92:mob+140], runtimeScore[:])
	for i, item := range ch.Equip {
		PutItem(b, mob+140+i*8, item)
	}
	for i, item := range ch.Inv {
		PutItem(b, mob+284+i*8, item)
	}
	putU32(b, mob+796, ch.LearnedSkill)
	putU32(b, mob+800, ch.SecondaryLearnedSkill)
	ext := scoreWireExtension(ch)
	putU16(b, mob+804, compatibilityU16(ext.StatusPts))
	putU16(b, mob+806, compatibilityU16(ext.MasterPts))
	putU16(b, mob+808, compatibilityU16(ext.SkillPts))
	b[mob+810] = clampByte(int(ext.Critical))
	b[mob+811] = clampByte(int(ext.SaveMana))
	copy(b[mob+812:mob+816], ch.ShortSkill[:4])
	b[mob+816] = ch.GuildRank
	b[mob+817] = clampByte(int(ext.MagicAmp))
	b[mob+818] = clampByte(int(ext.RegenHP))
	b[mob+819] = clampByte(int(ext.RegenMP))
	b[mob+820] = clampByte(int(ext.ResistFire))
	b[mob+821] = clampByte(int(ext.ResistIce))
	b[mob+822] = clampByte(int(ext.ResistHoly))
	b[mob+823] = clampByte(int(ext.ResistThunder))
	// MSG_CNFCharacterLogin carries the selected account slot separately from
	// ClientID. Sending the spawn marker here made every source client appear to
	// have selected slot two even when the account selected another character.
	putU16(b, 1056, slot)
	putU16(b, 1058, id)
	copy(b[sourceShortSkillOffset:sourceShortSkillOffset+16], ch.ShortSkill[:16])
	// The first affect is Ext1.Affect[0] at 1110 (not 1112): Ext1.Data is
	// eight DWORDs and is part of the native 0x114 packet, not optional padding.
	putSourceAffects(b, sourceExt1AffectOffset, ch.Affects[:], time.Now())
	return b
}

// EnterWorldForProtocol selects the source or stock character aggregate while
// keeping the call site independent from the packet ABI.
func EnterWorldForProtocol(protocol ClientProtocol, id, slot uint16, ch model.Char) []byte {
	if protocol == ClientProtocolSource748 {
		return SourceEnterWorld(id, slot, ch)
	}
	return EnterWorld(id, ch)
}

// SourceUpdateScore maps the authoritative runtime score to TMProject's
// 152-byte 0x336 structure. Fields absent from that native structure continue
// to be synchronized by dedicated packets/extensions rather than trusted back.
func SourceUpdateScore(id uint16, ch model.Char) []byte {
	b := Build(OpUpdateScore, id, 152)
	ext := scoreWireExtension(ch)
	score := EncodeClientScore48(ext)
	copy(b[12:60], score[:])
	b[60] = clampByte(int(ext.Critical))
	b[61] = clampByte(int(ext.SaveMana))
	putAffectWords(b, 62, ch.Affects[:], time.Now())
	putU16(b, 126, GuildWireID(ch.GuildID))
	putU16(b, 128, uint16(ch.GuildRank))
	b[130] = clampByte(int(ext.ResistFire))
	b[131] = clampByte(int(ext.ResistIce))
	b[132] = clampByte(int(ext.ResistHoly))
	b[133] = clampByte(int(ext.ResistThunder))
	// ReqHp/ReqMp are pending skill costs in TMProject, not current resources.
	// Score.Hp/Score.Mp already carry the authoritative values; leaving these
	// fields zero prevents a score refresh from charging HP/MP on a later cast.
	putU16(b, 144, compatibilityU16(ext.MagicAmp))
	// LearnedSkill is a one-byte avatar-effect selector in this packet, not the
	// character's 32-bit learned-skill mask (which travels in UpdateEtc).
	b[148] = 0
	return b
}

// SourceMobScore builds the source client's 152-byte score refresh for an NPC
// or monster. The stock XSC2 packet cannot be broadcast to a mixed client view
// because TMProject reads a 48-byte STRUCT_SCORE directly at byte 12.
func SourceMobScore(id uint16, ext *model.ExtendedScore, affects []model.Affect, resist model.ElementalResists) []byte {
	b := Build(OpUpdateScore, id, 152)
	score := EncodeClientScore48(ext)
	copy(b[12:60], score[:])
	if ext != nil {
		b[60] = clampByte(int(ext.Critical))
		b[61] = clampByte(int(ext.SaveMana))
		putU16(b, 144, compatibilityU16(ext.MagicAmp))
	}
	putAffectWords(b, 62, affects, time.Now())
	b[130] = clampByte(int(resist.Fire))
	b[131] = clampByte(int(resist.Ice))
	b[132] = clampByte(int(resist.Sacred))
	b[133] = clampByte(int(resist.Thunder))
	return b
}

// MobScoreForProtocol serializes one authoritative mob score for the observer
// ABI. Gameplay remains a single mutation regardless of client presentation.
func MobScoreForProtocol(protocol ClientProtocol, id uint16, ext *model.ExtendedScore, affects []model.Affect, resist model.ElementalResists) []byte {
	if protocol == ClientProtocolSource748 {
		return SourceMobScore(id, ext, affects, resist)
	}
	return MobScoreExtended(id, ext, affects, resist)
}

// MobHpMpForProtocol keeps the stock client's projected 20-byte resource
// update while giving TMProject the authoritative uint32 values carried by its
// 36-byte handler. This prevents a later mob hit from replacing the full score
// loaded at materialization with the legacy WORD projection.
func MobHpMpForProtocol(protocol ClientProtocol, id uint16, currentHP, maxHP, currentMP, maxMP uint32) []byte {
	if protocol != ClientProtocolSource748 {
		return SetMobHpMp(id, currentHP, maxHP, currentMP, maxMP)
	}
	ext := &model.ExtendedScore{
		Version: model.ExtendedScoreVersion,
		CurHP:   currentHP,
		MaxHP:   maxHP,
		CurMP:   currentMP,
		MaxMP:   maxMP,
	}
	return SetHpMpExtended(id, ext)
}

// HpMpForProtocol is the player-side counterpart to MobHpMpForProtocol. The
// source build consumes the 36-byte wide resource packet, while the deployed
// stock 7.48 executable only understands the original 20-byte projection.
// Selecting the layout from the recipient session prevents a mixed World
// from sending a newer tail to a legacy client without changing gameplay.
func HpMpForProtocol(protocol ClientProtocol, id uint16, ext *model.ExtendedScore) []byte {
	if ext == nil {
		ext = &model.ExtendedScore{Version: model.ExtendedScoreVersion}
	}
	if protocol == ClientProtocolSource748 {
		return SetHpMpExtended(id, ext)
	}
	return SetMobHpMp(id, ext.CurHP, ext.MaxHP, ext.CurMP, ext.MaxMP)
}

// UpdateScoreForProtocol is the authoritative score publication boundary for
// both supported clients.
func UpdateScoreForProtocol(protocol ClientProtocol, id uint16, ch model.Char) []byte {
	if protocol == ClientProtocolSource748 {
		return SourceUpdateScore(id, ch)
	}
	return UpdateScore(id, ch)
}

// UpdateAffectsForProtocol keeps the source client's 32-slot structure and the
// stock client's 16-slot structure separate without expanding the game model.
func UpdateAffectsForProtocol(protocol ClientProtocol, id uint16, ch model.Char) []byte {
	if protocol != ClientProtocolSource748 {
		return UpdateAffects(id, ch)
	}
	b := Build(OpUpdateAffect, id, 268)
	putSourceAffects(b, 12, ch.Affects[:], time.Now())
	return b
}

// sourceCreateMob serializes TMProject's 236-byte MSG_CreateMob. The source
// ABI expands equipment and affect arrays to 18/32 entries and embeds the
// 48-byte presentation score, but receives the same authoritative values as
// the stock-client builder.
func sourceCreateMob(id uint16, name string, x, y uint16, mesh []uint16, anct []byte, ext *model.ExtendedScore, affects []model.Affect, spawn, guild uint16, guildRank byte, cp *int16) []byte {
	b := Build(OpCreateMob, SceneField, 236)
	putU16(b, 12, x)
	putU16(b, 14, y)
	putU16(b, 16, id)
	copy(b[18:34], name)
	if cp != nil {
		b[30] = CPNameByte(*cp)
	}
	for i, value := range mesh {
		if i >= 18 {
			break
		}
		putU16(b, 34+i*2, value)
	}
	putAffectWords(b, 70, affects, time.Now())
	putU16(b, 134, GuildWireID(guild))
	b[136] = guildRank
	score := EncodeClientScore48(ext)
	copy(b[140:188], score[:])
	putU16(b, 188, spawn)
	copy(b[190:208], anct)
	return b
}

// CreateMobVisualExtendedForProtocol keeps entity materialization compatible
// with both clients. NPCs and monsters do not receive a player CP byte.
func CreateMobVisualExtendedForProtocol(protocol ClientProtocol, id uint16, name string, x, y uint16, mesh []uint16, anct []byte, ext *model.ExtendedScore, affects []model.Affect, spawn uint16) []byte {
	if protocol == ClientProtocolSource748 {
		return sourceCreateMob(id, name, x, y, mesh, anct, ext, affects, spawn, 0, 0, nil)
	}
	return CreateMobVisualExtended(id, name, x, y, mesh, anct, ext, affects, spawn)
}

// CreateMobExtendedWithGuildRankForProtocol is the player-specific
// materialization boundary. Guild rank and CP remain projections only.
func CreateMobExtendedWithGuildRankForProtocol(protocol ClientProtocol, id uint16, name string, x, y uint16, mesh []uint16, anct []byte, ext *model.ExtendedScore, affects []model.Affect, spawn, guild uint16, guildRank byte, cp int16) []byte {
	if protocol == ClientProtocolSource748 {
		return sourceCreateMob(id, name, x, y, mesh, anct, ext, affects, spawn, guild, guildRank, &cp)
	}
	return CreateMobExtendedWithGuildRank(id, name, x, y, mesh, anct, ext, affects, spawn, guild, guildRank, cp)
}

// CreateMobTradeExtendedForProtocol uses the source client's 260-byte trade
// clone without leaking its wider layout to stock 7.48 observers.
func CreateMobTradeExtendedForProtocol(protocol ClientProtocol, id uint16, name string, x, y uint16, mesh []uint16, ext *model.ExtendedScore, title string) []byte {
	if protocol != ClientProtocolSource748 {
		return CreateMobTradeExtended(id, name, x, y, mesh, ext, title)
	}
	normal := sourceCreateMob(id, name, x, y, mesh, nil, ext, nil, 2, 0, 0, nil)
	b := Build(OpCreateMobTrade, SceneField, 260)
	copy(b[12:234], normal[12:234])
	copy(b[234:258], title)
	return b
}

// ShopListForProtocol preserves the native limits of each renderer: stock
// 7.48 accepts 64 item records, while TMProject's source panel owns 27.
func ShopListForProtocol(protocol ClientProtocol, items []model.Item, tax, shopType uint32) []byte {
	if protocol != ClientProtocolSource748 {
		return ShopList(items, tax, shopType)
	}
	b := Build(OpShopList, SceneField, 236)
	putU32(b, 12, shopType)
	for i := 0; i < len(items) && i < 27; i++ {
		PutItem(b, 16+i*8, items[i])
	}
	putU32(b, 232, tax)
	return b
}

// MessageChatForProtocol keeps TMProject's 128-byte text field while stock
// observers continue receiving the captured 96-byte 7.48 layout.
func MessageChatForProtocol(protocol ClientProtocol, id uint16, message string) []byte {
	if protocol != ClientProtocolSource748 {
		return MessageChat(id, message)
	}
	b := Build(OpMessageChat, id, 140)
	copy(b[12:139], message)
	return b
}
