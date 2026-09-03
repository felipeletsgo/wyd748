package wire

import (
	"time"

	"wydgo/internal/model"
)

// putSelChar writes TMProject's 1272-byte STRUCT_SELCHAR. Its canonical 140-byte
// Score is copied without narrowing; uint64 EXP remains a presentation field.
func putSelChar(dst []byte, offset int, chars []model.Char) {
	for slot := 0; slot < 4 && slot < len(chars); slot++ {
		ch := chars[slot]
		if ch.Name == "" {
			continue
		}
		putU16(dst, offset+slot*2, ch.X)
		putU16(dst, offset+8+slot*2, ch.Y)
		copy(dst[offset+16+slot*16:offset+32+slot*16], ch.Name)
		score := EncodeClientScore(wireScore(ch))
		copy(dst[offset+80+slot*ClientScoreSize:], score[:])
		for equipSlot, item := range ch.Equip {
			// TMProject keeps 18 source-side equipment entries. The server model
			// has the canonical 16 slots, so the two trailing entries remain zero.
			PutItem(dst, offset+640+(slot*18+equipSlot)*8, item)
		}
		putU16(dst, offset+1216+slot*2, GuildWireID(ch.GuildID))
		putU32(dst, offset+1224+slot*4, ch.Gold)
		putU64(dst, offset+1240+slot*8, uint64(ch.Exp))
	}
}

// CharList builds the exact 2360-byte MSG_CNFAccountLogin expected by
// the source client: Header, alignment padding, SecretCode, 1272-byte SelChar,
// Cargo, cargo coin and account identity. SecretCode stays zero because the Go
// transport authenticates each frame from its own header keyword/checksum.
func CharList(accName string, chars []model.Char, cargo []model.Item, cargoGold uint32) []byte {
	const (
		packetSize = 2360
		selOffset  = 32
		cargoOff   = 1304
	)
	b := Build(OpCharList, SceneCharList, packetSize)
	putSelChar(b, selOffset, chars)
	for i := 0; i < len(cargo) && i < model.MaxCargo; i++ {
		PutItem(b, cargoOff+i*8, cargo[i])
	}
	putU32(b, 2328, cargoGold)
	copy(b[2332:2348], accName)
	return b
}

// putSourceAffects stores the Type/Level/Value/Time representation consumed by
// the 16-slot WYD 7.48 affect ABI. The destination bound also protects packet
// builders with a shorter payload from writing beyond their proven layout.
func putSourceAffects(dst []byte, offset int, affects []model.Affect, now time.Time) {
	for i := 0; i < len(affects) && i < 16 && offset+(i+1)*8 <= len(dst); i++ {
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

// EnterWorld serializes the 2104-byte MSG_CNFCharacterLogin produced by
// the Win32 compiler. Padding and the two source-only equipment slots are
// deliberately zero. This is a projection from Char, never a model copy-back.
func EnterWorld(id, slot uint16, ch model.Char) []byte {
	b := Build(OpEnterWorld, id, 2104)
	// MSG_CNFCharacterLogin places Ext1 immediately after ShortSkill[16].
	// Keeping these offsets named prevents the 32-byte Ext1.Data prefix from
	// being mistaken for padding and shifting every source affect by two bytes.
	const (
		sourceShortSkillOffset = 1246
		// MSVC aligns STRUCT_EXT1 to four bytes after ShortSkill[16].
		// The old serializer incorrectly started it two bytes early.
		sourceExt1Offset       = 1264
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
	baseScore := EncodeClientScore(ch.Score)
	runtimeScore := EncodeClientScore(wireScore(ch))
	copy(b[mob+44:mob+184], baseScore[:])
	copy(b[mob+184:mob+324], runtimeScore[:])
	for i, item := range ch.Equip {
		PutItem(b, mob+324+i*8, item)
	}
	for i, item := range ch.Inv {
		PutItem(b, mob+468+i*8, item)
	}
	putU32(b, mob+980, ch.LearnedSkill)
	putU32(b, mob+984, ch.SecondaryLearnedSkill)
	copy(b[mob+988:mob+992], ch.ShortSkill[:4])
	b[mob+992] = ch.GuildRank
	// MSG_CNFCharacterLogin carries the selected account slot separately from
	// ClientID. Sending the spawn marker here made every source client appear to
	// have selected slot two even when the account selected another character.
	putU16(b, 1240, slot)
	putU16(b, 1242, id)
	copy(b[sourceShortSkillOffset:sourceShortSkillOffset+16], ch.ShortSkill[:16])
	putU32(b, sourceExt1Offset, ch.Hold)
	// The first affect is Ext1.Affect[0] at 1296: Ext1.Data is eight DWORDs; the two bytes before Ext1 are MSVC alignment padding.
	putSourceAffects(b, sourceExt1AffectOffset, ch.Affects[:], time.Now())
	return b
}

// UpdateScore maps the authoritative runtime score to TMProject's
// 232-byte 0x336 structure. Fields absent from that native structure continue
// to be synchronized by dedicated packets/extensions rather than trusted back.
func UpdateScore(id uint16, ch model.Char) []byte {
	b := Build(OpUpdateScore, id, 232)
	score := EncodeClientScore(wireScore(ch))
	copy(b[12:152], score[:])
	putAffectWords(b, 152, ch.Affects[:], time.Now())
	putU16(b, 216, GuildWireID(ch.GuildID))
	putU16(b, 218, uint16(ch.GuildRank))
	// ReqHp/ReqMp are pending skill costs, not current resources. A plain score
	// refresh does not charge either resource.
	putU32(b, 220, 0)
	putU32(b, 224, 0)
	// LearnedSkill here is only the one-byte avatar-effect selector.
	b[228] = 0
	return b
}

// MobScore builds the source client's 232-byte score refresh for an NPC
// or monster. The removed stock score packet cannot be broadcast to a mixed client view
// because TMProject reads the canonical STRUCT_SCORE directly at byte 12.
func MobScore(id uint16, scoreState *model.Score, affects []model.Affect) []byte {
	b := Build(OpUpdateScore, id, 232)
	score := EncodeClientScore(scoreState)
	copy(b[12:152], score[:])
	putAffectWords(b, 152, affects, time.Now())
	return b
}

// createMob serializes TMProject's 328-byte MSG_CreateMob. The source
// ABI expands equipment and affect arrays to 18/32 entries and embeds the
// canonical 140-byte Score, but receives the same authoritative values as
// the canonical builder.
func createMob(id uint16, name string, x, y uint16, mesh []uint16, anct []byte, ext *model.Score, affects []model.Affect, spawn, guild uint16, guildRank byte, cp *int16) []byte {
	b := Build(OpCreateMob, SceneField, 328)
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
	score := EncodeClientScore(ext)
	copy(b[140:280], score[:])
	putU16(b, 280, spawn)
	copy(b[282:300], anct)
	return b
}

// CharacterSelectionUpdate rebuilds the canonical four-character selection
// aggregate after create/delete/evolution.
func CharacterSelectionUpdate(opcode, id uint16, chars []model.Char) []byte {
	b := Build(opcode, id, 1288)
	putSelChar(b, 16, chars)
	return b
}

// SetHpMp publishes the four authoritative uint32 resources directly. There
// is no WORD prefix and no legacy fallback.
func SetHpMp(id uint16, score *model.Score) []byte {
	if score == nil {
		score = &model.Score{Version: model.ScoreVersion}
	}
	b := Build(OpSetHpMp, id, 28)
	putU32(b, 12, score.CurHP)
	putU32(b, 16, score.CurMP)
	putU32(b, 20, score.MaxHP)
	putU32(b, 24, score.MaxMP)
	return b
}

func MobHpMp(id uint16, currentHP, maxHP, currentMP, maxMP uint32) []byte {
	return SetHpMp(id, &model.Score{
		Version: model.ScoreVersion,
		CurHP:   currentHP, MaxHP: maxHP,
		CurMP: currentMP, MaxMP: maxMP,
	})
}

func HpMp(id uint16, score *model.Score) []byte { return SetHpMp(id, score) }

func UpdateAffects(id uint16, ch model.Char) []byte {
	// Native 7.48 opcode 0x3B9 is Header(12) + 16 affects of eight bytes.
	b := Build(OpUpdateAffect, id, 140)
	putSourceAffects(b, 12, ch.Affects[:], time.Now())
	return b
}

func CreateMobVisual(id uint16, name string, x, y uint16, mesh []uint16, anct []byte,
	score *model.Score, affects []model.Affect, spawn uint16) []byte {
	return createMob(id, name, x, y, mesh, anct, score, affects, spawn, 0, 0, nil)
}

func CreateMobWithGuildRank(id uint16, name string, x, y uint16, mesh []uint16, anct []byte,
	score *model.Score, affects []model.Affect, spawn, guild uint16, guildRank byte, cp int16) []byte {
	return createMob(id, name, x, y, mesh, anct, score, affects, spawn, guild, guildRank, &cp)
}

func CreateMobTrade(id uint16, name string, x, y uint16, mesh []uint16, score *model.Score, title string) []byte {
	normal := createMob(id, name, x, y, mesh, nil, score, nil, 2, 0, 0, nil)
	b := Build(OpCreateMobTrade, SceneField, 352)
	copy(b[12:326], normal[12:326])
	copy(b[326:350], title)
	return b
}

func ShopList(items []model.Item, tax, shopType uint32) []byte {
	b := Build(OpShopList, SceneField, 236)
	putU32(b, 12, shopType)
	for i := 0; i < len(items) && i < 27; i++ {
		PutItem(b, 16+i*8, items[i])
	}
	putU32(b, 232, tax)
	return b
}

func MessageChat(id uint16, message string) []byte {
	b := Build(OpMessageChat, id, 108)
	copy(b[12:107], message)
	return b
}

// MessageIndexed and MessageParameterized form a coordinated extension used
// only by the source-built client. Stock 7.48 has no handlers for 0x105/0x106.
// Both packets keep ID=0 so TMScene owns the borrowed payload for this call.
func MessageIndexed(relativeIndex int16) []byte {
	b := Build(OpMessageIndexed, 0, 108)
	putU16(b, 14, uint16(relativeIndex))
	return b
}

func MessageParameterized(relativeIndex int16, params ...string) []byte {
	b := Build(OpMessageParameterized, 0, 108)
	putU16(b, 14, uint16(relativeIndex))

	// String[4:95] has 92 bytes, but the last byte remains NUL. Build the CSV
	// directly into a bounded scratch slice so large caller input cannot grow an
	// intermediate allocation. Commas/NULs would change the field count and are
	// therefore normalized to spaces inside each parameter.
	payload := make([]byte, 0, 91)
	for i := 0; i < len(params) && i < 6 && len(payload) < cap(payload); i++ {
		if i > 0 {
			payload = append(payload, ',')
		}
		for j := 0; j < len(params[i]) && len(payload) < cap(payload); j++ {
			value := params[i][j]
			if value == ',' || value == 0 {
				value = ' '
			}
			payload = append(payload, value)
		}
	}
	copy(b[16:107], payload)
	return b
}
