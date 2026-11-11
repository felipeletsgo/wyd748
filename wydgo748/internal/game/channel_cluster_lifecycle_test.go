package game

import (
	"sync"
	"testing"
	"time"
)

func TestChannelClusterRunProcessesCommandAndStops(t *testing.T) {
	world := &World{channel: 1, commands: make(chan command, 1)}
	cluster := &ChannelCluster{worlds: []*World{world}, wake: make(chan struct{}, 1)}
	world.cluster = cluster
	stop := make(chan struct{})
	processed := make(chan struct{})
	world.commands <- command{apply: func() {
		close(processed)
		close(stop)
	}}
	cluster.run(nil, stop)
	select {
	case <-processed:
	default:
		t.Fatal("cluster did not process its world's command")
	}
	if cluster.active != world {
		t.Fatal("cluster did not establish the command's owning world")
	}
}

func TestWorldRunRejectsWorldOwnedByChannelCluster(t *testing.T) {
	world := &World{cluster: &ChannelCluster{}}
	defer func() {
		if got := recover(); got != "cluster worlds must run through ChannelCluster.Run" {
			t.Fatalf("World.Run panic = %v", got)
		}
	}()
	world.Run()
}

func TestChannelClusterNotifyShutdownAndWorldLookup(t *testing.T) {
	world := &World{channel: 2}
	cluster := &ChannelCluster{worlds: []*World{world}, wake: make(chan struct{}, 1)}
	cluster.notify()
	cluster.notify()
	if len(cluster.wake) != 1 {
		t.Fatal("cluster wake queue was not bounded")
	}
	if cluster.world(2) != world || cluster.world(1) != nil {
		t.Fatal("cluster channel lookup returned the wrong world")
	}
	if !(&ChannelCluster{}).Shutdown(time.Second) {
		t.Fatal("empty cluster failed shutdown")
	}

	blocked := &ChannelCluster{worlds: []*World{{commands: make(chan command)}}}
	if blocked.Shutdown(10 * time.Millisecond) {
		t.Fatal("shutdown reported success without a world-loop acknowledgement")
	}
}

func TestChannelClusterRunProcessesCommandSentAfterLoopStarts(t *testing.T) {
	world := &World{channel: 1, commands: make(chan command)}
	cluster := &ChannelCluster{worlds: []*World{world}, wake: make(chan struct{}, 1)}
	world.cluster = cluster
	stop := make(chan struct{})
	var stopOnce sync.Once
	stopCluster := func() { stopOnce.Do(func() { close(stop) }) }
	processed := make(chan struct{})
	done := make(chan struct{})
	t.Cleanup(stopCluster)
	go func() {
		defer close(done)
		cluster.run(nil, stop)
	}()

	world.commands <- command{apply: func() {
		close(processed)
		stopCluster()
	}}
	select {
	case <-processed:
	case <-time.After(time.Second):
		stopCluster()
		t.Fatal("cluster did not process the command sent after startup")
	}
	select {
	case <-done:
	case <-time.After(time.Second):
		t.Fatal("cluster did not stop after the selected command")
	}
	if cluster.active != world {
		t.Fatal("cluster did not enter the selected command's world")
	}
}
