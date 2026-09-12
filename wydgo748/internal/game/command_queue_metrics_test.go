package game

import "testing"

func TestCommandQueueDepthIncludesDeferredBacklog(t *testing.T) {
	w := &World{
		commands:           make(chan command, 8),
		pendingCommands:    make([]command, 4),
		pendingCommandHead: 1,
	}
	w.commands <- command{}
	w.commands <- command{}

	if got := w.commandQueueDepth(); got != 5 {
		t.Fatalf("queue depth=%d; esperado=5 (2 no canal + 3 pendentes)", got)
	}
}

func TestCommandQueueDepthIgnoresConsumedPendingPrefix(t *testing.T) {
	w := &World{
		commands:           make(chan command, 4),
		pendingCommands:    make([]command, 3),
		pendingCommandHead: 3,
	}
	w.commands <- command{}

	if got := w.commandQueueDepth(); got != 1 {
		t.Fatalf("queue depth=%d; esperado=1", got)
	}
}

func TestCommandQueueDepthDoesNotAllocate(t *testing.T) {
	w := &World{
		commands:           make(chan command, 4),
		pendingCommands:    make([]command, 3),
		pendingCommandHead: 1,
	}
	w.commands <- command{}

	allocs := testing.AllocsPerRun(1000, func() {
		if w.commandQueueDepth() != 3 {
			panic("queue depth mudou")
		}
	})
	if allocs != 0 {
		t.Fatalf("commandQueueDepth alocou %.2f objeto(s) por leitura", allocs)
	}
}
