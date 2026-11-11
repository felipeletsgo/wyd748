package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func criticalGateQueueFixture(t *testing.T) (*World, *Player, *GroundItem, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	w.items = map[uint16]model.ItemDef{
		451: {Index: 451, Name: "First_Gate_Key", StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: 2}}},
		458: {Index: 458, Name: "First_Gate", StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: 2}}},
	}
	w.groundItems = make(map[uint16]*GroundItem)
	gate := &GroundItem{ID: 10000, Item: model.Item{Index: 458}, X: p.X, Y: p.Y, Permanent: true, State: gateClosed}
	w.groundItems[gate.ID] = gate
	p.Char.Inv[0] = model.Item{Index: 451, UID: "critical-gate-key"}
	return w, p, gate, backend, queue, release
}

func TestCriticalGateKeyCommitsGateAndKeyTogether(t *testing.T) {
	w, p, gate, backend, queue, release := criticalGateQueueFixture(t)
	before := accountStateSnapshot(p.Account)
	key := p.Char.Inv[0]

	if err := w.queueGateKey(p, gate, 0, true, 3); err != nil {
		t.Fatalf("queue gate opening: %v", err)
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	if proposal.accounts[0].Chars[p.CharSlot].Inv[0].Index != 0 ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Char.Inv[0] != key ||
		gate.State != gateClosed || !gate.criticalReserved() {
		t.Fatal("gate or key changed before the durable write settled")
	}

	settleCriticalCelestialWrite(t, w, queue, release)
	if p.Char.Inv[0].Index != 0 || gate.State != gateOpen || gate.criticalReserved() || w.criticalPlayerReserved(p) {
		t.Fatalf("gate/key commit did not settle together: key=%+v gate=%+v", p.Char.Inv[0], gate)
	}
}

func TestCriticalGateKeyCertifiedRejectionRestoresAdmission(t *testing.T) {
	w, p, gate, backend, queue, release := criticalGateQueueFixture(t)
	backend.failure = criticalCertifiedRejection(t)
	before := accountStateSnapshot(p.Account)
	key := p.Char.Inv[0]

	if err := w.queueGateKey(p, gate, 0, true, 3); err != nil {
		t.Fatalf("queue gate opening: %v", err)
	}
	awaitCriticalWrite(t, backend.entered)
	if !gate.criticalReserved() || gate.State != gateClosed || p.Char.Inv[0] != key {
		t.Fatal("gate reservation did not fence the object while persistence was pending")
	}
	settleCriticalCelestialWrite(t, w, queue, release)
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Char.Inv[0] != key ||
		gate.State != gateClosed || gate.criticalReserved() || w.criticalPlayerReserved(p) {
		t.Fatal("certified rejection did not preserve the key and release the gate")
	}
}

func TestCriticalGateKeyRejectsInvalidAdmissions(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*World, *Player, *GroundItem)
		slot   byte
		busy   bool
	}{
		{name: "invalid slot", slot: byte(model.PlayerCarrySlots)},
		{name: "unregistered gate", mutate: func(w *World, _ *Player, gate *GroundItem) { delete(w.groundItems, gate.ID) }},
		{name: "temporary gate", mutate: func(_ *World, _ *Player, gate *GroundItem) { gate.Permanent = false }},
		{name: "already open", mutate: func(_ *World, _ *Player, gate *GroundItem) { gate.State = gateOpen }},
		{name: "dead player", mutate: func(_ *World, p *Player, _ *GroundItem) { setPlayerCurHP(p.Char, 0) }},
		{name: "out of reach", mutate: func(_ *World, p *Player, gate *GroundItem) { p.X = gate.X + uint16(gateReach) + 1 }},
		{name: "gate reserved", mutate: func(_ *World, _ *Player, gate *GroundItem) { gate.criticalGate = true }, busy: true},
		{name: "wrong key id", mutate: func(w *World, _ *Player, _ *GroundItem) {
			def := w.items[458]
			def.StaticEffects[0].Value = 3
			w.items[458] = def
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, gate, _, queue, _ := criticalGateQueueFixture(t)
			if test.mutate != nil {
				test.mutate(w, p, gate)
			}
			before := accountStateSnapshot(p.Account)
			err := w.queueGateKey(p, gate, test.slot, true, 3)
			if test.busy {
				if !errors.Is(err, errCriticalPersistenceBusy) {
					t.Fatalf("gate admission error = %v, want persistence busy", err)
				}
			} else if err == nil {
				t.Fatal("invalid gate admission succeeded")
			}
			if len(queue.pending) != 0 || p.Char.Inv[0].Index != 451 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("rejected gate admission changed inventory, account, or persistence state")
			}
		})
	}
}

func TestCriticalContractSummonCommitsOnlyAfterAcknowledgement(t *testing.T) {
	for _, consume := range []bool{true, false} {
		t.Run(map[bool]string{true: "consume", false: "retain"}[consume], func(t *testing.T) {
			w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
			p.Char.Inv[0] = model.Item{Index: 900, UID: "contract-token"}
			rule := model.VolatileRule{Consume: consume, Summon: &model.VolatileSummon{
				Name: "Bound Guard", Face: 206, HP: 1200, Defense: 70, MoveSpeed: 8, Attack: 240, AttackRange: 3,
				Tank: true, Refinement: 7, Weapon: 1100, Shield: 1101, Armor: [5]uint16{1102, 1103, 1104, 1105, 1106},
			}}
			before := accountStateSnapshot(p.Account)
			key := p.Char.Inv[0]

			if err := w.queueContractSummon(p, 0, rule); err != nil {
				t.Fatalf("queue contract summon: %v", err)
			}
			proposal := awaitCriticalWrite(t, backend.entered)
			if len(w.criticalSummonIDs) != 1 || len(w.summons) != 0 || len(w.mobs) != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Char.Inv[0] != key {
				t.Fatal("pending contract creation changed live item state or published its entity")
			}
			var reserved *Mob
			for _, candidate := range w.criticalSummonIDs {
				reserved = candidate
			}
			if reserved == nil || proposal.accounts[0].Chars[p.CharSlot].Inv[0].Index != map[bool]uint16{true: 0, false: 900}[consume] ||
				reserved.SummonKind != summonKindContract || reserved.guardKind != guardAsder || reserved.Def.Score.MaxHP <= rule.Summon.HP {
				t.Fatalf("incorrect detached contract proposal: summon=%+v accountItem=%+v", reserved, proposal.accounts[0].Chars[p.CharSlot].Inv[0])
			}

			settleCriticalCelestialWrite(t, w, queue, release)
			if w.mobsByID[reserved.ID] != reserved || len(w.summons) != 1 || w.summons[reserved.ID] != reserved ||
				len(w.criticalSummonIDs) != 0 || p.Char.Inv[0].Index != map[bool]uint16{true: 0, false: 900}[consume] {
				t.Fatal("committed contract summon did not publish its entity and item result")
			}
		})
	}
}

func TestCriticalContractSummonRejectionReleasesDetachedEntity(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	backend.failure = criticalCertifiedRejection(t)
	p.Char.Inv[0] = model.Item{Index: 900, UID: "contract-token"}
	rule := model.VolatileRule{Consume: true, Summon: &model.VolatileSummon{Name: "Bound Guard", Face: 206, HP: 1200}}
	before := accountStateSnapshot(p.Account)
	key := p.Char.Inv[0]

	if err := w.queueContractSummon(p, 0, rule); err != nil {
		t.Fatalf("queue contract summon: %v", err)
	}
	awaitCriticalWrite(t, backend.entered)
	if len(w.criticalSummonIDs) != 1 || p.Char.Inv[0] != key {
		t.Fatal("pending summon did not retain its detached ID and source item")
	}
	settleCriticalCelestialWrite(t, w, queue, release)
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Char.Inv[0] != key ||
		len(w.criticalSummonIDs) != 0 || len(w.summons) != 0 || len(w.mobs) != 0 || w.criticalPlayerReserved(p) {
		t.Fatal("certified rejection consumed the contract or retained its summon allocation")
	}
}

func TestCriticalContractSummonAdmissionResyncsRejectedItem(t *testing.T) {
	w, p, backend, queue, release := criticalCelestialQueueFixture(t, 399)
	p.Char.Inv[0] = model.Item{Index: 900, UID: "contract-token"}
	key := p.Char.Inv[0]
	w.admitContractSummon(p, 0, model.VolatileRule{}) // Missing template is rejected before persistence.
	if p.Session.QueuedPacketsForTest() == 0 || len(queue.pending) != 0 || len(backend.entered) != 0 || p.Char.Inv[0] != key {
		t.Fatal("invalid contract summon did not return its item without a write")
	}

	for {
		if _, ok := p.Session.DequeuePacketForTest(); !ok {
			break
		}
	}
	if err := w.queueCriticalAccountMutation(p, accountStateSnapshot(p.Account), "hold contract admission", "", func() {}, nil); err != nil {
		t.Fatalf("queue persistence hold: %v", err)
	}
	awaitCriticalWrite(t, backend.entered)
	w.admitContractSummon(p, 0, model.VolatileRule{Summon: &model.VolatileSummon{Name: "Guard", Face: 206, HP: 100}})
	if p.Session.QueuedPacketsForTest() == 0 || len(queue.pending) != 1 || len(w.criticalSummonIDs) != 0 || p.Char.Inv[0] != key {
		t.Fatal("busy contract summon admission changed state or failed to resynchronize")
	}
	settleCriticalCelestialWrite(t, w, queue, release)
}
