package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

// Inspect the actual encrypted session output, not a replacement broadcaster.
func assertKefraPortalFrames(t *testing.T, p *Player, want ...bool) {
	t.Helper()
	var got []bool
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("session emitted an invalid encrypted packet")
		}
		header := wire.ParseHeader(packet)
		if header.Type != wire.OpKefraPortalState {
			continue
		}
		if len(packet) != 16 || header.Size != 16 || header.ID != wire.SceneField ||
			binary.LittleEndian.Uint16(packet[12:14]) != 1 || binary.LittleEndian.Uint16(packet[14:16]) > 1 {
			t.Fatalf("invalid portal snapshot: %x", packet)
		}
		got = append(got, binary.LittleEndian.Uint16(packet[14:16]) == 1)
	}
	if len(got) != len(want) {
		t.Fatalf("portal snapshots=%v, want %v", got, want)
	}
	for i := range got {
		if got[i] != want[i] {
			t.Fatalf("portal snapshots=%v, want %v", got, want)
		}
	}
}

func TestKefraPortalEntryUsesRestoredWorldState(t *testing.T) {
	for _, defeated := range []bool{false, true} {
		t.Run(map[bool]string{false: "alive", true: "defeated"}[defeated], func(t *testing.T) {
			w, p, session := newEnterWorldPlayer(t, 742, 1000)
			p.Account.Name, p.Account.Chars[0].Name = "PortalAccount", "PortalCharacter"
			durable := &model.GuildRegistry{Version: model.GuildRegistryVersion}
			durable.Wars.Kefra = model.KefraState{Defeated: defeated}
			st := &guildFlowStore{durable: durable}
			restartGuilds(t, w, st)
			w.onEnterWorld(session, enterWorldPacket(0))
			if !p.InWorld {
				t.Fatal("character entry failed")
			}
			assertKefraPortalFrames(t, p, defeated)
		})
	}
}

func TestKefraPortalDeathPublishesOnlyDurableStateToEveryWorldPlayer(t *testing.T) {
	w, st, alpha, beta, _, kefra := kefraTestWorld(t)
	beta.X, beta.Y = 100, 100 // Outside the boss and portal visibility regions.
	pending := &Player{Session: gameNet.NewTestSession(99, 16)}
	w.players[pending.Session] = pending
	st.durable = cloneGuildRegistry(w.guilds)
	drainPlayerPackets(alpha)
	drainPlayerPackets(beta)
	st.err = errors.New("injected storage failure")
	w.onKefraKilled(alpha, kefra)
	assertKefraPortalFrames(t, alpha)
	assertKefraPortalFrames(t, beta)
	if w.kefraDefeated() {
		t.Fatal("failed save changed the live portal state")
	}
	restartGuilds(t, w, st)
	st.err = nil
	w.onKefraKilled(alpha, kefra)
	if !st.durable.Wars.Kefra.Defeated {
		t.Fatal("opening preceded a durable defeat")
	}
	assertKefraPortalFrames(t, alpha, true)
	assertKefraPortalFrames(t, beta, true)
	assertKefraPortalFrames(t, pending)
	w.onKefraKilled(alpha, kefra)
	assertKefraPortalFrames(t, alpha)
	assertKefraPortalFrames(t, beta)
	restartGuilds(t, w, st)
	w.sendKefraPortalState(beta)
	assertKefraPortalFrames(t, beta, true)
}

func TestKefraPortalWeeklyResetIsDurableAndClosesAllViews(t *testing.T) {
	w, st, alpha, beta, clock, kefra := kefraTestWorld(t)
	w.onKefraKilled(alpha, kefra)
	assertKefraPortalFrames(t, alpha, true)
	assertKefraPortalFrames(t, beta, true)
	clock.Advance(15 * time.Hour)
	st.err = errors.New("injected reset storage failure")
	w.tickKefra(clock.Now())
	if !w.kefraDefeated() || !st.durable.Wars.Kefra.Defeated {
		t.Fatal("failed reset closed the durable passage")
	}
	assertKefraPortalFrames(t, alpha)
	assertKefraPortalFrames(t, beta)
	st.err = nil
	w.tickKefra(clock.Now())
	if w.kefraDefeated() || st.durable.Wars.Kefra.Defeated {
		t.Fatal("successful reset did not persist the closed passage")
	}
	assertKefraPortalFrames(t, alpha, false)
	assertKefraPortalFrames(t, beta, false)
	w.tickKefra(clock.Now())
	assertKefraPortalFrames(t, beta)
	restartGuilds(t, w, st)
	w.sendKefraPortalState(beta)
	assertKefraPortalFrames(t, beta, false)
}

func TestKefraPortalCriticalTransitionsWaitForOwnerAcknowledgement(t *testing.T) {
	for _, reset := range []bool{false, true} {
		for _, outcome := range []string{"commit", "rejected", "unknown"} {
			t.Run(map[bool]string{false: "death/", true: "reset/"}[reset]+outcome, func(t *testing.T) {
				w, _, alpha, beta, clock, kefra := kefraTestWorld(t)
				if reset {
					w.guilds.Wars.Kefra = model.KefraState{Defeated: true, DefeatedAt: clock.Now()}
					clock.Advance(15 * time.Hour)
				}
				gate, release := criticalGate(t)
				base := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
				switch outcome {
				case "rejected":
					base.failure = criticalCertifiedRejection(t)
				case "unknown":
					base.failure = errors.New("injected unknown commit outcome")
				}
				queue := newCriticalTestQueue(t, &criticalGameStateTestStore{base}, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				checkExperience := func(defeated bool) {
					t.Helper()
					want := uint32(1000)
					if defeated {
						want = 2000
					}
					if got := w.mobKillExperienceForReceiver(beta.Char, beta.Char, 1000); got != want {
						t.Fatalf("EXP before/after owner acknowledgement=%d, want %d", got, want)
					}
				}
				drainPlayerPackets(alpha)
				drainPlayerPackets(beta)
				if reset {
					w.tickKefra(clock.Now())
				} else if err := w.queueKefraDeathWithoutPlayer(kefra); err != nil {
					t.Fatal(err)
				}
				write := awaitCriticalWrite(t, base.entered)
				if write.guilds == nil || write.guilds.Wars.Kefra.Defeated != !reset || w.kefraDefeated() != reset {
					t.Fatal("pending transition changed the live state or proposed the wrong state")
				}
				checkExperience(reset)
				assertKefraPortalFrames(t, alpha)
				assertKefraPortalFrames(t, beta)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				if w.kefraDefeated() != reset {
					t.Fatal("a worker changed world state before actor acknowledgement")
				}
				checkExperience(reset)
				assertKefraPortalFrames(t, beta)
				w.processCriticalCompletions(1)
				if outcome == "commit" {
					if w.kefraDefeated() != !reset {
						t.Fatal("acknowledged commit did not install the new state")
					}
					checkExperience(!reset)
					assertKefraPortalFrames(t, alpha, !reset)
					assertKefraPortalFrames(t, beta, !reset)
					return
				}
				if w.kefraDefeated() != reset {
					t.Fatal("failed persistence published the proposed state")
				}
				checkExperience(reset)
				assertKefraPortalFrames(t, alpha)
				assertKefraPortalFrames(t, beta)
				if outcome == "unknown" && !w.shuttingDown {
					t.Fatal("unknown durability outcome escaped maintenance containment")
				}
			})
		}
	}
}
