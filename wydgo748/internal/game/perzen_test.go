package game

import (
	"bytes"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func perzenFixture(t *testing.T, name string) (*World, *Player, *craftStore, []byte) {
	t.Helper()
	w, p, store := handlerTestWorld(t)
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.items, w.skills = catalog.Items, catalog.Skills
	w.mounts, err = data.LoadMounts("../../data/content/mounts.json")
	if err != nil {
		t.Fatal(err)
	}
	npcs, geners := officialNPCDataset(t)
	def := npcs[name]
	if _, ok := perzenExchangeFor(def); !ok {
		t.Fatalf("missing usable Perzen template %q", name)
	}
	count := 0
	for _, g := range geners {
		if !g.Enabled || g.Leader != name {
			continue
		}
		count++
		origin := g.Segments[0]
		p.X, p.Y = origin.X-1, origin.Y
		w.updatePlayerSpatial(p)
		m := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: def}
		w.registerMobSpatial(m)
		p.show(m.ID)
	}
	if count != 1 {
		t.Fatalf("expected one %s generator, got %d", name, count)
	}
	w.recalcPlayer(p.Char)
	return w, p, store, useNPCPacket(1100)
}

func TestPerzenExchangesSphereAndPreservesInventory(t *testing.T) {
	for _, exchange := range []struct {
		name           string
		sphere, reward uint16
	}{{"Perzen_Normal", 4128, 3986}, {"Perzen_Myst", 4129, 3988}, {"Perzen_Arc", 4130, 3987}} {
		t.Run(exchange.name, func(t *testing.T) {
			w, p, store, packet := perzenFixture(t, exchange.name)
			p.Char.Score.Level = 399 // Mystic includes displayed level 400.
			if exchange.name != "Perzen_Myst" {
				p.Char.Score.Level = 400 // Only Mystic has a recovered level gate.
			}
			for i := range p.Char.Inv {
				p.Char.Inv[i] = circleTestItem(485, 1)
			}
			p.Char.Inv[59] = circleTestItem(exchange.sphere, 1)
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			reward := p.Char.Inv[59]
			if reward.Index != exchange.reward || reward.UID == "" || reward.UID == before.Chars[0].Inv[59].UID ||
				reward.Eff != [6]byte{} || store.saves != 1 || p.Char.Gold != before.Chars[0].Gold {
				t.Fatalf("incorrect exchange: %+v, saves=%d", reward, store.saves)
			}
			for slot, item := range p.Char.Inv {
				if slot != 59 && item != before.Chars[0].Inv[slot] {
					t.Fatalf("unrelated inventory slot %d changed", slot)
				}
			}
			update, ok := p.Session.DequeuePacketForTest()
			want := wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold)
			if !ok || !wire.Decrypt(update) || wire.ParseHeader(update).Type != wire.ParseHeader(want).Type ||
				!bytes.Equal(update[12:], want[12:]) {
				t.Fatal("client inventory does not match the committed exchange")
			}
			// Preserve the established premium-mount contract, including expiry
			// metadata following the server UID after the initial exchange.
			if reward.ActivatedUnix != 0 || reward.ExpiresUnix != 0 {
				t.Fatal("mount activated before first equip")
			}
			now := time.Unix(1_700_000_000, 0)
			if activated, err := w.activateTimedItem(&reward, mountSlot, now); err != nil || !activated ||
				reward.ExpiresUnix-reward.ActivatedUnix != int64((30*24*time.Hour)/time.Second) {
				t.Fatalf("reward cannot activate its 30-day lifetime: %+v, %v", reward, err)
			}
			after := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if store.saves != 1 || !reflect.DeepEqual(after, accountStateSnapshot(p.Account)) {
				t.Fatal("replay granted a reward without a sphere")
			}
		})
	}
}

func TestPerzenConsumesOneStackUnit(t *testing.T) {
	w, p, store, packet := perzenFixture(t, "Perzen_Normal")
	p.Char.Inv[7] = circleTestItem(4128, 3)
	before := p.Char.Inv[7]
	w.onUseNPC(p.Session, packet)
	if p.Char.Inv[0].Index != 3986 || itemStackAmount(p.Char.Inv[7]) != 2 ||
		p.Char.Inv[7].UID != before.UID || store.saves != 1 {
		t.Fatal("exchange consumed more than one sphere or replaced the remaining stack identity")
	}
}

func TestPerzenRejectsWithoutConsumption(t *testing.T) {
	for _, reason := range []string{"missing", "wrong", "hidden", "full", "level", "catalog", "dead", "range", "visibility", "save"} {
		t.Run(reason, func(t *testing.T) {
			w, p, store, packet := perzenFixture(t, "Perzen_Myst")
			p.Char.Inv[0] = circleTestItem(4129, 1)
			switch reason {
			case "missing":
				p.Char.Inv[0] = model.Item{}
			case "wrong":
				p.Char.Inv[0] = circleTestItem(4130, 1)
			case "hidden":
				p.Char.Inv[60], p.Char.Inv[0] = p.Char.Inv[0], model.Item{}
			case "full":
				for i := 1; i < circleCarrySlots; i++ {
					p.Char.Inv[i] = circleTestItem(485, 1)
				}
				p.Char.Inv[0] = circleTestItem(4129, 2)
			case "level":
				p.Char.Score.Level = 400 // Displayed 401 exceeds the official gate.
			case "catalog":
				delete(w.items, 3988)
			case "dead":
				p.Char.Score.CurHP, p.Char.RuntimeScore.CurHP = 0, 0
			case "range":
				p.X += 50
			case "visibility":
				p.Visible = nil
			case "save":
				store.err = errors.New("storage unavailable")
			}
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || reason != "save" && store.saves != 0 {
				t.Fatal("rejected or failed exchange changed the account")
			}
		})
	}
}

func TestPerzenWaitsForDurableCompletion(t *testing.T) {
	for _, outcome := range []string{"success", "rejected", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, fallback, packet := perzenFixture(t, "Perzen_Normal")
			p.Char.Inv[0] = circleTestItem(4128, 1)
			claimCriticalTestPlayers(t, w, p)
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if outcome == "rejected" {
				backend.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				backend.failure = errors.New("acknowledgement lost")
			}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			before := accountStateSnapshot(p.Account)
			w.onUseNPC(p.Session, packet)
			write := awaitCriticalWrite(t, backend.entered)
			if write.accounts[0].Chars[0].Inv[0].Index != 3986 ||
				!reflect.DeepEqual(before, accountStateSnapshot(p.Account)) ||
				p.Session.QueuedPacketsForTest() != 0 || !w.criticalTradeReserved(p) {
				t.Fatal("exchange exposed before durable acknowledgement")
			}
			w.onUseNPC(p.Session, packet)
			if len(backend.entered) != 0 {
				t.Fatal("duplicate exchange admitted while saving")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if fallback.saves != 0 {
				t.Fatal("exchange bypassed critical persistence")
			}
			if outcome == "success" {
				if !reflect.DeepEqual(write.accounts[0], accountStateSnapshot(p.Account)) ||
					w.criticalTradeReserved(p) || p.Session.QueuedPacketsForTest() == 0 {
					t.Fatal("committed exchange was not installed and published")
				}
			} else if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
				t.Fatal("failed exchange lost the sphere")
			}
			if outcome == "unknown" && (!p.PersistencePoisoned || !w.criticalTradeReserved(p)) {
				t.Fatal("unknown result was not fenced against replay")
			}
		})
	}
}

func TestPerzenDatasetAndServiceIdentity(t *testing.T) {
	npcs, _ := officialNPCDataset(t)
	for _, name := range []string{"Perzen_Normal", "Perzen_Myst", "Perzen_Arc"} {
		def := npcs[name]
		if def == nil || def.IsMonster() {
			t.Fatalf("missing neutral template %s", name)
		}
		if def.Score.Merchant&15 != 11 || def.Equip.Rosto.Index != 11 {
			t.Fatalf("%s no longer uses the existing client quest-click route", name)
		}
		if _, ok := reservedNPCKind(def); !ok {
			t.Fatalf("unreserved service %s", name)
		}
		quests := model.QuestFile{Version: model.QuestFileVersion, Quests: []model.QuestDef{simpleQuest(1, name)}}
		if _, err := indexQuests(quests, []model.NPCDef{*def}); err == nil {
			t.Fatalf("generic quest can shadow %s", name)
		}
	}
	for _, name := range []string{"Perzen", "Royal_Guard1"} {
		if _, ok := perzenExchangeFor(npcs[name]); ok {
			t.Fatalf("unverified recipe enabled for %s", name)
		}
	}
	for _, file := range []string{"../../data/spawns/NPCGener.txt"} {
		geners, err := data.LoadChannelNPCGener(file)
		if err != nil {
			t.Fatal(err)
		}
		for _, expected := range []struct {
			name string
			x, y uint16
		}{{"Perzen_Normal", 2463, 2009}, {"Perzen_Myst", 2133, 2080}, {"Perzen_Arc", 2480, 1705}} {
			count := 0
			for _, g := range geners {
				if g.Enabled && g.Leader == expected.name {
					count++
					if g.MaxNumMob != 1 || g.MinGroup != 0 || g.MaxGroup != 0 ||
						g.Segments[0].X != expected.x || g.Segments[0].Y != expected.y {
						t.Fatalf("incorrect %s generator in %s", expected.name, file)
					}
				}
			}
			if count != 1 {
				t.Fatalf("%s has %d enabled %s generators", file, count, expected.name)
			}
		}
	}
}
