package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestHuntressDiscountPurchaseTaxAndRejection(t *testing.T) {
	for _, scenario := range []string{"purchase", "save-rejection", "insufficient-gold", "inventory-full"} {
		t.Run(scenario, func(t *testing.T) {
			w, store, p, shop := cityEconomyFixture(t, model.ItemDef{Index: 400, Price: 8000})
			p.Char.Class, p.Char.LearnedSkill, p.Char.Gold = 3, 1<<(83-72), 10000
			p.Char.RuntimeScore = testScore(*p.Char.Score)
			p.Char.RuntimeScore.Mastery = [4]uint32{0, 250, 100, 250}
			switch scenario {
			case "save-rejection":
				store.err = errors.New("rejected purchase")
			case "insufficient-gold":
				p.Char.Gold = 7391
			case "inventory-full":
				for i := 0; i < model.PlayerCarrySlots; i++ {
					p.Char.Inv[i] = model.Item{Index: 400}
				}
			}
			before := cloneCharacterState(p.Char)
			request := make([]byte, 24)
			binary.LittleEndian.PutUint16(request[12:14], shop.ID)
			w.onBuyItem(p.Session, request)
			if scenario == "purchase" {
				if p.Char.Gold != 2608 || p.Char.Inv[0].Index != 400 ||
					w.guilds.Wars.Cities.Territories[0].Treasury != 168 || store.gameSaves != 1 {
					t.Fatalf("discounted purchase gold=%d treasury=%d", p.Char.Gold, w.guilds.Wars.Cities.Territories[0].Treasury)
				}
			} else if p.Char.Gold != before.Gold || p.Char.Inv != before.Inv ||
				w.guilds.Wars.Cities.Territories[0].Treasury != 0 {
				t.Fatal("rejected discounted purchase changed state")
			}
		})
	}
}

func TestCriticalHuntressDiscountSettlesOnlyAfterAcknowledgement(t *testing.T) {
	w, p, _, shop, backend, queue, release := criticalShopFixture(t, "buy")
	p.Char.Class, p.Char.LearnedSkill = 3, 1<<(83-72)
	p.Char.RuntimeScore = testScore(*p.Char.Score)
	p.Char.RuntimeScore.Mastery[2] = 100
	request := criticalShopRequest("buy", p.ID, shop.ID)
	w.onBuyItem(p.Session, request)
	proposal := awaitCriticalWrite(t, backend.entered)
	if proposal.accounts[0].Chars[0].Gold != 9160 || p.Char.Gold != 10000 {
		t.Fatal("discount proposal or live gold differs")
	}
	w.onBuyItem(p.Session, request)
	if len(queue.pending) != 1 || len(backend.entered) != 0 {
		t.Fatal("pending discounted purchase repeated")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Gold != 9160 || p.Char.Inv[1].Index != 400 {
		t.Fatal("discounted acknowledgement lost state")
	}
}

func TestCriticalHuntressLifeDrainSettlesWithKill(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		w, p, mob, _ := killRewardTestWorld(t, nil)
		p.Char.Class, p.Char.LearnedSkill = 3, 1<<(90-72)
		p.Char.Score.MaxHP, p.Char.Score.CurHP, p.Char.Score.Mastery[3] = 1000, 100, 90
		p.Char.RuntimeScore = testScore(*p.Char.Score)
		claimCriticalTestPlayers(t, w, p)
		gate, release := criticalGate(t)
		backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
		if rejected {
			backend.failure = criticalCertifiedRejection(t)
		}
		queue := newCriticalTestQueue(t, backend, 1, 4)
		t.Cleanup(release)
		w.criticalWrites = newCriticalPersistenceOwner(queue)
		w.killMobState(p, mob, 500, 100)
		proposal := awaitCriticalWrite(t, backend.entered)
		if proposal.accounts[0].Chars[0].Score.CurHP != 192 || playerCurHP(p.Char) != 100 {
			t.Fatal("Life Drain did not remain inside the detached kill proposal")
		}
		w.killMobState(p, mob, 500, 100)
		if len(queue.pending) != 1 {
			t.Fatal("pending kill replay submitted another heal")
		}
		release()
		awaitReadyCriticalResults(t, queue, 1)
		if playerCurHP(p.Char) != 100 {
			t.Fatal("heal published before acknowledgement")
		}
		w.processCriticalCompletions(1)
		want := uint32(192)
		if rejected {
			want = 100
		}
		if playerCurHP(p.Char) != want {
			t.Fatalf("rejected=%v HP=%d want=%d", rejected, playerCurHP(p.Char), want)
		}
	}
}

func TestArmorCriticalAlternateReplacesCatalog(t *testing.T) {
	items := necklaceCatalog(t)
	var armor model.ItemDef
	for _, def := range items {
		if isArmorSetPiece(def) && staticAbility(def, "EF_CRITICAL") == 45 {
			armor = def
			break
		}
	}
	if armor.Index == 0 {
		t.Fatal("missing real armor with 4.5% critical")
	}
	w := &World{items: items}
	for _, tc := range []struct {
		base, sanc byte
		want       int
	}{
		{45, 9, 85}, {50, 9, 95}, {55, 9, 104}, {60, 9, 114},
		{70, 9, 133}, {70, 230, 140}, {70, 234, 154},
	} {
		for _, slot := range []int{1, 2} {
			item := model.Item{Index: armor.Index, Eff: [6]byte{43, tc.sanc}}
			if tc.base != 45 {
				item.Eff[2*slot], item.Eff[2*slot+1] = 71, tc.base
			}
			if got := itemAbility(item, armor, "EF_CRITICAL"); got != tc.want {
				t.Fatalf("base=%d sanc=%d slot=%d critical=%d want=%d", tc.base, tc.sanc, slot, got, tc.want)
			}
			ch := &model.Char{Score: testScore(model.Score{MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100})}
			w.recalcPlayer(ch)
			before := effectiveScore(ch).Critical
			ch.Equip[1] = item
			for repeat := 0; repeat < 3; repeat++ {
				w.recalcPlayer(ch)
				if got := int(effectiveScore(ch).Critical - before); got != tc.want {
					t.Fatalf("recalc repeated=%d critical delta=%d want=%d", repeat, got, tc.want)
				}
			}
			ch.Equip[1] = model.Item{}
			w.recalcPlayer(ch)
			if effectiveScore(ch).Critical != before {
				t.Fatal("unequip retained armor critical")
			}
		}
	}
}

func TestMagicAdditionMatchesClientEligibilityAndRefinement(t *testing.T) {
	for _, unique := range []int{0, 40, 41, 45, 50, 51} {
		def := model.ItemDef{Unique: unique, DynamicEffectNames: map[byte]string{68: "EF_MAGICADD"},
			StaticEffects: []model.StaticEffect{{Name: "EF_MAGIC", Value: 20}}}
		item := model.Item{Eff: [6]byte{43, 9, 68, 12}}
		want := 0
		if unique >= 41 && unique <= 50 {
			want = 22
		}
		if got := itemAbility(item, def, "EF_MAGICADD"); got != want {
			t.Fatalf("unique=%d magic add=%d want=%d", unique, got, want)
		}
		if got := itemAbility(item, def, "EF_MAGIC"); got != 38 {
			t.Fatalf("static magic=%d want=38", got)
		}
	}
}

func TestHuntressLifeDrainKillCommitRollbackAndReplay(t *testing.T) {
	for _, reject := range []bool{false, true} {
		var err error
		if reject {
			err = errors.New("rejected reward")
		}
		w, p, mob, store := killRewardTestWorld(t, err)
		p.Char.Class, p.Char.LearnedSkill = 3, 1<<(90-72)
		p.Char.Score.MaxHP, p.Char.Score.CurHP, p.Char.Score.Mastery[3] = 1000, 100, 90
		p.Char.RuntimeScore = testScore(*p.Char.Score)
		w.killMobState(p, mob, 500, 100)
		want := uint32(192) // 100 monster HP / 50 + 90 Capture.
		if reject {
			want = 100
		}
		if got := playerCurHP(p.Char); got != want {
			t.Fatalf("rejected=%v HP=%d want=%d", reject, got, want)
		}
		if !reject {
			w.killMobState(p, mob, 500, 100)
			if playerCurHP(p.Char) != want || store.saves != 1 {
				t.Fatal("replayed death healed twice")
			}
		}
	}
}

func TestHuntressSurvivalSpectralAndRegeneration(t *testing.T) {
	w := &World{}
	ch := huntressSkillChar(0)
	ch.Affects[0] = model.Affect{Type: 37, ExpiresAt: time.Now().Add(time.Minute)}
	w.applyAffectStats(ch)
	if playerAttack(ch) != 530 {
		t.Fatalf("Spectral attack=%d want=530", playerAttack(ch))
	}
	ch.RuntimeScore = testScore(model.Score{MaxHP: 1000, CurHP: 100, MaxMP: 100, CurMP: 100,
		Mastery: [4]uint32{0, 100, 200, 250}})
	ch.Affects[0] = model.Affect{Type: 28, ExpiresAt: time.Now().Add(time.Minute)}
	applyPlayerRegen(ch)
	if playerCurHP(ch) != 112 {
		t.Fatalf("invisible HP=%d want=112", playerCurHP(ch))
	}
}

func TestHuntressVisionUsesCaptureDropBonus(t *testing.T) {
	w := &World{}
	ch := huntressSkillChar(1 << (91 - 72))
	p := &Player{Char: ch}
	if got := w.dropBonusFor(p); got != 19 {
		t.Fatalf("Vision drop bonus=%d want=19", got)
	}
	ch.LearnedSkill = 0
	if w.dropBonusFor(p) != 0 {
		t.Fatal("Vision bonus remains without skill")
	}
}
