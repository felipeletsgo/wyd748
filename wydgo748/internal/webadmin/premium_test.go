package webadmin

import (
	"context"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/control"
)

type testPremium struct {
	saves, credits int
	actor          string
	beforeExecute  func()
}

func (f *testPremium) PremiumCatalog(context.Context) (control.PremiumCatalogView, error) {
	return control.PremiumCatalogView{Version: 1, Available: true, CatalogVersion: 4, Items: []control.PremiumCatalogItem{}}, nil
}
func (f *testPremium) SavePremiumCatalog(_ context.Context, actor string, c control.PremiumCatalogCommand, authorize func() bool) (control.PremiumCatalogResult, error) {
	f.saves++
	f.actor = actor
	if f.beforeExecute != nil {
		f.beforeExecute()
	}
	if !authorize() {
		return control.PremiumCatalogResult{Code: "capability_required"}, nil
	}
	return control.PremiumCatalogResult{Code: "ok", Catalog: control.PremiumCatalogView{Version: 1, CatalogVersion: c.ExpectedCatalogVersion + 1}}, nil
}
func (f *testPremium) CreditDonate(_ context.Context, actor string, c control.DonateCreditCommand, authorize func() bool) (control.DonateCreditResult, error) {
	f.credits++
	f.actor = actor
	if !authorize() {
		return control.DonateCreditResult{Code: "capability_required"}, nil
	}
	return control.DonateCreditResult{Code: "ok", CreditID: 11}, nil
}
func (f *testPremium) DonateAccount(_ context.Context, account string) (control.DonateAccountView, error) {
	if account == "ghost" {
		return control.DonateAccountView{}, control.ErrUnknownAccount
	}
	return control.DonateAccountView{Version: 1, Account: account, Balance: 900}, nil
}

func catalogBody() string {
	b, _ := json.Marshal(control.PremiumCatalogCommand{Version: 1, ExpectedCatalogVersion: 4, Reason: "weekly catalog",
		Items: []control.PremiumCatalogItem{{Slot: 0, ItemID: 3394, Price: 1000}}})
	return string(b)
}

func creditBody(account string) string {
	b, _ := json.Marshal(control.DonateCreditCommand{Version: 1, RequestID: "credit-request-0001", Account: account,
		Amount: 500, Reason: "donation receipt 42"})
	return string(b)
}

func TestPremiumCatalogHTTPBoundaries(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	path := "/api/v1/staff/premium-shop"
	cookie, session, _ := loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 403 {
		t.Fatal("catalog readable without capability", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, control.PremiumCatalogCapability}}
	cookie, session, _ = loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 503 || !strings.Contains(r.Body.String(), "premium_unavailable") {
		t.Fatal("missing premium source must be unavailable", r.Code, r.Body.String())
	}
	f := &testPremium{}
	h.cfg.Premium = f
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"catalogVersion":4`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if r := request(h, "PUT", path, catalogBody(), cookie, ""); r.Code != 403 || f.saves != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	for _, bad := range []string{`{}`, catalogBody() + ` {}`,
		strings.Replace(catalogBody(), `"price":1000`, `"price":0`, 1),
		strings.Replace(catalogBody(), `"slot":0`, `"slot":27`, 1),
		strings.Replace(catalogBody(), `"reason":"weekly catalog"`, `"reason":"x"`, 1)} {
		if r := request(h, "PUT", path, bad, cookie, session.CSRF); r.Code != 400 || f.saves != 0 {
			t.Fatal("invalid catalog forwarded", r.Code, bad)
		}
	}
	if r := request(h, "PUT", path, catalogBody(), cookie, session.CSRF); r.Code != 200 || f.actor != "operator" ||
		!strings.Contains(r.Body.String(), `"code":"ok"`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "premium.catalog") || !strings.Contains(logs.String(), "weekly catalog") {
		t.Fatal("catalog save not audited")
	}
	f.beforeExecute = func() { delete(entries, "operator") }
	if r := request(h, "PUT", path, catalogBody(), cookie, session.CSRF); r.Code != 200 || !strings.Contains(r.Body.String(), "capability_required") {
		t.Fatal("revocation during execution bypassed", r.Code, r.Body.String())
	}
}

func TestDonateCreditHTTPBoundaries(t *testing.T) {
	h, entries, _, logs := newTestHandler(t)
	f := &testPremium{}
	h.cfg.Premium = f
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, control.PremiumCatalogCapability}}
	cookie, session, _ := loginTest(t, h)
	path := "/api/v1/staff/accounts/player/donate"
	if r := request(h, "POST", path, creditBody("player"), cookie, session.CSRF); r.Code != 403 || f.credits != 0 {
		t.Fatal("catalog capability must not grant credits", r.Code)
	}
	entries["operator"] = Staff{Capabilities: []string{StatusCapability, control.DonateCreditCapability}}
	cookie, session, _ = loginTest(t, h)
	if r := request(h, "GET", path, "", cookie, ""); r.Code != 200 || !strings.Contains(r.Body.String(), `"balance":900`) {
		t.Fatal(r.Code, r.Body.String())
	}
	if r := request(h, "GET", "/api/v1/staff/accounts/ghost/donate", "", cookie, ""); r.Code != 404 {
		t.Fatal("unknown account", r.Code)
	}
	for _, bad := range []string{creditBody("other"), strings.Replace(creditBody("player"), `"amount":500`, `"amount":0`, 1),
		strings.Replace(creditBody("player"), `credit-request-0001`, `short`, 1), creditBody("player") + ` {}`} {
		if r := request(h, "POST", path, bad, cookie, session.CSRF); r.Code != 400 || f.credits != 0 {
			t.Fatal("invalid credit forwarded", r.Code, bad)
		}
	}
	if r := request(h, "POST", path, creditBody("player"), cookie, ""); r.Code != 403 || f.credits != 0 {
		t.Fatal("CSRF bypass", r.Code)
	}
	if r := request(h, "POST", path, creditBody("player"), cookie, session.CSRF); r.Code != 200 ||
		!strings.Contains(r.Body.String(), `"creditId":11`) || f.actor != "operator" {
		t.Fatal(r.Code, r.Body.String())
	}
	if !strings.Contains(logs.String(), "donate.credit") || !strings.Contains(logs.String(), "credit-request-0001") {
		t.Fatal("credit not audited")
	}
}

func TestStaffFileAcceptsPremiumCapabilities(t *testing.T) {
	path := filepath.Join(t.TempDir(), "staff.json")
	raw := `{"owner":{"capabilities":["server.status","` + control.PremiumCatalogCapability + `","` + control.DonateCreditCapability + `"]}}`
	if err := os.WriteFile(path, []byte(raw), 0o600); err != nil {
		t.Fatal(err)
	}
	entries, err := ReadStaff(path)
	if err != nil || !entries["owner"].permits(control.DonateCreditCapability) {
		t.Fatalf("premium capabilities rejected: %v", err)
	}
}
