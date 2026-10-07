package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type criticalGameStateTestStore struct{ *criticalTestStore }

func (s *criticalGameStateTestStore) SaveCriticalGameState(guilds *model.GuildRegistry,
	instances *model.InstanceStateSnapshot, uid string, state *model.CharState, accounts ...*model.Account) error {
	write := criticalWrite{guilds: guilds, instances: instances, accounts: accounts}
	if uid != "" {
		write.character = &criticalCharacterState{uid: uid, state: state}
	}
	return s.record(write)
}

func TestCriticalGameStateDetachedFormsAndReservations(t *testing.T) {
	for _, form := range []string{"guild-only", "guild-accounts", "guild-sidecar", "guild-instance", "instance-sidecar", "all", "delete"} {
		t.Run(form, func(t *testing.T) {
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 3), release: gate}
			q := newCriticalTestQueue(t, &criticalGameStateTestStore{base}, 2, 3)
			t.Cleanup(release)
			write := criticalWrite{}
			if form != "instance-sidecar" {
				write.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion,
					Guilds: []model.Guild{{ID: 1, Name: "Guild", Members: []model.GuildMember{{Character: "Owner", Account: "owner", Rank: model.GuildRankLeader}}}}}
				write.guilds.Wars.Cities.Entries = []model.CityWarEntry{{Guild: 1, City: 2}}
			}
			if form != "guild-only" && form != "guild-instance" {
				write.accounts = []*model.Account{{Name: "owner", Chars: []model.Char{{UID: "owner-uid", Gold: 10}}}, {Name: "peer"}}
			}
			if form == "guild-sidecar" || form == "instance-sidecar" || form == "all" || form == "delete" {
				write.character = &criticalCharacterState{uid: "owner-uid"}
				if form != "delete" {
					write.character.state = &model.CharState{SpecialCoins: map[string]uint32{"fame": 3}}
				}
			}
			if form == "guild-instance" || form == "instance-sidecar" || form == "all" {
				write.instances = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
					NightmarePartyRuns: map[string]int{"window": 2}, Instances: []model.InstanceRuntimeState{{MemberCharacterUIDs: []string{"owner-uid"}}}}
			}
			id, err := q.submit(write)
			if err != nil {
				t.Fatal(err)
			}
			attempt := awaitCriticalWrite(t, base.entered)
			if (attempt.guilds != nil) != (write.guilds != nil) || (attempt.instances != nil) != (write.instances != nil) ||
				(attempt.character != nil) != (write.character != nil) || len(attempt.accounts) != len(write.accounts) {
				t.Fatal("combined adapter lost an aggregate or participant")
			}
			checkFences := func() {
				t.Helper()
				if write.guilds != nil {
					if q.guilds != id {
						t.Fatal("guild reservation was released before acknowledgement")
					}
					if _, err := q.submit(criticalWrite{guilds: write.guilds}); !errors.Is(err, errCriticalPersistenceBusy) {
						t.Fatalf("overlapping guild registry accepted: %v", err)
					}
				}
				if write.instances != nil {
					if q.instances != id {
						t.Fatal("instance reservation was released before acknowledgement")
					}
					if _, err := q.submit(criticalWrite{instances: write.instances}); !errors.Is(err, errCriticalPersistenceBusy) {
						t.Fatalf("overlapping instance aggregate accepted: %v", err)
					}
				}
				for _, account := range write.accounts {
					if _, err := q.submit(criticalWrite{accounts: []*model.Account{account}}); !errors.Is(err, errCriticalPersistenceBusy) {
						t.Fatalf("overlapping account accepted: %v", err)
					}
				}
			}
			checkFences()
			if attempt.guilds != nil {
				attempt.guilds.Guilds[0].Members[0].Character = "worker"
				attempt.guilds.Wars.Cities.Entries[0].City = 3
				if write.guilds.Guilds[0].Members[0].Character != "Owner" || write.guilds.Wars.Cities.Entries[0].City != 2 {
					t.Fatal("worker received nested guild aliases")
				}
			}
			if attempt.instances != nil {
				attempt.instances.NightmarePartyRuns["window"] = 7
				attempt.instances.Instances[0].MemberCharacterUIDs[0] = "worker"
				if write.instances.NightmarePartyRuns["window"] != 2 || write.instances.Instances[0].MemberCharacterUIDs[0] != "owner-uid" {
					t.Fatal("worker received nested instance aliases")
				}
			}
			if attempt.character != nil && attempt.character.state != nil {
				attempt.character.state.SpecialCoins["fame"] = 9
				if write.character.state.SpecialCoins["fame"] != 3 {
					t.Fatal("worker received nested sidecar aliases")
				}
			}
			if len(attempt.accounts) != 0 {
				attempt.accounts[0].Chars[0].Gold = 999
				if write.accounts[0].Chars[0].Gold != 10 {
					t.Fatal("worker received nested account aliases")
				}
			}
			release()
			result := awaitCriticalResult(t, q)
			if result.id != id || result.err != nil {
				t.Fatalf("combined adapter completion: %+v", result)
			}
			checkFences()
			if err := q.acknowledge(id); err != nil || q.guilds != 0 || q.instances != 0 || len(q.accounts) != 0 || len(q.characters) != 0 {
				t.Fatalf("combined acknowledgement did not release every reservation: %v", err)
			}
			if _, err := q.submit(write); err != nil {
				t.Fatalf("explicit repetition after acknowledgement rejected: %v", err)
			}
			awaitCriticalWrite(t, base.entered)
			awaitCriticalResult(t, q)
		})
	}
}

func TestCriticalGameStateUnsupportedAndInvalidAdmission(t *testing.T) {
	for _, form := range []string{"unsupported-guild", "unsupported-combined", "missing-owner", "missing-uid", "foreign-sidecar", "duplicate-account"} {
		t.Run(form, func(t *testing.T) {
			base := &criticalTestStore{entered: make(chan criticalWrite, 1)}
			var backend store.Store = &criticalGameStateTestStore{base}
			write := criticalWrite{guilds: &model.GuildRegistry{Version: model.GuildRegistryVersion}}
			if form == "unsupported-guild" {
				backend = base
			} else if form == "unsupported-combined" {
				backend = &criticalPreferredStateTestStore{&criticalBatchOnlyStateTestStore{base}}
				write.guilds = nil
				write.instances = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
				write.accounts = []*model.Account{{Name: "owner"}}
				write.character = &criticalCharacterState{uid: "uid"}
			} else {
				write.character = &criticalCharacterState{uid: "uid"}
				if form != "missing-owner" {
					write.accounts = []*model.Account{{Name: "owner"}, {Name: "peer", Chars: []model.Char{{UID: "uid"}}}}
				}
				if form == "missing-uid" {
					write.character.uid = ""
				} else if form == "duplicate-account" {
					write.accounts[1].Name = " OWNER "
				}
			}
			q := newCriticalTestQueue(t, backend, 1, 2)
			if id, err := q.submit(write); id != 0 || err == nil || q.guilds != 0 || q.instances != 0 ||
				len(q.accounts) != 0 || len(q.characters) != 0 || len(q.pending) != 0 || len(base.entered) != 0 {
				t.Fatalf("rejected form wrote or reserved: id=%d error=%v", id, err)
			}
		})
	}
}
