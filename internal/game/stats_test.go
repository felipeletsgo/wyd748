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

	// UpdateScore + UpdateEtc. O SetHpMp NAO vai mais para o dono: o 0x336 ja
	// leva HP/MP nos WORDs legados e na cauda wide, e cada pacote de vitals
	// custa um redesenho nativo no client patcheado -- dois seguidos faziam a
	// barra piscar. Os observadores continuam recebendo o 0x181.
	//
	// Um terceiro pacote aqui seria o CreateMob que encaixava o avatar no
	// destino enquanto a caminhada ainda interpolava.
	if got := session.QueuedPacketsForTest(); got != 2 {
		t.Fatalf("apply bonus enfileirou %d pacotes, esperado 2 sem CreateMob", got)
	}
	if !p.MovePublished || p.X != 2105 || p.Y != 2100 {
		t.Fatalf("estado de movimento alterado: published=%v pos=(%d,%d)", p.MovePublished, p.X, p.Y)
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
	e := effectiveExtended(&ch)
	if e.Attack != 33 {
		t.Fatalf("attack=%d, esperado 33", e.Attack)
	}
	if e.Defense != 21 || e.MaxHP != 130 || e.MaxMP != 60 {
		t.Fatalf("derivados incorretos: %+v", e)
	}
	if e.MagicAttack == 0 || combatAccuracyPercent(int(e.Dex), int(e.Accuracy), int(e.Evasion), false) != 100 {
		t.Fatalf("combate derivado nao calculado: %+v", e)
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
		t.Fatalf("CON/HP incorretos: pts=%d baseCON=%d maxHP=%d",
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
			t.Fatalf("classe=%d HP/MP=%d/%d, quer %d/%d", class,
				playerMaxHP(&ch), playerMaxMP(&ch), want[class][0], want[class][1])
		}
	}
}

func TestMortalPointBudgetsUseHighLevelThresholds(t *testing.T) {
	if got := mortalStatusPointBudget(253); got != 1265 {
		t.Fatalf("status level 253=%d", got)
	}
	if got := mortalStatusPointBudget(400); got != 3372 {
		t.Fatalf("status level 400=%d, quer 3372", got)
	}
	if got := mortalSkillPointBudget(200); got != 601 {
		t.Fatalf("skill level 200=%d, quer 601", got)
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
		t.Fatalf("limite oitava skill=%d", got)
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
		t.Fatal("especial recusado")
	}
	w.recalcPlayer(&ch)
	if ch.Score.MasterPts != 19 || ch.Score.Mastery[0] != 1 || playerAttack(&ch) != before+1 {
		t.Fatalf("especial fisico nao aplicado: mastery=%v attack=%d",
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
		t.Fatalf("mastery disponivel=%d, esperado 65", ch.Score.MasterPts)
	}
}

func TestSkillPointsFollowMortalLevel(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 50
	ch.Score.SkillPts = 3
	syncSkillPoints(&ch)
	if ch.Score.SkillPts != 150 {
		t.Fatalf("skill points=%d, esperado 150", ch.Score.SkillPts)
	}
}

func TestApplyBonusRejectsInvalidAndEmpty(t *testing.T) {
	ch := testChar()
	ch.Score.Str += 50
	syncStatusPoints(&ch)
	if applyBonus(&ch, 0, 0) || applyBonus(&ch, 0, 4) || applyBonus(&ch, 2, 0) {
		t.Fatal("bonus invalido foi aceito")
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
		t.Fatalf("level 50 com 250 gastos ainda possui %d pontos", ch.Score.StatusPts)
	}
	if applyBonus(&ch, 0, 0) {
		t.Fatal("permitiu ultrapassar 250 pontos gastos no level 50")
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
