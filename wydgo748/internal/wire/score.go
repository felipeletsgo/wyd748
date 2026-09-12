package wire

import (
	"encoding/binary"

	"wydgo/internal/model"
)

// ClientScoreSize is the exact 7.48+ source-client STRUCT_SCORE size.
// The layout is intentionally simple: 35 little-endian uint32 words shared
// with model.Score and the C++ STRUCT_SCORE declaration.
const ClientScoreSize = 35 * 4

// EncodeClientScore serializes the canonical authoritative Score without
// narrowing. There is intentionally no decoder: client-sent score state is
// never authoritative.
func EncodeClientScore(score *model.Score) [ClientScoreSize]byte {
	var out [ClientScoreSize]byte
	if score == nil {
		return out
	}

	values := [...]uint32{
		score.Version,
		score.Level,
		score.Attack,
		score.MagicAttack,
		score.Defense,
		score.MaxHP,
		score.MaxMP,
		score.CurHP,
		score.CurMP,
		score.Str,
		score.Int,
		score.Dex,
		score.Con,
		score.Accuracy,
		score.Evasion,
		score.Parry,
		score.Critical,
		score.Range,
		score.ResistFire,
		score.ResistIce,
		score.ResistHoly,
		score.ResistThunder,
		score.SaveMana,
		score.MagicAmp,
		score.RegenHP,
		score.RegenMP,
		score.StatusPts,
		score.MasterPts,
		score.SkillPts,
		score.Mastery[0],
		score.Mastery[1],
		score.Mastery[2],
		score.Mastery[3],
		score.AttackRun,
		score.Merchant,
	}
	for i, value := range values {
		binary.LittleEndian.PutUint32(out[i*4:(i+1)*4], value)
	}
	return out
}
