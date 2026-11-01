package store

import (
	"context"
	"crypto/sha256"
	"errors"
	"sync"
	"testing"
	"time"
)

// W9 phase 2 durable channel tickets.

func ticketHash(token string) []byte {
	sum := sha256.Sum256([]byte(token))
	return sum[:]
}

func channelTicketsStore(t *testing.T, accounts ...string) *PostgresStore {
	t.Helper()
	st, conn := accountLeasesStore(t, accounts...)
	ctx := context.Background()
	cleanup := func() {
		_, _ = conn.Exec(ctx, `DELETE FROM channel_tickets WHERE account_key = ANY($1::text[])`, accounts)
	}
	cleanup()
	t.Cleanup(cleanup)
	for _, process := range []string{"w9-test/source", "w9-test/target", "w9-test/other"} {
		if err := st.HeartbeatGameProcess(process); err != nil {
			t.Fatal(err)
		}
	}
	return st
}

func TestChannelTicketMovesTheAccountLease(t *testing.T) {
	const account = "w9ticket1"
	st := channelTicketsStore(t, account)
	if owner, err := st.ClaimAccountLease(account, "w9-test/source", 1, 5); err != nil || owner != "" {
		t.Fatalf("source claim: owner=%q err=%v", owner, err)
	}
	token := ticketHash("*2.ticket-one")
	if err := st.IssueChannelTicket(token, account, 2, 3, time.Minute); err != nil {
		t.Fatal(err)
	}
	if err := st.IssueChannelTicket(ticketHash("*2.ticket-two"), account, 2, 1, time.Minute); !errors.Is(err, ErrChannelTransferPending) {
		t.Fatalf("second ticket: %v", err)
	}
	if _, err := st.ClaimAccountLease(account, "w9-test/other", 1, 9); !errors.Is(err, ErrChannelTransferPending) {
		t.Fatalf("plain login during a transfer: %v", err)
	}
	if valid, err := st.ChannelTicketValid(token, account, 2); err != nil || !valid {
		t.Fatalf("ticket valid=%t err=%v", valid, err)
	}
	if valid, _ := st.ChannelTicketValid(token, account, 1); valid {
		t.Fatal("ticket is valid on another channel")
	}
	if _, err := st.ClaimAccountLeaseWithTicket("w9ticket2", "w9-test/target", 2, 7, token); !errors.Is(err, ErrChannelTicketInvalid) {
		t.Fatalf("ticket of another account: %v", err)
	}
	// The source process is alive and still holds the lease; the ticket moves it.
	slot, err := st.ClaimAccountLeaseWithTicket(account, "w9-test/target", 2, 7, token)
	if err != nil || slot != 3 {
		t.Fatalf("consume: slot=%d err=%v", slot, err)
	}
	if _, err := st.ClaimAccountLeaseWithTicket(account, "w9-test/target", 2, 8, token); !errors.Is(err, ErrChannelTicketInvalid) {
		t.Fatalf("replayed ticket: %v", err)
	}
	// The source's late release does not remove the target's lease.
	if err := st.ReleaseAccountLease(account, "w9-test/source", 5); err != nil {
		t.Fatal(err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/other", 1, 9); err != nil || owner != "w9-test/target" {
		t.Fatalf("lease after transfer: owner=%q err=%v", owner, err)
	}
}

func TestChannelTicketExpiresAndCancels(t *testing.T) {
	const account = "w9ticket3"
	st := channelTicketsStore(t, account)
	_, conn := instanceRowsStore(t)
	token := ticketHash("*2.expiring")
	if err := st.IssueChannelTicket(token, account, 2, 0, time.Minute); err != nil {
		t.Fatal(err)
	}
	if _, err := conn.Exec(context.Background(), `UPDATE channel_tickets SET expires_at = now() - interval '1 second'
		WHERE token_hash=$1`, token); err != nil {
		t.Fatal(err)
	}
	if _, err := st.ClaimAccountLeaseWithTicket(account, "w9-test/target", 2, 1, token); !errors.Is(err, ErrChannelTicketInvalid) {
		t.Fatalf("expired ticket: %v", err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/source", 1, 2); err != nil || owner != "" {
		t.Fatalf("login after expiry: owner=%q err=%v", owner, err)
	}
	cancelled := ticketHash("*2.cancelled")
	if err := st.IssueChannelTicket(cancelled, account, 2, 0, time.Minute); err != nil {
		t.Fatalf("ticket after an expired one: %v", err)
	}
	if err := st.CancelChannelTicket(cancelled); err != nil {
		t.Fatal(err)
	}
	if owner, err := st.ClaimAccountLease(account, "w9-test/source", 1, 3); err != nil || owner != "" {
		t.Fatalf("login after cancel: owner=%q err=%v", owner, err)
	}
}

func TestChannelTicketHasOneConsumer(t *testing.T) {
	const account = "w9ticket4"
	st := channelTicketsStore(t, account)
	token := ticketHash("*2.contended")
	if err := st.IssueChannelTicket(token, account, 2, 1, time.Minute); err != nil {
		t.Fatal(err)
	}
	var wg sync.WaitGroup
	errs := make([]error, 4)
	for i := range errs {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			_, errs[i] = st.ClaimAccountLeaseWithTicket(account, "w9-test/target", 2, int64(i+1), token)
		}(i)
	}
	wg.Wait()
	consumed := 0
	for _, err := range errs {
		switch {
		case err == nil:
			consumed++
		case !errors.Is(err, ErrChannelTicketInvalid):
			t.Fatalf("consume: %v", err)
		}
	}
	if consumed != 1 {
		t.Fatalf("%d sessions consumed one ticket", consumed)
	}
}
