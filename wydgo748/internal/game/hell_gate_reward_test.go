package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func hellGateRewardTestInstance(w *World, leader, member *Player, now time.Time) *ItemInstance {
	cfg := *instanceTestRule().Instance
	cfg.ID = "hell-gate-reward"
	cfg.Name = "Hell Gate Reward Test"
	cfg.StateMachine = "hell_gate"
	cfg.RewardItem = 0
	cfg.HellGate = &model.VolatileHellGate{Version: 2, RewardEXP: 100}
	inst := &ItemInstance{
		Config:              cfg,
		RuntimeID:           cfg.ID,
		LeaderID:            leader.ID,
		LeaderCharacterUID:  leader.Char.UID,
		MemberIDs:           []uint16{leader.ID, member.ID},
		MemberCharacterUIDs: []string{leader.Char.UID, member.Char.UID},
		State:               "protection",
		HardDeadline:        now,
		HellGateDionysHP:    1,
		NPCIDs:              map[uint16]struct{}{900: {}},
	}
	w.itemInstances = map[string]*ItemInstance{inst.RuntimeID: inst}
	w.rebuildPlayerInstanceIndex()
	return inst
}

func TestGrantHellGateExperienceIsAtomicAndExactlyOnce(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	inst := hellGateRewardTestInstance(w, leader, member, clock.Now())
	claimCriticalTestPlayers(t, w, leader, member)

	leaderBefore, memberBefore := leader.Char.Exp, member.Char.Exp
	now := clock.Now()
	if !w.grantHellGateExperience(inst, now) {
		t.Fatal("Hell Gate EXP reward was not committed")
	}
	if leader.Char.Exp != leaderBefore+100 || member.Char.Exp != memberBefore+100 {
		t.Fatalf("wrong EXP grant: leader=%d/%d member=%d/%d",
			leader.Char.Exp, leaderBefore+100, member.Char.Exp, memberBefore+100)
	}
	if !inst.RewardGranted || !inst.ExitAt.Equal(now.Add(10*time.Second)) {
		t.Fatalf("missing reward marker or exit grace: granted=%v exit=%v", inst.RewardGranted, inst.ExitAt)
	}
	if st.saves != 1 {
		t.Fatalf("expected one atomic reward save, got %d", st.saves)
	}

	if !w.grantHellGateExperience(inst, now) {
		t.Fatal("repeated Hell Gate reward was not treated as already committed")
	}
	if leader.Char.Exp != leaderBefore+100 || member.Char.Exp != memberBefore+100 {
		t.Fatal("repeated Hell Gate reward changed EXP")
	}
	if st.saves != 1 {
		t.Fatalf("repeated Hell Gate reward wrote again: %d saves", st.saves)
	}
}

func TestHellGateDeadlineRewardsSurvivingProtectorOnlyOnce(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	inst := hellGateRewardTestInstance(w, leader, member, clock.Now())
	claimCriticalTestPlayers(t, w, leader, member)
	w.mobsByID[900] = &Mob{ID: 900, HP: 1, InstanceID: inst.RuntimeID}
	inst.HardDeadline = clock.Now().Add(time.Second)
	before := leader.Char.Exp
	if !w.tickHellGate(inst, clock.Now()) || inst.RewardGranted {
		t.Fatal("survival reward was granted before the deadline")
	}
	clock.Advance(time.Second)
	if !w.tickHellGate(inst, clock.Now()) || !inst.RewardGranted || leader.Char.Exp != before+100 {
		t.Fatal("a surviving protector did not commit the reward at the deadline")
	}
	snapshot := w.instanceStateSnapshot()
	if len(snapshot.Instances) != 1 || !snapshot.Instances[0].RewardGranted {
		t.Fatal("the post-deadline snapshot lost the committed reward marker")
	}
	if !w.tickHellGate(inst, clock.Now()) || leader.Char.Exp != before+100 {
		t.Fatal("the exit grace granted the reward again")
	}
	clock.Advance(10 * time.Second)
	if w.tickHellGate(inst, clock.Now()) {
		t.Fatal("the expired exit grace did not release normal cleanup")
	}
}

func TestGrantHellGateExperiencePreservesEmptyVitals(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	inst := hellGateRewardTestInstance(w, leader, member, clock.Now())
	claimCriticalTestPlayers(t, w, leader, member)
	setPlayerCurHP(leader.Char, 0)
	setPlayerCurMP(leader.Char, 0)
	if !w.grantHellGateExperience(inst, clock.Now()) {
		t.Fatal("Hell Gate EXP reward failed")
	}
	if playerCurHP(leader.Char) != 0 || playerCurMP(leader.Char) != 0 {
		t.Fatal("completion EXP resurrected a character or refilled empty mana")
	}
}

func TestGrantHellGateExperienceExcludesDisconnectedMember(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	inst := hellGateRewardTestInstance(w, leader, member, clock.Now())
	claimCriticalTestPlayers(t, w, leader, member)
	member.InWorld = false
	member.Session.Close()

	leaderBefore, memberBefore := leader.Char.Exp, member.Char.Exp
	if !w.grantHellGateExperience(inst, clock.Now()) {
		t.Fatal("Hell Gate EXP reward with a disconnected member was not committed")
	}
	if leader.Char.Exp != leaderBefore+100 || member.Char.Exp != memberBefore {
		t.Fatalf("wrong online/disconnected EXP result: leader=%d/%d member=%d/%d",
			leader.Char.Exp, leaderBefore+100, member.Char.Exp, memberBefore)
	}
	if !inst.RewardGranted || st.saves != 1 {
		t.Fatalf("disconnected-member reward was not persisted: granted=%v saves=%d",
			inst.RewardGranted, st.saves)
	}
}

func TestGrantHellGateExperienceHonorsCappedParticipant(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	inst := hellGateRewardTestInstance(w, leader, member, clock.Now())
	claimCriticalTestPlayers(t, w, leader, member)

	leader.Char.Score.Level = maxMortalLevel
	leader.Char.Exp = mortalNextLevel[len(mortalNextLevel)-1]
	syncProgression(leader.Char)
	applyScore(leader.Char)
	leaderBefore, memberBefore := leader.Char.Exp, member.Char.Exp
	if _, applied := grantExp(leader.Char, inst.Config.HellGate.RewardEXP); applied != 0 {
		t.Fatalf("fixture participant was not capped: applied=%d", applied)
	}

	if !w.grantHellGateExperience(inst, clock.Now()) {
		t.Fatal("capped participant blocked the Hell Gate reward")
	}
	if leader.Char.Exp != leaderBefore {
		t.Fatalf("capped participant EXP changed: got %d want %d", leader.Char.Exp, leaderBefore)
	}
	if member.Char.Exp != memberBefore+inst.Config.HellGate.RewardEXP {
		t.Fatalf("uncapped participant missed reward: got %d want %d",
			member.Char.Exp, memberBefore+inst.Config.HellGate.RewardEXP)
	}
	if !inst.RewardGranted {
		t.Fatal("capped-participant reward did not commit its instance marker")
	}
}

func TestGrantHellGateExperienceRollsBackOnPersistenceFailure(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	inst := hellGateRewardTestInstance(w, leader, member, clock.Now())
	claimCriticalTestPlayers(t, w, leader, member)
	st.err = errors.New("injected Hell Gate reward failure")

	leaderBefore, memberBefore := leader.Char.Exp, member.Char.Exp
	if w.grantHellGateExperience(inst, clock.Now()) {
		t.Fatal("failed Hell Gate reward reported success")
	}
	if leader.Char.Exp != leaderBefore || member.Char.Exp != memberBefore {
		t.Fatal("failed Hell Gate reward did not roll back account EXP")
	}
	if inst.RewardGranted || !inst.ExitAt.IsZero() || !inst.ExitDeadline.IsZero() {
		t.Fatalf("failed Hell Gate reward left instance marker: granted=%v exit=%v deadline=%v",
			inst.RewardGranted, inst.ExitAt, inst.ExitDeadline)
	}
}

func TestQueueCriticalHellGateExperienceCommitsAccountsAndInstanceOnce(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	inst := hellGateRewardTestInstance(w, leader, member, clock.Now())
	claimCriticalTestPlayers(t, w, leader, member)

	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
	backend := &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	leaderBefore, memberBefore := leader.Char.Exp, member.Char.Exp
	now := clock.Now()
	if err := w.queueCriticalInstanceReward(inst, now); err != nil {
		t.Fatalf("critical Hell Gate reward admission failed: %v", err)
	}

	write := awaitCriticalWrite(t, base.entered)
	if len(write.accounts) != 2 {
		t.Fatalf("critical reward wrote %d accounts, want 2", len(write.accounts))
	}
	for _, account := range write.accounts {
		switch account.Name {
		case leader.Account.Name:
			if account.Chars[0].Exp != leaderBefore+100 {
				t.Fatalf("detached leader EXP=%d, want %d", account.Chars[0].Exp, leaderBefore+100)
			}
		case member.Account.Name:
			if account.Chars[0].Exp != memberBefore+100 {
				t.Fatalf("detached member EXP=%d, want %d", account.Chars[0].Exp, memberBefore+100)
			}
		default:
			t.Fatalf("unexpected critical reward account %q", account.Name)
		}
	}
	if len(write.instances.Instances) != 1 {
		t.Fatalf("critical reward snapshot contains %d instances, want 1", len(write.instances.Instances))
	}
	saved := write.instances.Instances[0]
	if !saved.RewardGranted || !saved.ExitAt.Equal(now.Add(10*time.Second)) {
		t.Fatalf("critical reward snapshot lost marker/grace: granted=%v exit=%v", saved.RewardGranted, saved.ExitAt)
	}
	if leader.Char.Exp != leaderBefore || member.Char.Exp != memberBefore || inst.RewardGranted {
		t.Fatal("critical reward became visible before acknowledgement")
	}

	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if leader.Char.Exp != leaderBefore+100 || member.Char.Exp != memberBefore+100 ||
		!inst.RewardGranted || !inst.ExitAt.Equal(now.Add(10*time.Second)) {
		t.Fatal("critical Hell Gate reward did not install after acknowledgement")
	}
	if err := w.queueCriticalInstanceReward(inst, now); !errors.Is(err, errCriticalPersistenceIdentity) {
		t.Fatalf("duplicate critical Hell Gate reward returned %v", err)
	}
}
