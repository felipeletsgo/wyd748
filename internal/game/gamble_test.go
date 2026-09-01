package game

import (
	"encoding/binary"
	"errors"
	"math"
	"testing"

	"wydgo/internal/model"
)

type gambleSequenceRNG struct {
	values []int
	next   int
}

func (r *gambleSequenceRNG) Intn(n int) int {
	if n <= 0 || r.next >= len(r.values) {
		return 0
	}
	value := r.values[r.next]
	r.next++
	if value < 0 {
		return 0
	}
	return value % n
}

type gambleStore struct {
	saves         int
	err           error
	snapshot      *model.InstanceStateSnapshot
	account       *model.Account
	packetsAtSave int
	player        *Player
	load          *model.InstanceStateSnapshot
}

func (s *gambleStore) LoadAccount(string) (*model.Account, error) { return nil, errors.New("not used") }
func (s *gambleStore) SaveAccount(*model.Account) error           { return errors.New("not used") }
func (s *gambleStore) CharacterNameExists(string) (bool, error)   { return false, nil }
func (s *gambleStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return s.load, nil
}
func (s *gambleStore) SaveInstanceState(snapshot *model.InstanceStateSnapshot) error {
	s.snapshot = snapshot
	return s.err
}
func (s *gambleStore) SaveGameStateWithInstanceState(_ *model.GuildRegistry,
	snapshot *model.InstanceStateSnapshot, accounts ...*model.Account) error {
	s.saves++
	s.snapshot = snapshot
	if len(accounts) != 0 {
		s.account = accounts[0]
	}
	if s.player != nil {
		s.packetsAtSave = s.player.Session.QueuedPacketsForTest()
	}
	return s.err
}

func gambleBetPacket(kind, bet uint32) []byte {
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint16(pkt[0:2], uint16(len(pkt)))
	binary.LittleEndian.PutUint16(pkt[4:6], 0x2BE)
	binary.LittleEndian.PutUint32(pkt[12:16], kind)
	binary.LittleEndian.PutUint32(pkt[16:20], bet)
	return pkt
}

func findGambleStops(t *testing.T, predicate func([5]byte) bool) [3]byte {
	t.Helper()
	for a := 0; a < gambleReelLength; a++ {
		for b := 0; b < gambleReelLength; b++ {
			for c := 0; c < gambleReelLength; c++ {
				stops := [3]byte{byte(a), byte(b), byte(c)}
				if predicate(gambleResults(stops)) {
					return stops
				}
			}
		}
	}
	t.Fatal("no reel stop matched predicate")
	return [3]byte{}
}

func gambleHandlerWorld(t *testing.T, stops [3]byte) (*World, *Player, *gambleStore) {
	t.Helper()
	w, p, _ := handlerTestWorld(t)
	store := &gambleStore{player: p}
	w.store = store
	w.rng = &gambleSequenceRNG{values: []int{99, int(stops[0]), int(stops[1]), int(stops[2])}}
	p.Char.Gold = 10_000
	return w, p, store
}

func TestClassifyGambleLineNativeRules(t *testing.T) {
	tests := []struct {
		line [3]byte
		want byte
	}{
		{[3]byte{4, 5, 6}, 7},
		{[3]byte{7, 8, 9}, 6},
		{[3]byte{7, 7, 7}, 15},
		{[3]byte{0, 0, 4}, 5},
		{[3]byte{5, 5, 1}, 3},
		{[3]byte{6, 6, 1}, 4},
		{[3]byte{0, 4, 4}, 1},
		{[3]byte{3, 4, 5}, 0},
	}
	for _, tt := range tests {
		if got := classifyGambleLine(tt.line); got != tt.want {
			t.Fatalf("classify(%v)=%d, want %d", tt.line, got, tt.want)
		}
	}
}

func TestGambleReelsAndPrizeFormula(t *testing.T) {
	for a := 0; a < gambleReelLength; a++ {
		for b := 0; b < gambleReelLength; b++ {
			for c := 0; c < gambleReelLength; c++ {
				for _, result := range gambleResults([3]byte{byte(a), byte(b), byte(c)}) {
					if int(result) >= len(gambleMultipliers) {
						t.Fatalf("stop (%d,%d,%d) generated invalid result %d", a, b, c, result)
					}
				}
			}
		}
	}
	if got := gamblePrize(1_000, [5]byte{1, 2, 3, 4, 5}); got != 7_500 {
		t.Fatalf("five-line payout=%d, want 7500", got)
	}
}

func TestGamblePayoutItemsUsesNativeGoldBars(t *testing.T) {
	tests := []struct {
		total uint64
		gold  uint32
		items []uint16
	}{
		{uint64(maxCharacterGold) - 1, maxCharacterGold - 1, nil},
		{uint64(maxCharacterGold), 0, []uint16{4011, 4011}},
		{2_147_483_647, 47_483_647, []uint16{4011, 4011, 4010}},
	}
	for _, tt := range tests {
		gold, items := gamblePayoutItems(tt.total)
		if gold != tt.gold || len(items) != len(tt.items) {
			t.Fatalf("payout(%d)=(%d,%v), want (%d,%v)", tt.total, gold, items, tt.gold, tt.items)
		}
		for i := range items {
			if items[i] != tt.items[i] {
				t.Fatalf("payout(%d) items=%v, want %v", tt.total, items, tt.items)
			}
		}
	}
}

func TestGamblePersistsBeforePublishing(t *testing.T) {
	stops := findGambleStops(t, func(result [5]byte) bool { return gamblePrize(1_000, result) == 0 })
	w, p, store := gambleHandlerWorld(t, stops)
	w.gambleJackpot = 100
	w.gamblePool = 200

	w.onDoJackpotBet(p.Session, gambleBetPacket(1, 1_000))

	if p.Char.Gold != 9_000 || w.gambleJackpot != 101 || w.gamblePool != 230 {
		t.Fatalf("authoritative state incorrect: gold=%d jackpot=%d pool=%d", p.Char.Gold, w.gambleJackpot, w.gamblePool)
	}
	if store.saves != 1 || store.snapshot == nil || store.account == nil || store.packetsAtSave != 0 {
		t.Fatalf("persist-before-publish violated: saves=%d snapshot=%v account=%v packets=%d",
			store.saves, store.snapshot != nil, store.account != nil, store.packetsAtSave)
	}
	if store.snapshot.GambleJackpot != 101 || store.snapshot.GamblePool != 230 ||
		store.account.Chars[p.CharSlot].Gold != 9_000 {
		t.Fatalf("persisted snapshot incomplete: instance=%+v gold=%d",
			store.snapshot, store.account.Chars[p.CharSlot].Gold)
	}
	if got := p.Session.QueuedPacketsForTest(); got != 3 {
		t.Fatalf("success published %d packets, want ResultGamble+UpdateCarry+UpdateEtc", got)
	}
}

func TestGambleJackpotPaysAndResets(t *testing.T) {
	stops := findGambleStops(t, func(result [5]byte) bool { return result[1] == 14 })
	w, p, _ := gambleHandlerWorld(t, stops)
	w.gambleJackpot = 7_777
	w.gamblePool = 1_000_000
	result := gambleResults(stops)
	wantPrize := uint32(gamblePrize(1_000, result) + 7_778)

	w.onDoJackpotBet(p.Session, gambleBetPacket(2, 1_000))

	if w.gambleJackpot != 0 || p.Char.Gold != 9_000+wantPrize {
		t.Fatalf("jackpot state incorrect: jackpot=%d gold=%d wantGold=%d result=%v",
			w.gambleJackpot, p.Char.Gold, 9_000+wantPrize, result)
	}
}

func TestGambleRejectsInvalidRequestsAndUnavailableStore(t *testing.T) {
	losing := findGambleStops(t, func(result [5]byte) bool { return gamblePrize(1_000, result) == 0 })
	tests := []struct {
		name string
		pkt  []byte
		gold uint32
	}{
		{"kind zero", gambleBetPacket(0, 1_000), 10_000},
		{"kind above range", gambleBetPacket(3, 1_000), 10_000},
		{"bet below minimum", gambleBetPacket(1, 999), 10_000},
		{"bet not multiple", gambleBetPacket(1, 1_500), 10_000},
		{"bet above maximum", gambleBetPacket(1, 101_000), 200_000},
		{"insufficient gold", gambleBetPacket(1, 10_000), 9_999},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w, p, store := gambleHandlerWorld(t, losing)
			p.Char.Gold = tt.gold
			beforeGold, beforeInv := p.Char.Gold, p.Char.Inv
			w.onDoJackpotBet(p.Session, tt.pkt)
			if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv || store.saves != 0 || p.Session.QueuedPacketsForTest() != 1 {
				t.Fatalf("rejection changed state: saves=%d packets=%d", store.saves, p.Session.QueuedPacketsForTest())
			}
		})
	}

	w, p, _ := gambleHandlerWorld(t, losing)
	w.store = nonAtomicInstanceStore{}
	beforeGold, beforeInv := p.Char.Gold, p.Char.Inv
	w.onDoJackpotBet(p.Session, gambleBetPacket(1, 1_000))
	if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv || p.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("store without mixed transaction did not fail closed")
	}

	w, p, store := gambleHandlerWorld(t, losing)
	w.onDoJackpotBet(p.Session, gambleBetPacket(1, 1_000)[:19])
	if store.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("short packet reached Gamble state mutation")
	}
}

func TestGambleRollsBackAccountAndPoolsOnFailure(t *testing.T) {
	losing := findGambleStops(t, func(result [5]byte) bool { return gamblePrize(1_000, result) == 0 })
	w, p, store := gambleHandlerWorld(t, losing)
	store.err = errors.New("disk unavailable")
	w.gambleJackpot, w.gamblePool = 77, 88
	beforeGold, beforeInv := p.Char.Gold, p.Char.Inv

	w.onDoJackpotBet(p.Session, gambleBetPacket(1, 1_000))

	if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv || w.gambleJackpot != 77 || w.gamblePool != 88 || store.saves != 1 {
		t.Fatalf("rollback incomplete: gold=%d jackpot=%d pool=%d saves=%d",
			p.Char.Gold, w.gambleJackpot, w.gamblePool, store.saves)
	}
	if p.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("failed save must publish only an error message")
	}
}

func TestGambleInventoryFailureRollsBackBeforePersisting(t *testing.T) {
	jackpotStops := findGambleStops(t, func(result [5]byte) bool { return result[1] == 14 })
	w, p, store := gambleHandlerWorld(t, jackpotStops)
	p.Char.Gold = maxCharacterGold
	w.gambleJackpot = gambleJackpotMax
	for i := 0; i < model.PlayerCarrySlots; i++ {
		p.Char.Inv[i] = model.Item{Index: 1}
	}
	beforeGold, beforeInv := p.Char.Gold, p.Char.Inv

	w.onDoJackpotBet(p.Session, gambleBetPacket(1, 1_000))

	if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv || w.gambleJackpot != gambleJackpotMax || store.saves != 0 {
		t.Fatalf("inventory failure changed state: gold=%d jackpot=%d saves=%d", p.Char.Gold, w.gambleJackpot, store.saves)
	}
	if p.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("inventory failure must publish only an error message")
	}
}

func TestGambleOutcomeSaturatesSignedWirePrize(t *testing.T) {
	jackpotStops := findGambleStops(t, func(result [5]byte) bool { return result[1] == 14 })
	// Exercise the defensive signed-wire clamp even though the persisted pool
	// is normally bounded below MaxInt32 by restore and every bet update.
	w := &World{gambleJackpot: math.MaxUint32,
		rng: &gambleSequenceRNG{values: []int{99, int(jackpotStops[0]), int(jackpotStops[1]), int(jackpotStops[2])}}}
	if got := w.rollGamble(1, gambleMaxBet).prize; got != math.MaxInt32 {
		t.Fatalf("signed wire saturation=%d, want %d", got, math.MaxInt32)
	}
}

func TestGamblePoolsSnapshotAndRestoreAreBounded(t *testing.T) {
	w := &World{gambleJackpot: 123, gamblePool: 456}
	snapshot := w.instanceStateSnapshot()
	if snapshot.GambleJackpot != 123 || snapshot.GamblePool != 456 {
		t.Fatalf("snapshot lost Gamble pools: %+v", snapshot)
	}

	store := &gambleStore{load: &model.InstanceStateSnapshot{
		Version: model.InstanceStateVersion, GambleJackpot: math.MaxUint32, GamblePool: math.MaxUint64,
	}}
	restored := &World{store: store, nightmarePartyRuns: make(map[string]int)}
	if err := restored.restoreInstanceState(); err != nil {
		t.Fatal(err)
	}
	if restored.gambleJackpot != gambleJackpotMax || restored.gamblePool != gamblePoolMax {
		t.Fatalf("restored pools were not clamped: jackpot=%d pool=%d", restored.gambleJackpot, restored.gamblePool)
	}
}
