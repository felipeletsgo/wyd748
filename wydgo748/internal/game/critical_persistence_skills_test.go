package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func configureCriticalSkill(w *World, p *Player, master *Mob, local int) {
	p.Char.Class, p.Char.Score.Level, p.Char.Gold = 0, 20, 3_000_000
	p.Char.LearnedSkill = 0
	if local == 7 {
		p.Char.LearnedSkill = 0x7f
	}
	for i := range p.Char.ShortSkill {
		p.Char.ShortSkill[i] = 0xff
	}
	p.Char.ShortSkill[0], p.Char.ShortSkill[1] = byte(local), 23
	p.Char.Inv[0] = model.Item{Index: 200, UID: "skill-unrelated-item"}
	w.skills = map[int]model.SkillDef{
		0: {Index: 0, Name: "Basic", SkillPoint: 1},
		7: {Index: 7, Name: "Ultimate", SkillPoint: 1},
	}
	if w.items == nil {
		w.items = make(map[uint16]model.ItemDef)
	}
	w.items[uint16(5000+local)] = model.ItemDef{Index: uint16(5000 + local)}
	master.Def.Score.Merchant = skillMasterMerchant
	master.Def.Vende = []model.Item{{Index: uint16(5000 + local)}}
	p.ShopNPC = master.ID
	applyScore(p.Char)
}

func criticalSkillFixture(t *testing.T, local int) (*World, *Player, *craftStore, *Mob, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, fallback, master := skillMasterPacketTestWorld(t)
	configureCriticalSkill(w, p, master, local)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("skill fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, fallback, master, backend, queue, release
}

func criticalSkillRequest(kind string, id, master uint16, local int) []byte {
	packet := inboundPacket(wire.OpBuyItem, 24)
	binary.LittleEndian.PutUint16(packet[12:14], master)
	if kind == "bonus" {
		packet = inboundPacket(wire.OpApplyBonus, applyBonusPacketSize)
		binary.LittleEndian.PutUint16(packet[12:14], 2)
		binary.LittleEndian.PutUint16(packet[14:16], uint16(5000+local))
		binary.LittleEndian.PutUint16(packet[16:18], master)
	}
	binary.LittleEndian.PutUint16(packet[6:8], id)
	return packet
}

func assertCriticalSkillPublication(t *testing.T, p *Player) {
	t.Helper()
	for _, want := range [][]byte{playerScorePacket(p), wire.UpdateEtc(p.ID, *p.Char), wire.SetShortSkill(p.ID, p.Char.ShortSkill)} {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || len(packet) != len(want) {
			t.Fatal("missing or incorrectly sized skill response")
		}
		header, expected := wire.ParseHeader(packet), wire.ParseHeader(want)
		if header.Type != expected.Type || header.ID != expected.ID || int(header.Size) != len(want) || !bytes.Equal(packet[12:], want[12:]) {
			t.Fatal("skill response opcode, recipient, order or body differs from committed state")
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("skill learning published an extra response")
	}
}

func TestCriticalSkillDetachedCommitAndRepeatedRequests(t *testing.T) {
	for _, kind := range []string{"buy", "bonus"} {
		for _, local := range []int{0, 7} {
			for _, slot := range []int{0, 1} {
				t.Run(fmt.Sprintf("%s/skill-%d/slot-%d", kind, local, slot), func(t *testing.T) {
					w, p, fallback, master, backend, queue, release := criticalSkillFixture(t, local)
					if slot == 1 {
						p.Account.Chars = append(p.Account.Chars, cloneCharacterState(p.Char))
						p.CharSlot, p.Char = 1, &p.Account.Chars[1]
						p.Char.UID += "-second"
						w.indexPlayerCharacter(p)
					}
					before := accountStateSnapshot(p.Account)
					account, character, score := p.Account, p.Char, p.Char.Score
					// Derive the complete expected persisted proposal, including points,
					// vitals and shortcut filtering, independently of the continuation.
					wantLive := accountStateSnapshot(p.Account)
					ch := &wantLive.Chars[slot]
					w.recalcPlayer(ch)
					if local == 7 {
						ch.Gold -= ultimateSkillGold
					}
					ch.LearnedSkill |= 1 << local
					w.recalcPlayer(ch)
					filterShortSkills(ch)
					request := criticalSkillRequest(kind, p.ID, master.ID, local)
					w.handle(command{s: p.Session, pkt: request})
					proposal := awaitCriticalWrite(t, backend.entered)
					if !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(wantLive)) || proposal.accounts[0] == account || proposal.accounts[0].Chars[slot].Score == score {
						t.Fatal("skill proposal is incomplete, incorrect or aliases live state")
					}
					checkHeld := func() {
						w.handle(command{s: p.Session, pkt: request})
						w.onLearnSkillAtMaster(p.Session, p, 5000+local, master.ID)
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 || len(queue.pending) != 1 || !w.criticalPlayerReserved(p) {
							t.Fatal("held skill write mutated, saved inline, confirmed, repeated or released ownership")
						}
					}
					checkHeld()
					release()
					awaitReadyCriticalResults(t, queue, 1)
					checkHeld()
					discardCriticalDeferredDuplicates(w)

					w.processCriticalCompletions(1)
					if p.Account != account || p.Char != character || p.Char.Score != score || !reflect.DeepEqual(accountStateSnapshot(p.Account), wantLive) || fallback.saves != 0 || len(queue.pending) != 0 {
						t.Fatal("skill commit replaced live identity, lost prepared fields or wrote inline")
					}
					assertCriticalSkillPublication(t, p)
					w.handle(command{s: p.Session, pkt: request})
					if dequeueSkillMessage(t, p) != "This skill has already been learned." || len(queue.pending) != 0 || len(backend.entered) != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), wantLive) {
						t.Fatal("post-commit duplicate charged or saved skill learning twice")
					}
				})
			}
		}
	}
}

func TestCriticalSkillRejectionDoesNotMutateLiveScore(t *testing.T) {
	for _, reason := range []string{"points", "level", "mastery", "chain", "other-ultimate", "gold", "catalog", "class", "distant", "hidden", "wrong-master", "wrong-character", "unclaimed", "closed", "stopping", "full"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, master, backend, queue, _ := criticalSkillFixture(t, 7)
			switch reason {
			case "points":
				def := w.skills[7]
				def.SkillPoint = 100000
				w.skills[7] = def
			case "level":
				def := w.items[5007]
				def.ReqLevel = 399
				w.items[5007] = def
			case "mastery":
				def := w.items[5007]
				def.ReqInt = 1000
				w.items[5007] = def
			case "chain":
				p.Char.LearnedSkill &^= 1
			case "other-ultimate":
				p.Char.LearnedSkill |= 1 << 15
			case "gold":
				p.Char.Gold = ultimateSkillGold - 1
			case "catalog":
				delete(w.skills, 7)
			case "class":
				p.Char.Class = 1
			case "distant":
				master.X = p.X + 9
			case "hidden":
				p.hide(master.ID)
			case "wrong-master":
				p.ShopNPC++
			case "wrong-character":
				ch := cloneCharacterState(p.Char)
				p.Char = &ch
			case "unclaimed":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			case "closed":
				p.Session.Close()
			case "stopping":
				queue.closeAdmission()
			case "full":
				for _, name := range []string{"foreign-one", "foreign-two"} {
					if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
			}
			// If validation recalculates the live character, this stale balance
			// would change even though learning is rejected.
			p.Char.Score.SkillPts = 123456
			before, charBefore := accountStateSnapshot(p.Account), cloneCharacterState(p.Char)
			w.onLearnSkillAtMaster(p.Session, p, 5007, master.ID)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(*p.Char, charBefore) || fallback.saves != 0 || len(backend.entered) != 0 || p.criticalLifecycle != nil || w.criticalPlayerReserved(p) || (reason != "full" && len(queue.pending) != 0) {
				t.Fatal("rejected skill learning changed live data, saved inline or retained a proposal")
			}
			for p.Session.QueuedPacketsForTest() > 0 {
				packet, _ := p.Session.DequeuePacketForTest()
				if !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpMessagePanel {
					t.Fatal("rejection published skill state")
				}
			}
		})
	}
}

func TestCriticalSkillUnknownOutcomeNeverPublishesOrOverwrites(t *testing.T) {
	for _, reason := range []string{"error", "panic", "identity"} {
		t.Run(reason, func(t *testing.T) {
			w, p, fallback, master, backend, queue, release := criticalSkillFixture(t, 7)
			if reason == "error" {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			backend.panicAfterWrite = reason == "panic"
			before := accountStateSnapshot(p.Account)
			request := criticalSkillRequest("bonus", p.ID, master.ID, 7)
			w.handle(command{s: p.Session, pkt: request})
			awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if reason == "identity" {
				p.criticalLifecycle = &criticalPlayerLifecycle{}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			w.onLearnSkillAtMaster(p.Session, p, 5007, master.ID)
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 || !w.shuttingDown || !p.PersistencePoisoned || !w.criticalPlayerReserved(p) || !p.Session.IsClosed() || p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 || w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
				t.Fatal("uncertain learning applied, confirmed, replayed or released account ownership")
			}
			done := make(chan error, 1)
			w.runShutdown(done)
			if err := <-done; err == nil || fallback.saves != 0 {
				t.Fatal("shutdown overwrote an uncertain skill transaction")
			}
		})
	}
}

func TestCriticalSkillDisconnectFinalSnapshotContainsCommit(t *testing.T) {
	for _, kind := range []string{"buy", "bonus"} {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, master, backend, queue, release := criticalSkillFixture(t, 7)
			queue.store = &criticalStateTestStore{&criticalBatchTestStore{backend}}
			w.handle(command{s: p.Session, pkt: criticalSkillRequest(kind, p.ID, master.ID, 7)})
			proposal := awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if fallback.saves != 0 || len(backend.entered) != 0 || p.Char.Gold != 3_000_000 {
				t.Fatal("disconnect saved or applied uncommitted skill state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			final := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(final.accounts, proposal.accounts) || final.character == nil || final.character.uid != p.Char.UID || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) || p.Session.QueuedPacketsForTest() != 0 || p.Char.Gold != 1_000_000 || p.Char.LearnedSkill != 0xff || fallback.saves != 0 || w.claimAccountSession(gameNet.NewTestSession(98, 8), p.Account.Name) {
				t.Fatal("final disconnect snapshot lost learned skill, gold, score or account ownership")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.handle(command{s: p.Session})
			if len(backend.entered) != 0 || !w.criticalPlayerReserved(p) {
				t.Fatal("final unacknowledged save repeated or released ownership")
			}
			w.processCriticalCompletions(1)
			if w.players[p.Session] != nil || len(queue.pending) != 0 || fallback.saves != 0 || !w.claimAccountSession(gameNet.NewTestSession(99, 8), p.Account.Name) {
				t.Fatal("disconnect acknowledgement did not release account ownership")
			}
		})
	}
}

func TestCriticalSkillShutdownWaitsForCommittedSnapshot(t *testing.T) {
	for _, kind := range []string{"buy", "bonus"} {
		t.Run(kind, func(t *testing.T) {
			w, p, fallback, master, backend, queue, release := criticalSkillFixture(t, 7)
			terminal := &criticalPurchaseTerminalStore{craftStore: fallback}
			w.store = terminal
			before := accountStateSnapshot(p.Account)
			w.handle(command{s: p.Session, pkt: criticalSkillRequest(kind, p.ID, master.ID, 7)})
			proposal := awaitCriticalWrite(t, backend.entered)
			first, second := make(chan error, 1), make(chan error, 1)
			w.runShutdown(first)
			w.runShutdown(second)
			w.handle(command{s: p.Session})
			if terminal.account != nil || fallback.saves != 0 || terminal.flushes != 0 || !queue.stopping || w.shutdownFinished || !p.Session.IsClosed() || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
				t.Fatal("shutdown saved uncommitted skill state before acknowledgement")
			}
			for _, done := range []chan error{first, second} {
				select {
				case <-done:
					t.Fatal("shutdown finished while accepted skill write was held")
				default:
				}
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			for _, done := range []chan error{first, second} {
				select {
				case err := <-done:
					if err != nil {
						t.Fatal(err)
					}
				default:
					t.Fatal("shutdown did not complete after skill acknowledgement")
				}
			}
			if terminal.account == nil || !reflect.DeepEqual(accountPersistenceSnapshot(terminal.account), proposal.accounts[0]) || p.Char.Gold != 1_000_000 || p.Char.LearnedSkill != 0xff || fallback.saves != 1 || terminal.flushes != 1 || !w.shutdownFinished || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("terminal save lost skill fields, repeated persistence or published to closed transport")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || fallback.saves != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed terminal skill state")
			}
		})
	}
}

func TestCriticalSkillAdvancedEvolutionPreservesRequirements(t *testing.T) {
	for _, evolution := range []string{"", archEvolution, "celestial", "subcelestial"} {
		for _, masteryRequired := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/mastery-%t", evolution, masteryRequired), func(t *testing.T) {
				w, p, fallback, master, backend, queue, release := criticalSkillFixture(t, 0)
				p.Char.Evolution, p.Char.Score.Level = evolution, 0
				def := w.items[5000]
				def.ReqLevel = 399
				if masteryRequired {
					def.ReqInt = 1000
				}
				w.items[5000] = def
				before := accountStateSnapshot(p.Account)
				w.handle(command{s: p.Session, pkt: criticalSkillRequest("bonus", p.ID, master.ID, 0)})
				if evolution == "" || masteryRequired {
					if len(queue.pending) != 0 || len(backend.entered) != 0 || fallback.saves != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
						t.Fatal("level bypass ignored another requirement or mutated on rejection")
					}
					return
				}
				awaitCriticalWrite(t, backend.entered)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if p.Char.LearnedSkill != 1 || fallback.saves != 0 {
					t.Fatal("staged learning lost advanced-evolution level exemption")
				}
				assertCriticalSkillPublication(t, p)
			})
		}
	}
}

func TestCriticalSkillActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, kind := range []string{"buy", "bonus"} {
		t.Run(kind, func(t *testing.T) {
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, nil,
				func(w *World, p *Player) { configureCriticalSkill(w, p, w.mobsByID[1102], 7) })
			request := criticalSkillRequest(kind, 1, 1102, 7)
			deliver := func() {
				select {
				case w.commands <- command{s: p.Session, pkt: request}:
				case <-time.After(time.Second):
					t.Fatal("World did not accept skill intention")
				}
			}
			deliver()
			awaitCriticalWrite(t, backend.entered)
			for range 3 {
				deliverCriticalLoopPing(t, w, p)
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				regenerated := false
				for _, other := range overview.Players {
					if other.UID == unrelatedUID && other.HP > 500 {
						regenerated = true
					}
				}
				if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
					t.Fatal("held skill save stalled other players, repeated or confirmed early")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for p.Session.QueuedPacketsForTest() == 0 && time.Now().Before(deadline) {
				readCriticalLoopOverview(t, w)
			}
			stop() // Join the actor before inspecting its mutable state.
			if p.Char.Gold != 1_000_000 || p.Char.LearnedSkill != 0xff || len(queue.pending) != 0 || w.shuttingDown || w.mobTickCounter < 3 {
				t.Fatal("actual actor did not apply skill commit while advancing the world")
			}
			assertCriticalSkillPublication(t, p)
		})
	}
}
