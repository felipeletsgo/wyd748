package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func TestApplyBonusDoesNotRematerializeMovingPlayer(t *testing.T) {
	session := net.NewTestSession(1, 8)
	ch := testChar()
	ch.Score.StatusPts = 50
	p := &Player{
		Session: session, Char: &ch, ID: 1, InWorld: true, X: 2105, Y: 2100,
		MovePublished: true, MovePublishedTargetX: 2105, MovePublishedTargetY: 2100,
		Visible: make(map[uint16]struct{}),
	}
	w := &World{
		players:     map[*net.Session]*Player{session: p},
		playerCells: make(map[uint32]map[uint16]*Player),
		playerCell:  make(map[uint16]uint32),
		mobCells:    make(map[uint32]map[uint16]*Mob),
		activeMobs:  make(map[uint16]*Mob),
	}
	w.updatePlayerSpatial(p)
	pkt := make([]byte, applyBonusPacketSize)
	binary.LittleEndian.PutUint16(pkt[12:14], 0)
	binary.LittleEndian.PutUint16(pkt[14:16], 0)

	w.onApplyBonus(session, pkt)

	// UpdateScore + UpdateEtc. SetHpMp no longer goes to the owner: 0x336
	// already carries HP/MP in the legacy WORDs and the wide tail, and every
	// vitals packet costs a native redraw in the rebuilt client; two in a row
	// made the bar flicker. Observers still receive 0x181.
	//
	// A third packet here would be the CreateMob that snapped the avatar to
	// the destination while the walk was still interpolating.
	if got := session.QueuedPacketsForTest(); got != 2 {
		t.Fatalf("apply bonus queued %d packets, want 2 without CreateMob", got)
	}
	if !p.MovePublished || p.X != 2105 || p.Y != 2100 {
		t.Fatalf("movement state changed: published=%v pos=(%d,%d)", p.MovePublished, p.X, p.Y)
	}
}

func testChar() model.Char {
	return model.Char{Score: &model.Score{
		Version: model.ScoreVersion,
		Level:   10, Attack: 20, Defense: 10, MaxHP: 100, MaxMP: 50,
		CurHP: 100, CurMP: 50, Str: 8, Int: 4, Dex: 7, Con: 6,
	}}
}

func TestRecalcExtendedUsesAllAttributes(t *testing.T) {
	ch := testChar()
	(&World{}).recalcPlayer(&ch)
	e := effectiveScore(&ch)
	// 7.59 score: 20 + STR 8/3 + DEX 7/8 + level 10; defense 10 + level 10.
	if e.Attack != 32 {
		t.Fatalf("attack=%d, want 32", e.Attack)
	}
	if e.Defense != 20 || e.MaxHP != 130 || e.MaxMP != 60 {
		t.Fatalf("incorrect derived values: %+v", e)
	}
	if e.MagicAttack == 0 || combatAccuracyPercent(int(e.Dex), int(e.Accuracy), int(e.Evasion), false) != 100 {
		t.Fatalf("derived combat not calculated: %+v", e)
	}
}

func TestApplyStatusConsumesPointAndDoesNotCompound(t *testing.T) {
	ch := testChar()
	w := &World{}
	w.recalcPlayer(&ch)
	if !applyBonus(&ch, 0, 3) {
		t.Fatal("aplicacao recusada")
	}
	w.recalcPlayer(&ch)
	if ch.Score.StatusPts != 49 || ch.Score.Con != 7 || playerMaxHP(&ch) != 132 {
		t.Fatalf("incorrect CON/HP: pts=%d baseCON=%d maxHP=%d",
			ch.Score.StatusPts, ch.Score.Con, playerMaxHP(&ch))
	}
	w.recalcPlayer(&ch)
	if playerMaxHP(&ch) != 132 {
		t.Fatalf("recalculo acumulou bonus: maxHP=%d", playerMaxHP(&ch))
	}
}

func TestBaseHPMPProgressionForAllMortalClasses(t *testing.T) {
	want := [4][2]uint32{{230, 95}, {110, 215}, {120, 155}, {175, 110}}
	for class := 0; class < 4; class++ {
		stats := baseClassStats[class]
		ch := model.Char{Class: byte(class), Score: &model.Score{
			Version: model.ScoreVersion,
			Level:   50, MaxHP: uint32(baseClassHPMP[class][0]), MaxMP: uint32(baseClassHPMP[class][1]),
			Str: uint32(stats[0]), Int: uint32(stats[1]),
			Dex: uint32(stats[2]), Con: uint32(stats[3]),
		}}
		(&World{}).recalcPlayer(&ch)
		if playerMaxHP(&ch) != want[class][0] || playerMaxMP(&ch) != want[class][1] {
			t.Fatalf("class=%d HP/MP=%d/%d, want %d/%d", class,
				playerMaxHP(&ch), playerMaxMP(&ch), want[class][0], want[class][1])
		}
	}
}

func TestMortalPointBudgetsUseHighLevelThresholds(t *testing.T) {
	if got := mortalStatusPointBudget(253); got != 1265 {
		t.Fatalf("status level 253=%d", got)
	}
	if got := mortalStatusPointBudget(400); got != 3372 {
		t.Fatalf("status level 400=%d, want 3372", got)
	}
	if got := mortalSkillPointBudget(200); got != 601 {
		t.Fatalf("skill level 200=%d, want 601", got)
	}
}

func TestMasteryCapFollowsEighthAndSecondarySkills(t *testing.T) {
	ch := &model.Char{Score: &model.Score{
		Version: model.ScoreVersion, Level: 400,
	}}
	if got := masteryPointLimit(ch, 1); got != 200 {
		t.Fatalf("limite base=%d", got)
	}
	ch.LearnedSkill = 1 << 7
	if got := masteryPointLimit(ch, 1); got != 255 {
		t.Fatalf("eighth skill limit=%d", got)
	}
	ch.SecondaryLearnedSkill = 1 << 4
	if got := masteryPointLimit(ch, 1); got != 320 {
		t.Fatalf("limite secundaria=%d", got)
	}
}

func TestApplySpecialAffectsPhysicalAttack(t *testing.T) {
	ch := testChar()
	w := &World{}
	w.recalcPlayer(&ch)
	before := playerAttack(&ch)
	if !applyBonus(&ch, 1, 0) {
		t.Fatal("special refused")
	}
	w.recalcPlayer(&ch)
	if ch.Score.MasterPts != 19 || ch.Score.Mastery[0] != 1 || playerAttack(&ch) != before+1 {
		t.Fatalf("physical special not applied: mastery=%v attack=%d",
			ch.Score.Mastery, playerAttack(&ch))
	}
}

func TestMasteryPointsFollowLevelAndSpent(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 50
	ch.Score.Mastery = [4]uint32{10, 20, 5, 0}
	ch.Score.MasterPts = 60_000
	syncMasteryPoints(&ch)
	if ch.Score.MasterPts != 65 {
		t.Fatalf("available mastery=%d, want 65", ch.Score.MasterPts)
	}
}

func TestSkillPointsFollowMortalLevel(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 50
	ch.Score.SkillPts = 3
	syncSkillPoints(&ch)
	if ch.Score.SkillPts != 150 {
		t.Fatalf("skill points=%d, want 150", ch.Score.SkillPts)
	}
}

func TestApplyBonusRejectsInvalidAndEmpty(t *testing.T) {
	ch := testChar()
	ch.Score.Str += 50
	syncStatusPoints(&ch)
	if applyBonus(&ch, 0, 0) || applyBonus(&ch, 0, 4) || applyBonus(&ch, 2, 0) {
		t.Fatal("invalid bonus was accepted")
	}
}

func TestStatusPointsAreFixedByLevelAndSpentStats(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 50
	ch.Score.Str += 100
	ch.Score.Int += 75
	ch.Score.Dex += 50
	ch.Score.Con += 25
	ch.Score.StatusPts = 60_000
	syncStatusPoints(&ch)
	if ch.Score.StatusPts != 0 {
		t.Fatalf("level 50 with 250 spent still has %d points", ch.Score.StatusPts)
	}
	if applyBonus(&ch, 0, 0) {
		t.Fatal("allowed exceeding 250 spent points at level 50")
	}
}

func TestStatusBudgetRepairsLegacyOverspend(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 10
	ch.Score.Str += 40
	ch.Score.Int += 30
	syncStatusPoints(&ch)
	spent := int(ch.Score.Str-8) + int(ch.Score.Int-4) +
		int(ch.Score.Dex-7) + int(ch.Score.Con-6)
	if spent != 50 || ch.Score.StatusPts != 0 {
		t.Fatalf("reparo deixou spent=%d available=%d stats=%+v",
			spent, ch.Score.StatusPts, ch.Score)
	}
	if ch.Score.Str < 8 || ch.Score.Int < 4 ||
		ch.Score.Dex < 7 || ch.Score.Con < 6 {
		t.Fatalf("reparo reduziu atributo natural: %+v", ch.Score)
	}
}

// TMSRVIN 7.59 score (FUN_004bb4c0): DEX feeds physical damage as DEX/8 and
// never defense, so a high-DEX amulet must not inflate defense.
func TestDexterityFollows759Score(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		500: {Index: 500, Pos: 1 << 9, StaticEffects: []model.StaticEffect{{Name: "EF_DEX", Value: 100_000}}},
	}}
	plain, amulet := testChar(), testChar()
	amulet.Equip[9] = model.Item{Index: 500}
	w.recalcPlayer(&plain)
	w.recalcPlayer(&amulet)
	before, after := effectiveScore(&plain), effectiveScore(&amulet)
	if after.Dex != before.Dex+100_000 {
		t.Fatalf("amulet DEX %d -> %d, want +100000", before.Dex, after.Dex)
	}
	if after.Defense != before.Defense {
		t.Fatalf("DEX 100000 changed defense: %d -> %d", before.Defense, after.Defense)
	}
	if got, want := after.Attack-before.Attack, uint32((7+100_000)/8-7/8); got != want {
		t.Fatalf("DEX 100000 added %d physical attack, want %d", got, want)
	}
}
