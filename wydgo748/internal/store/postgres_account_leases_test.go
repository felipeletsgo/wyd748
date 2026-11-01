package store

import (
	"context"
	"errors"
	"sync"
	"testing"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

// W9 phase 1 account leases. Each test uses its own process names and
// account keys and removes them at the end.

func accountLeasesStore(t *testing.T, accounts ...string) (*PostgresStore, *pgx.Conn) {
	t.Helper()
	st, conn := instanceRowsStore(t) // applies migrations and skips without a database
	ctx := context.Background()
	cleanup := func() {
		_, _ = conn.Exec(ctx, `DELETE FROM account_leases WHERE account_key = ANY($1::text[])`, accounts)
		_, _ = conn.Exec(ctx, `DELETE FROM game_processes WHERE process LIKE 'w9-test/%'`)
	}
	cleanup()
	t.Cleanup(cleanup)
	return st, conn
}

func leaseOwner(t *testing.T, conn *pgx.Conn, account string) (string, int64) {
	t.Helper()
	var process string
	var session int64
	err := conn.QueryRow(context.Background(), `SELECT process, session FROM account_leases WHERE account_key=$1`,
		account).Scan(&process, &session)
	if err == pgx.ErrNoRows {
		return "", 0
	}
	if err != nil {
		t.Fatal(err)
	}
	return process, session
}

func TestAccountLeaseIsSingleAcrossProcesses(t *testing.T) {
	const account = "w9lease1"
	st, conn := accountLeasesStore(t, account)
	for _, process := range []string{"w9-test/a", "w9-test/b"} {
		if err := st.HeartbeatGameProcess(process); err != nil {
			t.Fatal(err)
		}
	}
	if owner, err := st.ClaimAccountLease("W9Lease1", "w9-test/a", 1, 10); err != nil || owner != "" {
		t.Fatalf("first claim: owner=%q err=%v", owner, err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/b", 2, 20); err != nil || owner != "w9-test/a" {
		t.Fatalf("claim of a live process's account: owner=%q err=%v", owner, err)
	}
	// The same process moves the account to a newer session.
	if owner, err := st.ClaimAccountLease(account, "w9-test/a", 2, 11); err != nil || owner != "" {
		t.Fatalf("same-process claim: owner=%q err=%v", owner, err)
	}
	// A late release of the older session keeps the newer lease.
	if err := st.ReleaseAccountLease(account, "w9-test/a", 10); err != nil {
		t.Fatal(err)
	}
	if process, session := leaseOwner(t, conn, account); process != "w9-test/a" || session != 11 {
		t.Fatalf("stale release changed the lease: %q/%d", process, session)
	}
	if err := st.ReleaseAccountLease(account, "w9-test/a", 11); err != nil {
		t.Fatal(err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/b", 2, 20); err != nil || owner != "" {
		t.Fatalf("claim after release: owner=%q err=%v", owner, err)
	}
}

func TestAccountLeaseOfStoppedProcessIsTakenOver(t *testing.T) {
	const account = "w9lease2"
	st, conn := accountLeasesStore(t, account)
	for _, process := range []string{"w9-test/stopped", "w9-test/live"} {
		if err := st.HeartbeatGameProcess(process); err != nil {
			t.Fatal(err)
		}
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/stopped", 1, 1); err != nil || owner != "" {
		t.Fatalf("first claim: owner=%q err=%v", owner, err)
	}
	if _, err := conn.Exec(context.Background(), `UPDATE game_processes SET heartbeat_at = now() - interval '1 minute'
		WHERE process='w9-test/stopped'`); err != nil {
		t.Fatal(err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/live", 2, 2); err != nil || owner != "" {
		t.Fatalf("takeover: owner=%q err=%v", owner, err)
	}
	if process, _ := leaseOwner(t, conn, account); process != "w9-test/live" {
		t.Fatalf("lease owner %q after takeover", process)
	}
}

func TestAccountLeaseConcurrentClaimsHaveOneWinner(t *testing.T) {
	const account = "w9lease3"
	st, _ := accountLeasesStore(t, account)
	processes := []string{"w9-test/c1", "w9-test/c2", "w9-test/c3", "w9-test/c4"}
	for _, process := range processes {
		if err := st.HeartbeatGameProcess(process); err != nil {
			t.Fatal(err)
		}
	}
	var wg sync.WaitGroup
	results := make([]string, len(processes))
	errs := make([]error, len(processes))
	for i, process := range processes {
		wg.Add(1)
		go func(i int, process string) {
			defer wg.Done()
			results[i], errs[i] = st.ClaimAccountLease(account, process, 1, int64(i+1))
		}(i, process)
	}
	wg.Wait()
	winners := 0
	for i := range processes {
		if errs[i] != nil {
			t.Fatalf("claim %d: %v", i, errs[i])
		}
		if results[i] == "" {
			winners++
		}
	}
	if winners != 1 {
		t.Fatalf("%d processes own the account: %v", winners, results)
	}
}

func TestAccountLeaseReconcileKeepsHeldAndRecentLeases(t *testing.T) {
	held, recent, lost := "w9lease4", "w9lease5", "w9lease6"
	st, conn := accountLeasesStore(t, held, recent, lost)
	if err := st.HeartbeatGameProcess("w9-test/r"); err != nil {
		t.Fatal(err)
	}
	for i, account := range []string{held, recent, lost} {
		if owner, err := st.ClaimAccountLease(account, "w9-test/r", 1, int64(i+1)); err != nil || owner != "" {
			t.Fatalf("claim %s: owner=%q err=%v", account, owner, err)
		}
	}
	if _, err := conn.Exec(context.Background(), `UPDATE account_leases SET claimed_at = now() - interval '1 minute'
		WHERE account_key = ANY($1::text[])`, []string{held, lost}); err != nil {
		t.Fatal(err)
	}
	if err := st.ReconcileAccountLeases("w9-test/r", []string{held}); err != nil {
		t.Fatal(err)
	}
	for account, want := range map[string]bool{held: true, recent: true, lost: false} {
		if process, _ := leaseOwner(t, conn, account); (process != "") != want {
			t.Fatalf("lease %s present=%t, want %t", account, process != "", want)
		}
	}
}

func TestExclusiveProcessRegistrationEndsOtherLeases(t *testing.T) {
	const account = "w9lease7"
	st, conn := accountLeasesStore(t, account)
	if err := st.HeartbeatGameProcess("w9-test/previous"); err != nil {
		t.Fatal(err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/previous", 1, 1); err != nil || owner != "" {
		t.Fatalf("claim: owner=%q err=%v", owner, err)
	}
	// A restarted process that fenced the previous instance owns the database.
	st.gameInstance = "w9-test/restarted"
	name, err := st.RegisterGameProcess()
	if err != nil || name != "w9-test/restarted" {
		t.Fatalf("register: %q %v", name, err)
	}
	if process, _ := leaseOwner(t, conn, account); process != "" {
		t.Fatalf("lease of the previous instance survived the restart: %q", process)
	}
	if owner, err := st.ClaimAccountLease(account, name, 1, 1); err != nil || owner != "" {
		t.Fatalf("claim after restart: owner=%q err=%v", owner, err)
	}
	if err := st.ReleaseProcessLeases(name); err != nil {
		t.Fatal(err)
	}
	if process, _ := leaseOwner(t, conn, account); process != "" {
		t.Fatalf("shutdown release kept %q", process)
	}
}

// A registered process cannot write an account that another live process
// owns; the rejection is certified. Its own lease, or the lease of a stopped
// process, does not block the write.
func TestAccountWriteIsFencedByTheLeaseOwner(t *testing.T) {
	const account = "w9lease9"
	st, conn := accountLeasesStore(t, account)
	ctx := context.Background()
	t.Cleanup(func() { _, _ = conn.Exec(ctx, `DELETE FROM accounts WHERE name_key=$1`, account) })
	_, _ = conn.Exec(ctx, `DELETE FROM accounts WHERE name_key=$1`, account)
	acc := &model.Account{Name: "W9Lease9", PasswordHash: "hash"}
	if err := st.CreateAccount(acc); err != nil {
		t.Fatal(err)
	}
	st.gameInstance = "w9-test/writer"
	if _, err := st.RegisterGameProcess(); err != nil {
		t.Fatal(err)
	}
	if err := st.HeartbeatGameProcess("w9-test/other"); err != nil {
		t.Fatal(err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/other", 2, 1); err != nil || owner != "" {
		t.Fatalf("claim: owner=%q err=%v", owner, err)
	}
	err := st.SaveCriticalAccounts(acc)
	if !errors.Is(err, ErrAccountLeaseFenced) || !IsWriteRejected(err) {
		t.Fatalf("write of another process's account: %v", err)
	}
	if _, err := conn.Exec(ctx, `UPDATE game_processes SET heartbeat_at = now() - interval '1 minute'
		WHERE process='w9-test/other'`); err != nil {
		t.Fatal(err)
	}
	if err := st.SaveCriticalAccounts(acc); err != nil {
		t.Fatalf("write after the owner stopped: %v", err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/writer", 1, 2); err != nil || owner != "" {
		t.Fatalf("takeover: owner=%q err=%v", owner, err)
	}
	if err := st.SaveCriticalAccounts(acc); err != nil {
		t.Fatalf("write of an owned account: %v", err)
	}
}
