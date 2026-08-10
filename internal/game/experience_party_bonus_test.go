package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func equipCombatExperienceBonuses(t *testing.T, w *World, p *Player) {
	t.Helper()
	const coralItem = 65000
	if w.items == nil {
		w.items = make(map[uint16]model.ItemDef)
	}
	w.items[coralItem] = model.ItemDef{Index: coralItem, Grade: 7}
	w.items[3914] = fairyTestDef(3914, 7)
	p.Char.Equip[1] = model.Item{Index: coralItem}    // Coral Ancient: +2% EXP.
	p.Char.Equip[fairySlot] = model.Item{Index: 3914} // Silver Angel: +16% EXP.
	p.Char.Affects[0] = model.Affect{
		Type: affectDoubleExp, ExpiresAt: time.Now().Add(time.Hour),
	}
}

func experiencePartyKillWorld(t *testing.T) (*World, *Player, *Player) {
	t.Helper()
	killer, _ := networkedTestPlayer(1, "Killer", 2200, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2201, 2100)
	party := &Party{Members: []*Player{killer, member}}
	killer.Party, member.Party = party, party
	w := worldWithNetworkedPlayers(killer, member)
	w.gameplay = model.GameplayConfig{EXPRatePercent: 100, PartyEXPBonusPercent: 2}
	w.items = make(map[uint16]model.ItemDef)
	return w, killer, member
}

func killExperienceTestMob(w *World, killer *Player, id uint16) {
	mob := &Mob{
		ID: id, X: killer.X + 1, Y: killer.Y, HP: 100,
		Def: &model.NPCDef{
			Name: "EXP Test Mob", Tipo: model.TipoMonstro, ExpReward: 10_000,
			Extended: &model.ExtendedScore{
				Version: model.ExtendedScoreVersion, Level: 1, MaxHP: 100, CurHP: 100,
			},
		},
	}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	w.killMobState(killer, mob, 100, 100, false)
}

func TestMobKillExperienceCalculatesEveryItemBonusFromKiller(t *testing.T) {
	tests := []struct {
		name  string
		setup func(*testing.T, *World, *Player)
		want  uint32
	}{
		{name: "none", want: 10_400},
		{name: "coral", want: 10_608, setup: func(_ *testing.T, w *World, p *Player) {
			const coralItem = 65000
			w.items[coralItem] = model.ItemDef{Index: coralItem, Grade: 7}
			p.Char.Equip[1] = model.Item{Index: coralItem}
		}},
		{name: "silver fairy", want: 12_064, setup: func(_ *testing.T, w *World, p *Player) {
			w.items[3914] = fairyTestDef(3914, 7)
			p.Char.Equip[fairySlot] = model.Item{Index: 3914}
		}},
		{name: "experience box", want: 20_800, setup: func(_ *testing.T, _ *World, p *Player) {
			p.Char.Affects[0] = model.Affect{
				Type: affectDoubleExp, ExpiresAt: time.Now().Add(time.Hour),
			}
		}},
	}

	for index, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			w, killer, member := experiencePartyKillWorld(t)
			if test.setup != nil {
				test.setup(t, w, killer)
			}
			killExperienceTestMob(w, killer, 1100+uint16(index))
			if killer.Char.Exp != test.want || member.Char.Exp != test.want {
				t.Fatalf("killer=%d member=%d, want=%d", killer.Char.Exp, member.Char.Exp, test.want)
			}
		})
	}
}

func TestMobKillSharesKillerItemExperienceBonusesWithWholeParty(t *testing.T) {
	w, killer, member := experiencePartyKillWorld(t)
	equipCombatExperienceBonuses(t, w, killer)

	killExperienceTestMob(w, killer, 1000)

	// 10,000 + party de 4% = 10,400; Coral+Silver = +18%; bau = x2.
	const want = uint32(24_544)
	if killer.Char.Exp != want || member.Char.Exp != want {
		t.Fatalf("EXP do abate com bonus do matador: killer=%d member=%d, want=%d para ambos",
			killer.Char.Exp, member.Char.Exp, want)
	}
}

func TestMobKillIgnoresItemExperienceBonusesOwnedOnlyByReceiver(t *testing.T) {
	w, killer, member := experiencePartyKillWorld(t)
	equipCombatExperienceBonuses(t, w, member)

	killExperienceTestMob(w, killer, 1001)

	// O membro nao matou: seus itens nao amplificam o abate do killer.
	const want = uint32(10_400)
	if killer.Char.Exp != want || member.Char.Exp != want {
		t.Fatalf("bonus do receptor vazou para o abate: killer=%d member=%d, want=%d para ambos",
			killer.Char.Exp, member.Char.Exp, want)
	}
}

func TestMobKillUsesNewKillersBonusesWhenPartyMemberGetsTheKill(t *testing.T) {
	w, leader, member := experiencePartyKillWorld(t)
	equipCombatExperienceBonuses(t, w, member)

	killExperienceTestMob(w, member, 1002)

	const want = uint32(24_544)
	if leader.Char.Exp != want || member.Char.Exp != want {
		t.Fatalf("bonus do membro matador nao foi compartilhado: leader=%d member=%d, want=%d",
			leader.Char.Exp, member.Char.Exp, want)
	}
}

func TestMobKillKeepsReceiverSpecificCelestialReduction(t *testing.T) {
	w, killer, member := experiencePartyKillWorld(t)
	equipCombatExperienceBonuses(t, w, killer)
	member.Char.Evolution = "celestial"
	member.Char.Extended.Level = 159 // duas quedas cumulativas pela metade.

	const partyReward = uint32(10_400)
	killerReward := w.mobKillExperienceForReceiver(killer.Char, killer.Char, partyReward)
	memberReward := w.mobKillExperienceForReceiver(killer.Char, member.Char, partyReward)
	if killerReward != 24_544 || memberReward != 6_136 {
		t.Fatalf("reducao por receptor incorreta: mortal=%d celestial=%d", killerReward, memberReward)
	}
}
