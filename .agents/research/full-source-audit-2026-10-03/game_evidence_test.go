package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

// These evidence tests describe the audited defects, not desired behavior.
// Inverting their expectations is necessary before using them as fix gates.
func TestAuditEvidenceRemoteSkillLearning(t *testing.T) {
	w, p, st, master := skillMasterPacketTestWorld(t)
	p.X += 100
	p.hide(master.ID)
	if _, err := w.resolveNPCInteraction(p, master.ID); err == nil {
		t.Fatal("fixture must fail the authoritative NPC boundary")
	}
	w.onApplyBonus(p.Session, skillMasterApplyBonusPacket(5000, master.ID))
	if p.Char.LearnedSkill&1 == 0 || st.saves != 1 {
		t.Fatal("audited remote-learning defect was not reproduced")
	}
	t.Log("ApplyBonus learned and saved a skill despite an out-of-range, invisible master")
}

func TestAuditEvidenceGhostShopCrossSpacePublication(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.itemInstances = map[string]*ItemInstance{
		"audit-private": {RuntimeID: "audit-private", MemberIDs: []uint16{p.ID}},
	}
	w.setPlayerInstanceIndex(p.ID, "audit-private")
	if w.gameplaySpaceForPlayer(p) != "audit-private" {
		t.Fatal("fixture has no authoritative private membership")
	}
	shop := &GhostShop{ID: 700, Name: "PublicShop", X: p.X, Y: p.Y}
	w.showGhostShop(p, shop)
	if !p.hasVisible(shop.ID) {
		t.Fatal("audited cross-space publication defect was not reproduced")
	}
	t.Log("public ghost shop was published to a private runtime participant at the same coordinates")
}

type auditSplitStateStore struct {
	craftStore
	state *model.CharState
}

func (s *auditSplitStateStore) LoadCharState(string) (*model.CharState, error) {
	return s.state, nil
}

func (s *auditSplitStateStore) SaveCharState(_ string, state *model.CharState) error {
	s.state = state
	return nil
}

func TestAuditEvidenceSplitStateCommit(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	st := &auditSplitStateStore{craftStore: craftStore{err: errors.New("account write failed")}}
	w.store = st
	state := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"audit": 1}}
	if err := w.saveAccountAndCharStateForUID(p.Account, "audit-uid", state); err == nil {
		t.Fatal("fixture must reject the account write")
	}
	if st.state != state || st.saves != 1 {
		t.Fatal("audited partial commit was not reproduced")
	}
	t.Log("charstate changed before the failing account write; no compensating write occurred")
}
