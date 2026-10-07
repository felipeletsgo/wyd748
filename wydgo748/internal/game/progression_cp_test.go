package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestProgressionCPAwardsEveryActualLevelAndClamps(t *testing.T) {
	for cp := -75; cp <= 75; cp++ {
		for levels := 0; levels <= 30; levels++ {
			ch := testChar()
			ch.CP, ch.Score.Level, ch.Exp = int16(cp), 10, mortalNextLevel[10]
			reward := uint32(1)
			if levels > 0 {
				reward = mortalNextLevel[10+levels] - ch.Exp
			}
			gained, _ := grantExp(&ch, reward)
			want := min(cp+5*levels, 75)
			if gained != levels || int(ch.CP) != want {
				t.Fatalf("CP=%d levels=%d: gained=%d CP=%d, want %d", cp, levels, gained, ch.CP, want)
			}
		}
	}
}

func TestProgressionCPNoAwardWithoutLevel(t *testing.T) {
	for _, mode := range []string{"zero-reward", "cap", "debt-only", "arch-lock", "celestial-lock"} {
		t.Run(mode, func(t *testing.T) {
			ch := testChar()
			ch.CP, ch.Score.Level, ch.Exp = -20, 10, mortalNextLevel[10]
			reward := uint32(1)
			switch mode {
			case "zero-reward":
				reward = 0
			case "cap":
				ch.Score.Level, ch.Exp = maxMortalLevel, mortalNextLevel[400]
			case "debt-only":
				ch.Hold = reward
				levels, applied := grantCombatExp(&ch, reward)
				if levels != 0 || applied != 0 || ch.CP != -20 || ch.Hold != 0 {
					t.Fatal("debt repayment must not award CP")
				}
				return
			case "arch-lock":
				ch.Evolution, ch.Score.Level = "arch", archLockLevel355
				ch.Exp = mortalNextLevel[ch.Score.Level]
			case "celestial-lock":
				ch.Evolution, ch.Score.Level = "celestial", 39
				ch.Exp = celestialNextLevel[39]
			}
			levels, _ := grantExp(&ch, reward)
			if levels != 0 || ch.CP != -20 {
				t.Fatal("a rejected or non-leveling reward must preserve CP")
			}
		})
	}
}

func TestProgressionCPPersistsAndPublishesOnlyAfterAcknowledgement(t *testing.T) {
	for _, outcome := range []string{"commit", "rejected", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			if outcome == "rejected" {
				backend.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				backend.failure = errors.New("injected uncertain commit")
			}
			p.Char.CP, p.Char.Score.Level, p.Char.Exp = 0, 10, mortalNextLevel[11]-1
			p.Char.Inv[0] = model.Item{Index: 412, UID: "cp-reward"}
			observer, _ := networkedTestPlayer(2, "Observer", p.X+1, p.Y)
			w.players[observer.Session], w.playersByID[observer.ID] = observer, observer
			w.indexPlayerCharacter(observer)
			w.updatePlayerSpatial(observer)
			observer.show(p.ID)
			before := accountStateSnapshot(p.Account)
			if err := w.queueItemExperience(p, 0, model.VolatileRule{Action: "grant_exp", Exp: 1, Consume: true}, 0); err != nil {
				t.Fatal(err)
			}
			write := awaitCriticalWrite(t, backend.entered)
			if len(write.accounts) != 1 || write.accounts[0].Chars[0].CP != 5 || write.accounts[0].Chars[0].Score.Level != 11 {
				t.Fatal("level and CP must belong to the same detached durable proposal")
			}
			assertCriticalCargoUnchanged(t, p, before, fallback)
			if p.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("pending CP leaked to a client")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			assertCriticalCargoUnchanged(t, p, before, fallback)
			w.processCriticalCompletions(1)
			if outcome != "commit" {
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if observer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("uncommitted CP was broadcast")
				}
				if outcome == "unknown" && (!p.PersistencePoisoned || !w.criticalPlayerReserved(p)) {
					t.Fatal("uncertain CP reward must retain its fence")
				}
				return
			}
			if p.Char.CP != 5 || p.Char.Score.Level != 11 || p.Char.Inv[0].Index != 0 || w.criticalPlayerReserved(p) {
				t.Fatal("acknowledgement did not install the committed CP reward")
			}
			for _, receiver := range []*Player{p, observer} {
				updates := 0
				for {
					packet, ok := receiver.Session.DequeuePacketForTest()
					if !ok {
						break
					}
					if !wire.Decrypt(packet) {
						t.Fatal("invalid encrypted publication")
					}
					if wire.ParseHeader(packet).Type == wire.OpCreateMob {
						updates++
						if len(packet) <= 30 || packet[30] != 80 {
							t.Fatal("CreateMob MobName[12] must carry committed signed CP + 75")
						}
					}
				}
				if updates != 1 {
					t.Fatalf("receiver %d got %d CP updates, want one", receiver.ID, updates)
				}
			}
		})
	}
}
