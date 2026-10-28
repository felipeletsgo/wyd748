package game

import (
	"fmt"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

func TestChannelTransferRejectsUnsafeEntryWithoutSaving(t *testing.T) {
	for _, channel := range []byte{1, 2} {
		for _, name := range []string{
			"same-channel", "missing-channel", "zero-channel", "negative-channel", "overflow-channel", "invalid-channel",
			"nil-player", "nil-session", "nil-account", "nil-character", "outside-world", "closed-session",
			"poisoned-persistence", "dead", "trade", "shop", "instance", "city-war", "tower-war",
			"source-shutdown", "target-shutdown", "source-maintenance", "target-maintenance",
		} {
			t.Run(fmt.Sprintf("channel-%d/%s", channel, name), func(t *testing.T) {
				c, one, two, players := channelIsolationFixture(t)
				w, target, p := one, two, players[0]
				if channel == 2 {
					w, target, p = two, one, players[2]
				}
				base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
				backend := &channelTransferStore{criticalStateTestStore: &criticalStateTestStore{criticalBatchTestStore: &criticalBatchTestStore{base}}}
				one.store, two.store = backend, backend
				c.enter(w)
				session, original := p.Session, p
				argument := fmt.Sprint(target.channel)
				switch name {
				case "same-channel":
					argument = fmt.Sprint(channel)
				case "missing-channel":
					argument = "3"
				case "zero-channel":
					argument = "0"
				case "negative-channel":
					argument = "-1"
				case "overflow-channel":
					argument = "256"
				case "invalid-channel":
					argument = "2 trailing-data"
				case "nil-player":
					p = nil
				case "nil-session":
					p.Session = nil
				case "nil-account":
					p.Account = nil
				case "nil-character":
					p.Char = nil
				case "outside-world":
					p.InWorld = false
				case "closed-session":
					p.Session.Close()
				case "poisoned-persistence":
					p.PersistencePoisoned = true
				case "dead":
					setPlayerCurHP(p.Char, 0)
				case "trade":
					p.Trade = &TradeState{}
				case "shop":
					p.GhostShop = &GhostShop{}
				case "instance":
					w.itemInstances = map[string]*ItemInstance{"private": {RuntimeID: "private", MemberIDs: []uint16{p.ID}}}
					w.setPlayerInstanceIndex(p.ID, "private")
				case "city-war":
					w.cityFighters = map[*Player]cityFighter{p: {}}
				case "tower-war":
					p.X, p.Y = towerWarArena.x1, towerWarArena.y1
				case "source-shutdown":
					w.shuttingDown = true
				case "target-shutdown":
					target.shuttingDown = true
				case "source-maintenance":
					w.staffOps.shutdown = &shutdownPlan{requested: true}
				case "target-maintenance":
					target.staffOps.shutdown = &shutdownPlan{requested: true}
				}
				w.requestChannelTransfer(p, argument)
				if len(base.entered) != 0 || backend.account != nil || backend.state != nil || len(c.tickets) != 0 || len(c.transfers) != 0 {
					t.Fatal("rejected transfer saved state or reserved a ticket")
				}
				if w.players[session] != original || w.playersByID[original.ID] != original {
					t.Fatal("rejected transfer detached the source")
				}
			})
		}
	}
}

func TestChannelTransferRechecksLifecycleAfterCommit(t *testing.T) {
	for _, channel := range []byte{1, 2} {
		for _, phase := range []string{"held", "completed-unacknowledged"} {
			for _, name := range []string{"source-disconnected", "source-shutdown", "target-shutdown", "source-maintenance", "target-maintenance", "source-cutoff", "target-cutoff"} {
				t.Run(fmt.Sprintf("channel-%d/%s/%s", channel, phase, name), func(t *testing.T) {
					w, p, session := newEnterWorldPlayer(t, 742, 1000)
					p.Account.Chars[0].UID = "channel-transfer-lifecycle"
					w.onEnterWorld(session, enterWorldPacket(0))
					if !p.InWorld {
						t.Fatal("source entry failed")
					}
					channelIsolationPackets(t, p)
					gate, release := criticalGate(t)
					base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
					backend := &channelTransferStore{criticalStateTestStore: &criticalStateTestStore{criticalBatchTestStore: &criticalBatchTestStore{base}}}
					w.store, w.channel = backend, channel
					target := newZoneTestWorld()
					target.store, target.channel = backend, 3-channel
					clock := newFakeClock(time.Date(2026, 10, 26, 12, 0, 0, 0, time.UTC))
					w.clock, target.clock = clock, clock
					if name == "source-cutoff" {
						w.staffOps.shutdown = &shutdownPlan{at: clock.Now().Add(control.ShutdownLoginCutoff + time.Second)}
					} else if name == "target-cutoff" {
						target.staffOps.shutdown = &shutdownPlan{at: clock.Now().Add(control.ShutdownLoginCutoff + time.Second)}
					}
					c, err := NewChannelCluster(backend)
					if err != nil {
						t.Fatal(err)
					}
					q := newCriticalTestQueue(t, backend, 1, 4)
					t.Cleanup(release)
					c.owner = newCriticalPersistenceOwner(q)
					w.criticalWrites, target.criticalWrites = c.owner, c.owner
					for _, world := range []*World{w, target} {
						world.accountSessions = make(map[string]*gameNet.Session)
						if err := c.attach(world); err != nil {
							t.Fatal(err)
						}
					}
					c.enter(w)
					if !w.claimAccountSession(session, p.Account.Name) {
						t.Fatal("source account claim failed")
					}
					w.requestChannelTransfer(p, fmt.Sprint(target.channel))
					awaitCriticalWrite(t, base.entered)
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, q, 1)
					}
					switch name {
					case "source-disconnected":
						session.Close()
					case "source-shutdown":
						w.shuttingDown = true
					case "target-shutdown":
						target.shuttingDown = true
					case "source-maintenance":
						w.staffOps.shutdown = &shutdownPlan{requested: true}
					case "target-maintenance":
						target.staffOps.shutdown = &shutdownPlan{requested: true}
					case "source-cutoff", "target-cutoff":
						clock.Advance(time.Second)
					}
					release()
					awaitReadyCriticalResults(t, q, 1)
					c.enter(target)
					target.processCriticalCompletions(1)
					if len(c.tickets) != 0 || len(c.transfers) != 0 || w.players[session] != p {
						t.Fatalf("transfer published after %s: tickets=%d reservations=%d source-retained=%t", name, len(c.tickets), len(c.transfers), w.players[session] == p)
					}
					if w.criticalPlayerReserved(p) {
						t.Fatal("settled transfer leaked the persistence reservation")
					}
					if len(q.pending) != 0 || w.playersByID[p.ID] != p || !p.InWorld || w.accountSessions[accountSessionKey(p.Account.Name)] != session {
						t.Fatal("cancelled transfer lost source ownership or left a pending write")
					}
					for _, packet := range channelIsolationPackets(t, p) {
						if wire.ParseHeader(packet).Type == 0x52A {
							t.Fatal("cancelled transfer sent a migration packet")
						}
					}
					if name == "source-maintenance" || name == "target-maintenance" || name == "source-cutoff" || name == "target-cutoff" {
						if session.IsClosed() {
							t.Fatal("maintenance cancellation closed the source session")
						}
						// Cancelling maintenance must allow a fresh transfer with no stale reservation.
						w.staffOps.shutdown, target.staffOps.shutdown = nil, nil
						c.enter(w)
						w.requestChannelTransfer(p, fmt.Sprint(target.channel))
						awaitCriticalWrite(t, base.entered)
						awaitReadyCriticalResults(t, q, 1)
						c.enter(target)
						target.processCriticalCompletions(1)
						if len(c.tickets) != 1 || len(c.transfers) != 1 || w.players[session] != nil || w.criticalPlayerReserved(p) {
							t.Fatal("fresh transfer failed after maintenance cancellation")
						}
						packets := channelIsolationPackets(t, p)
						if len(packets) != 1 || wire.ParseHeader(packets[0]).Type != 0x52A {
							t.Fatal("fresh transfer did not send its migration packet")
						}
					}
				})
			}
		}
	}
}

func TestChannelTicketStaleExpiryCannotReleaseReplacement(t *testing.T) {
	c, one, two, _ := channelIsolationFixture(t)
	now := time.Date(2026, 10, 26, 12, 0, 0, 0, time.UTC)
	clock := newFakeClock(now)
	one.clock, two.clock = clock, clock
	old, err := c.issueTicket("Shared", 2, 0, now)
	if err != nil {
		t.Fatal(err)
	}
	claimed := gameNet.NewTestSession(10, 16)
	if _, ok := two.consumeTicket(old, "shared", claimed); !ok {
		t.Fatal("initial claim failed")
	}
	clock.Advance(channelTicketTTL)
	fresh, err := c.issueTicket("SHARED", 1, 1, clock.Now())
	if err != nil {
		t.Fatal(err)
	}
	if old == fresh {
		t.Fatal("replacement reused the old secret")
	}
	if _, ok := two.consumeTicket(old, "shared", claimed); ok {
		t.Fatal("old claimed ticket became usable after expiry")
	}
	if one.transferAdmissionAllowed("shared", claimed) || two.transferAdmissionAllowed("shared", claimed) {
		t.Fatal("stale claim bypassed the replacement reservation")
	}
	destination := gameNet.NewTestSession(11, 16)
	ticket, ok := one.consumeTicket(fresh, "shared", destination)
	if !ok || ticket.slot != 1 || !one.transferAdmissionAllowed("shared", destination) {
		t.Fatal("old expiry cleared or damaged the replacement")
	}
}

func TestChannelTicketExpiredLoginCannotClaimAccount(t *testing.T) {
	c, one, two, _ := channelIsolationFixture(t)
	now := time.Date(2026, 10, 26, 12, 0, 0, 0, time.UTC)
	clock := newFakeClock(now)
	one.clock, two.clock = clock, clock
	token, err := c.issueTicket("Expired", 2, 0, now)
	if err != nil {
		t.Fatal(err)
	}
	session := gameNet.NewTestSessionWithRemoteIP(20, 16, "192.0.2.20")
	ticket, ok := two.consumeTicket(token, "expired", session)
	if !ok {
		t.Fatal("ticket claim failed")
	}
	clock.Advance(channelTicketTTL)
	two.authPending = map[*gameNet.Session]bool{session: true}
	two.onLoginResult(session, &loginResult{account: &model.Account{Name: "Expired"}, transfer: ticket})
	if !session.AdmissionRejected() || two.players[session] != nil || one.accountSessions["expired"] != nil {
		t.Fatal("expired asynchronous login claimed account ownership")
	}
	if !one.transferAdmissionAllowed("expired", gameNet.NewTestSession(21, 16)) || len(c.tickets) != 0 || len(c.transfers) != 0 {
		t.Fatal("expired authentication permanently blocked the account")
	}
}
