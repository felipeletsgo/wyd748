package store

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestValidateDonateCreditBoundaries(t *testing.T) {
	valid := DonateCredit{
		Account: "Player01", Amount: 1, Reason: "Support credit", Source: DonateSourceStaff,
		Actor: "staff", RequestID: "request-1",
	}
	if err := ValidateDonateCredit(valid); err != nil {
		t.Fatalf("valid staff credit: %v", err)
	}
	valid.Source = DonateSourceWeb
	if err := ValidateDonateCredit(valid); err != nil {
		t.Fatalf("valid web credit: %v", err)
	}

	tests := []struct {
		name string
		edit func(*DonateCredit)
	}{
		{name: "empty account", edit: func(c *DonateCredit) { c.Account = "  " }},
		{name: "long account", edit: func(c *DonateCredit) { c.Account = strings.Repeat("a", 13) }},
		{name: "zero amount", edit: func(c *DonateCredit) { c.Amount = 0 }},
		{name: "amount exceeds balance", edit: func(c *DonateCredit) { c.Amount = model.MaxDonateBalance + 1 }},
		{name: "short reason", edit: func(c *DonateCredit) { c.Reason = " x " }},
		{name: "long reason", edit: func(c *DonateCredit) { c.Reason = strings.Repeat("x", 201) }},
		{name: "invalid UTF-8", edit: func(c *DonateCredit) { c.Reason = string([]byte{0xff, 'a', 'b'}) }},
		{name: "control character", edit: func(c *DonateCredit) { c.Reason = "valid\nreason" }},
		{name: "unknown source", edit: func(c *DonateCredit) { c.Source = "import" }},
		{name: "long actor", edit: func(c *DonateCredit) { c.Actor = strings.Repeat("a", 65) }},
		{name: "long request ID", edit: func(c *DonateCredit) { c.RequestID = strings.Repeat("a", 65) }},
		{name: "spaced request ID", edit: func(c *DonateCredit) { c.RequestID = "request 1" }},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			credit := valid
			credit.Source = DonateSourceStaff
			tt.edit(&credit)
			if err := ValidateDonateCredit(credit); err == nil {
				t.Fatal("invalid credit was accepted")
			}
		})
	}
}

func TestValidateDonateLedgerRejectsInvalidRows(t *testing.T) {
	account := &model.Account{Name: "Player01"}
	valid := DonateLedgerEntry{Account: "player01", Delta: 1, BalanceAfter: 1, Kind: DonateKindCredit}
	if err := ValidateDonateLedger(account, []DonateLedgerEntry{valid,
		{Account: "PLAYER01", Delta: -1, BalanceAfter: 0, Kind: DonateKindPurchase},
		{Account: "Player01", Delta: 1, BalanceAfter: 1, Kind: DonateKindCoinUse},
	}); err != nil {
		t.Fatalf("valid ledger entries: %v", err)
	}
	tests := []struct {
		name string
		edit func(*DonateLedgerEntry)
	}{
		{name: "zero delta", edit: func(e *DonateLedgerEntry) { e.Delta = 0 }},
		{name: "balance exceeds maximum", edit: func(e *DonateLedgerEntry) { e.BalanceAfter = model.MaxDonateBalance + 1 }},
		{name: "unknown kind", edit: func(e *DonateLedgerEntry) { e.Kind = "refund" }},
		{name: "another account", edit: func(e *DonateLedgerEntry) { e.Account = "other" }},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			entry := valid
			tt.edit(&entry)
			if err := ValidateDonateLedger(account, []DonateLedgerEntry{entry}); err == nil {
				t.Fatal("invalid ledger entry was accepted")
			}
		})
	}
}
