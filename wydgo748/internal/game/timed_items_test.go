package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func premiumTestDef(index uint16, slot byte) model.ItemDef {
	return model.ItemDef{
		Index: index,
		Pos:   1 << slot,
		StaticEffects: []model.StaticEffect{
			{Name: "EF_CLASS", Value: 15},
			{Name: premiumTimedMarker, Value: 1},
			{Name: "EF_WDAY", Value: 30},
			{Name: "EF_AC", Value: 80},
			{Name: "EF_SAVEMANA", Value: 10},
			{Name: "EF_RUNSPEED", Value: 6},
			{Name: "EF_DAMAGE", Value: 520},
			{Name: "EF_MAGIC", Value: 65},
		},
	}
}

func timedSwapPacket(srcType, srcPos, dstType, dstPos byte) []byte {
	pkt := make([]byte, 20)
	pkt[12], pkt[13], pkt[14], pkt[15] = srcType, srcPos, dstType, dstPos
	return pkt
}

func newTimedSwapWorld(t *testing.T, saveErr error) (*World, *Player, *craftStore, *net.Session) {
	t.Helper()
	st := &craftStore{err: saveErr}
	item := model.Item{Index: 4300, UID: "33333333333343338333333333333333"}
	acc := &model.Account{Name: "premium", Chars: []model.Char{{
		Name:  "Premium",
		Class: 0,
		Inv:   [64]model.Item{item},
		Score: &model.Score{
			Version: model.ScoreVersion,
		},
	}}}
	session := net.NewTestSession(1, 64)
	p := &Player{Session: session, Account: acc, Char: &acc.Chars[0], CharSlot: 0, ID: 1, InWorld: true}
	w := &World{
		store:   st,
		items:   map[uint16]model.ItemDef{4300: premiumTestDef(4300, fairySlot)},
		players: map[*net.Session]*Player{session: p},
		clock:   newFakeClock(time.Unix(1_700_000_000, 0).UTC()),
	}
	return w, p, st, session
}

func TestPremiumFirstEquipPersistsDeadlineBeforeConfirm(t *testing.T) {
	w, p, st, session := newTimedSwapWorld(t, nil)
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	item := p.Char.Equip[fairySlot]
	if item.Index != 4300 || item.ActivatedUnix != w.now().Unix() ||
		item.ExpiresUnix != w.now().Add(30*24*time.Hour).Unix() {
		t.Fatalf("premium not activated: %+v", item)
	}
	if p.Char.Inv[0].Index != 0 || st.saves != 1 {
		t.Fatalf("swap not committed exactly once: inv=%+v saves=%d", p.Char.Inv[0], st.saves)
	}
}

func costumeSlotTestWorld(t *testing.T) (*World, *Player, *craftStore, *net.Session) {
	t.Helper()
	w, p, st, session := newTimedSwapWorld(t, nil)
	def := premiumTestDef(4300, model.CostumeSlot)
	def.Mesh = model.CostumeMesh
	w.items[4300] = def
	w.items[3900] = model.ItemDef{Index: 3900, Mesh: 2773, Pos: model.NativeCostumeSlotMask,
		StaticEffects: []model.StaticEffect{{Name: "EF_WDAY", Value: 3}}}
	return w, p, st, session
}

// Costume-slot extension: costumes equip and time out in slot 12, beside a
// familiar in slot 13; slot 13 no longer accepts them.
func TestCostumeSlotEquipsBesideFamiliar(t *testing.T) {
	w, p, st, session := costumeSlotTestWorld(t)
	p.Char.Inv[1] = model.Item{Index: 3900, UID: "44444444444444448444444444444444"}
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	if p.Char.Equip[fairySlot].Index != 0 || p.Char.Inv[0].Index != 4300 {
		t.Fatalf("slot 13 accepted a costume: equip=%+v", p.Char.Equip[fairySlot])
	}
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, model.CostumeSlot))
	w.onSwapItem(session, timedSwapPacket(placeInv, 1, placeEquip, fairySlot))
	costume, familiar := p.Char.Equip[model.CostumeSlot], p.Char.Equip[fairySlot]
	if costume.Index != 4300 || costume.ExpiresUnix != w.now().Add(30*24*time.Hour).Unix() ||
		familiar.Index != 3900 || familiar.ExpiresUnix != w.now().Add(3*24*time.Hour).Unix() || st.saves != 2 {
		t.Fatalf("costume=%+v familiar=%+v saves=%d", costume, familiar, st.saves)
	}
}

func TestEnterMigratesLegacyCostumeSlotAndKeepsDeadline(t *testing.T) {
	w, p, st, _ := costumeSlotTestWorld(t)
	now := w.now()
	legacy := model.Item{Index: 4300, UID: "33333333333343338333333333333333",
		ActivatedUnix: now.Add(-time.Hour).Unix(), ExpiresUnix: now.Add(time.Hour).Unix()}
	p.Char.Inv[0], p.Char.Equip[fairySlot] = model.Item{}, legacy
	if err := w.prepareTimedEquipmentForEnter(p, now); err != nil {
		t.Fatal(err)
	}
	if p.Char.Equip[model.CostumeSlot] != legacy || p.Char.Equip[fairySlot].Index != 0 || st.saves != 1 {
		t.Fatalf("costume=%+v slot13=%+v saves=%d", p.Char.Equip[model.CostumeSlot], p.Char.Equip[fairySlot], st.saves)
	}
	if err := w.prepareTimedEquipmentForEnter(p, now); err != nil || st.saves != 1 {
		t.Fatalf("migration repeated: err=%v saves=%d", err, st.saves)
	}
}

// Timed horses carry no stats or lifetime in the item list; the mount catalog
// supplies the client g_pMountBonus2 values, final speed 6 and the deadline.
func TestPremiumHorseUsesCatalogSpeedBonusAndLifetime(t *testing.T) {
	w, p, st, session := newTimedSwapWorld(t, nil)
	w.items[3980] = model.ItemDef{Index: 3980, Pos: 1 << mountSlot,
		StaticEffects: []model.StaticEffect{{Name: "EF_CLASS", Value: 255}, {Name: premiumTimedMarker, Value: 1}}}
	w.mounts = model.MountCatalog{Premium: map[uint16]model.PremiumMountStats{
		3980: {Attack: 35, MagicPercent: 7, Evasion: 3, Resist: 4, Speed: 6, DurationDays: 3}}}
	p.Char.Inv[0] = model.Item{Index: 3980, UID: "55555555555555558555555555555555"}
	w.recalcPlayer(p.Char)
	before := *p.Char.RuntimeScore

	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, mountSlot))
	horse := p.Char.Equip[mountSlot]
	if horse.Index != 3980 || horse.ExpiresUnix != w.now().Add(3*24*time.Hour).Unix() || st.saves != 1 {
		t.Fatalf("horse not activated with its catalog lifetime: %+v saves=%d", horse, st.saves)
	}
	after := p.Char.RuntimeScore
	if after.AttackRun&0x0F != 6 || after.Attack != before.Attack+35 ||
		after.Evasion != before.Evasion+3 || after.ResistFire != before.ResistFire+4 {
		t.Fatalf("horse bonus not applied: before=%+v after=%+v", before, *after)
	}

	w.clock.(*fakeClock).Advance(3*24*time.Hour + time.Second)
	w.recalcPlayer(p.Char)
	if p.Char.RuntimeScore.Attack != before.Attack {
		t.Fatal("expired horse kept its bonus")
	}
}

func TestPremiumFirstEquipSaveFailureRollsBackItemAndDeadline(t *testing.T) {
	w, p, st, session := newTimedSwapWorld(t, errors.New("db unavailable"))
	before := p.Char.Inv[0]
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	if p.Char.Inv[0] != before || p.Char.Equip[fairySlot] != (model.Item{}) || st.saves != 1 {
		t.Fatalf("failed activation leaked: inv=%+v equip=%+v saves=%d",
			p.Char.Inv[0], p.Char.Equip[fairySlot], st.saves)
	}
}

func TestTimedEquipmentRelogDoesNotRestartUIDDeadline(t *testing.T) {
	w, p, st, _ := newTimedSwapWorld(t, nil)
	now := w.now()
	existing := model.Item{
		Index: 4300, UID: "33333333333343338333333333333333",
		ActivatedUnix: now.Add(-7 * 24 * time.Hour).Unix(),
		ExpiresUnix:   now.Add(23 * 24 * time.Hour).Unix(),
	}
	p.Char.Inv[0] = model.Item{}
	p.Char.Equip[fairySlot] = existing

	if err := w.prepareTimedEquipmentForEnter(p, now); err != nil {
		t.Fatalf("prepare timed equipment: %v", err)
	}
	if p.Char.Equip[fairySlot] != existing {
		t.Fatalf("relog restarted UID deadline: got=%+v want=%+v", p.Char.Equip[fairySlot], existing)
	}
	if st.saves != 0 {
		t.Fatalf("unchanged deadline caused %d saves", st.saves)
	}
}

func TestTimedExpiryPersistsBeforeRemovalAndRollsBackOnFailure(t *testing.T) {
	for _, tc := range []struct {
		name    string
		saveErr error
		removed bool
	}{
		{name: "commit", removed: true},
		{name: "rollback", saveErr: errors.New("db unavailable")},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st, _ := newTimedSwapWorld(t, tc.saveErr)
			p.Char.Inv[0] = model.Item{}
			p.Char.Equip[fairySlot] = model.Item{
				Index: 4300, UID: "33333333333343338333333333333333",
				ActivatedUnix: w.now().Add(-31 * 24 * time.Hour).Unix(),
				ExpiresUnix:   w.now().Add(-24 * time.Hour).Unix(),
			}
			before := p.Char.Equip[fairySlot]
			w.tickTimedItems(w.now())
			if st.saves != 1 {
				t.Fatalf("saves=%d", st.saves)
			}
			if tc.removed && p.Char.Equip[fairySlot] != (model.Item{}) {
				t.Fatalf("expired item survived commit: %+v", p.Char.Equip[fairySlot])
			}
			if !tc.removed && p.Char.Equip[fairySlot] != before {
				t.Fatalf("failed expiry was not rolled back: %+v", p.Char.Equip[fairySlot])
			}
		})
	}
}

func TestTimedExpiryConsumesUIDFromInventoryCargoAndInactiveCharacter(t *testing.T) {
	w, p, st, _ := newTimedSwapWorld(t, nil)
	expired := model.Item{
		Index: 4300, UID: "33333333333343338333333333333333",
		ActivatedUnix: w.now().Add(-31 * 24 * time.Hour).Unix(),
		ExpiresUnix:   w.now().Add(-24 * time.Hour).Unix(),
	}
	p.Char.Inv[0] = expired
	p.Account.Cargo[1] = expired
	p.Account.Cargo[1].UID = "44444444444444448444444444444444"
	p.Account.Chars = append(p.Account.Chars, model.Char{Name: "Offline"})
	p.Char = &p.Account.Chars[0]
	p.Account.Chars[1].Inv[2] = expired
	p.Account.Chars[1].Inv[2].UID = "55555555555545558555555555555555"

	w.tickTimedItems(w.now())
	if st.saves != 1 || p.Char.Inv[0] != (model.Item{}) ||
		p.Account.Cargo[1] != (model.Item{}) || p.Account.Chars[1].Inv[2] != (model.Item{}) {
		t.Fatalf("expired UID survived: saves=%d inv=%+v cargo=%+v offline=%+v",
			st.saves, p.Char.Inv[0], p.Account.Cargo[1], p.Account.Chars[1].Inv[2])
	}
}

func TestPremiumMountCatalogBonusIsExactAndExpires(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{4200: premiumTestDef(4200, mountSlot)}}
	now := time.Unix(1_700_000_000, 0).UTC()
	ch := &model.Char{}
	ch.Equip[mountSlot] = model.Item{
		Index: 4200, UID: "44444444444444448444444444444444",
		ActivatedUnix: now.Unix(), ExpiresUnix: now.Add(30 * 24 * time.Hour).Unix(),
	}
	if got := w.premiumMountBonus(ch, now); got != (premiumMountStats{attack: 520, magicPercent: 65, runSpeed: 6}) {
		t.Fatalf("premium mount bonus=%+v", got)
	}
	if got := w.premiumMountBonus(ch, now.Add(30*24*time.Hour)); got != (premiumMountStats{}) {
		t.Fatalf("expired premium mount bonus=%+v", got)
	}
}

func TestPremiumMountRecalcAppliesExactAttackMagicAndSpeed(t *testing.T) {
	now := time.Unix(1_700_000_000, 0).UTC()
	w := &World{
		items: map[uint16]model.ItemDef{4200: premiumTestDef(4200, mountSlot)},
		clock: newFakeClock(now),
	}
	newChar := func() *model.Char {
		return &model.Char{Class: 1, Score: &model.Score{
			Version: model.ScoreVersion,
			Attack:  100, MagicAttack: 200, AttackRun: 1,
			MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100,
		}}
	}
	plain := newChar()
	w.recalcPlayer(plain)
	premium := newChar()
	premium.Equip[mountSlot] = model.Item{
		Index: 4200, UID: "44444444444444448444444444444444",
		ActivatedUnix: now.Unix(), ExpiresUnix: now.Add(30 * 24 * time.Hour).Unix(),
	}
	w.recalcPlayer(premium)
	if got, want := playerAttack(premium), playerAttack(plain)+520; got != want {
		t.Fatalf("attack=%d want=%d", got, want)
	}
	if got, want := playerMagicAttack(premium), playerMagicAttack(plain)*165/100; got != want {
		t.Fatalf("magic attack=%d want=%d", got, want)
	}
	if got, want := int(playerAttackRun(premium)&0x0f), 6; got != want {
		t.Fatalf("run speed=%d want=%d", got, want)
	}
}
