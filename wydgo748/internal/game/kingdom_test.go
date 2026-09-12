package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestKingdomNPCMapping(t *testing.T) {
	if got := kingdomForNPC(&model.NPCDef{Name: "King_Harabard"}); got != model.KingdomHekalotia {
		t.Fatalf("Harabard=%d", got)
	}
	if got := kingdomForNPC(&model.NPCDef{Name: "King_Glantuar"}); got != model.KingdomAkelonia {
		t.Fatalf("Glantuar=%d", got)
	}
	if !isKingdomBroker(&model.NPCDef{Name: "Kingdom_Brocker"}) {
		t.Fatal("broker convertido nao foi reconhecido")
	}
}

func TestConsumeSapphiresRespectsStacksAndBundles(t *testing.T) {
	ch := &model.Char{}
	ch.Inv[0] = model.Item{Index: model.SapphireItem, Eff: [6]byte{effectAmount, 5}}
	// O itemlist define EF_AMOUNT=10 no proprio bundle. Ainda assim ele vale
	// dez Safiras, nao cem, e o slot inteiro desaparece no uso.
	ch.Inv[1] = model.Item{Index: model.SapphirePack, Eff: [6]byte{effectAmount, 10}}
	if got := sapphireCount(ch); got != 15 {
		t.Fatalf("safiras=%d, quer 15", got)
	}
	if !consumeSapphires(ch, 4) {
		t.Fatal("nao consumiu quatro safiras")
	}
	if got := sapphireCount(ch); got != 11 || itemStackAmount(ch.Inv[0]) != 1 {
		t.Fatalf("restante incorreto: total=%d item=%+v", got, ch.Inv[0])
	}
	if !consumeSapphires(ch, 11) || sapphireCount(ch) != 0 {
		t.Fatalf("nao consumiu o restante: %+v", ch.Inv[:2])
	}
}

func TestKingdomEmblemIsAlternativePaymentOnlyForNonCelestial(t *testing.T) {
	mortal := kingdomTestChar(219, "", 0)
	mortal.Equip[13] = model.Item{Index: model.KingdomEmblem}
	if !kingdomUsesEmblem(mortal) {
		t.Fatal("pre-condicao do emblema Mortal invalida")
	}
	celestial := kingdomTestChar(39, "celestial", 3199)
	celestial.Equip[13] = model.Item{Index: model.KingdomEmblem}
	if kingdomUsesEmblem(celestial) {
		t.Fatal("Celestial usaria indevidamente o emblema no lugar da regra Master")
	}
}

func TestCharacterKingdomUsesOnlyCapeSlot(t *testing.T) {
	ch := &model.Char{}
	ch.Equip[14] = model.Item{Index: 545} // montaria nunca define reino
	if got := characterKingdom(ch); got != model.KingdomNeutral {
		t.Fatalf("slot 14 definiu reino %d", got)
	}
	ch.Equip[model.CapeSlot] = model.Item{Index: 546}
	if got := characterKingdom(ch); got != model.KingdomAkelonia {
		t.Fatalf("capa definiu reino %d", got)
	}
}

func TestOnlyGuildLeaderChangesGuildKingdom(t *testing.T) {
	w := guildTestWorld(guildWithMembers(1, "Alfa",
		member("Lider", "c1", model.GuildRankLeader),
		member("Membro", "c2", model.GuildRankMember)))
	if w.setLeaderGuildKingdom(&model.Char{Name: "Membro"}, model.KingdomAkelonia) {
		t.Fatal("membro comum alterou o reino da guild")
	}
	if w.guilds.Guilds[0].Kingdom != model.KingdomNeutral {
		t.Fatal("reino mudou pelo membro")
	}
	if !w.setLeaderGuildKingdom(&model.Char{Name: "Lider"}, model.KingdomHekalotia) ||
		w.guilds.Guilds[0].Kingdom != model.KingdomHekalotia {
		t.Fatal("lider nao atualizou o reino da guild")
	}
}

func kingdomTestChar(level uint32, evolution string, cape uint16) *model.Char {
	ch := &model.Char{Evolution: evolution, Score: &model.Score{Version: 2, Level: level}}
	ch.Equip[model.CapeSlot] = model.Item{Index: cape, Eff: [6]byte{54, 16}}
	return ch
}

func TestKingdomCapeProgressionHasSecondMortalCape(t *testing.T) {
	ch := kingdomTestChar(254, "", 545)
	if _, err := kingdomCapeForJoin(ch, model.KingdomHekalotia); !errors.Is(err, errKingdomAlready) {
		t.Fatalf("nivel 255 exibido promoveu antes da hora: %v", err)
	}
	ch.Score.Level = 255
	change, err := kingdomCapeForJoin(ch, model.KingdomHekalotia)
	if err != nil || change.Index != 543 || change.Tier != model.CapeTierKnight || change.Preserve {
		t.Fatalf("segunda capa azul = %+v, %v", change, err)
	}

	ch = kingdomTestChar(255, "", 546)
	change, err = kingdomCapeForJoin(ch, model.KingdomAkelonia)
	if err != nil || change.Index != 544 || change.Tier != model.CapeTierKnight {
		t.Fatalf("segunda capa vermelha = %+v, %v", change, err)
	}
}

func TestHighLevelWithoutCapeReceivesBasicBeforeKnight(t *testing.T) {
	change, err := kingdomCapeForJoin(kingdomTestChar(399, "", 0), model.KingdomHekalotia)
	if err != nil || change.Index != 545 || change.Tier != model.CapeTierBasic {
		t.Fatalf("primeira adesao de high level = %+v, %v; quer capa basica", change, err)
	}
}

func TestKingdomCapeProgressionPreservesAdvancedTier(t *testing.T) {
	for neutral, want := range map[uint16]uint16{549: 543, 3193: 3191, 3196: 3194} {
		change, err := kingdomCapeForJoin(kingdomTestChar(399, "arch", neutral), model.KingdomHekalotia)
		if err != nil || change.Index != want || !change.Preserve {
			t.Fatalf("capa neutra %d -> %+v, %v; quer %d preservada", neutral, change, err, want)
		}
	}
}

func TestKingdomCapeProgressionCelestialUsesMaster(t *testing.T) {
	change, err := kingdomCapeForJoin(kingdomTestChar(39, "celestial", 3199), model.KingdomAkelonia)
	if err != nil || change.Index != 3198 || change.Tier != model.CapeTierMaster || !change.Preserve {
		t.Fatalf("capa Master = %+v, %v", change, err)
	}
}

func TestMortalCannotConvertNeutralMasterCape(t *testing.T) {
	_, err := kingdomCapeForJoin(kingdomTestChar(399, "", 3199), model.KingdomHekalotia)
	if !errors.Is(err, errKingdomNeedCelestial) {
		t.Fatalf("Mortal converteu capa Master: %v", err)
	}
}

func TestKingdomCapeProgressionRejectsOppositeRealm(t *testing.T) {
	_, err := kingdomCapeForJoin(kingdomTestChar(399, "", 545), model.KingdomAkelonia)
	if !errors.Is(err, errKingdomWrongRealm) {
		t.Fatalf("capa do reino oposto foi aceita: %v", err)
	}
}

func TestLindyCapeUsesCurrentKingdom(t *testing.T) {
	for cape, want := range map[uint16]uint16{0: 3193, 545: 3191, 546: 3192} {
		ch := kingdomTestChar(354, "arch", cape)
		if got := lindyCapeIndex(ch); got != want {
			t.Fatalf("Lindy com capa %d entregaria %d; quer %d", cape, got, want)
		}
	}
}
