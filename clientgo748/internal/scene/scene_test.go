package scene

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydclient748/internal/input"
)

type testScene struct {
	id       ID
	events   *[]string
	enterErr error
}

func (s *testScene) ID() ID { return s.id }
func (s *testScene) Enter() error {
	*s.events = append(*s.events, string(s.id)+".enter")
	return s.enterErr
}
func (s *testScene) HandleEvent(input.Event) error {
	*s.events = append(*s.events, string(s.id)+".event")
	return nil
}
func (s *testScene) Update(time.Duration) error {
	*s.events = append(*s.events, string(s.id)+".update")
	return nil
}
func (s *testScene) Render() error {
	*s.events = append(*s.events, string(s.id)+".render")
	return nil
}
func (s *testScene) Exit() error {
	*s.events = append(*s.events, string(s.id)+".exit")
	return nil
}
func (s *testScene) Close() error {
	*s.events = append(*s.events, string(s.id)+".close")
	return nil
}

func TestManagerTransitionsAtUpdateBoundaryAndClosesInOrder(t *testing.T) {
	var events []string
	manager := New(map[ID]Factory{
		"boot":  func() (Scene, error) { return &testScene{id: "boot", events: &events}, nil },
		"login": func() (Scene, error) { return &testScene{id: "login", events: &events}, nil },
	})
	if err := manager.Start("boot"); err != nil {
		t.Fatal(err)
	}
	if err := manager.Dispatch([]input.Event{{Kind: input.KindKeyDown}}); err != nil {
		t.Fatal(err)
	}
	if err := manager.Request("login"); err != nil {
		t.Fatal(err)
	}
	if got := reflect.DeepEqual(events, []string{"boot.enter", "boot.event"}); !got {
		t.Fatalf("events before boundary = %v", events)
	}
	if err := manager.Update(time.Millisecond); err != nil {
		t.Fatal(err)
	}
	if err := manager.Render(); err != nil {
		t.Fatal(err)
	}
	if err := manager.Close(); err != nil {
		t.Fatal(err)
	}
	want := []string{"boot.enter", "boot.event", "login.enter", "boot.exit", "boot.close", "login.update", "login.render", "login.exit", "login.close"}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events = %v, want %v", events, want)
	}
	if err := manager.Close(); err != nil {
		t.Fatal(err)
	}
}

func TestManagerFailedEnterKeepsPreviousScene(t *testing.T) {
	var events []string
	manager := New(map[ID]Factory{
		"boot": func() (Scene, error) { return &testScene{id: "boot", events: &events}, nil },
		"broken": func() (Scene, error) {
			return &testScene{id: "broken", events: &events, enterErr: errors.New("missing resource")}, nil
		},
	})
	if err := manager.Start("boot"); err != nil {
		t.Fatal(err)
	}
	if err := manager.Request("broken"); err != nil {
		t.Fatal(err)
	}
	if err := manager.Update(0); err == nil {
		t.Fatal("expected failed scene entry")
	}
	if id, ok := manager.Current(); !ok || id != "boot" {
		t.Fatalf("current scene = %q, %v; want boot", id, ok)
	}
	want := []string{"boot.enter", "broken.enter", "broken.close"}
	if !reflect.DeepEqual(events, want) {
		t.Fatalf("events = %v, want %v", events, want)
	}
}

func TestManagerRejectsInvalidLifecycleOperations(t *testing.T) {
	manager := New(nil)
	if err := manager.Start("missing"); err == nil {
		t.Fatal("expected missing scene error")
	}
	if err := manager.Update(0); err == nil {
		t.Fatal("expected no active scene error")
	}
	if err := manager.Close(); err != nil {
		t.Fatal(err)
	}
	if err := manager.Request("late"); err == nil {
		t.Fatal("expected request after close error")
	}
}
