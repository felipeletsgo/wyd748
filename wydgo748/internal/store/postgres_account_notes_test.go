package store

import (
	"context"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

func TestPostgresAccountNotes(t *testing.T) {
	st, _ := criticalAccountsPostgresFixture(t)
	ctx := context.Background()
	name := fmt.Sprintf("nt%010d", time.Now().UnixNano()%10_000_000_000)
	if err := st.CreateAccount(postgresTestAccount(name, "Noted", model.Item{Index: 4011})); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, name) })
	first, err := st.AddAccountNote(ctx, name, "operator", control.AccountNoteCommand{Version: 1, Body: "  Reported lost item\nchecked logs  ", BugRefs: []string{"BUG-004"}})
	if err != nil || first.ID == 0 || first.Body != "Reported lost item\nchecked logs" {
		t.Fatal(first, err)
	}
	if _, err := st.AddAccountNote(ctx, name, "operator", control.AccountNoteCommand{Version: 1, Body: "Second note"}); err != nil {
		t.Fatal(err)
	}
	if _, err := st.AddAccountNote(ctx, "nosuchacct1", "operator", control.AccountNoteCommand{Version: 1, Body: "Nobody"}); !IsAccountMissing(err) {
		t.Fatal(err)
	}
	if _, err := st.AddAccountNote(ctx, name, "operator", control.AccountNoteCommand{Version: 1, Body: "x"}); err == nil {
		t.Fatal("short note accepted")
	}
	notes, _, err := st.ListAccountNotes(ctx, name, 10)
	if err != nil || len(notes) != 2 || notes[0].Body != "Second note" || len(notes[0].BugRefs) != 0 || notes[1].BugRefs[0] != "BUG-004" {
		t.Fatal(notes, err)
	}
}
