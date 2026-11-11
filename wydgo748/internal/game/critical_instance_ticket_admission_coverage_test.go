package game

import "testing"

func TestAdmitCriticalInstanceTicketResyncsRejectedItem(t *testing.T) {
	w, leader, _, _, _, _, queue, rule, _ := criticalInstanceEntryFixture(t, "water", "commit")
	leader.Char.Inv[0].Index = 65535
	before := leader.Session.QueuedPacketsForTest()

	if w.admitCriticalInstanceTicket(leader, 0, rule, "entry-ticket") {
		t.Fatal("ticket missing from the catalog was admitted")
	}
	if got := leader.Session.QueuedPacketsForTest(); got != before+2 {
		t.Fatalf("rejected ticket queued %d packets, want one inventory resync and one explanation", got-before)
	}
	if len(queue.pending) != 0 {
		t.Fatal("rejected ticket submitted a persistence write")
	}
}
