package game

import (
	"context"
	"errors"
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// Premium shop and donate balance, EXTENSAO_COORDENADA v1/v3/v4
// (DOCS/protocol/premium-shop.md).

const (
	// premiumMerchantFace is the face item of Premium_Trader_Nell, the NPC the
	// native 7.48 client classifies as the premium merchant.
	premiumMerchantFace = 57
	// donateCounter is the volatile counter name that credits the account-wide
	// donate balance instead of a per-character counter (Wyden coins).
	donateCounter        = "donate"
	premiumPollInterval  = 10 * time.Second
	premiumPollTimeout   = 3 * time.Second
	premiumCreditBatch   = 200
	premiumUnavailable   = "The premium shop is not available on this server."
	premiumShopChanged   = "The shop changed. Please open it again."
	premiumNotEnough     = "You do not have enough donate."
	premiumPurchaseError = "The purchase could not be completed."
)

// premiumStore is implemented by the PostgreSQL store.
type premiumStore interface {
	LoadPremiumCatalog(context.Context) (store.PremiumCatalog, error)
	SavePremiumCatalog(context.Context, uint32, []store.PremiumCatalogEntry, string) (store.PremiumCatalog, error)
	AddDonateCredit(context.Context, store.DonateCredit) (store.DonateCredit, bool, error)
	PendingDonateCredits(context.Context, []string, int) ([]store.DonateCredit, error)
	DonateSummary(context.Context, string, int) (store.DonateSummary, error)
	criticalDonateStore
}

// premiumEvent carries the background poll result into the World goroutine.
type premiumEvent struct {
	catalog    *store.PremiumCatalog
	credits    []store.DonateCredit
	pollFailed bool
}

// premiumBackend reports whether this server can run the premium shop: the
// store must provide the donate adapters and writes must go through the
// critical coordinator, which commits balance and items together.
func (w *World) premiumBackend() (premiumStore, bool) {
	backend, ok := w.store.(premiumStore)
	return backend, ok && w.criticalWrites != nil
}

func isPremiumMerchant(def *model.NPCDef) bool {
	return def != nil && !def.IsMonster() && def.Equip.Rosto.Index == premiumMerchantFace
}

// isDonateCoinRule marks the Wyden coins: a grant_counter whose only counter
// is the account donate balance.
func isDonateCoinRule(rule model.VolatileRule) bool {
	if rule.Action != "grant_counter" || len(rule.Counters) != 1 {
		return false
	}
	_, ok := rule.Counters[donateCounter]
	return ok
}

// premiumEntry resolves a shop cell against the current catalog.
func (w *World) premiumEntry(slot int) (store.PremiumCatalogEntry, bool) {
	for _, entry := range w.premiumCatalog.Entries {
		if entry.Slot == slot {
			return entry, true
		}
	}
	return store.PremiumCatalogEntry{}, false
}

func (w *World) openPremiumShop(s *net.Session, p *Player, m *Mob) {
	if _, ok := w.premiumBackend(); !ok {
		s.Send(wire.MessagePanel(premiumUnavailable))
		return
	}
	// An empty catalog still opens the (empty) window and shows the balance.
	var items [wire.PremiumShopSlots]model.Item
	var prices [wire.PremiumShopSlots]uint32
	var categories [wire.PremiumShopSlots]uint8
	for _, entry := range w.premiumCatalog.Entries {
		items[entry.Slot] = model.Item{Index: entry.Item.Index, Eff: entry.Item.Eff}
		prices[entry.Slot] = entry.Price
		categories[entry.Slot] = entry.Category
	}
	p.PremiumNPC, p.PremiumVersion = m.ID, w.premiumCatalog.Version
	s.Send(wire.ShopList(items[:], 0, wire.ShopPremium))
	s.Send(wire.PremiumCategorizedShopState(p.ID, w.premiumCatalog.Version, p.Account.Donate, prices, categories))
	log.Printf("[#%d] premium shop opened: %q catalog v%d balance=%d", s.ID, m.Def.Name, w.premiumCatalog.Version, p.Account.Donate)
}

// The high-water mark survives shop reopenings for this player connection.
// Lower IDs never execute; an equal ID can only replay its original result.
type premiumPurchaseReceipt struct {
	request wire.PremiumBuyRequest
	code    uint16
	settled bool
}

func completePremiumPurchase(p *Player, req wire.PremiumBuyRequest, code uint16) {
	if req.Version != wire.PremiumWebVersion {
		return
	}
	p.premiumReceipt.code, p.premiumReceipt.settled = code, true
	if !p.Session.IsClosed() {
		p.Session.Send(wire.PremiumBuyResult(p.ID, code, p.Account.Donate, req.RequestID))
	}
}

// onPremiumBuy: 0x7F21. Valid requests receive a v1 message or v3 completion;
// success commits item, balance and ledger in one transaction.
func (w *World) onPremiumBuy(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld || w.shuttingDown ||
		p.PersistencePoisoned || s.IsClosed() {
		return
	}
	req, err := wire.ParsePremiumBuy(pkt)
	if err != nil || req.ID != p.ID {
		w.recordSecurityViolation(s, wire.OpPremiumBuy, "malformed premium purchase")
		return
	}
	if req.Version == wire.PremiumWebVersion {
		previous := p.premiumReceipt
		if req.RequestID <= previous.request.RequestID {
			if req == previous.request {
				if previous.settled {
					s.Send(wire.PremiumBuyResult(p.ID, previous.code, p.Account.Donate, req.RequestID))
				}
			} else {
				// Do not overwrite a newer receipt with a stale or conflicting request.
				s.Send(wire.PremiumBuyResult(p.ID, wire.PremiumResultFailed, p.Account.Donate, req.RequestID))
			}
			return
		}
		if previous.request.RequestID != 0 && !previous.settled {
			s.Send(wire.PremiumBuyResult(p.ID, wire.PremiumResultBusy, p.Account.Donate, req.RequestID))
			return
		}
		p.premiumReceipt = premiumPurchaseReceipt{request: req}
	}
	reject := func(code uint16, message string) {
		if req.Version == wire.PremiumWebVersion {
			completePremiumPurchase(p, req, code)
		} else {
			s.Send(wire.MessagePanel(message))
		}
	}
	if playerCurHP(p.Char) == 0 {
		reject(wire.PremiumResultExpired, "The merchant is no longer available.")
		return
	}
	if _, ok := w.premiumBackend(); !ok {
		reject(wire.PremiumResultUnavailable, premiumUnavailable)
		return
	}
	if w.criticalPlayerReserved(p) || w.criticalTradeReserved(p) {
		reject(wire.PremiumResultBusy, "Please wait for your last action to finish.")
		return
	}
	if p.PremiumNPC == 0 || req.NPC != p.PremiumNPC {
		reject(wire.PremiumResultExpired, "Open the premium shop before buying.")
		return
	}
	m, interactionErr := w.resolveNPCInteraction(p, p.PremiumNPC)
	if interactionErr != nil || !isPremiumMerchant(m.Def) {
		p.PremiumNPC, p.PremiumVersion = 0, 0
		reject(wire.PremiumResultExpired, "The merchant is no longer available.")
		return
	}
	entry, found := w.premiumEntry(int(req.ShopSlot))
	if !found || req.CatalogVersion != w.premiumCatalog.Version || p.PremiumVersion != w.premiumCatalog.Version ||
		req.Price != entry.Price {
		reject(wire.PremiumResultCatalogChanged, premiumShopChanged)
		return
	}
	if p.Account.Donate < entry.Price {
		reject(wire.PremiumResultInsufficient, premiumNotEnough)
		return
	}
	if _, exists := w.items[entry.Item.Index]; !exists {
		reject(wire.PremiumResultItemUnavailable, "That item is unavailable on this server.")
		return
	}
	w.cancelTrade(p, "premium purchase")
	if err := w.queuePremiumPurchase(p, entry, req); err != nil {
		if errors.Is(err, errShopInventoryFull) {
			reject(wire.PremiumResultInventoryFull, "Your inventory is full.")
			return
		}
		log.Printf("[#%d] premium purchase not submitted slot=%d: %v", s.ID, req.ShopSlot, err)
		reject(wire.PremiumResultFailed, premiumPurchaseError)
	}
}

func (w *World) queuePremiumPurchase(p *Player, entry store.PremiumCatalogEntry, req wire.PremiumBuyRequest) error {
	proposed, err := criticalShopProposal(p)
	if err != nil {
		return err
	}
	if proposed.Donate < entry.Price {
		return fmt.Errorf("balance %d below price %d", proposed.Donate, entry.Price)
	}
	ch := &proposed.Chars[p.CharSlot]
	dst := addToInv(ch, model.Item{Index: entry.Item.Index, Eff: entry.Item.Eff})
	if dst < 0 {
		return errShopInventoryFull
	}
	proposed.Donate -= entry.Price
	bought, balance := ch.Inv[dst], proposed.Donate
	ledger := []store.DonateLedgerEntry{{
		Account: proposed.Name, Delta: -int64(entry.Price), BalanceAfter: balance,
		Kind: store.DonateKindPurchase, ItemIndex: bought.Index, ItemUID: bought.UID, Character: ch.Name,
	}}
	_, err = w.submitCriticalWriteWithRejection(criticalWrite{
		accounts: []*model.Account{proposed}, donate: &criticalDonateWrite{ledger: ledger},
	}, []*Player{p}, false, func(criticalPersistenceResult) error {
		p.Char.Inv[dst], p.Account.Donate = bought, balance
		if !p.Session.IsClosed() {
			if req.Version == wire.PremiumWebVersion {
				// The browser does not infer the delivered item from a UI grid.
				p.Session.Send(wire.SendItem(p.ID, placeInv, byte(dst), bought))
			} else {
				p.Session.Send(wire.BuyItem(p.ID, req.NPC, req.ShopSlot, uint16(dst), p.Char.Gold))
			}
			p.Session.Send(wire.PremiumBalance(p.ID, balance))
		}
		completePremiumPurchase(p, req, wire.PremiumResultSuccess)
		log.Printf("[#%d] premium purchase item=%d price=%d -> inv[%d] balance=%d",
			p.Session.ID, bought.Index, entry.Price, dst, balance)
		return nil
	}, nil, func(criticalPersistenceResult) error {
		if req.Version == wire.PremiumWebVersion {
			completePremiumPurchase(p, req, wire.PremiumResultFailed)
		} else if !p.Session.IsClosed() {
			p.Session.Send(wire.MessagePanel(premiumPurchaseError))
		}
		return nil
	})
	return err
}

// admitDonateCoin uses a Wyden coin: the item is consumed and its value is
// credited to the account balance in one transaction.
func (w *World) admitDonateCoin(p *Player, slot byte, rule model.VolatileRule) {
	s := p.Session
	reject := func(message string) {
		resyncUsedItem(s, p, slot)
		if message != "" {
			s.Send(wire.MessagePanel(message))
		}
	}
	if _, ok := w.premiumBackend(); !ok {
		reject("Donate is not available on this server.")
		return
	}
	if !bindCriticalPlayer(p).current(w) || int(slot) >= model.PlayerCarrySlots || p.Char.Inv[slot].Index == 0 {
		reject("")
		return
	}
	if w.criticalPlayerReserved(p) || w.criticalTradeReserved(p) {
		reject("Please wait for your last action to finish.")
		return
	}
	value := rule.Counters[donateCounter]
	if value == 0 || uint64(p.Account.Donate)+uint64(value) > uint64(model.MaxDonateBalance) {
		reject("Your donate balance is full.")
		return
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	used := ch.Inv[slot]
	consumeOne(&ch.Inv[slot])
	proposed.Donate += value
	remaining, balance := ch.Inv[slot], proposed.Donate
	ledger := []store.DonateLedgerEntry{{
		Account: proposed.Name, Delta: int64(value), BalanceAfter: balance,
		Kind: store.DonateKindCoinUse, ItemIndex: used.Index, ItemUID: used.UID, Character: ch.Name,
	}}
	_, err := w.submitCriticalWriteWithRejection(criticalWrite{
		accounts: []*model.Account{proposed}, donate: &criticalDonateWrite{ledger: ledger},
	}, []*Player{p}, false, func(criticalPersistenceResult) error {
		p.Char.Inv[slot], p.Account.Donate = remaining, balance
		if !s.IsClosed() {
			s.Send(wire.SendItem(p.ID, placeInv, slot, remaining))
			s.Send(wire.PremiumBalance(p.ID, balance))
			s.Send(wire.MessagePanel(fmt.Sprintf("You received %d donate. Balance: %d.", value, balance)))
		}
		log.Printf("[#%d] donate coin item=%d +%d balance=%d", s.ID, used.Index, value, balance)
		return nil
	}, nil, func(criticalPersistenceResult) error {
		if !s.IsClosed() {
			resyncUsedItem(s, p, slot)
			s.Send(wire.MessagePanel("Item use is unavailable. Please try again later."))
		}
		return nil
	})
	if err != nil {
		log.Printf("[#%d] donate coin not submitted: %v", s.ID, err)
		reject("Item use is unavailable. Please try again later.")
	}
}

// tickPremium starts a background poll of the catalog and of pending credits
// for accounts in the world. Database I/O never runs on the World goroutine.
func (w *World) tickPremium(now time.Time) {
	backend, ok := w.premiumBackend()
	if !ok || now.Before(w.premiumNextPoll) || w.premiumPolling.Load() {
		return
	}
	w.premiumNextPoll = now.Add(premiumPollInterval)
	var accounts []string
	for _, p := range w.players {
		if p.InWorld && p.Account != nil {
			accounts = append(accounts, p.Account.Name)
		}
	}
	w.premiumPolling.Store(true)
	go func() {
		event := &premiumEvent{}
		ctx, cancel := context.WithTimeout(context.Background(), premiumPollTimeout)
		defer cancel()
		catalog, err := backend.LoadPremiumCatalog(ctx)
		if err == nil {
			event.catalog = &catalog
			if len(accounts) > 0 {
				event.credits, err = backend.PendingDonateCredits(ctx, accounts, premiumCreditBatch)
			}
		}
		if err != nil {
			event.pollFailed = true
			log.Printf("premium poll failed: %v", err)
		}
		select {
		case w.commands <- command{premium: event, queuedAt: time.Now()}:
		default:
			w.premiumPolling.Store(false)
		}
	}()
}

func (w *World) handlePremiumEvent(event *premiumEvent) {
	if event.catalog != nil || event.credits != nil || event.pollFailed {
		w.premiumPolling.Store(false)
	}
	if event.catalog != nil && event.catalog.Version != w.premiumCatalog.Version {
		w.installPremiumCatalog(*event.catalog)
	}
	if len(event.credits) > 0 {
		w.applyDonateCredits(event.credits)
	}
}

// installPremiumCatalog keeps only entries whose item exists on this server.
func (w *World) installPremiumCatalog(catalog store.PremiumCatalog) {
	entries := make([]store.PremiumCatalogEntry, 0, len(catalog.Entries))
	for _, entry := range catalog.Entries {
		if _, exists := w.items[entry.Item.Index]; !exists {
			log.Printf("premium catalog v%d: slot %d item %d is not in the item list; skipped",
				catalog.Version, entry.Slot, entry.Item.Index)
			continue
		}
		entries = append(entries, entry)
	}
	catalog.Entries = entries
	w.premiumCatalog = catalog
	log.Printf("premium catalog v%d installed: %d item(s)", catalog.Version, len(entries))
}

// applyDonateCredits credits pending inbox rows to accounts in the world. A
// credit already in flight is skipped; the store applies each credit once.
func (w *World) applyDonateCredits(credits []store.DonateCredit) {
	if w.premiumInFlight == nil {
		w.premiumInFlight = make(map[int64]bool)
	}
	byAccount := make(map[string][]store.DonateCredit)
	for _, credit := range credits {
		if !w.premiumInFlight[credit.ID] {
			key := accountSessionKey(credit.Account)
			byAccount[key] = append(byAccount[key], credit)
		}
	}
	for key, pending := range byAccount {
		session := w.accountSessions[key]
		p := w.players[session]
		if p == nil || !p.InWorld || p.Account == nil || p.Char == nil || p.Session == nil || p.Session.IsClosed() ||
			w.criticalPlayerReserved(p) || !bindCriticalPlayer(p).current(w) {
			continue
		}
		proposed := accountStateSnapshot(p.Account)
		var ids []int64
		var ledger []store.DonateLedgerEntry
		var total uint32
		for _, credit := range pending {
			if uint64(proposed.Donate)+uint64(credit.Amount) > uint64(model.MaxDonateBalance) {
				log.Printf("donate credit %d for %q exceeds the balance cap; left pending", credit.ID, key)
				continue
			}
			proposed.Donate += credit.Amount
			total += credit.Amount
			ids = append(ids, credit.ID)
			ledger = append(ledger, store.DonateLedgerEntry{
				Account: proposed.Name, Delta: int64(credit.Amount), BalanceAfter: proposed.Donate,
				Kind: store.DonateKindCredit, CreditID: credit.ID, Character: p.Char.Name,
			})
		}
		if len(ids) == 0 {
			continue
		}
		balance := proposed.Donate
		release := func() {
			for _, id := range ids {
				delete(w.premiumInFlight, id)
			}
		}
		for _, id := range ids {
			w.premiumInFlight[id] = true
		}
		_, err := w.submitCriticalWriteWithRejection(criticalWrite{
			accounts: []*model.Account{proposed}, donate: &criticalDonateWrite{credits: ids, ledger: ledger},
		}, []*Player{p}, false, func(criticalPersistenceResult) error {
			release()
			p.Account.Donate = balance
			if !p.Session.IsClosed() {
				p.Session.Send(wire.PremiumBalance(p.ID, balance))
				p.Session.Send(wire.MessagePanel(fmt.Sprintf("You received %d donate. Balance: %d.", total, balance)))
			}
			log.Printf("[#%d] donate credits %v applied to %q: +%d balance=%d", p.Session.ID, ids, key, total, balance)
			return nil
		}, nil, func(criticalPersistenceResult) error {
			release()
			return nil
		})
		if err != nil {
			release()
			log.Printf("donate credits for %q not submitted: %v", key, err)
		}
	}
}
