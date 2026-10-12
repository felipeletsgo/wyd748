package game

import (
	"fmt"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

// Exercise the production handlers, including slice growth before selection
// and a transport closure while a kill reward is still being persisted.
func TestCriticalCreatedCharactersRewardsAndDisconnect(t *testing.T) {
	for _, workers := range []int{1, 4} {
		for _, pendingKill := range []bool{false, true} {
			t.Run(fmt.Sprintf("workers=%d/pending-kill=%t", workers, pendingKill), func(t *testing.T) {
				w, p, session := newEnterWorldPlayer(t, 100, 100)
				p.Account.Name = "created-session"
				p.Account.Chars[0].Name = "Original"
				p.Account.Chars[0].UID = "original-character"
				installTestCharacterTemplates(w)
				claimCriticalTestPlayers(t, w, p)
				permits := make(chan struct{}, 1)
				base := &criticalTestStore{entered: make(chan criticalWrite, 16), release: permits}
				backend := &criticalStateTestStore{&criticalBatchTestStore{base}}
				queue := newCriticalTestQueue(t, backend, workers, 8)
				t.Cleanup(func() { close(permits) })
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				w.criticalAccountContainment = true
				settle := func() {
					t.Helper()
					permits <- struct{}{}
					awaitReadyCriticalResults(t, queue, 1)
					w.processCriticalCompletions(1)
					if w.shuttingDown || p.PersistencePoisoned {
						t.Fatalf("valid lifecycle was quarantined: %v", w.shutdownErr)
					}
				}
				for slot := 1; slot < 4; slot++ {
					name := []string{"Original", "CreatedFoema", "CreatedBeast", "CreatedHunter"}[slot]
					w.onCreateCharacter(session, newCharacterPacket(slot, name, slot))
					write := awaitCriticalWrite(t, base.entered)
					if write.accounts[0].Chars[slot].Name != name || p.Char != nil {
						t.Fatal("creation did not retain selection and the detached proposal")
					}
					settle()
					drainPlayerPackets(p)
				}
				w.onEnterWorld(session, enterWorldPacket(2))
				if !p.InWorld || p.Char != &p.Account.Chars[2] || p.Char.UID == "" {
					t.Fatal("entry did not bind the newly created character's current slot")
				}
				selected, uid, id := p.Char, p.Char.UID, p.ID
				w.skills = map[int]model.SkillDef{
					41: {Index: 41, AffectType: 2, AffectValue: 1, AffectTime: 150},
					43: {Index: 43, AffectType: 11, AffectValue: 15, AffectTime: 150},
					44: {Index: 44, AffectType: 9, AffectValue: 5, AffectTime: 150},
					45: {Index: 45, AffectType: 15, AffectValue: 7, AffectTime: 150},
				}
				master := &Mob{ID: 1900, X: p.X + 1, Y: p.Y, Def: &model.NPCDef{
					Name: "Carb_Mstr1", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 43},
				}}
				w.registerMobSpatial(master)
				p.show(master.ID)
				w.onUseNPC(session, useNPCPacket(master.ID))
				blessing := awaitCriticalWrite(t, base.entered)
				if blessing.character == nil || blessing.character.uid != uid {
					t.Fatal("beginner blessing was not persisted atomically with its character")
				}
				settle()
				for _, kind := range []byte{2, 11, 9, 15} {
					if activePlayerAffect(p.Char, kind) == nil {
						t.Fatalf("acknowledged blessing lost affect %d", kind)
					}
				}
				for kill := 0; kill < 4; kill++ {
					drainPlayerPackets(p)
					mob := &Mob{ID: uint16(1400 + kill), X: p.X + 1, Y: p.Y, GenerIndex: -1,
						Def: &model.NPCDef{Name: "RewardMob", Tipo: model.TipoMonstro, ExpReward: 10200,
							Score: &model.Score{Version: model.ScoreVersion, Level: 20, MaxHP: 100, CurHP: 100}}}
					w.appendMobInstance(mob)
					w.registerMobSpatial(mob)
					w.killMobState(p, mob, 100, 100)
					awaitCriticalWrite(t, base.entered)
					if pendingKill && kill == 3 {
						w.handle(command{s: session})
						w.handle(command{s: session})
						w.resumeCriticalDisconnects(8)
						if p.Char != selected || w.players[session] != p || len(queue.pending) != 1 {
							t.Fatal("disconnect changed or prematurely released the pending participant")
						}
					}
					settle()
				}
				if !pendingKill {
					w.handle(command{s: session})
				}
				final := awaitCriticalWrite(t, base.entered)
				if final.character == nil || final.character.uid != uid || len(final.accounts[0].Chars) != 4 ||
					final.accounts[0].Chars[2].Exp == 0 || final.accounts[0].Chars[2].Score.Level == 0 ||
					final.accounts[0].Chars[2].CP <= 0 {
					t.Fatal("final atomic snapshot lost the selected character, rewards or character list")
				}
				w.handle(command{s: session})
				w.resumeCriticalDisconnects(8)
				if w.claimAccountSession(net.NewTestSession(2, 8), p.Account.Name) {
					t.Fatal("readmission bypassed the unacknowledged final write")
				}
				settle()
				if w.players[session] != nil || w.playersByID[id] != nil || w.playersByCharacterUID[uid] != nil ||
					len(queue.pending) != 0 || len(w.criticalDisconnects) != 0 || len(w.criticalQuarantine) != 0 ||
					!w.claimAccountSession(net.NewTestSession(3, 8), p.Account.Name) {
					t.Fatal("acknowledged disconnect stranded ownership or an orphan session")
				}
				w.handle(command{s: session})
				if len(base.entered) != 0 || len(queue.pending) != 0 {
					t.Fatal("repeated transport closure repeated the final write")
				}
			})
		}
	}
}
