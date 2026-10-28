package game

import (
	"context"
	"fmt"
	"testing"
	"time"
)

func TestChannelActorBusyWorldCannotStarveAnotherMailbox(t *testing.T) {
	for _, busy := range []byte{1, 2} {
		t.Run(fmt.Sprintf("busy-channel-%d", busy), func(t *testing.T) {
			c, _, _, _ := channelIsolationFixture(t)
			a, b := c.world(busy), c.world(3-busy)
			total := 3 * worldCommandBatchLimit
			a.commands, b.commands = make(chan command, total), make(chan command, 1)
			processed := 0
			wrongWorld := false
			for range total {
				a.commands <- command{staff: &staffRequest{
					ctx: context.Background(), label: "test.channel.busy", release: func() {},
					run: func() {
						processed++
						wrongWorld = wrongWorld || c.active != a
					},
				}}
			}
			type observation struct {
				processed int
				wrong     bool
			}
			observed := make(chan observation, 1)
			b.commands <- command{staff: &staffRequest{
				ctx: context.Background(), label: "test.channel.peer", release: func() {},
				run: func() { observed <- observation{processed, wrongWorld || c.active != b} },
			}}
			// Deliberately use the mailboxes directly, as internal producers do.
			stop, done := make(chan struct{}), make(chan struct{})
			go func() { defer close(done); c.run(nil, stop) }()
			t.Cleanup(func() {
				close(stop)
				select {
				case <-done:
				case <-time.After(2 * time.Second):
					t.Error("channel actor did not stop")
				}
			})
			select {
			case got := <-observed:
				if got.wrong || got.processed >= total {
					t.Fatalf("peer starved or ran in another world: processed=%d total=%d wrong-world=%t", got.processed, total, got.wrong)
				}
			case <-time.After(2 * time.Second):
				t.Fatal("peer mailbox did not wake the actor")
			}
		})
	}
}

func TestChannelActorNestedRoutingRestoresWorldAfterPanic(t *testing.T) {
	for _, active := range []byte{1, 2} {
		t.Run(fmt.Sprintf("active-channel-%d", active), func(t *testing.T) {
			c, _, _, _ := channelIsolationFixture(t)
			a, b := c.world(active), c.world(3-active)
			c.owner = newCriticalPersistenceOwner(nil)
			c.enter(a)
			recovered := false
			func() {
				defer func() { recovered = recover() == "injected routing panic" }()
				c.inWorld(b, func() {
					if c.active != b || c.owner.activeWorld != b {
						t.Fatal("outer routing did not select the target")
					}
					c.inWorld(a, func() { panic("injected routing panic") })
				})
			}()
			if !recovered || c.active != a || c.owner.activeWorld != a {
				t.Fatal("nested routing panic leaked actor or persistence ownership")
			}
		})
	}
}

func TestChannelActorRejectsInvalidWorldWithoutReplacingOwner(t *testing.T) {
	for _, name := range []string{"duplicate-channel", "reserved-zero", "different-store"} {
		t.Run(name, func(t *testing.T) {
			c, one, _, _ := channelIsolationFixture(t)
			candidate := newZoneTestWorld()
			candidate.store, candidate.channel = one.store, 3
			switch name {
			case "duplicate-channel":
				candidate.channel = 1
			case "reserved-zero":
				candidate.channel = 0
			case "different-store":
				candidate.store = &craftStore{}
			}
			if err := c.attach(candidate); err == nil {
				t.Fatal("invalid world attached")
			}
			if len(c.worlds) != 2 || c.active != one || c.world(1) != one || candidate.cluster != nil || candidate.guilds == one.guilds {
				t.Fatal("rejected attachment mutated world ownership")
			}
		})
	}
}
