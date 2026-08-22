from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]

# Visibility has one packet ABI. Rewrite the old observer-protocol block once;
# subsequent cleanup runs verify the canonical function instead of failing.
path = root / "internal/game/visibility.go"
text = path.read_text(encoding="utf-8")
pattern = re.compile(
    r"// playerEnterViewPackets monta.*?func sendPlayerEnterView\(observer, subject \*Player\) \{.*?\n\}",
    re.S,
)
replacement = r'''// playerEnterViewPackets materializes one player with the canonical 7.48+
// packet ABI. HP/MP follows CreateMob so a reused entity ID cannot retain a
// stale dead-state resource cache.
func playerEnterViewPackets(subject *Player) [][]byte {
	if subject == nil || subject.Char == nil {
		return nil
	}
	return [][]byte{
		wire.CreateMobWithGuildRank(subject.ID, subject.Char.Name, subject.X, subject.Y,
			bodyMesh(subject.Char), bodyAncient(subject.Char), wireScoreState(subject.Char),
			subject.Char.Affects[:], 2, subject.Char.GuildID, subject.Char.GuildRank, subject.Char.CP),
		wire.HpMp(subject.ID, wireScoreState(subject.Char)),
		wire.ActionStop(subject.ID, subject.X, subject.Y),
	}
}

func sendPlayerEnterView(observer, subject *Player) {
	if observer == nil || observer.Session == nil {
		return
	}
	for _, pkt := range playerEnterViewPackets(subject) {
		observer.Session.Send(pkt)
	}
}'''
text, count = pattern.subn(replacement, text, count=1)
if count == 0 and "func playerEnterViewPackets(subject *Player)" not in text:
    raise RuntimeError("visibility.go has neither legacy nor canonical materialization block")
path.write_text(text, encoding="utf-8", newline="\n")

# HP debug, when still present during an intermediate pass, reports only the
# persisted and effective copies of the same canonical Score type.
path = root / "internal/game/hp_debug.go"
if path.exists():
    path.write_text(r'''package game

import (
	"fmt"
	"log"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// dumpHPState reports persisted and effective values from the same Score model.
func (w *World) dumpHPState(s *net.Session, p *Player) {
	if p == nil || p.Char == nil {
		return
	}
	base := p.Char.Score
	effective := effectiveScore(p.Char)
	if base == nil || effective == nil {
		s.Send(wire.MessagePanel("No score."))
		return
	}
	percent := func(cur, max uint32) float64 {
		if max == 0 {
			return 0
		}
		return 100 * float64(cur) / float64(max)
	}
	for _, line := range []string{
		fmt.Sprintf("base HP %d/%d (%.1f%%)", base.CurHP, base.MaxHP, percent(base.CurHP, base.MaxHP)),
		fmt.Sprintf("runtime HP %d/%d (%.1f%%)", effective.CurHP, effective.MaxHP, percent(effective.CurHP, effective.MaxHP)),
	} {
		s.Send(wire.MessagePanel(line))
	}
	log.Printf("[#%d] HPDEBUG base=%d/%d runtime=%d/%d", s.ID,
		base.CurHP, base.MaxHP, effective.CurHP, effective.MaxHP)
}
''', encoding="utf-8", newline="\n")

for rel in ["internal/game/commands.go", "internal/game/coverage_more_test.go"]:
    path = root / rel
    if path.exists():
        data = path.read_text(encoding="utf-8")
        data = data.replace("dumpHPProjection", "dumpHPState")
        path.write_text(data, encoding="utf-8", newline="\n")

print("gameplay protocol compatibility remnants removed")
