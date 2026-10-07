package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"slices"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

type advancedCraftCase struct {
	name, artisan                                 string
	items                                         []uint16
	output                                        uint16
	preferred, soul, level, chance, roll, element int
}

type advancedCraftRNG struct{ roll, element, calls int }

func (r *advancedCraftRNG) Intn(n int) int {
	r.calls++
	if r.calls == 1 {
		return (r.roll - 1) % n
	}
	return r.element % n
}

func advancedCraftCases() []advancedCraftCase {
	cases := []advancedCraftCase{{name: "mystery", artisan: "Ehre", items: []uint16{5110, 5133, 413}, output: mysteriousStoneItem, preferred: 2}}
	for i, recipe := range soulRecipes {
		cases = append(cases, advancedCraftCase{name: fmt.Sprintf("soul-%d", i+1), artisan: "Ehre", items: recipe[:], soul: i + 1})
	}
	for _, lc := range [][2]int{{39, 30}, {149, 30}, {150, 35}, {159, 35}, {160, 40}, {169, 40}, {170, 50}, {179, 50}, {180, 70}, {189, 70}, {190, 100}} {
		for _, roll := range []int{lc[1], lc[1] + 1} {
			if roll > 100 {
				continue
			}
			cases = append(cases, advancedCraftCase{name: fmt.Sprintf("purified-%d-%d", lc[0], roll), artisan: "Ehre",
				items: []uint16{697, 697, 3338}, preferred: 2, level: lc[0], chance: lc[1], roll: roll})
		}
	}
	for i, recipe := range secretStoneRecipes {
		for _, roll := range []int{95, 96} {
			cases = append(cases, advancedCraftCase{name: fmt.Sprintf("stone-%d-%d", i, roll), artisan: "Odin",
				items: recipe[:7], output: recipe[7], chance: 95, roll: roll})
		}
	}
	for element := range 4 {
		cases = append(cases, advancedCraftCase{name: fmt.Sprintf("random-%d", element), artisan: "Odin",
			items: []uint16{5110, 5110, 5110, 5110, 5110, 5110, 5110}, output: uint16(5334 + element), chance: 5, roll: 5, element: element})
	}
	cases = append(cases,
		advancedCraftCase{name: "random-failure", artisan: "Odin", items: []uint16{5110, 5110, 5110, 5110, 5110, 5110, 5110}, chance: 5, roll: 6},
		advancedCraftCase{name: "fury", artisan: "Odin", items: []uint16{5125, 5115, 5111, 5112, 5120, 5128, 5119}, output: 3020},
		advancedCraftCase{name: "clue", artisan: "Odin", items: []uint16{413, 413, 413, 413, 413, 413, 413}, output: 5134},
		advancedCraftCase{name: "crystal", artisan: "Odin", items: []uint16{421, 422, 423, 424, 425, 426, 427}, output: 4032},
		advancedCraftCase{name: "cape", artisan: "Odin", items: []uint16{4127, 4127, 5135, 413, 413, 413, 413}},
		advancedCraftCase{name: "unlock", artisan: "Odin", items: []uint16{4127, 4127, 5135, 5113, 5129, 5112, 5110}},
	)
	for _, lc := range [][2]int{{11, 5}, {12, 3}, {13, 2}, {14, 1}} {
		for _, catalysts := range []bool{false, true} {
			chance := lc[1]
			items := []uint16{4043, 4043, 500}
			if catalysts {
				items = append(items, 3338, 0, 3338, 0)
				chance += 10
			}
			for _, roll := range []int{chance, chance + 1} {
				cases = append(cases, advancedCraftCase{name: fmt.Sprintf("refine-%d-%t-%d", lc[0], catalysts, roll), artisan: "Odin",
					items: items, preferred: 2, level: lc[0], chance: chance, roll: roll})
			}
		}
	}
	return cases
}

func configureCriticalAdvanced(w *World, p *Player, tc advancedCraftCase) (combineRequest, *advancedCraftRNG) {
	configureCriticalTiny(w, p, true, true)
	w.mobsByID[1500].Def.Name = tc.artisan
	req := combineRequest{Pos: [combineSlots]int8{62, 0, 1, 2, 3, 4, 5, 6}}
	for _, pos := range req.Pos {
		p.Char.Inv[int(pos)] = model.Item{}
	}
	for i, index := range tc.items {
		if index == 0 {
			continue
		}
		item := model.Item{Index: index, UID: fmt.Sprintf("advanced-ingredient-%d", i), ActivatedUnix: int64(100 + i), ExpiresUnix: int64(4102444800 + i)}
		if tc.name == "mystery" && i == 2 {
			setItemAmount(&item, 10)
		}
		if tc.roll != 0 && tc.artisan == "Ehre" && i == 2 {
			item.Eff = [6]byte{43, 8}
		}
		if tc.artisan == "Odin" && tc.preferred == 2 {
			if i == 2 {
				item.Eff = [6]byte{43, byte(230 + 4*(tc.level-10))}
			}
			if i == 3 || i == 5 {
				item.Eff = [6]byte{43, 3}
			}
		}
		req.Items[i], p.Char.Inv[int(req.Pos[i])] = item, item
	}
	p.Char.Gold, p.Char.Exp, p.Char.SoulInfo = 2_000_321, 10_000_000, 9
	p.Char.Evolution, p.Char.CelestialLevel40Unlocked = "celestial", false
	level := uint32(39)
	if tc.artisan == "Ehre" && tc.roll != 0 {
		level = uint32(tc.level)
	}
	p.Char.Score = testScore(model.Score{Level: level, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100})
	runtime := *p.Char.Score
	p.Char.RuntimeScore = &runtime
	p.Char.Equip[15] = model.Item{Index: 3197, UID: "11111111111141118111111111111111", Eff: [6]byte{43, 8}}
	p.SpecialCoins = map[string]uint32{fameCounter: 200, "unrelated": 9}
	if tc.output != 0 {
		w.items[tc.output] = model.ItemDef{Index: tc.output}
	}
	rng := &advancedCraftRNG{roll: tc.roll, element: tc.element}
	w.rng = rng
	return req, rng
}

func criticalAdvancedFixture(t *testing.T, tc advancedCraftCase) (*World, *Player, *craftStore, *criticalTestStore, *criticalPersistence, combineRequest, *advancedCraftRNG, func()) {
	t.Helper()
	w, p, fallback, backend, queue, _, _, release := criticalTinyFixture(t, true, true)
	req, rng := configureCriticalAdvanced(w, p, tc)
	queue.store = &criticalPlayerBatchTestStore{&criticalStateTestStore{&criticalBatchTestStore{backend}}}
	return w, p, fallback, backend, queue, req, rng, release
}

func criticalAdvancedRequest(p *Player, req combineRequest, artisan string) []byte {
	packet := criticalTinyRequest(p.ID, req)
	op := wire.OpCombineEhre
	if artisan == "Odin" {
		op = wire.OpCombineOdin
	}
	binary.LittleEndian.PutUint16(packet[4:6], uint16(op))
	return packet
}

// Only a newly generated UID is taken from the proposal. Recipe effects,
// consumption, currency, flags and probability expectations are independent.
func criticalAdvancedExpected(t *testing.T, w *World, p *Player, req combineRequest, tc advancedCraftCase, proposal *model.Account) (*model.Account, map[string]uint32, []int, uint32, string) {
	t.Helper()
	want, coins := accountStateSnapshot(p.Account), copyCounters(p)
	ch := &want.Chars[p.CharSlot]
	var positions []int
	for i, item := range req.Items {
		if item.Index != 0 {
			pos := int(req.Pos[i])
			ch.Inv[pos] = model.Item{}
			positions = append(positions, pos)
		}
	}
	success := tc.roll == 0 || tc.roll <= tc.chance
	code, message := uint32(1), "Processing complete."
	if tc.roll != 0 {
		message = percentRoll{Roll: tc.roll, Chance: tc.chance, Success: success}.message()
		if !success {
			code = 2
		}
	}
	if tc.output != 0 && success {
		uid := proposal.Chars[p.CharSlot].Inv[int(req.Pos[tc.preferred])].UID
		if normalized, err := model.NormalizeItemUID(uid); err != nil || normalized != uid || uid == "" {
			t.Fatal("new craft result has no canonical identity")
		}
		for _, item := range req.Items {
			if uid == item.UID {
				t.Fatal("new result reused an ingredient identity")
			}
		}
		item := model.Item{Index: tc.output, UID: uid}
		if tc.name == "mystery" {
			item.Eff = [6]byte{effectAmount, 10}
		}
		ch.Inv[int(req.Pos[tc.preferred])] = item
	}
	if tc.soul != 0 {
		ch.SoulInfo = uint8(tc.soul)
	}
	if tc.artisan == "Ehre" && tc.roll != 0 {
		ch.Exp -= 5_000_000
		if success {
			item := req.Items[2]
			item.Eff[1] = 9
			ch.Inv[int(req.Pos[2])] = item
		}
	}
	if tc.artisan == "Odin" && tc.roll != 0 {
		if tc.preferred == 2 {
			item := req.Items[2]
			item.Eff[1] -= 4
			if success {
				item.Eff[1] += 8
			}
			ch.Inv[int(req.Pos[2])] = item
		} else {
			ch.Gold -= odinSecretStoneGoldCost
		}
	}
	if tc.name == "cape" {
		ch.Equip[15].Eff[1] = 9
		w.recalcPlayer(ch)
	}
	if tc.name == "unlock" {
		ch.CelestialLevel40Unlocked = true
		delete(coins, fameCounter)
	}
	slices.Sort(positions)
	return want, coins, positions, code, message
}

func assertCriticalAdvancedPublication(t *testing.T, p *Player, tc advancedCraftCase, positions []int, code uint32, message string) {
	t.Helper()
	for _, pos := range positions {
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
	}
	if tc.name == "cape" {
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeEquip, 15, p.Char.Equip[15]))
	}
	assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
	if tc.name == "cape" {
		assertCriticalPickupPacket(t, p, observedPlayerScorePacket(p, p))
	}
	assertCriticalPickupPacket(t, p, wire.MessagePanel(message))
	assertCriticalPickupPacket(t, p, wire.StandardParm(wire.OpCombineComplete, wire.SceneField, code))
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("advanced composition published extra packets")
	}
}

func TestCriticalAdvancedCraftDetachedRecipesReplayAndTrade(t *testing.T) {
	for _, tc := range advancedCraftCases() {
		for _, trading := range []bool{false, true} {
			for slot := range 4 {
				t.Run(fmt.Sprintf("%s/trade-%t/slot-%d", tc.name, trading, slot), func(t *testing.T) {
					w, p, fallback, backend, queue, req, rng, release := criticalAdvancedFixture(t, tc)
					if slot != 0 {
						p.Account.Chars[0], p.Account.Chars[slot] = p.Account.Chars[slot], p.Account.Chars[0]
						p.CharSlot, p.Char = slot, &p.Account.Chars[slot]
					}
					var peer *Player
					if trading {
						peer = criticalTinyTradePeer(t, w, p)
					}
					before, coins, account, character, trade := accountStateSnapshot(p.Account), copyCounters(p), p.Account, p.Char, p.Trade
					packet := criticalAdvancedRequest(p, req, tc.artisan)
					w.handle(command{s: p.Session, pkt: packet})
					proposal := awaitCriticalWrite(t, backend.entered)
					want, wantCoins, positions, code, message := criticalAdvancedExpected(t, w, p, req, tc, proposal.accounts[0])
					if proposal.accounts[0] == account || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
						t.Fatal("advanced recipe proposed incorrect or aliased account state")
					}
					if tc.name == "unlock" {
						if proposal.character == nil || proposal.character.uid != p.Char.UID || !reflect.DeepEqual(proposal.character.state, buildCharState(&Player{Char: &want.Chars[slot], SpecialCoins: wantCoins}, w.now())) {
							t.Fatal("unlock omitted atomic Fame state")
						}
					} else if proposal.character != nil {
						t.Fatal("inventory recipe unexpectedly changed sidecar")
					}
					if trading && (len(proposal.accounts) != 2 || !reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peer.Account))) {
						t.Fatal("reciprocal account missing from transaction")
					}
					craft, calls := p.LastCraft, rng.calls
					for _, completed := range []bool{false, true} {
						if completed {
							release()
							awaitReadyCriticalResults(t, queue, 1)
						}
						w.handle(command{s: p.Session, pkt: packet})
						if tc.artisan == "Ehre" {
							w.onCombineEhre(p.Session, packet)
						} else {
							w.onCombineOdin(p.Session, packet)
						}
						if !reflect.DeepEqual(accountStateSnapshot(account), before) || !reflect.DeepEqual(p.SpecialCoins, coins) || p.LastCraft != craft || rng.calls != calls || p.Trade != trade || !w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 || trading && (!w.criticalPlayerReserved(peer) || peer.Session.QueuedPacketsForTest() != 0) {
							t.Fatal("held/unacknowledged composition changed state or repeated")
						}
					}
					discardCriticalDeferredDuplicates(w)

					w.processCriticalCompletions(1)
					if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(account), want) || !reflect.DeepEqual(p.SpecialCoins, wantCoins) || p.Trade != trade || len(queue.pending) != 0 || fallback.saves != 0 || w.shuttingDown {
						t.Fatal("advanced composition failed to settle exact state and retain trade")
					}
					assertCriticalAdvancedPublication(t, p, tc, positions, code, message)
					expectedCalls := 0
					if tc.roll != 0 {
						expectedCalls = 1
						if tc.chance == 5 && tc.roll <= 5 && tc.preferred == 0 {
							expectedCalls = 2
						}
					}
					if rng.calls != expectedCalls {
						t.Fatal("craft rerolled or randomized a deterministic/failed recipe")
					}
				})
			}
		}
	}
}

func TestCriticalAdvancedCraftUnknownAndStaleInputs(t *testing.T) {
	for _, name := range []string{"soul-1", "purified-39-30", "cape", "unlock", "refine-11-false-5"} {
		var tc advancedCraftCase
		for _, candidate := range advancedCraftCases() {
			if candidate.name == name {
				tc = candidate
			}
		}
		for _, mode := range []string{"unknown", "panic", "exp", "soul", "level", "evolution", "cape", "coins", "trade", "inventory"} {
			t.Run(name+"/"+mode, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalAdvancedFixture(t, tc)
				peer := criticalTinyTradePeer(t, w, p)
				if mode == "unknown" {
					backend.failure = errors.New("advanced acknowledgement lost")
				}
				backend.panicAfterWrite = mode == "panic"
				w.handle(command{s: p.Session, pkt: criticalAdvancedRequest(p, req, tc.artisan)})
				awaitCriticalWrite(t, backend.entered)
				switch mode {
				case "exp":
					p.Char.Exp++
				case "soul":
					p.Char.SoulInfo++
				case "level":
					p.Char.RuntimeScore.Level++
				case "evolution":
					p.Char.Evolution = "arch"
				case "cape":
					p.Char.Equip[15].Eff[1]++
				case "coins":
					p.SpecialCoins[fameCounter]++
				case "trade":
					peer.Trade.Gold++
				case "inventory":
					p.Char.Inv[0].UID = "conflicting-identity"
				}
				retained, coins := accountStateSnapshot(p.Account), copyCounters(p)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), retained) || !reflect.DeepEqual(p.SpecialCoins, coins) || len(queue.pending) != 1 || !p.PersistencePoisoned || !peer.PersistencePoisoned || !w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || !w.shuttingDown || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("unknown/stale advanced outcome applied or released state")
				}
			})
		}
	}
}

func TestCriticalAdvancedCraftAdmissionRejectsWithoutFallback(t *testing.T) {
	for _, name := range []string{"mystery", "purified-39-30", "stone-0-95", "cape", "unlock", "refine-11-false-5"} {
		var tc advancedCraftCase
		for _, candidate := range advancedCraftCases() {
			if candidate.name == name {
				tc = candidate
			}
		}
		for _, mode := range []string{"identity", "ingredient", "duplicate", "hidden", "blocked", "missing-batch", "stopping", "peer", "invalid-recipe"} {
			t.Run(name+"/"+mode, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, _ := criticalAdvancedFixture(t, tc)
				criticalTinyTradePeer(t, w, p)
				switch mode {
				case "identity":
					delete(w.accountSessions, accountSessionKey(p.Account.Name))
				case "ingredient":
					req.Items[0].UID = "forged"
				case "duplicate":
					req.Pos[1] = req.Pos[0]
				case "hidden":
					req.Pos[0] = model.PlayerCarrySlots
				case "blocked":
					req.Items[0].Index = blockedCombineItem
					p.Char.Inv[int(req.Pos[0])] = req.Items[0]
				case "missing-batch":
					queue.store = &criticalStateTestStore{&criticalBatchTestStore{backend}}
					if tc.name != "unlock" {
						queue.store = backend
					}
				case "stopping":
					queue.closeAdmission()
				case "peer":
					p.Trade.OpponentID = 9999
				case "invalid-recipe":
					req.Items[0].Index = 600
					p.Char.Inv[int(req.Pos[0])] = req.Items[0]
				}
				before, coins := accountStateSnapshot(p.Account), copyCounters(p)
				if err := w.queueAdvancedComposition(p, req, tc.artisan); err == nil {
					t.Fatal("invalid composition admitted")
				}
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(p.SpecialCoins, coins) || fallback.saves != 0 || len(queue.pending) != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("rejected composition mutated or saved inline")
				}
			})
		}
	}
}

func TestCriticalAdvancedCraftDisconnectPersistsCommittedState(t *testing.T) {
	for _, tc := range advancedCraftCases() {
		t.Run(tc.name, func(t *testing.T) {
			w, p, fallback, backend, queue, req, _, release := criticalAdvancedFixture(t, tc)
			w.handle(command{s: p.Session, pkt: criticalAdvancedRequest(p, req, tc.artisan)})
			proposal := awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session})
			if w.players[p.Session] != p || len(backend.entered) != 0 || fallback.saves != 0 {
				t.Fatal("disconnect discarded accepted crafting ownership")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			final := awaitCriticalWrite(t, backend.entered)
			if !reflect.DeepEqual(final.accounts[0], proposal.accounts[0]) || final.character == nil || !reflect.DeepEqual(final.character.state, buildCharState(p, w.now())) {
				t.Fatal("final disconnect lost committed crafting state")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.players[p.Session] != nil || len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("disconnect did not settle without early publication")
			}
		})
	}
}

func TestCriticalAdvancedCraftActualWorldRemainsResponsive(t *testing.T) {
	for _, name := range []string{"mystery", "purified-39-31", "stone-0-95", "cape", "unlock", "refine-11-false-6"} {
		for _, unknown := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/unknown-%t", name, unknown), func(t *testing.T) {
				var tc advancedCraftCase
				for _, candidate := range advancedCraftCases() {
					if candidate.name == name {
						tc = candidate
					}
				}
				var failure error
				if unknown {
					failure = errors.New("advanced acknowledgement lost")
				}
				var req combineRequest
				var rng *advancedCraftRNG
				w, p, uid, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure, func(w *World, p *Player) {
					req, rng = configureCriticalAdvanced(w, p, tc)
					w.criticalWrites.queue.store = &criticalPlayerBatchTestStore{&criticalStateTestStore{&criticalBatchTestStore{w.store.(*criticalTestStore)}}}
				})
				packet := criticalAdvancedRequest(p, req, tc.artisan)
				deliver := func() {
					select {
					case w.commands <- command{s: p.Session, pkt: packet}:
					case <-time.After(time.Second):
						t.Fatal("World did not accept advanced intention")
					}
				}
				deliver()
				awaitCriticalWrite(t, backend.entered)
				for range 3 {
					deliver()
					deliverCriticalLoopTick(t, ticks)
					overview := readCriticalLoopOverview(t, w)
					regen := false
					for _, player := range overview.Players {
						regen = regen || player.UID == uid && player.HP > 500
					}
					if overview.State != "running" || !regen || p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("advanced write blocked World or published early")
					}
				}
				release()
				deadline := time.Now().Add(2 * time.Second)
				for {
					overview := readCriticalLoopOverview(t, w)
					if unknown && overview.State == "maintenance" || !unknown && p.Session.QueuedPacketsForTest() >= 3 {
						break
					}
					if time.Now().After(deadline) {
						t.Fatal("World did not deliver advanced completion")
					}
				}
				stop()
				calls := 0
				if tc.roll != 0 {
					calls = 1
				}
				if w.mobTickCounter < 3 || rng.calls != calls || len(backend.entered) != 0 || unknown && len(queue.pending) != 1 || !unknown && len(queue.pending) != 0 {
					t.Fatal("advanced dispatcher blocked, rerolled or failed to settle")
				}
			})
		}
	}
}

func TestCriticalAdvancedCraftTradeDisconnect(t *testing.T) {
	for _, tc := range advancedCraftCases() {
		for _, closing := range []string{"owner", "peer", "both"} {
			t.Run(tc.name+"/"+closing, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalAdvancedFixture(t, tc)
				peer := criticalTinyTradePeer(t, w, p)
				w.handle(command{s: p.Session, pkt: criticalAdvancedRequest(p, req, tc.artisan)})
				proposal := awaitCriticalWrite(t, backend.entered)
				_, wantCoins, positions, code, message := criticalAdvancedExpected(t, w, p, req, tc, proposal.accounts[0])
				closed := []*Player{p}
				if closing == "peer" {
					closed = []*Player{peer}
				} else if closing == "both" {
					closed = append(closed, peer)
				}
				for _, player := range closed {
					w.handle(command{s: player.Session})
				}
				if len(backend.entered) != 0 || fallback.saves != 0 || p.Trade == nil || peer.Trade == nil {
					t.Fatal("disconnect persisted or cancelled trade before composition committed")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if p.Trade == nil || peer.Trade == nil || !reflect.DeepEqual(p.SpecialCoins, wantCoins) {
					t.Fatal("composition lost currency or cancelled trade before final persistence")
				}
				if closing == "peer" {
					assertCriticalAdvancedPublication(t, p, tc, positions, code, message)
				}
				seen := make(map[string]bool)
				for range closed {
					final := awaitCriticalWrite(t, backend.entered)
					var participant *Player
					var expected *model.Account
					for i, player := range []*Player{p, peer} {
						if final.accounts[0].Name == player.Account.Name {
							participant, expected = player, proposal.accounts[i]
						}
					}
					if participant == nil || seen[participant.Account.Name] || !reflect.DeepEqual(final.accounts[0], expected) ||
						final.character == nil || final.character.uid != participant.Char.UID ||
						!reflect.DeepEqual(final.character.state, buildCharState(participant, w.now())) {
						t.Fatal("final save lost or repeated committed account/sidecar state")
					}
					seen[participant.Account.Name] = true
				}
				awaitReadyCriticalResults(t, queue, len(closed))
				for _, player := range closed {
					if !w.criticalPlayerReserved(player) || w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
						t.Fatal("final write released account ownership before acknowledgement")
					}
				}
				w.processCriticalCompletions(1)
				if closing == "both" {
					if w.players[p.Session] != p || w.players[peer.Session] != peer || p.Trade == nil || peer.Trade == nil {
						t.Fatal("first final acknowledgement destroyed peer identity")
					}
					w.processCriticalCompletions(1)
				}
				for _, player := range closed {
					if w.players[player.Session] != nil || !w.claimAccountSession(gameNet.NewTestSession(98, 8), player.Account.Name) {
						t.Fatal("final acknowledgement failed to release disconnected ownership")
					}
				}
				if p.Trade != nil || peer.Trade != nil || len(queue.pending) != 0 || w.shuttingDown || fallback.saves != 0 {
					t.Fatal("trade disconnect did not settle without inline persistence")
				}
				for _, player := range []*Player{p, peer} {
					if !player.Session.IsClosed() {
						assertCriticalPickupPacket(t, player, wire.CloseTrade(player.ID))
					}
					if player.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("disconnect sent extra responses or replied to a closed session")
					}
				}
			})
		}
	}
}

func TestCriticalAdvancedCraftShutdown(t *testing.T) {
	for _, tc := range advancedCraftCases() {
		t.Run(tc.name, func(t *testing.T) {
			w, p, fallback, backend, queue, req, _, release := criticalAdvancedFixture(t, tc)
			terminal := &criticalLindyTerminalStore{craftStore: fallback}
			w.store = terminal
			w.handle(command{s: p.Session, pkt: criticalAdvancedRequest(p, req, tc.artisan)})
			proposal := awaitCriticalWrite(t, backend.entered)
			waiters := []chan error{make(chan error, 1), make(chan error, 1)}
			for _, done := range waiters {
				w.runShutdown(done)
				select {
				case <-done:
					t.Fatal("shutdown finished before composition acknowledgement")
				default:
				}
			}
			w.handle(command{s: p.Session})
			if len(terminal.final) != 0 || terminal.flushes != 0 || fallback.saves != 0 || !queue.stopping || w.shutdownFinished {
				t.Fatal("shutdown saved uncommitted composition state")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			for _, done := range waiters {
				select {
				case err := <-done:
					if err != nil {
						t.Fatal(err)
					}
				default:
					t.Fatal("shutdown did not finish after composition acknowledgement")
				}
			}
			if len(terminal.final) != 1 || !reflect.DeepEqual(terminal.final[0].accounts, proposal.accounts) ||
				terminal.final[0].character.uid != p.Char.UID || !reflect.DeepEqual(terminal.final[0].character.state, buildCharState(p, w.now())) ||
				terminal.flushes != 1 || fallback.saves != 0 || len(queue.pending) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("shutdown lost committed state, repeated a save or published to a closed session")
			}
			third := make(chan error, 1)
			w.runShutdown(third)
			if err := <-third; err != nil || len(terminal.final) != 1 || terminal.flushes != 1 {
				t.Fatal("repeated shutdown replayed composition persistence")
			}
		})
	}
}
