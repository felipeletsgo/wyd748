package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestPartyRequestTarget754(t *testing.T) {
	pkt := make([]byte, 44)
	binary.LittleEndian.PutUint32(pkt[40:44], 321)
	target, ok := partyRequestTarget(pkt)
	if !ok || target != 321 {
		t.Fatalf("alvo 7.54 incorreto: target=%d ok=%v", target, ok)
	}
}

func TestPartyRequestTarget759Compatibility(t *testing.T) {
	pkt := make([]byte, 48)
	binary.LittleEndian.PutUint16(pkt[44:46], 654)
	target, ok := partyRequestTarget(pkt)
	if !ok || target != 654 {
		t.Fatalf("alvo 7.59 incorreto: target=%d ok=%v", target, ok)
	}
}

func partyTestPlayer(id uint16, x, y uint16) *Player {
	return &Player{
		ID: id, X: x, Y: y, InWorld: true,
		Char: &model.Char{Name: "Player", Extended: testExtended(model.ExtendedScore{MaxHP: 100, CurHP: 100})},
	}
}

func TestPartyExpGivesFullRewardWithTwoPercentPerMember(t *testing.T) {
	leader := partyTestPlayer(1, 2200, 2100)
	memberA := partyTestPlayer(2, 2201, 2101)
	memberB := partyTestPlayer(3, 2202, 2102)
	party := &Party{Members: []*Player{leader, memberA, memberB}}
	leader.Party, memberA.Party, memberB.Party = party, party, party

	shares := partyExpShares(leader, 1000, 2)
	if len(shares) != 3 {
		t.Fatalf("parcelas=%d, quer 3", len(shares))
	}
	got := map[uint16]uint32{}
	for _, share := range shares {
		got[share.player.ID] = share.reward
	}
	if got[1] != 1060 || got[2] != 1060 || got[3] != 1060 {
		t.Fatalf("bonus para tres membros=%v, quer 1060 para cada", got)
	}
}

func TestPartyExpExcludesDeadOfflineAndOtherSector(t *testing.T) {
	killer := partyTestPlayer(1, 2200, 2100)
	near := partyTestPlayer(2, 2201, 2101)
	dead := partyTestPlayer(3, 2202, 2102)
	setPlayerCurHP(dead.Char, 0)
	offline := partyTestPlayer(4, 2203, 2103)
	offline.InWorld = false
	far := partyTestPlayer(5, 2304, 2100) // setor X 18; killer esta no 17
	party := &Party{Members: []*Player{killer, near, dead, offline, far}}
	for _, member := range party.Members {
		member.Party = party
	}

	shares := partyExpShares(killer, 999, 2)
	if len(shares) != 2 || shares[0].player != killer || shares[1].player != near {
		t.Fatalf("membros elegiveis incorretos: %+v", shares)
	}
	if shares[0].reward != 1038 || shares[1].reward != 1038 { // floor(999 * 104%)
		t.Fatalf("bonus incorreto: %+v", shares)
	}
}

func TestPartyExpBonusFromOneToThirteenMembers(t *testing.T) {
	for members := 1; members <= maxPartyMembers; members++ {
		players := make([]*Player, 0, members)
		for i := 0; i < members; i++ {
			players = append(players, partyTestPlayer(uint16(i+1), 2200+uint16(i), 2100))
		}
		party := &Party{Members: players}
		for _, player := range players {
			player.Party = party
		}
		shares := partyExpShares(players[0], 1000, 2)
		want := uint32(1000 + members*20)
		if len(shares) != members {
			t.Fatalf("membros=%d parcelas=%d", members, len(shares))
		}
		for _, share := range shares {
			if share.reward != want {
				t.Fatalf("membros=%d recompensa=%d, quer %d", members, share.reward, want)
			}
		}
	}
}

func TestPartyExpUsesConfiguredBonus(t *testing.T) {
	leader := partyTestPlayer(1, 2200, 2100)
	member := partyTestPlayer(2, 2201, 2101)
	party := &Party{Members: []*Player{leader, member}}
	leader.Party, member.Party = party, party

	shares := partyExpShares(leader, 10_000, 5)
	if len(shares) != 2 || shares[0].reward != 11_000 || shares[1].reward != 11_000 {
		t.Fatalf("bonus configurado nao aplicado: %+v", shares)
	}
}

func TestDefaultGlobalExperienceProducesTenThousandTwoHundredSolo(t *testing.T) {
	config := model.DefaultGameplayConfig()
	player := partyTestPlayer(1, 2200, 2100)
	base := scaledMobExperience(1, config)
	shares := partyExpShares(player, base, config.PartyEXPBonusPercent)
	if len(shares) != 1 || shares[0].reward != 10_200 {
		t.Fatalf("recompensa solo padrao incorreta: base=%d shares=%+v", base, shares)
	}
}

func TestPartyExperienceExcludesInternalLevel399AtFinalEXPCap(t *testing.T) {
	maxed := partyTestPlayer(1, 2200, 2100)
	maxed.Char.Extended.Level = maxMortalLevel
	maxed.Char.Exp = mortalNextLevel[400]
	if shares := partyExpShares(maxed, 10_000, 2); len(shares) != 0 {
		t.Fatalf("personagem no limite recebeu EXP: %+v", shares)
	}
}

func TestPartyCapacityMatchesLeaderPlusTwelveMembers(t *testing.T) {
	if maxPartyMembers != 13 {
		t.Fatalf("capacidade=%d, quer lider + 12", maxPartyMembers)
	}
}
