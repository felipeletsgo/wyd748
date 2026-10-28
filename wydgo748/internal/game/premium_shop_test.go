package game

import (
	"context"
	"encoding/binary"
	"strings"
	"testing"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// premiumTestStore records donate writes through the critical test store.
type premiumTestStore struct {
	*criticalTestStore
}

func (s *premiumTestStore) LoadPremiumCatalog(context.Context) (store.PremiumCatalog, error) {
	return store.PremiumCatalog{}, nil
}
func (s *premiumTestStore) SavePremiumCatalog(_ context.Context, _ uint32, entries []store.PremiumCatalogEntry, actor string) (store.PremiumCatalog, error) {
	return store.PremiumCatalog{Version: 1, Entries: entries, UpdatedBy: actor}, nil
}
func (s *premiumTestStore) AddDonateCredit(_ context.Context, c store.DonateCredit) (store.DonateCredit, bool, error) {
	c.ID = 1
	return c, false, nil
}
func (s *premiumTestStore) PendingDonateCredits(context.Context, []string, int) ([]store.DonateCredit, error) {
	return nil, nil
}
func (s *premiumTestStore) DonateSummary(context.Context, string, int) (store.DonateSummary, error) {
	return store.DonateSummary{}, nil
}
func (s *premiumTestStore) SaveDonateAccount(account *model.Account, credits []int64, ledger []store.DonateLedgerEntry) error {
	return s.record(criticalWrite{accounts: []*model.Account{account},
		donate: &criticalDonateWrite{credits: credits, ledger: ledger}})
}

const premiumTestPrice = 1000

func TestPremiumCatalogCategoriesReachStaffAndPlayer(t *testing.T) {
	w, p, shop, _, _, _ := premiumFixture(t)
	view := w.premiumCatalogView(w.premiumCatalog)
	if len(view.Items) != 1 || view.Items[0].Category != 2 {
		t.Fatalf("staff lost category: %+v", view)
	}
	command := control.PremiumCatalogCommand{Version: 1, Reason: "Move item to Nifleheim", Items: view.Items}
	command.Items[0].Category = 3
	result, err := w.SavePremiumCatalog(context.Background(), "staff", command, func() bool { return true })
	if err != nil || result.Code != "ok" || result.Catalog.Items[0].Category != 3 {
		t.Fatalf("category save failed: %+v %v", result, err)
	}
	w.premiumCatalog.Entries[0].Category = 3
	w.openPremiumShop(p.Session, p, shop)
	_ = nextPacket(t, p)
	packet := nextPacket(t, p)
	if len(packet) != wire.PremiumCategoryStateSize || binary.LittleEndian.Uint16(packet[12:14]) != wire.PremiumCategoryVersion || packet[134] != 3 {
		t.Fatalf("player lost category for slot 2: %x", packet)
	}
	command.Items[0].Category = 4
	result, err = w.SavePremiumCatalog(context.Background(), "staff", command, func() bool { return true })
	if err != nil || result.Code != "invalid_command" {
		t.Fatalf("unknown category accepted: %+v %v", result, err)
	}
}

func premiumWebBuy(p *Player, npc uint16, version, price uint32) []byte {
	return wire.PremiumBuyPacket(wire.PremiumBuyRequest{Version: wire.PremiumWebVersion,
		ID: p.ID, NPC: npc, ShopSlot: 2, CatalogVersion: version, Price: price, RequestID: 1})
}

func expectPremiumResult(t *testing.T, p *Player, code uint16, balance uint32) {
	t.Helper()
	b := nextPacket(t, p)
	if len(b) != wire.PremiumBuyResultSize || wire.ParseHeader(b).Type != wire.OpPremiumBuyResult ||
		binary.LittleEndian.Uint16(b[12:]) != wire.PremiumWebVersion ||
		binary.LittleEndian.Uint16(b[14:]) != code || binary.LittleEndian.Uint32(b[16:]) != balance ||
		binary.LittleEndian.Uint32(b[20:]) != 1 {
		t.Fatalf("wrong web completion: %x", b)
	}
}

func TestPremiumWebSettlementWaitsForDurability(t *testing.T) {
	w, p, shop, backend, queue, release := premiumFixture(t)
	openPremiumForTest(t, w, p, shop)
	w.handle(command{s: p.Session, pkt: premiumWebBuy(p, shop.ID, 3, premiumTestPrice)})
	write := awaitCriticalWrite(t, backend.entered)
	if p.Account.Donate != 5000 || p.Char.Inv[1].Index != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("web purchase acknowledged or mutated before durable settlement")
	}
	// Exercise real ingress: duplicate commands are deferred by critical ownership.
	w.handle(command{s: p.Session, pkt: premiumWebBuy(p, shop.ID, 3, premiumTestPrice)})
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("a replay must not settle before the original purchase")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	item := nextPacket(t, p)
	if wire.ParseHeader(item).Type != wire.OpSendItem || p.Char.Inv[1] != write.accounts[0].Chars[p.CharSlot].Inv[1] {
		t.Fatal("web settlement must deliver the authoritative inventory item")
	}
	if wire.ParseHeader(nextPacket(t, p)).Type != wire.OpPremiumShopState {
		t.Fatal("missing settled balance update")
	}
	expectPremiumResult(t, p, wire.PremiumResultSuccess, 4000)
	expectPremiumResult(t, p, wire.PremiumResultSuccess, 4000)
	// Reopening does not reset the replay high-water mark.
	openPremiumForTest(t, w, p, shop)
	w.handle(command{s: p.Session, pkt: premiumWebBuy(p, shop.ID, 3, premiumTestPrice)})
	expectPremiumResult(t, p, wire.PremiumResultSuccess, 4000)
	if p.Account.Donate != 4000 || p.Char.Inv[2].Index != 0 {
		t.Fatal("duplicate ingress charged or delivered twice")
	}
	if p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
		t.Fatal("duplicate settlement or write")
	}
}

func TestPremiumWebNewIntentAndStaleOrConflictingReplay(t *testing.T) {
	w, p, shop, backend, queue, release := premiumFixture(t)
	openPremiumForTest(t, w, p, shop)
	first := premiumWebBuy(p, shop.ID, 3, premiumTestPrice)
	w.handle(command{s: p.Session, pkt: first})
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	nextPacket(t, p)
	nextPacket(t, p)
	expectPremiumResult(t, p, wire.PremiumResultSuccess, 4000)
	// A changed payload cannot reuse an already settled request ID.
	conflict := premiumWebBuy(p, shop.ID, 3, premiumTestPrice-1)
	w.handle(command{s: p.Session, pkt: conflict})
	expectPremiumResult(t, p, wire.PremiumResultFailed, 4000)
	second := append([]byte(nil), first...)
	binary.LittleEndian.PutUint32(second[28:], 2)
	w.handle(command{s: p.Session, pkt: second})
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	nextPacket(t, p)
	nextPacket(t, p)
	result := nextPacket(t, p)
	if binary.LittleEndian.Uint16(result[14:]) != wire.PremiumResultSuccess ||
		binary.LittleEndian.Uint32(result[20:]) != 2 || p.Account.Donate != 3000 || p.Char.Inv[2].Index != 400 {
		t.Fatal("a new intent must allow another legitimate purchase")
	}
	w.handle(command{s: p.Session, pkt: first})
	expectPremiumResult(t, p, wire.PremiumResultFailed, 3000)
	if len(backend.entered) != 0 || p.Char.Inv[3].Index != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("stale or conflicting replay initiated another write")
	}
}

func TestPremiumWebRejectionsHaveExplicitCompletion(t *testing.T) {
	for _, name := range []string{"catalog", "price", "balance", "inventory", "merchant", "dead", "item"} {
		t.Run(name, func(t *testing.T) {
			w, p, shop, backend, _, _ := premiumFixture(t)
			openPremiumForTest(t, w, p, shop)
			version, price := uint32(3), uint32(premiumTestPrice)
			code := wire.PremiumResultCatalogChanged
			switch name {
			case "catalog":
				version = 2
			case "price":
				price--
			case "balance":
				p.Account.Donate = 1
				code = wire.PremiumResultInsufficient
			case "inventory":
				for i := range p.Char.Inv {
					p.Char.Inv[i] = model.Item{Index: 200, UID: "full"}
				}
				code = wire.PremiumResultInventoryFull
			case "merchant":
				p.PremiumNPC = 0
				code = wire.PremiumResultExpired
			case "dead":
				p.Char.Score.CurHP = 0
				p.Char.RuntimeScore = nil
				code = wire.PremiumResultExpired
			case "item":
				delete(w.items, 400)
				code = wire.PremiumResultItemUnavailable
			}
			before := *p.Account
			w.onPremiumBuy(p.Session, premiumWebBuy(p, shop.ID, version, price))
			expectPremiumResult(t, p, code, before.Donate)
			if p.Account.Donate != before.Donate || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("rejected web purchase wrote, changed balance or left extra packets")
			}
		})
	}
}

func TestPremiumWebRejectedWriteDoesNotChargeOrDeliver(t *testing.T) {
	w, p, shop, backend, queue, release := premiumFixture(t)
	openPremiumForTest(t, w, p, shop)
	backend.failure = criticalCertifiedRejection(t)
	w.handle(command{s: p.Session, pkt: premiumWebBuy(p, shop.ID, 3, premiumTestPrice)})
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	expectPremiumResult(t, p, wire.PremiumResultFailed, 5000)
	backend.failure = nil
	w.handle(command{s: p.Session, pkt: premiumWebBuy(p, shop.ID, 3, premiumTestPrice)})
	expectPremiumResult(t, p, wire.PremiumResultFailed, 5000)
	if len(backend.entered) != 0 {
		t.Fatal("a rejected intent must not become a new purchase on replay")
	}
	if p.Account.Donate != 5000 || p.Char.Inv[1].Index != 0 || p.PersistencePoisoned ||
		p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("rejected write must preserve the balance and inventory without publishing success")
	}
}

func premiumFixture(t *testing.T) (*World, *Player, *Mob, *premiumTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, _ := saleIngressWorld(t)
	shop := w.mobsByID[p.ShopNPC]
	shop.Def.Equip.Rosto.Index = premiumMerchantFace
	shop.Def.Score.Merchant = 0
	p.ShopNPC = 0
	w.items[3394] = model.ItemDef{Index: 3394}
	w.premiumCatalog = store.PremiumCatalog{Version: 3, Entries: []store.PremiumCatalogEntry{
		{Slot: 2, Item: model.Item{Index: 400, Eff: [6]byte{61, 7}}, Price: premiumTestPrice, Category: 2},
	}}
	p.Account.Donate = 5000
	p.Char.Inv[0] = model.Item{Index: 200, UID: "occupied"}
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &premiumTestStore{criticalTestStore: &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}}
	queue := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	w.store = backend
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, shop, backend, queue, release
}

func nextPacket(t *testing.T, p *Player) []byte {
	t.Helper()
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) {
		t.Fatal("missing response packet")
	}
	return packet
}

func expectMessage(t *testing.T, p *Player, contains string) {
	t.Helper()
	packet := nextPacket(t, p)
	if wire.ParseHeader(packet).Type != wire.OpMessagePanel || !strings.Contains(string(packet[12:]), contains) {
		t.Fatalf("expected message %q, got opcode 0x%X %q", contains, wire.ParseHeader(packet).Type, packet[12:])
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("rejection published extra packets")
	}
}

func openPremiumForTest(t *testing.T, w *World, p *Player, shop *Mob) {
	t.Helper()
	request := inboundPacket(wire.OpReqShopList, 16)
	binary.LittleEndian.PutUint16(request[6:8], p.ID)
	binary.LittleEndian.PutUint16(request[12:14], shop.ID)
	w.handle(command{s: p.Session, pkt: request})
	list := nextPacket(t, p)
	if wire.ParseHeader(list).Type != wire.OpShopList || binary.LittleEndian.Uint32(list[12:16]) != wire.ShopPremium ||
		binary.LittleEndian.Uint16(list[16+2*8:]) != 400 || binary.LittleEndian.Uint16(list[16:]) != 0 ||
		binary.LittleEndian.Uint32(list[232:236]) != 0 {
		t.Fatal("premium shop list must place catalog items in their cells with ShopType 4 and no tax")
	}
	state := nextPacket(t, p)
	if wire.ParseHeader(state).Type != wire.OpPremiumShopState ||
		binary.LittleEndian.Uint16(state[14:16]) != wire.PremiumStateOpen ||
		binary.LittleEndian.Uint32(state[16:20]) != 3 || binary.LittleEndian.Uint32(state[20:24]) != p.Account.Donate ||
		binary.LittleEndian.Uint32(state[24+2*4:]) != premiumTestPrice {
		t.Fatal("premium state must carry version, balance and per-cell prices")
	}
	if p.PremiumNPC != shop.ID || p.PremiumVersion != 3 || p.ShopNPC != 0 {
		t.Fatal("opening the premium shop must bind only the premium context")
	}
}

func premiumBuy(p *Player, npc uint16, slot uint16, version, price uint32) []byte {
	return wire.PremiumBuyPacket(wire.PremiumBuyRequest{ID: p.ID, NPC: npc, ShopSlot: slot,
		CarryHint: 1, CatalogVersion: version, Price: price})
}

func TestPremiumShopPurchaseCommitsItemBalanceAndLedgerTogether(t *testing.T) {
	w, p, shop, backend, queue, release := premiumFixture(t)
	openPremiumForTest(t, w, p, shop)
	w.handle(command{s: p.Session, pkt: premiumBuy(p, shop.ID, 2, 3, premiumTestPrice)})
	write := awaitCriticalWrite(t, backend.entered)
	proposed := write.accounts[0]
	if write.donate == nil || len(write.donate.credits) != 0 || len(write.donate.ledger) != 1 ||
		proposed == p.Account || proposed.Donate != 4000 {
		t.Fatalf("purchase write must be a detached donate write: %+v", write.donate)
	}
	bought := proposed.Chars[p.CharSlot].Inv[1]
	entry := write.donate.ledger[0]
	if bought.Index != 400 || bought.Eff != [6]byte{61, 7} || bought.UID == "" ||
		entry.Kind != store.DonateKindPurchase || entry.Delta != -premiumTestPrice || entry.BalanceAfter != 4000 ||
		entry.ItemIndex != 400 || entry.ItemUID != bought.UID || entry.Character != p.Char.Name {
		t.Fatalf("purchase proposal or ledger is wrong: item=%+v ledger=%+v", bought, entry)
	}
	if p.Account.Donate != 5000 || p.Char.Inv[1].Index != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("purchase published or mutated before the commit was acknowledged")
	}
	// A repeated request reaching the handler while the account is reserved is
	// refused (World.handle defers it until the commit is acknowledged).
	w.onPremiumBuy(p.Session, premiumBuy(p, shop.ID, 2, 3, premiumTestPrice))
	expectMessage(t, p, "last action")
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Account.Donate != 4000 || p.Char.Inv[1] != bought {
		t.Fatal("acknowledged purchase did not install item and balance")
	}
	confirm := nextPacket(t, p)
	if wire.ParseHeader(confirm).Type != wire.OpBuyItem || binary.LittleEndian.Uint16(confirm[12:14]) != shop.ID ||
		binary.LittleEndian.Uint16(confirm[14:16]) != 2 || binary.LittleEndian.Uint16(confirm[16:18]) != 1 {
		t.Fatal("purchase must be confirmed with the native 0x379 for the bought cell")
	}
	balance := nextPacket(t, p)
	if wire.ParseHeader(balance).Type != wire.OpPremiumShopState || binary.LittleEndian.Uint32(balance[20:24]) != 4000 {
		t.Fatal("purchase must publish the new balance")
	}
}

func TestPremiumShopRejectionsChangeNothing(t *testing.T) {
	w, p, shop, backend, _, _ := premiumFixture(t)
	w.handle(command{s: p.Session, pkt: premiumBuy(p, shop.ID, 2, 3, premiumTestPrice)})
	expectMessage(t, p, "Open the premium shop")
	openPremiumForTest(t, w, p, shop)
	for _, tc := range []struct {
		name    string
		packet  []byte
		message string
	}{
		{"stale version", premiumBuy(p, shop.ID, 2, 2, premiumTestPrice), "shop changed"},
		{"stale price", premiumBuy(p, shop.ID, 2, 3, premiumTestPrice-1), "shop changed"},
		{"empty cell", premiumBuy(p, shop.ID, 5, 3, premiumTestPrice), "shop changed"},
		{"other npc", premiumBuy(p, shop.ID+1, 2, 3, premiumTestPrice), "Open the premium shop"},
	} {
		w.handle(command{s: p.Session, pkt: tc.packet})
		expectMessage(t, p, tc.message)
	}
	p.Account.Donate = premiumTestPrice - 1
	w.handle(command{s: p.Session, pkt: premiumBuy(p, shop.ID, 2, 3, premiumTestPrice)})
	expectMessage(t, p, "not have enough donate")
	p.Account.Donate = 5000
	for slot := 1; slot < model.PlayerCarrySlots; slot++ {
		p.Char.Inv[slot] = model.Item{Index: 200, UID: "filler"}
	}
	w.handle(command{s: p.Session, pkt: premiumBuy(p, shop.ID, 2, 3, premiumTestPrice)})
	expectMessage(t, p, "inventory is full")
	forged := premiumBuy(p, shop.ID, 2, 3, premiumTestPrice)
	binary.LittleEndian.PutUint16(forged[6:8], p.ID+1)
	w.handle(command{s: p.Session, pkt: forged})
	if p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 || p.Account.Donate != 5000 {
		t.Fatal("rejected purchases must not write, publish or change the balance")
	}
}

func TestPremiumShopUnavailableWithoutDonateStore(t *testing.T) {
	w, p, _ := saleIngressWorld(t)
	shop := w.mobsByID[p.ShopNPC]
	shop.Def.Equip.Rosto.Index = premiumMerchantFace
	request := inboundPacket(wire.OpReqShopList, 16)
	binary.LittleEndian.PutUint16(request[6:8], p.ID)
	binary.LittleEndian.PutUint16(request[12:14], shop.ID)
	w.handle(command{s: p.Session, pkt: request})
	expectMessage(t, p, "premium shop is not available")
}

func TestDonateCoinCreditsAccountInOneWrite(t *testing.T) {
	w, p, _, backend, queue, release := premiumFixture(t)
	p.Char.Inv[5] = model.Item{Index: 3394, UID: "coin-uid"}
	rule := model.VolatileRule{Action: "grant_counter", Consume: true, Counters: map[string]uint32{donateCounter: 1000}}
	if !isDonateCoinRule(rule) || isDonateCoinRule(model.VolatileRule{Action: "grant_counter",
		Counters: map[string]uint32{donateCounter: 1, "fame": 1}}) {
		t.Fatal("only a donate-only counter is a Wyden coin")
	}
	w.admitDonateCoin(p, 5, rule)
	write := awaitCriticalWrite(t, backend.entered)
	if write.donate == nil || write.accounts[0].Donate != 6000 || write.accounts[0].Chars[p.CharSlot].Inv[5].Index != 0 ||
		write.donate.ledger[0].Kind != store.DonateKindCoinUse || write.donate.ledger[0].ItemUID != "coin-uid" ||
		write.donate.ledger[0].Delta != 1000 {
		t.Fatalf("coin use proposal is wrong: %+v", write.donate)
	}
	if p.Char.Inv[5].Index != 3394 || p.Account.Donate != 5000 {
		t.Fatal("coin use mutated live state before the commit")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Inv[5].Index != 0 || p.Account.Donate != 6000 {
		t.Fatal("acknowledged coin use did not consume the coin and credit the balance")
	}
	if wire.ParseHeader(nextPacket(t, p)).Type != wire.OpSendItem {
		t.Fatal("coin use must republish the consumed slot")
	}
	if balance := nextPacket(t, p); binary.LittleEndian.Uint32(balance[20:24]) != 6000 {
		t.Fatal("coin use must publish the balance")
	}
	expectMessage(t, p, "You received 1000 donate")

	p.Char.Inv[6] = model.Item{Index: 3394, UID: "coin-two"}
	p.Account.Donate = model.MaxDonateBalance
	w.admitDonateCoin(p, 6, rule)
	if wire.ParseHeader(nextPacket(t, p)).Type != wire.OpSendItem {
		t.Fatal("a refused coin must resync the slot")
	}
	expectMessage(t, p, "balance is full")
	if len(backend.entered) != 0 || p.Char.Inv[6].Index != 3394 {
		t.Fatal("a coin over the cap must not be consumed")
	}
}

func TestDonateCreditsApplyOnceWhileInFlight(t *testing.T) {
	w, p, _, backend, queue, release := premiumFixture(t)
	credits := []store.DonateCredit{{ID: 7, Account: p.Account.Name, Amount: 300}, {ID: 8, Account: p.Account.Name, Amount: 200}}
	w.applyDonateCredits(credits)
	write := awaitCriticalWrite(t, backend.entered)
	if write.donate == nil || len(write.donate.credits) != 2 || write.accounts[0].Donate != 5500 ||
		len(write.donate.ledger) != 2 || write.donate.ledger[1].BalanceAfter != 5500 {
		t.Fatalf("credit proposal is wrong: %+v", write.donate)
	}
	w.applyDonateCredits(credits)
	if len(backend.entered) != 0 {
		t.Fatal("credits in flight were submitted twice")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Account.Donate != 5500 || len(w.premiumInFlight) != 0 {
		t.Fatal("applied credits did not update the balance or release the in-flight set")
	}
	if balance := nextPacket(t, p); binary.LittleEndian.Uint32(balance[20:24]) != 5500 {
		t.Fatal("credit must publish the balance")
	}
	expectMessage(t, p, "You received 500 donate")
}

func TestPremiumCatalogInstallSkipsUnknownItems(t *testing.T) {
	w, _, _, _, _, _ := premiumFixture(t)
	w.installPremiumCatalog(store.PremiumCatalog{Version: 9, Entries: []store.PremiumCatalogEntry{
		{Slot: 0, Item: model.Item{Index: 400}, Price: 10},
		{Slot: 1, Item: model.Item{Index: 9999}, Price: 10},
	}})
	if w.premiumCatalog.Version != 9 || len(w.premiumCatalog.Entries) != 1 || w.premiumCatalog.Entries[0].Slot != 0 {
		t.Fatalf("catalog install kept unknown items: %+v", w.premiumCatalog)
	}
}

func TestPremiumShopOpensWithEmptyCatalog(t *testing.T) {
	w, p, shop, _, _, _ := premiumFixture(t)
	w.premiumCatalog = store.PremiumCatalog{}
	request := inboundPacket(wire.OpReqShopList, 16)
	binary.LittleEndian.PutUint16(request[6:8], p.ID)
	binary.LittleEndian.PutUint16(request[12:14], shop.ID)
	w.handle(command{s: p.Session, pkt: request})
	list := nextPacket(t, p)
	if wire.ParseHeader(list).Type != wire.OpShopList || binary.LittleEndian.Uint32(list[12:16]) != wire.ShopPremium ||
		binary.LittleEndian.Uint16(list[16+2*8:]) != 0 {
		t.Fatal("an empty catalog must still open an empty premium window")
	}
	state := nextPacket(t, p)
	if wire.ParseHeader(state).Type != wire.OpPremiumShopState || binary.LittleEndian.Uint32(state[16:20]) != 0 ||
		binary.LittleEndian.Uint32(state[20:24]) != p.Account.Donate || p.PremiumNPC != shop.ID {
		t.Fatal("an empty premium window must still show the balance")
	}
	w.handle(command{s: p.Session, pkt: premiumBuy(p, shop.ID, 2, 0, premiumTestPrice)})
	expectMessage(t, p, "shop changed")
}
