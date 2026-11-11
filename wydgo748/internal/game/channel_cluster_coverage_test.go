package game

import (
	"errors"
	"sync"
	"testing"
	"time"
)

func TestNewChannelClusterRejectsNilStore(t *testing.T) {
	cluster, err := NewChannelCluster(nil)
	if cluster != nil || err == nil {
		t.Fatalf("NewChannelCluster(nil)=(%v, %v), want error", cluster, err)
	}
}

func TestChannelClusterRunStopsBeforeProcessingWhenCanceled(t *testing.T) {
	stop := make(chan struct{})
	close(stop)
	(&ChannelCluster{}).run(nil, stop)
}

func TestChannelClusterRunConsumesWakeAndSelectsDirectMailbox(t *testing.T) {
	world := &World{channel: 1, commands: make(chan command)}
	cluster := &ChannelCluster{worlds: []*World{world}, wake: make(chan struct{}, 1)}
	world.cluster = cluster
	cluster.wake <- struct{}{}
	stop, done := make(chan struct{}), make(chan struct{})
	var stopOnce sync.Once
	stopCluster := func() { stopOnce.Do(func() { close(stop) }) }
	go func() { defer close(done); cluster.run(nil, stop) }()
	t.Cleanup(func() {
		select {
		case <-done:
			return
		default:
			stopCluster()
		}
		select {
		case <-done:
		case <-time.After(time.Second):
			t.Error("cluster actor did not stop")
		}
	})

	deadline := time.Now().Add(time.Second)
	for len(cluster.wake) != 0 {
		if time.Now().After(deadline) {
			t.Fatal("cluster did not consume its wake signal")
		}
		time.Sleep(time.Millisecond)
	}
	processed := make(chan struct{})
	select {
	case world.commands <- command{apply: func() { close(processed); stopCluster() }}:
	case <-time.After(time.Second):
		t.Fatal("cluster did not select the direct command mailbox")
	}
	select {
	case <-processed:
	case <-time.After(time.Second):
		t.Fatal("cluster did not process the selected mailbox command")
	}
	select {
	case <-done:
	case <-time.After(time.Second):
		t.Fatal("cluster did not stop after the command")
	}
}

func TestChannelClusterRunTicksEveryWorld(t *testing.T) {
	world := newZoneTestWorld()
	world.channel = 1
	clock := &channelClusterSignalClock{called: make(chan struct{})}
	world.clock = clock
	cluster := &ChannelCluster{worlds: []*World{world}, wake: make(chan struct{}, 1)}
	world.cluster = cluster
	ticks := make(chan time.Time, 1)
	ticks <- time.Now()
	stop, done := make(chan struct{}), make(chan struct{})
	go func() { defer close(done); cluster.run(ticks, stop) }()
	select {
	case <-clock.called:
		close(stop)
	case <-time.After(time.Second):
		close(stop)
		t.Fatal("cluster did not tick its world")
	}
	select {
	case <-done:
	case <-time.After(time.Second):
		t.Fatal("cluster did not stop after the tick")
	}
	if world.mobTickCounter != 1 {
		t.Fatalf("world tick counter=%d, want 1", world.mobTickCounter)
	}
}

type channelClusterSignalClock struct {
	once   sync.Once
	called chan struct{}
}

func (c *channelClusterSignalClock) Now() time.Time {
	c.once.Do(func() { close(c.called) })
	return time.Now()
}

func TestChannelClusterShutdownReportsCompletionAndFailure(t *testing.T) {
	for _, failure := range []bool{false, true} {
		name := "complete"
		if failure {
			name = "persistence failure"
		}
		t.Run(name, func(t *testing.T) {
			world := newZoneTestWorld()
			world.channel = 1
			world.commands = make(chan command, 1)
			if failure {
				world.shutdownErr = errors.New("injected final persistence failure")
			}
			cluster := &ChannelCluster{worlds: []*World{world}, wake: make(chan struct{}, 1)}
			world.cluster = cluster
			stop, done := make(chan struct{}), make(chan struct{})
			go func() { defer close(done); cluster.run(nil, stop) }()
			got := cluster.Shutdown(time.Second)
			if got == failure {
				close(stop)
				<-done
				t.Fatalf("Shutdown()=%t, failure=%t", got, failure)
			}
			close(stop)
			select {
			case <-done:
			case <-time.After(time.Second):
				t.Fatal("cluster did not stop after shutdown")
			}
		})
	}
}

func TestChannelClusterShutdownTimesOutAfterCommandAdmission(t *testing.T) {
	cluster := &ChannelCluster{worlds: []*World{{commands: make(chan command, 1)}}}
	if cluster.Shutdown(5 * time.Millisecond) {
		t.Fatal("shutdown succeeded without a world-loop reply")
	}
}
