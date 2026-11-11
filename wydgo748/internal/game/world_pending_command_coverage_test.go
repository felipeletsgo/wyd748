package game

import "testing"

func TestPopPendingCommandClearsExhaustedQueueWithStaleHead(t *testing.T) {
	queued := []command{{apply: func() {}}}
	w := &World{pendingCommands: queued, pendingCommandHead: 1}

	if cmd, ok := w.popPendingCommand(); ok || cmd.apply != nil {
		t.Fatal("exhausted pending queue returned a command")
	}
	if len(w.pendingCommands) != 0 || w.pendingCommandHead != 0 || queued[0].apply != nil {
		t.Fatal("exhausted pending queue retained stale command state")
	}
}
