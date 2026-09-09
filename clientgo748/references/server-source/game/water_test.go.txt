package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestWaterChainContractTableDriven(t *testing.T) {
	tests := []struct {
		name       string
		reward     uint16
		chain      uint16
		allowGrace bool
		wantNext   uint16
		wantGrace  bool
	}{
		{name: "room 1 to room 2", reward: 3174, allowGrace: true, wantNext: 3174, wantGrace: true},
		{name: "room 8 to boss", reward: 3181, allowGrace: true, wantNext: 3181, wantGrace: true},
		{name: "boss to room 1 without reward", chain: 3173, allowGrace: true, wantNext: 3173, wantGrace: true},
		{name: "terminal instance", wantNext: 0, wantGrace: false},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			now := time.Unix(1_700_000_000, 0)
			cfg := model.VolatileInstance{RewardItem: tc.reward, ChainNextItem: tc.chain,
				AllowChainDuringExitGrace: tc.allowGrace}
			inst := &ItemInstance{Config: cfg, RewardGranted: true, ExitAt: now.Add(time.Second)}
			if got := expectedChainItem(&cfg); got != tc.wantNext {
				t.Fatalf("expectedChainItem=%d, want %d", got, tc.wantNext)
			}
			if got := itemInstanceInExitGraceAt(inst, now); got != tc.wantGrace {
				t.Fatalf("exit grace=%v, want %v", got, tc.wantGrace)
			}
		})
	}
}

func TestWaterExitGraceExpiresBeforeCleanupTick(t *testing.T) {
	now := time.Unix(1_700_000_000, 0)
	clock := newFakeClock(now.Add(11 * time.Second))
	inst := &ItemInstance{
		Config: model.VolatileInstance{ChainNextItem: 3181,
			AllowChainDuringExitGrace: true},
		MemberIDs:     []uint16{7},
		RewardGranted: true,
		ExitAt:        now.Add(10 * time.Second),
	}
	w := &World{clock: clock, itemInstances: map[string]*ItemInstance{"water-normal-8": inst}}
	if w.itemInstanceExitGraceForPlayer(7) != nil {
		t.Fatal("exit grace expirada continuou elegivel antes do tick")
	}
	if w.itemInstanceExitGraceAllowsItem(7, 3181) {
		t.Fatal("ticket da cadeia foi aceito depois de ExitAt")
	}
	// Until the cleanup tick removes the old room, membership remains a
	// blocking safety invariant and cannot be bypassed through its entry area.
	if w.itemInstanceForPlayer(7) != inst {
		t.Fatal("instancia expirada perdeu a barreira de membership antes do cleanup")
	}
}

func TestWaterChainRejectsEveryNonNextTicket(t *testing.T) {
	now := time.Unix(1_700_000_000, 0)
	inst := &ItemInstance{
		Config: model.VolatileInstance{ChainNextItem: 3181,
			AllowChainDuringExitGrace: true},
		MemberIDs:     []uint16{7},
		RewardGranted: true,
		ExitAt:        now.Add(time.Second),
	}
	w := &World{clock: newFakeClock(now), itemInstances: map[string]*ItemInstance{"water-normal-8": inst}}
	for _, itemID := range []uint16{3173, 3174, 3180, 3182, 785, 3190} {
		if w.itemInstanceExitGraceAllowsItem(7, itemID) {
			t.Fatalf("ticket %d aceito fora da progressao; proximo=3181", itemID)
		}
	}
	if !w.itemInstanceExitGraceAllowsItem(7, 3181) {
		t.Fatal("ticket correto foi recusado durante exit grace")
	}
}

func TestWaterRuntimeIDsDoNotReuseLiveRoom(t *testing.T) {
	w := &World{itemInstances: map[string]*ItemInstance{
		"water-normal-1":   {},
		"water-normal-1:1": {},
	}}
	cfg := &model.VolatileInstance{ID: "water-normal-1"}
	if got := w.nextItemInstanceRuntimeID(cfg); got != "water-normal-1:2" {
		t.Fatalf("runtime id reutilizado: got %q", got)
	}
}

func TestWaterLaterScrollCannotStartWithoutCompletedPredecessor(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	later := instanceTestRule()
	later.Instance.ID = "water-normal-2"
	later.Instance.ChainStart = false
	w.volatiles.Items[3174] = later
	w.volatiles.ItemCodes[3174] = 21
	leader.Char.Inv[0] = model.Item{Index: 3174}

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 3174 || len(w.itemInstances) != 0 || st.gameSaves != 0 {
		t.Fatalf("scroll posterior abriu sem Room 1 concluida: item=%d instances=%v saves=%d",
			leader.Char.Inv[0].Index, w.itemInstances, st.gameSaves)
	}
}

func TestWaterDetachAfterCommitLeavesOldRuntimeIndexed(t *testing.T) {
	w := &World{}
	old := &ItemInstance{MemberIDs: []uint16{1, 2, 3}, LeaderID: 1}
	w.detachInstanceMembersAfterCommit(old, map[uint16]struct{}{1: {}, 2: {}})
	if itemInstanceHasMember(old, 1) || itemInstanceHasMember(old, 2) ||
		!itemInstanceHasMember(old, 3) || old.LeaderID != 3 {
		t.Fatalf("membros antigos nao foram destacados corretamente: %+v", old)
	}
}
