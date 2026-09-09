package lifecycle

import "testing"

func TestNormalSessionLifecycle(t *testing.T) {
	machine := Machine{}
	for _, event := range []Event{
		EventConfigure,
		EventConnect,
		EventAuthenticate,
		EventEnterWorld,
		EventLogout,
	} {
		if err := machine.Apply(event); err != nil {
			t.Fatalf("event %d failed: %v", event, err)
		}
	}
	if got := machine.State(); got != StateConfigured {
		t.Fatalf("state after logout = %d, want %d", got, StateConfigured)
	}
}

func TestInvalidTransitionDoesNotMutate(t *testing.T) {
	machine := Machine{}
	if err := machine.Apply(EventEnterWorld); err == nil {
		t.Fatal("expected invalid transition")
	}
	if got := machine.State(); got != StateCreated {
		t.Fatalf("state after invalid transition = %d, want %d", got, StateCreated)
	}
}

func TestCloseIsIdempotent(t *testing.T) {
	machine := Machine{}
	if err := machine.Apply(EventClose); err != nil {
		t.Fatal(err)
	}
	if err := machine.Apply(EventClose); err != nil {
		t.Fatal(err)
	}
	if err := machine.Apply(EventConnect); err == nil {
		t.Fatal("expected events after close to fail")
	}
}
