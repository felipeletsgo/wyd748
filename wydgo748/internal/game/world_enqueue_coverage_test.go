package game

import (
	"testing"

	"wydgo/internal/net"
)

func TestEnqueueNotifiesClusterAfterQueuingPacket(t *testing.T) {
	session := net.NewTestSession(1, 8)
	cluster := &ChannelCluster{wake: make(chan struct{}, 1)}
	w := &World{commands: make(chan command, 1), cluster: cluster}
	packet := []byte{1, 2, 3}

	w.Enqueue(session, packet)

	queued := <-w.commands
	if queued.s != session || len(queued.pkt) != len(packet) || queued.queuedAt.IsZero() {
		t.Fatalf("queued command=%+v", queued)
	}
	for i := range packet {
		if queued.pkt[i] != packet[i] {
			t.Fatalf("packet changed at byte %d: got %d want %d", i, queued.pkt[i], packet[i])
		}
	}
	select {
	case <-cluster.wake:
	default:
		t.Fatal("cluster actor was not notified")
	}
}
