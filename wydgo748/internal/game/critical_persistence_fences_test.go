package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

type criticalFenceTestStore struct {
	*criticalStateTestStore
	accountAutosaves int
	sidecarAutosaves int
	sidecarSaves     int
	sidecarLoads     int
}

func (s *criticalFenceTestStore) SaveAccountAsync(*model.Account) error {
	s.accountAutosaves++
	return nil
}
func (s *criticalFenceTestStore) SaveCharStateAsync(string, *model.CharState) error {
	s.sidecarAutosaves++
	return nil
}
func (s *criticalFenceTestStore) SaveCharState(string, *model.CharState) error {
	s.sidecarSaves++
	return nil
}
func (s *criticalFenceTestStore) LoadCharState(string) (*model.CharState, error) {
	s.sidecarLoads++
	return nil, nil
}

func TestCriticalFencesProtectAllAccountCharactersAndRemovedSidecar(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 4)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	account := &model.Account{Name: "one", Chars: []model.Char{{UID: "active"}, {UID: "sibling"}}}
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{account},
		character: &criticalCharacterState{uid: "removed"}}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	world := &World{criticalWrites: owner}
	for _, uid := range []string{"active", "sibling", "removed"} {
		if !world.criticalCharacterReserved(uid) {
			t.Fatalf("character fence missing: %s", uid)
		}
		_, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: "other", Chars: []model.Char{{UID: uid}}}}})
		if !errors.Is(err, errCriticalPersistenceBusy) || len(queue.pending) != 1 || queue.accounts["other"] != 0 {
			t.Fatalf("overlapping UID admitted or retained: %s, %v", uid, err)
		}
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	if !world.criticalCharacterReserved("sibling") {
		t.Fatal("worker completion released fence before World delivery")
	}
	world.processCriticalCompletions(1)
	if len(queue.characters) != 0 || len(queue.accounts) != 0 {
		t.Fatal("successful owner acknowledgement leaked fences")
	}
}

func TestCriticalFencesRejectPersistenceReadsAndAutosavesBeforeStoreAccess(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	store := &criticalFenceTestStore{criticalStateTestStore: &criticalStateTestStore{&criticalBatchTestStore{backend}}}
	queue := newCriticalTestQueue(t, store, 1, 4)
	t.Cleanup(release)
	owner := newCriticalPersistenceOwner(queue)
	player, _ := networkedTestPlayer(1, "Held", 2100, 2100)
	player.Account.Name = " HELD "
	player.Char.UID = "held-character"
	player.SpecialCoins = map[string]uint32{"test": 7}
	account := player.Account
	world := worldWithNetworkedPlayers(player)
	world.store, world.criticalWrites = store, owner
	if !world.claimAccountSession(player.Session, account.Name) {
		t.Fatal("fixture account admission failed")
	}
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{account}}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	for name, call := range map[string]func() error{
		"account":          func() error { return world.saveAccount(player.Account) },
		"account-autosave": func() error { return world.saveAccountAsync(player.Account) },
		"account-with-fenced-uid": func() error {
			return world.saveAccount(&model.Account{Name: "other", Chars: []model.Char{{UID: player.Char.UID}}})
		},
		"sidecar":       func() error { return world.saveCharStateResult(player) },
		"aggregate":     func() error { return world.saveAccountAndCharStateResult(player) },
		"load":          func() error { return world.loadCharStateInto(player) },
		"uid-only-load": func() error { return world.loadCharStateInto(&Player{Char: player.Char}) },
		"uid-only-save": func() error { return world.saveCharStateResult(&Player{Char: player.Char}) },
		"replacement-account": func() error {
			return world.saveAccountAndCharStateForUID(&model.Account{Name: "other"}, player.Char.UID, nil)
		},
	} {
		if err := call(); !errors.Is(err, errCriticalPersistenceBusy) {
			t.Fatalf("%s bypassed pending fence: %v", name, err)
		}
	}
	world.saveCharStateAsync(player)
	world.autoSaveBucket = accountAutoSaveBucket(player.Account.Name)
	world.autoSaveAccounts(time.Now())
	if store.accountAutosaves != 0 || store.sidecarAutosaves != 0 || store.sidecarSaves != 0 || store.sidecarLoads != 0 || len(backend.entered) != 0 {
		t.Fatal("pending snapshot reached the persistence adapter")
	}
	if player.SpecialCoins["test"] != 7 {
		t.Fatal("rejected load cleared live sidecar state")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	world.autoSaveBucket = accountAutoSaveBucket(player.Account.Name)
	world.autoSaveAccounts(time.Now())
	write := awaitCriticalWrite(t, backend.entered)
	if len(write.accounts) != 1 || write.character == nil || write.character.uid != player.Char.UID ||
		store.accountAutosaves != 0 || store.sidecarAutosaves != 0 {
		t.Fatal("autosave did not resume through the atomic coordinator after acknowledgement")
	}
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
}

func TestCriticalFencePreventsAccountReadmissionAfterSessionRelease(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	owner := newCriticalPersistenceOwner(queue)
	world := &World{criticalWrites: owner}
	first, second := net.NewTestSession(1, 4), net.NewTestSession(2, 4)
	account := &model.Account{Name: "one"}
	if !world.claimAccountSession(first, account.Name) {
		t.Fatal("initial claim rejected")
	}
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{account}}, func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitReadyCriticalResults(t, queue, 1)
	world.releaseAccountSession(first, account)
	if world.claimAccountSession(second, " ONE ") || world.accountSessions["one"] != nil {
		t.Fatal("pending durable state readmitted after socket ownership release")
	}
	world.processCriticalCompletions(1)
	if !world.claimAccountSession(second, " ONE ") {
		t.Fatal("settled account remained unavailable")
	}
}

func TestCriticalFailedCompletionRetainsCharacterFence(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1), failure: errors.New("unknown outcome")}
	queue := newCriticalTestQueue(t, backend, 1, 1)
	owner := newCriticalPersistenceOwner(queue)
	if _, err := owner.submit(criticalWrite{accounts: []*model.Account{{Name: "one", Chars: []model.Char{{UID: "character"}}}}},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	world := &World{criticalWrites: owner}
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if !world.criticalAccountReserved("ONE") || !world.criticalCharacterReserved("character") ||
		world.claimAccountSession(net.NewTestSession(1, 1), "one") {
		t.Fatal("uncertain outcome lost account/character fences or allowed readmission")
	}
}

func TestCriticalFencesRejectSharedUIDAcrossBatchAccounts(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 1)}
	queue := newCriticalTestQueue(t, &criticalBatchTestStore{backend}, 1, 1)
	_, err := queue.submit(criticalWrite{accounts: []*model.Account{
		{Name: "one", Chars: []model.Char{{UID: "shared"}}},
		{Name: "two", Chars: []model.Char{{UID: "shared"}}},
	}})
	if err == nil || len(queue.pending) != 0 || len(queue.accounts) != 0 || len(queue.characters) != 0 || len(backend.entered) != 0 {
		t.Fatal("ambiguous character ownership admitted or caused side effects")
	}
}
