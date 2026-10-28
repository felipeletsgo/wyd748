package game

import (
	"context"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"sync/atomic"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/npccontent"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

func TestNPCDefinitionUpdateIsolatesChannelsWithEqualEntityIDs(t *testing.T) {
	source, err := data.LoadNPCs("../../data/npcs/Aylin.json")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	original, err := json.Marshal(source[0])
	if err != nil {
		t.Fatal(err)
	}
	worlds := make([]*World, 2)
	for i := range worlds {
		w := &World{channel: uint8(i + 1), npcs: data.CloneNPCs(source), items: catalog.Items,
			npcDefinitionState: &npcDefinitionRuntime{
				source: map[string]json.RawMessage{"Aylin": original}, active: map[string]bool{"Aylin": true},
				versions: map[string]uint32{"Aylin": 1},
			},
		}
		w.mobsByID = map[uint16]*Mob{1300: {ID: 1300, Def: &w.npcs[0], X: 2468, Y: 1996, HP: w.npcs[0].Score.MaxHP, InstanceID: "local-instance"}}
		worlds[i] = w
	}
	changed := data.CloneNPCs(source)[0]
	changed.ExpReward++
	changed.Score.MaxHP--
	changed.Score.CurHP = changed.Score.MaxHP
	raw, err := json.Marshal(changed)
	if err != nil {
		t.Fatal(err)
	}
	entry := store.NPCDefinitionRecord{Channel: 1, Name: "Aylin", Version: 2, ChangeID: 1, Definition: raw}
	for _, w := range worlds {
		w.installNPCDefinitionChanges([]store.NPCDefinitionRecord{entry})
	}
	first, second := worlds[0].mobsByID[1300], worlds[1].mobsByID[1300]
	if first.Def != &worlds[0].npcs[0] || first.X != 2468 || first.Y != 1996 || first.InstanceID != "local-instance" ||
		first.HP != changed.Score.MaxHP || first.Def.ExpReward != changed.ExpReward {
		t.Fatalf("live entity was replaced or not updated: %+v", first)
	}
	if second.Def.ExpReward != source[0].ExpReward || second.HP != source[0].Score.MaxHP ||
		second.Def.Score.MaxHP != source[0].Score.MaxHP || worlds[1].npcDefinitionState.versions["Aylin"] != 1 {
		t.Fatal("update crossed channel boundary or mutated the source")
	}
	// Replayed and invalid revisions cannot replace the accepted definition.
	entry.Definition = original
	worlds[0].installNPCDefinitionChanges([]store.NPCDefinitionRecord{entry})
	entry.Version = 3
	changed.Name = "AnotherNPC"
	entry.Definition, _ = json.Marshal(changed)
	worlds[0].installNPCDefinitionChanges([]store.NPCDefinitionRecord{entry})
	if first.Def.Name != "Aylin" || first.Def.ExpReward != source[0].ExpReward+1 || worlds[0].npcDefinitionState.versions["Aylin"] != 2 {
		t.Fatal("stale or identity-changing revision replaced the live definition")
	}
}

func TestNPCStockEditRejectsOldGridUntilReopen(t *testing.T) {
	w, p, backend := handlerTestWorld(t)
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	defs, err := data.LoadNPCs("../../data/npcs/Aki.json")
	if err != nil {
		t.Fatal(err)
	}
	defs[0].Vende = []model.Item{{Index: 411}}
	w.npcs, w.items, w.channel = defs, catalog.Items, 1
	raw, _ := json.Marshal(defs[0])
	w.npcDefinitionState = &npcDefinitionRuntime{source: map[string]json.RawMessage{"Aki": raw}, active: map[string]bool{"Aki": true}, versions: map[string]uint32{"Aki": 1}}
	shop := &Mob{ID: 1100, X: 2101, Y: 2100, Def: &w.npcs[0], HP: w.npcs[0].Score.MaxHP}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.ShopNPC, p.Char.Gold, p.Char.Inv = shop.ID, 2_000_000_000, [64]model.Item{}
	request := make([]byte, 24)
	binary.LittleEndian.PutUint16(request[12:14], shop.ID)
	next := data.CloneNPCs(w.npcs)[0]
	next.Vende = []model.Item{{Index: 412}}
	change, _ := json.Marshal(next)
	w.installNPCDefinitionChanges([]store.NPCDefinitionRecord{{Channel: 1, Name: "Aki", Version: 2, ChangeID: 8, Definition: change}})
	w.onBuyItem(p.Session, request)
	if p.ShopNPC != 0 || p.Char.Inv[0].Index != 0 || p.Char.Gold != 2_000_000_000 || backend.saves != 0 {
		t.Fatal("old grid request bought replacement stock")
	}
	// Only an explicit validated interaction may reopen the merchant.
	reopen := make([]byte, 20)
	binary.LittleEndian.PutUint16(reopen[12:14], shop.ID)
	w.onUseNPC(p.Session, reopen)
	if p.ShopNPC != shop.ID {
		t.Fatal("merchant could not reopen")
	}
	w.onBuyItem(p.Session, request)
	if p.Char.Inv[0].Index != 412 || backend.saves != 1 {
		t.Fatal("reopened shop did not use the new stock")
	}
}

func TestNPCPointNotificationDoesNotSkipPollRevisions(t *testing.T) {
	w := &World{channel: 1, npcDefinitionState: &npcDefinitionRuntime{lastChange: 3}}
	w.npcDefinitionState.polling.Store(true)
	w.handleNPCDefinitionEvent(&npcDefinitionEvent{entries: []store.NPCDefinitionRecord{{Channel: 1, ChangeID: 9}}})
	if w.npcDefinitionState.lastChange != 3 || !w.npcDefinitionState.polling.Load() {
		t.Fatal("point notification changed the poll checkpoint")
	}
	w.handleNPCDefinitionEvent(&npcDefinitionEvent{poll: true, entries: []store.NPCDefinitionRecord{{Channel: 1, ChangeID: 5}, {Channel: 2, ChangeID: 20}}})
	if w.npcDefinitionState.lastChange != 5 || w.npcDefinitionState.polling.Load() {
		t.Fatal("poll checkpoint did not follow its own channel batch")
	}
	w.npcDefinitionState.polling.Store(true)
	w.handleNPCDefinitionEvent(&npcDefinitionEvent{poll: true})
	if w.npcDefinitionState.polling.Load() {
		t.Fatal("empty poll did not release polling")
	}
}

type npcPollTestStore struct {
	*craftStore
	store.NPCDefinitionStore
	calls   atomic.Int32
	entered chan struct{}
	release <-chan struct{}
	err     error
}

func (s *npcPollTestStore) LoadNPCDefinitionChanges(ctx context.Context, _ uint8, _ int64) ([]store.NPCDefinitionRecord, error) {
	s.calls.Add(1)
	s.entered <- struct{}{}
	select {
	case <-s.release:
		return []store.NPCDefinitionRecord{}, s.err
	case <-ctx.Done():
		return nil, ctx.Err()
	}
}

func TestNPCDefinitionPollingBoundsDatabaseWork(t *testing.T) {
	release := make(chan struct{})
	backend := &npcPollTestStore{craftStore: &craftStore{}, entered: make(chan struct{}, 2), release: release}
	w := &World{store: backend, channel: 1, commands: make(chan command, 2), npcDefinitionState: &npcDefinitionRuntime{}}
	now := time.Now()
	w.tickNPCDefinitionChanges(now)
	select {
	case <-backend.entered:
	case <-time.After(time.Second):
		t.Fatal("poll did not start")
	}
	for i := 0; i < 100; i++ {
		w.tickNPCDefinitionChanges(now.Add(time.Hour))
	}
	if backend.calls.Load() != 1 {
		t.Fatal("overlapping ticks started concurrent database polls")
	}
	close(release)
	var cmd command
	select {
	case cmd = <-w.commands:
	case <-time.After(time.Second):
		t.Fatal("poll did not return to actor")
	}
	w.handleNPCDefinitionEvent(cmd.npcDefinitions)
	w.tickNPCDefinitionChanges(now.Add(time.Second))
	if backend.calls.Load() != 1 {
		t.Fatal("poll bypassed its interval")
	}
	w.tickNPCDefinitionChanges(now.Add(npcDefinitionPollInterval))
	select {
	case <-backend.entered:
	case <-time.After(time.Second):
		t.Fatal("next due poll did not start")
	}
	select {
	case cmd = <-w.commands:
	case <-time.After(time.Second):
		t.Fatal("next poll did not complete")
	}
	w.handleNPCDefinitionEvent(cmd.npcDefinitions)
	if backend.calls.Load() != 2 || w.npcDefinitionState.polling.Load() {
		t.Fatal("poll lifecycle did not complete")
	}
}

func TestNPCPollFailureAndFullQueueRetainCheckpoint(t *testing.T) {
	for _, mode := range []string{"database failure", "full queue"} {
		t.Run(mode, func(t *testing.T) {
			release := make(chan struct{})
			close(release)
			backend := &npcPollTestStore{craftStore: &craftStore{}, entered: make(chan struct{}, 1), release: release}
			if mode == "database failure" {
				backend.err = errors.New("injected poll failure")
			}
			w := &World{store: backend, channel: 1, commands: make(chan command), npcDefinitionState: &npcDefinitionRuntime{lastChange: 11}}
			failures, drops := metricNPCDefinitionPollFailures.Value(), metricNPCDefinitionQueueDrops.Value()
			w.tickNPCDefinitionChanges(time.Now())
			deadline := time.Now().Add(time.Second)
			for w.npcDefinitionState.polling.Load() && time.Now().Before(deadline) {
				time.Sleep(time.Millisecond)
			}
			if w.npcDefinitionState.polling.Load() || w.npcDefinitionState.lastChange != 11 {
				t.Fatal("failure advanced or stranded checkpoint")
			}
			if mode == "database failure" && metricNPCDefinitionPollFailures.Value() != failures+1 {
				t.Fatal("poll failure metric missing")
			}
			if mode == "full queue" && metricNPCDefinitionQueueDrops.Value() != drops+1 {
				t.Fatal("queue drop metric missing")
			}
		})
	}
}

type npcSaveTestStore struct {
	*craftStore
	store.NPCDefinitionStore
	record    store.NPCDefinitionRecord
	afterSave func()
}

type npcBootTestStore struct {
	*craftStore
	store.NPCDefinitionStore
	record         store.NPCDefinitionRecord
	checkpointRead bool
}

func (s *npcBootTestStore) SyncNPCDefinitions(context.Context, uint8, []store.NPCDefinitionSeed) error {
	return nil
}
func (s *npcBootTestStore) NPCDefinitionChangeCursor(context.Context, uint8) (int64, error) {
	s.checkpointRead = true
	return 3, nil
}
func (s *npcBootTestStore) LoadNPCDefinitions(context.Context, uint8) ([]store.NPCDefinitionRecord, error) {
	if !s.checkpointRead {
		return nil, fmt.Errorf("snapshot read before checkpoint")
	}
	return []store.NPCDefinitionRecord{s.record}, nil
}

func TestNPCBootReadsCheckpointBeforeSnapshot(t *testing.T) {
	w, _, base := handlerTestWorld(t)
	defs, err := data.LoadNPCs("../../data/npcs/Aki.json")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.npcs, w.items, w.channel = defs, catalog.Items, 1
	raw, _ := json.Marshal(defs[0])
	w.store = &npcBootTestStore{craftStore: base, record: store.NPCDefinitionRecord{Channel: 1, Name: "Aki", Version: 2, ChangeID: 4, Definition: raw}}
	if err := w.loadNPCDefinitionsAtBoot([]model.NPCGener{{Enabled: true, Leader: "Aki"}}); err != nil {
		t.Fatal(err)
	}
	if w.npcDefinitionState.lastChange != 3 || w.npcDefinitionState.versions["Aki"] != 2 {
		t.Fatal("concurrent snapshot revision was not replayable")
	}
}

func (s *npcSaveTestStore) SaveNPCDefinition(_ context.Context, channel uint8, name string, expected uint32, raw json.RawMessage, _ bool, _, _ string) (store.NPCDefinitionRecord, error) {
	s.record = store.NPCDefinitionRecord{Channel: channel, Name: name, Version: expected + 1, Definition: raw}
	if s.afterSave != nil {
		s.afterSave()
	}
	return s.record, nil
}

func TestNPCSaveReportsActorAcknowledgement(t *testing.T) {
	for _, mode := range []string{"applied", "cancelled", "busy"} {
		t.Run(mode, func(t *testing.T) {
			pending := mode != "applied"
			w, _, base := handlerTestWorld(t)
			defs, err := data.LoadNPCs("../../data/npcs/Aki.json")
			if err != nil {
				t.Fatal(err)
			}
			catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
			if err != nil {
				t.Fatal(err)
			}
			w.npcs, w.items, w.channel = defs, catalog.Items, 1
			w.commands = make(chan command, 8)
			raw, _ := json.Marshal(defs[0])
			w.npcDefinitionState = &npcDefinitionRuntime{source: map[string]json.RawMessage{"Aki": raw}, active: map[string]bool{"Aki": true}, versions: map[string]uint32{"Aki": 1}}
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
			defer cancel()
			backend := &npcSaveTestStore{craftStore: base}
			w.store = backend
			if mode == "cancelled" {
				backend.afterSave = cancel
			}
			if mode == "busy" {
				backend.afterSave = func() { w.staffOps.pending.Store(maxStaffInFlight) }
			}
			stop, stopped := make(chan struct{}), make(chan struct{})
			go func() { defer close(stopped); runStaffQueue(w, stop) }()
			result, err := w.SaveNPCDefinition(ctx, "editor", control.NPCDefinitionCommand{Version: 1, Channel: 1, ExpectedVersion: 1, Name: "Aki", Reason: "Update shop", Definition: raw}, func() bool { return true })
			close(stop)
			<-stopped
			if err != nil {
				t.Fatal(err)
			}
			want := "ok"
			if pending {
				want = "saved_pending"
			}
			if result.Code != want || (!pending && result.NPC.AppliedVersion != 2) || (pending && result.NPC.AppliedVersion != 0) {
				t.Fatalf("unexpected save acknowledgement: code=%s applied=%d", result.Code, result.NPC.AppliedVersion)
			}
		})
	}
}

func TestNPCStockEditDuringPurchasePreservesCommittedItem(t *testing.T) {
	w, p, _, shop, backend, queue, release := criticalShopFixture(t, "buy")
	w.handle(command{s: p.Session, pkt: criticalShopRequest("buy", p.ID, shop.ID)})
	awaitCriticalWrite(t, backend.entered)
	w.invalidateNPCShopWindows(shop.Def.Name)
	shop.Def.Vende = []model.Item{{Index: 4147}}
	// A reopen packet stays behind the pending transaction.
	reopen := inboundPacket(wire.OpUseNPC, 20)
	binary.LittleEndian.PutUint16(reopen[12:14], shop.ID)
	if !w.deferCriticalCommand(command{s: p.Session, pkt: reopen}) {
		t.Fatal("shop interaction bypassed purchase reservation")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Inv[1].Index != 400 || p.Char.Gold != 9000 {
		t.Fatal("live edit changed the admitted purchase")
	}
	nextPacket(t, p) // Original buy confirmation.
	packet := nextPacket(t, p)
	want := wire.SendItem(p.ID, placeInv, 1, p.Char.Inv[1])
	if string(packet[12:]) != string(want[12:]) || wire.ParseHeader(packet).Type != wire.ParseHeader(want).Type {
		t.Fatal("committed item was not reconciled after shop invalidation")
	}
	packet = nextPacket(t, p)
	want = wire.UpdateEtc(p.ID, *p.Char)
	if string(packet[12:]) != string(want[12:]) || wire.ParseHeader(packet).Type != wire.ParseHeader(want).Type {
		t.Fatal("committed gold was not reconciled")
	}
}

func TestNPCDefinitionReferencesUseExactTemplateNames(t *testing.T) {
	npcs := []model.NPCDef{{Name: "Shop"}, {Name: "shop"}, {Name: "BlackOracle", ServiceIdentity: "Black_Oracle"}}
	used, err := npccontent.RequiredNames(npcs, npccontent.Sources{Generators: []model.NPCGener{{Enabled: true, Leader: "shop", Follower: "shop"}}, Quests: model.QuestFile{Quests: []model.QuestDef{{NPC: "BlackOracle"}}}})
	if err != nil {
		t.Fatal(err)
	}
	if used["Shop"] || !used["shop"] || !used["BlackOracle"] || len(used) != 2 {
		t.Fatalf("reference discovery changed template identity: %v", used)
	}
}
