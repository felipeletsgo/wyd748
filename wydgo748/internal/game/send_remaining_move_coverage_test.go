package game

import "testing"

func TestSendRemainingPlayerMoveRejectsInvalidObserversAndRoutes(t *testing.T) {
	observer, _ := networkedTestPlayer(1, "Observer", 2100, 2100)
	subject, _ := networkedTestPlayer(2, "Subject", 2101, 2100)
	w := &World{}

	tests := []struct {
		name string
		call func()
	}{
		{name: "missing observer", call: func() { w.sendRemainingPlayerMove(nil, subject) }},
		{name: "missing subject", call: func() { w.sendRemainingPlayerMove(observer, nil) }},
		{name: "self observer", call: func() { w.sendRemainingPlayerMove(subject, subject) }},
		{name: "route not published", call: func() { w.sendRemainingPlayerMove(observer, subject) }},
		{
			name: "route already consumed",
			call: func() {
				subject.MovePublished = true
				subject.MoveAuthorityStep = len(subject.MoveAuthorityRoute)
				w.sendRemainingPlayerMove(observer, subject)
			},
		},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			subject.MovePublished = false
			subject.MoveAuthorityStep = 0
			before := observer.Session.QueuedPacketsForTest()
			test.call()
			if got := observer.Session.QueuedPacketsForTest(); got != before {
				t.Fatalf("invalid route queued %d packets", got-before)
			}
		})
	}

	subject.MovePublished = true
	subject.MoveAuthorityStep = 1
	subject.MoveAuthorityRoute = []byte{1, 2}
	subject.MovePublishedTargetX, subject.MovePublishedTargetY = 2104, 2102
	before := observer.Session.QueuedPacketsForTest()
	w.sendRemainingPlayerMove(observer, subject)
	if got := observer.Session.QueuedPacketsForTest(); got != before+1 {
		t.Fatalf("valid remainder queued %d packets, want 1", got-before)
	}
}
