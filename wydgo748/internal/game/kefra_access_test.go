package game

import (
	"bytes"
	"errors"
	"maps"
	"path/filepath"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type kefraAccessStore struct {
	craftStore
	writes   int
	failure  error
	states   map[string]*model.CharState
	accounts map[string]*model.Account
}

func (s *kefraAccessStore) SavePlayerState(_ *model.GuildRegistry, account *model.Account,
	uid string, state *model.CharState) error {
	s.writes++
	if s.failure != nil {
		return s.failure
	}
	s.accounts[account.Name] = accountStateSnapshot(account)
	s.states[uid] = state
	return nil
}

func (s *kefraAccessStore) LoadCharState(uid string) (*model.CharState, error) {
	return s.states[uid], nil
}

func (*kefraAccessStore) SaveCharState(string, *model.CharState) error {
	return errors.New("Kefra entry must use an atomic account/character write")
}

type kefraAccessCriticalStore struct{ *criticalGameStateTestStore }

func (s *kefraAccessCriticalStore) SavePlayerStateBatch(_ *model.GuildRegistry,
	uid string, state *model.CharState, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts,
		character: &criticalCharacterState{uid: uid, state: state}})
}

func kefraAccessFixture(t *testing.T) (*World, *Player, *kefraAccessStore) {
	t.Helper()
	w, p, _ := handlerTestWorld(t)
	portals, err := data.LoadTeleports(filepath.Join("..", "..", "data", "teleports.ini"))
	if err != nil {
		t.Fatal(err)
	}
	w.teleports = portals
	st := &kefraAccessStore{states: make(map[string]*model.CharState), accounts: make(map[string]*model.Account)}
	w.store = st
	p.Char.Gold = 50_000
	setKefraAccessPosition(w, p, 2366, 3893)
	portal, ok := teleportAt(w.teleports, p.X, p.Y)
	if !ok || !isKefraHallEntrance(portal) || portal.DestX != 2366 || portal.DestY != 3910 {
		t.Fatal("installed data does not contain the expected Kefra hall entrance")
	}
	return w, p, st
}

func setKefraAccessPosition(w *World, p *Player, x, y uint16) {
	p.X, p.Y, p.Char.X, p.Char.Y = x, y, x, y
	w.updatePlayerSpatial(p)
}

func survivorAccessQuest(t *testing.T) *model.QuestDef {
	t.Helper()
	quests, err := data.LoadQuests(filepath.Join("..", "..", "data", "quests.json"))
	if err != nil {
		t.Fatal(err)
	}
	for i := range quests.Quests {
		q := &quests.Quests[i]
		if q.NPC == "Survivor" {
			if q.Recharge == nil || q.Recharge.Item != 4127 || q.Recharge.Amount != 50 ||
				q.Recharge.Counter != kefraTicketCounter || q.ConsumeCounters[kefraTicketCounter] != 1 {
				t.Fatal("Survivor must grant 50 entries per Locked Seal and charge one per visit")
			}
			return q
		}
	}
	t.Fatal("Survivor quest missing from installed data")
	return nil
}

func TestKefraHallPortalRejectsZeroEntriesAcrossEntireBlock(t *testing.T) {
	for x := uint16(2364); x <= 2367; x++ {
		for y := uint16(3892); y <= 3895; y++ {
			w, p, st := kefraAccessFixture(t)
			setKefraAccessPosition(w, p, x, y)
			// A seal in the bag cannot bypass exchanging it with Survivor.
			p.Char.Inv[0] = model.Item{Index: 4127}
			before := accountStateSnapshot(p.Account)
			w.onReqTeleport(p.Session, make([]byte, 16))
			packet, ok := p.Session.DequeuePacketForTest()
			if !ok || !wire.Decrypt(packet) || !bytes.Contains(packet, []byte(kefraEntriesRequired)) ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(p.SpecialCoins) != 0 ||
				p.X != x || p.Y != y || st.writes != 0 || st.saves != 0 {
				t.Fatalf("zero-entry portal request at (%d,%d) was not rejected without charging", x, y)
			}
		}
	}
}

func TestKefraHallPortalConsumesOneEntryAndPersistsByCharacter(t *testing.T) {
	for _, balance := range []uint32{1, 2, 50} {
		w, p, st := kefraAccessFixture(t)
		p.SpecialCoins = map[string]uint32{kefraTicketCounter: balance, "fame": 7}
		p.Char.Inv[0] = model.Item{Index: 4127}
		beforeItem := p.Char.Inv[0]
		w.onReqTeleport(p.Session, make([]byte, 16))
		saved := st.accounts[p.Account.Name]
		if st.writes != 1 || st.saves != 0 || saved == nil || p.Char.Gold != 49_300 ||
			p.X != 2366 || p.Y != 3910 || p.Char.X != p.X || p.Char.Y != p.Y ||
			counterBalance(p, kefraTicketCounter) != balance-1 || counterBalance(p, "fame") != 7 ||
			p.Char.Inv[0] != beforeItem || !reflect.DeepEqual(saved, accountPersistenceSnapshot(p.Account)) {
			t.Fatalf("entry, gold and position were not committed together: balance=%d", balance)
		}
		fresh, _ := networkedTestPlayer(1, p.Char.Name, 2100, 2100)
		if err := w.loadCharStateInto(fresh); err != nil {
			t.Fatal(err)
		}
		if counterBalance(fresh, kefraTicketCounter) != balance-1 || counterBalance(fresh, "fame") != 7 {
			t.Fatal("relogin restored a spent entry or lost another currency")
		}
		fresh.Char.UID = "another-character-on-the-same-account"
		if err := w.loadCharStateInto(fresh); err != nil {
			t.Fatal(err)
		}
		if counterBalance(fresh, kefraTicketCounter) != 0 {
			t.Fatal("Kefra entries leaked to a different character")
		}
		// Replaying the packet from inside the hall must not charge again.
		w.onReqTeleport(p.Session, make([]byte, 16))
		if st.writes != 1 || counterBalance(p, kefraTicketCounter) != balance-1 {
			t.Fatal("replayed portal request consumed an extra entry")
		}
	}
}

func TestSurvivorLockedSealGrantsExactlyFiftyVisits(t *testing.T) {
	w, p, st := kefraAccessFixture(t)
	quest := survivorAccessQuest(t)
	npc := &Mob{Def: &model.NPCDef{Name: "Survivor"}}
	p.Char.Inv[0] = model.Item{Index: 4127, Eff: [6]byte{effectAmount, 2}}
	w.executeQuest(p.Session, p, npc, quest)
	if counterBalance(p, kefraTicketCounter) != 49 || itemStackAmount(p.Char.Inv[0]) != 1 ||
		p.Char.Inv[0].Index != 4127 || p.X != 2365 || p.Y != 3907 || st.writes != 1 {
		t.Fatal("Survivor must consume one seal, grant 50 entries and spend the first visit")
	}
	for remaining := uint32(49); remaining > 0; remaining-- {
		drainPlayerPackets(p)
		setKefraAccessPosition(w, p, 2366, 3893)
		w.onReqTeleport(p.Session, make([]byte, 16))
		if counterBalance(p, kefraTicketCounter) != remaining-1 || p.Y != 3910 || p.Char.Inv[0].Index != 4127 {
			t.Fatalf("visit with %d entries did not consume exactly one existing entry", remaining)
		}
	}
	drainPlayerPackets(p)
	setKefraAccessPosition(w, p, 2366, 3893)
	w.onReqTeleport(p.Session, make([]byte, 16))
	if st.writes != 50 || p.Y != 3893 || counterBalance(p, kefraTicketCounter) != 0 || p.Char.Inv[0].Index != 4127 {
		t.Fatal("the 51st portal visit bypassed Survivor or consumed a spare seal")
	}
	w.executeQuest(p.Session, p, npc, quest)
	if st.writes != 51 || p.Char.Inv[0].Index != 0 || counterBalance(p, kefraTicketCounter) != 49 {
		t.Fatal("only returning to Survivor should exchange the next Locked Seal")
	}
}

func TestKefraAccessFailureNeverConsumesEntrySealGoldOrPosition(t *testing.T) {
	for _, mode := range []string{"gold", "blocked-destination", "missing-uid", "non-atomic-store", "save", "Survivor-save"} {
		t.Run(mode, func(t *testing.T) {
			w, p, st := kefraAccessFixture(t)
			p.Char.Inv[0] = model.Item{Index: 4127}
			p.SpecialCoins = map[string]uint32{kefraTicketCounter: 2, "fame": 7}
			switch mode {
			case "gold":
				p.Char.Gold = 699
			case "blocked-destination":
				w.terrain = model.TerrainMap{Height: make([]byte, model.TerrainCells), Attribute: make([]byte, model.AttributeCells)}
				w.terrain.Attribute[(3910/4)*model.AttributeWidth+2366/4] = 0x02
			case "missing-uid":
				p.Char.UID = ""
			case "non-atomic-store":
				w.store = &craftStore{}
			case "save", "Survivor-save":
				st.failure = errors.New("injected Kefra access save failure")
				if mode == "Survivor-save" {
					p.SpecialCoins = map[string]uint32{"fame": 7}
				}
			}
			before, coins := accountStateSnapshot(p.Account), copyCounters(p)
			if mode == "Survivor-save" {
				w.executeQuest(p.Session, p, &Mob{Def: &model.NPCDef{Name: "Survivor"}}, survivorAccessQuest(t))
			} else {
				w.onReqTeleport(p.Session, make([]byte, 16))
			}
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !maps.Equal(p.SpecialCoins, coins) ||
				p.X != 2366 || p.Y != 3893 || len(st.states) != 0 || len(st.accounts) != 0 {
				t.Fatal("failed Kefra access changed inventory, gold, entries, position or durable state")
			}
		})
	}
}

func TestCriticalKefraHallEntryWaitsForAtomicAcknowledgement(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, fallback := kefraAccessFixture(t)
			p.SpecialCoins = map[string]uint32{kefraTicketCounter: 2, "fame": 7}
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("Kefra fixture could not claim the account")
			}
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if outcome == "rejection" {
				base.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				base.failure = errors.New("Kefra entry acknowledgement lost")
			}
			q := newCriticalTestQueue(t, &kefraAccessCriticalStore{&criticalGameStateTestStore{base}}, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			before, coins := accountStateSnapshot(p.Account), copyCounters(p)
			expected := accountStateSnapshot(p.Account)
			expected.Chars[p.CharSlot].Gold -= 700
			stagePlayerHomeCity(&Player{Char: &expected.Chars[p.CharSlot], X: 2366, Y: 3910})
			w.onReqTeleport(p.Session, make([]byte, 16))
			proposal := awaitCriticalWrite(t, base.entered)
			if len(proposal.accounts) != 1 || proposal.character == nil || proposal.character.uid != p.Char.UID ||
				proposal.character.state.SpecialCoins[kefraTicketCounter] != 1 || proposal.character.state.SpecialCoins["fame"] != 7 ||
				!reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(expected)) {
				t.Fatal("Kefra teleport proposal split the balance, gold or destination")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, q, 1)
				}
				w.onReqTeleport(p.Session, make([]byte, 16))
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !maps.Equal(p.SpecialCoins, coins) ||
					p.X != 2366 || p.Y != 3893 || p.Session.QueuedPacketsForTest() != 0 ||
					len(base.entered) != 0 || !w.criticalPlayerReserved(p) || fallback.writes != 0 || fallback.saves != 0 {
					t.Fatalf("%s entry published, charged twice, saved inline or released ownership", phase)
				}
			}
			w.processCriticalCompletions(1)
			switch outcome {
			case "commit":
				if p.X != 2366 || p.Y != 3910 || p.Char.Gold != 49_300 || counterBalance(p, kefraTicketCounter) != 1 ||
					w.criticalPlayerReserved(p) || len(w.criticalTeleportTiles) != 0 || w.shuttingDown {
					t.Fatal("committed Kefra entry was not installed exactly once")
				}
			case "rejection", "unknown":
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !maps.Equal(p.SpecialCoins, coins) ||
					p.X != 2366 || p.Y != 3893 || w.shuttingDown != (outcome == "unknown") ||
					w.criticalPlayerReserved(p) != (outcome == "unknown") {
					t.Fatal("uncommitted Kefra entry changed state or lost the required failure fence")
				}
				if outcome == "rejection" && len(w.criticalTeleportTiles) != 0 {
					t.Fatal("rejected Kefra entry retained its destination reservation")
				}
			}
		})
	}
}

func TestCriticalKefraHallEntryRejectsAccountOnlyPersistence(t *testing.T) {
	w, p, fallback := kefraAccessFixture(t)
	p.SpecialCoins = map[string]uint32{kefraTicketCounter: 1}
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("Kefra fixture could not claim the account")
	}
	base := &criticalTestStore{entered: make(chan criticalWrite, 1)}
	q := newCriticalTestQueue(t, base, 1, 1)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	before := accountStateSnapshot(p.Account)
	w.onReqTeleport(p.Session, make([]byte, 16))
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || counterBalance(p, kefraTicketCounter) != 1 ||
		p.Y != 3893 || len(base.entered) != 0 || w.criticalPlayerReserved(p) || fallback.writes != 0 {
		t.Fatal("account-only persistence granted a non-durable Kefra entry")
	}
}
