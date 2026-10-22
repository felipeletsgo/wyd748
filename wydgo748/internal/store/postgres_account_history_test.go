package store

import (
	"context"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

func TestPostgresAccountHistoryAndStorage(t *testing.T) {
	st, _ := criticalAccountsPostgresFixture(t)
	ctx := context.Background()
	name := fmt.Sprintf("ah%010d", time.Now().UnixNano()%10_000_000_000)
	account := postgresTestAccount(name, "Historian", model.Item{Index: 4011})
	account.Chars[0].Equip[6] = model.Item{Index: 801, Eff: [6]byte{43, 5}}
	account.Chars[0].Gold = 123
	account.Cargo[2] = model.Item{Index: 412}
	account.CargoGold = 9
	if err := st.CreateAccount(account); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM staff_audit WHERE details->>'target_account'=$1`, name)
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, name)
	})
	for i := range 3 {
		if err := st.RecordAccountLogin(ctx, name, fmt.Sprint("198.51.100.", i), 1); err != nil {
			t.Fatal(err)
		}
	}
	if err := st.RecordAccountLogin(ctx, "nosuchacct1", "1.2.3.4", 1); err != nil {
		t.Fatal("login of a deleted account must be ignored, not fail", err)
	}
	logins, _, err := st.ListAccountLogins(ctx, name, 2)
	if err != nil || len(logins) != 2 || logins[0].ClientIP != "198.51.100.2" || logins[0].Channel != 1 {
		t.Fatal(logins, err)
	}
	if _, _, err := st.ListAccountLogins(ctx, "nosuchacct1", 5); !IsAccountMissing(err) {
		t.Fatal(err)
	}
	if _, err := st.pool.Exec(ctx, `UPDATE account_logins SET created_at=now()-interval '100 days'
		WHERE id=(SELECT min(id) FROM account_logins WHERE account_key=$1)`, name); err != nil {
		t.Fatal(err)
	}
	if n, err := st.PruneAccountLogins(ctx, time.Now().Add(-control.LoginHistoryRetention), 500); err != nil || n != 1 {
		t.Fatal(n, err)
	}
	for _, details := range []map[string]any{{"target_account": name, "kind": "mute"}, {"account": name}, {"target_account": "someone"}} {
		if err := st.AppendStaffAudit(ctx, control.AuditEntry{Actor: "operator", Action: "account.sanction", Result: "ok", Details: details}); err != nil {
			t.Fatal(err)
		}
	}
	actions, _, err := st.ListStaffAuditForAccount(ctx, name, 10)
	if err != nil || len(actions) != 2 {
		t.Fatal(actions, err)
	}
	_, _ = st.pool.Exec(ctx, `DELETE FROM staff_audit WHERE details->>'account'=$1 OR details->>'target_account'='someone'`, name)

	storage, err := st.ReadCharacterStorage(ctx, name, account.Chars[0].UID)
	if err != nil || storage.Name != "Historian" || storage.Gold != 123 || storage.CargoGold != 9 {
		t.Fatal(storage, err)
	}
	if len(storage.Equipment) != 1 || storage.Equipment[0].Slot != 6 || storage.Equipment[0].ItemID != 801 || storage.Equipment[0].Effects[0] != 43 ||
		len(storage.Inventory) != 1 || storage.Inventory[0].ItemID != 4011 || len(storage.Cargo) != 1 || storage.Cargo[0].Slot != 2 {
		t.Fatal(storage)
	}
	if _, err := st.ReadCharacterStorage(ctx, name, "00000000000000000000000000000001"); !IsAccountMissing(err) {
		t.Fatal(err)
	}
}
