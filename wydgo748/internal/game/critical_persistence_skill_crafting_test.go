package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func configureCriticalSkillCraft(w *World, p *Player, artisan string, roll int) (combineRequest, *criticalTinyRNG) {
	configureCriticalInventory(p, 7, 0)
	p.Char.Inv[7] = model.Item{}
	p.Char.Class = 3
	p.Char.LearnedSkill = 1<<(83-72) | 1<<(84-72)
	p.Char.SecondaryLearnedSkill = 0x10
	p.Char.Score = testScore(model.Score{MaxHP: 100, CurHP: 100, Mastery: [4]uint32{0, 0, 20, 0}})
	// Live players already have effective scores. Exercise the runtime mastery,
	// not a replacement base score hidden behind the old runtime fixture.
	runtimeScore := *p.Char.Score
	runtimeScore.Mastery[2] = 220
	p.Char.RuntimeScore = &runtimeScore
	p.Char.Gold = 12345
	req := combineRequest{Pos: [combineSlots]int8{62, 0, 1, 2, 3, 4, 5, 6}}
	if artisan == "Extracao" {
		w.items = map[uint16]model.ItemDef{500: {Index: 500, Pos: 4,
			StaticEffects:      []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 2}, {Name: "EF_DAMAGE", Value: 10}},
			DynamicEffectNames: map[byte]string{2: "EF_DAMAGE"}}}
		req.Items[0] = model.Item{Index: 500, Eff: [6]byte{0, 0, 2, 5, 2, 6}}
		req.Items[1] = model.Item{Index: extracaoCatalyst, Eff: [6]byte{effectAmount, 5}}
	} else {
		for i, index := range []uint16{413, 2441, 2442, 900} {
			req.Items[i] = model.Item{Index: index}
		}
	}
	for i := range req.Items {
		if req.Items[i].Index == 0 {
			continue
		}
		req.Items[i].UID = fmt.Sprintf("skill-ingredient-%d", i)
		req.Items[i].ActivatedUnix, req.Items[i].ExpiresUnix = int64(123+i), int64(4102444800+i)
		p.Char.Inv[int(req.Pos[i])] = req.Items[i]
	}
	rng := &criticalTinyRNG{first: roll - 1}
	w.rng = rng
	return req, rng
}

func criticalSkillCraftRequest(p *Player, req combineRequest, artisan string) []byte {
	if artisan == "Extracao" {
		packet := inboundPacket(wire.OpCombineExtracao, 20)
		binary.LittleEndian.PutUint16(packet[6:8], p.ID)
		binary.LittleEndian.PutUint32(packet[16:20], uint32(req.Pos[0]))
		return packet
	}
	packet := criticalTinyRequest(p.ID, req)
	binary.LittleEndian.PutUint16(packet[4:6], wire.OpCombineAlquimia)
	return packet
}

func criticalSkillCraftFixture(t *testing.T, artisan string, roll int) (*World, *Player, *craftStore, *criticalTestStore, *criticalPersistence, combineRequest, *criticalTinyRNG, func()) {
	t.Helper()
	w, p, fallback := handlerTestWorld(t)
	w.clock = newFakeClock(time.Unix(100, 0))
	req, rng := configureCriticalSkillCraft(w, p, artisan, roll)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("skill fixture account admission failed")
	}
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 8), release: gate}
	queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 2)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	return w, p, fallback, backend, queue, req, rng, release
}

// Expected state is independent of recipe and queued-plan implementations.
func criticalSkillCraftExpected(p *Player, req combineRequest, artisan string, roll int) (*model.Account, []int, percentRoll, uint32) {
	want := accountStateSnapshot(p.Account)
	ch := &want.Chars[p.CharSlot]
	result := percentRoll{Roll: roll, Chance: 40, Success: roll <= 40}
	positions := []int{0, 62}
	if artisan == "Extracao" {
		result.Success = roll < 40
		ch.Inv[0], ch.Inv[62] = model.Item{}, model.Item{}
		if result.Success {
			item := req.Items[0]
			item.Index, item.Eff = 3022, [6]byte{effectItemLevel, 2, 2, 15, 2, 16}
			ch.Inv[62] = item
		}
	} else {
		positions = []int{0, 1, 2, 62}
		for _, pos := range positions {
			ch.Inv[pos] = model.Item{}
		}
		if result.Success {
			ch.Inv[62] = model.Item{Index: 3200, UID: req.Items[0].UID, Eff: [6]byte{effectAmount, 5}}
		}
	}
	code := uint32(2)
	if result.Success {
		code = 1
	}
	return want, positions, result, code
}

func assertCriticalSkillCraftPublication(t *testing.T, p *Player, artisan string, positions []int, roll percentRoll, code uint32, trade bool) {
	t.Helper()
	if artisan != "Extracao" {
		assertCriticalTinyPublication(t, p, positions, roll, code, trade)
		return
	}
	if trade {
		assertCriticalPickupPacket(t, p, wire.CloseTrade(p.ID))
	}
	for _, pos := range positions {
		assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, byte(pos), p.Char.Inv[pos]))
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("extraction published a text, result, or other extra response")
	}
}

func TestCriticalSkillCraftDetachedBoundaryReplayAndTrade(t *testing.T) {
	for _, artisan := range []string{"Extracao", "Alquimia"} {
		for _, roll := range []int{39, 40, 41} {
			for _, trade := range []bool{false, true} {
				for slot := range 4 {
					t.Run(fmt.Sprintf("%s/roll-%d/trade-%t/slot-%d", artisan, roll, trade, slot), func(t *testing.T) {
						w, p, fallback, backend, queue, req, rng, release := criticalSkillCraftFixture(t, artisan, roll)
						if slot != 0 {
							p.Account.Chars[0], p.Account.Chars[slot] = p.Account.Chars[slot], p.Account.Chars[0]
							p.CharSlot, p.Char = slot, &p.Account.Chars[slot]
						}
						var peer *Player
						if trade {
							peer = criticalTinyTradePeer(t, w, p)
						}
						account, character, before := p.Account, p.Char, accountStateSnapshot(p.Account)
						want, positions, result, code := criticalSkillCraftExpected(p, req, artisan, roll)
						request := criticalSkillCraftRequest(p, req, artisan)
						w.handle(command{s: p.Session, pkt: request})
						proposal := awaitCriticalWrite(t, backend.entered)
						count := 1
						if trade {
							count = 2
							if !reflect.DeepEqual(proposal.accounts[1], accountPersistenceSnapshot(peer.Account)) {
								t.Fatal("trade peer was omitted or changed in the atomic batch")
							}
						}
						if len(proposal.accounts) != count || proposal.accounts[0] == account ||
							!reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
							t.Fatal("skill proposal aliases live state or changes the recipe contract")
						}
						cooldown := p.LastCraft
						for _, phase := range []string{"held", "completed-unacknowledged"} {
							if phase == "completed-unacknowledged" {
								release()
								awaitReadyCriticalResults(t, queue, 1)
							}
							w.handle(command{s: p.Session, pkt: request})
							if artisan == "Extracao" {
								w.onCombineExtracao(p.Session, request)
							} else {
								w.onCombineAlquimia(p.Session, request)
							}
							if peer != nil {
								w.cancelTrade(peer, "test held skill write")
								w.tickTrades(w.now().Add(time.Hour))
								if p.Trade == nil || peer.Trade == nil || !w.criticalPlayerReserved(peer) {
									t.Fatal("held composition released its trade context")
								}
							}
							if !reflect.DeepEqual(accountStateSnapshot(account), before) || fallback.saves != 0 ||
								p.Session.QueuedPacketsForTest() != 0 || rng.calls != 1 || p.LastCraft != cooldown || len(queue.pending) != 1 {
								t.Fatalf("%s skill write mutated, published, rerolled, or persisted inline", phase)
							}
						}
						discardCriticalDeferredDuplicates(w)

						w.processCriticalCompletions(1)
						if p.Account != account || p.Char != character || !reflect.DeepEqual(accountStateSnapshot(account), want) ||
							len(queue.pending) != 0 || w.shuttingDown || w.criticalPlayerReserved(p) || p.Trade != nil {
							t.Fatal("skill commit failed to settle its state and reservation")
						}
						assertCriticalSkillCraftPublication(t, p, artisan, positions, result, code, trade)
						if peer != nil {
							if peer.Trade != nil || w.criticalPlayerReserved(peer) {
								t.Fatal("skill commit retained the canceled trade peer")
							}
							assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
							if peer.Session.QueuedPacketsForTest() != 0 {
								t.Fatal("trade peer received the skill result")
							}
						}
						p.LastCraft = time.Time{}
						w.handle(command{s: p.Session, pkt: request})
						if rng.calls != 1 || len(queue.pending) != 0 || len(backend.entered) != 0 ||
							!reflect.DeepEqual(accountStateSnapshot(account), want) {
							t.Fatal("stale skill request consumed ingredients twice")
						}
					})
				}
			}
		}
	}
}

func TestCriticalSkillCraftUnknownAndStaleInputsRetainOwnership(t *testing.T) {
	for _, artisan := range []string{"Extracao", "Alquimia"} {
		for _, mode := range []string{"unknown", "panic", "mastery", "secondary-skill", "class", "inventory", "trade"} {
			t.Run(artisan+"/"+mode, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalSkillCraftFixture(t, artisan, 39)
				peer := criticalTinyTradePeer(t, w, p)
				before := accountStateSnapshot(p.Account)
				if mode == "unknown" {
					backend.failure = errors.New("skill commit acknowledgement lost")
				}
				backend.panicAfterWrite = mode == "panic"
				w.handle(command{s: p.Session, pkt: criticalSkillCraftRequest(p, req, artisan)})
				awaitCriticalWrite(t, backend.entered)
				switch mode {
				case "mastery":
					p.Char.RuntimeScore.Mastery[2]++
				case "secondary-skill":
					p.Char.SecondaryLearnedSkill = 0
				case "class":
					p.Char.Class = 0
				case "inventory":
					p.Char.Inv[0].UID = "conflicting-identity"
				case "trade":
					peer.Trade.Gold++
				}
				retained := accountStateSnapshot(p.Account)
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), retained) || len(queue.pending) != 1 ||
					!w.criticalPlayerReserved(p) || !w.criticalPlayerReserved(peer) || !p.PersistencePoisoned || !peer.PersistencePoisoned ||
					!w.shuttingDown || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 0 || peer.Session.QueuedPacketsForTest() != 0 ||
					p.Trade == nil || peer.Trade == nil {
					t.Fatal("unknown/stale skill completion applied or released ownership")
				}
				if (mode == "unknown" || mode == "panic") && !reflect.DeepEqual(retained, before) {
					t.Fatal("unknown commit changed the live skill aggregate")
				}
			})
		}
	}
}

func TestCriticalSkillCraftActualDispatcherKeepsWorldResponsive(t *testing.T) {
	for _, artisan := range []string{"Extracao", "Alquimia"} {
		for _, outcome := range []string{"success", "failure", "unknown"} {
			t.Run(artisan+"/"+outcome, func(t *testing.T) {
				var failure error
				if outcome == "unknown" {
					failure = errors.New("skill acknowledgement lost")
				}
				roll := 39
				if outcome == "failure" {
					roll = 41
				}
				var req combineRequest
				var rng *criticalTinyRNG
				var before, want *model.Account
				var positions []int
				var result percentRoll
				var code uint32
				w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure, func(w *World, p *Player) {
					req, rng = configureCriticalSkillCraft(w, p, artisan, roll)
					before = accountStateSnapshot(p.Account)
					want, positions, result, code = criticalSkillCraftExpected(p, req, artisan, roll)
				})
				request := criticalSkillCraftRequest(p, req, artisan)
				deliver := func() {
					select {
					case w.commands <- command{s: p.Session, pkt: request}:
					case <-time.After(time.Second):
						t.Fatal("actor did not accept the skill intention")
					}
				}
				deliver()
				proposal := awaitCriticalWrite(t, backend.entered)
				if !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
					t.Fatal("actual skill dispatcher submitted an incorrect proposal")
				}
				for range 3 {
					deliverCriticalLoopPing(t, w, p)
					deliverCriticalLoopTick(t, ticks)
					overview := readCriticalLoopOverview(t, w)
					regenerated := false
					for _, player := range overview.Players {
						regenerated = regenerated || player.UID == unrelatedUID && player.HP > 500
					}
					if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("held skill transaction blocked unrelated ticks or published early")
					}
				}
				release()
				deadline := time.Now().Add(2 * time.Second)
				for {
					overview := readCriticalLoopOverview(t, w)
					if outcome == "unknown" && overview.State == "maintenance" ||
						outcome != "unknown" && p.Session.QueuedPacketsForTest() >= len(positions) {
						break
					}
					if time.Now().After(deadline) {
						t.Fatal("actor did not deliver the skill outcome")
					}
				}
				stop()
				if w.mobTickCounter < 3 || rng.calls != 1 || len(backend.entered) != 0 {
					t.Fatal("skill path rerolled, repeated persistence, or stopped ticks")
				}
				if outcome == "unknown" {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 1 || !p.PersistencePoisoned {
						t.Fatal("unknown actor result changed or released the skill aggregate")
					}
				} else {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || len(queue.pending) != 0 || w.shuttingDown {
						t.Fatal("actor failed to settle the skill transaction")
					}
					assertCriticalSkillCraftPublication(t, p, artisan, positions, result, code, false)
				}
			})
		}
	}
}

func TestCriticalSkillCraftAdmissionRejectsWithoutInlineFallback(t *testing.T) {
	for _, artisan := range []string{"Extracao", "Alquimia"} {
		for _, reason := range []string{"unlearned", "dead", "shop", "poisoned", "missing-ingredient", "invalid-position", "unsupported-batch", "closed-admission"} {
			t.Run(artisan+"/"+reason, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, _ := criticalSkillCraftFixture(t, artisan, 39)
				switch reason {
				case "unlearned":
					p.Char.LearnedSkill = 0
				case "dead":
					setPlayerCurHP(p.Char, 0)
				case "shop":
					p.GhostShop = &GhostShop{}
				case "poisoned":
					p.PersistencePoisoned = true
				case "missing-ingredient":
					p.Char.Inv[0] = model.Item{}
				case "invalid-position":
					req.Pos[0] = model.PlayerCarrySlots
				case "unsupported-batch":
					criticalTinyTradePeer(t, w, p)
					queue = newCriticalTestQueue(t, backend, 1, 2)
					w.criticalWrites = newCriticalPersistenceOwner(queue)
				case "closed-admission":
					queue.closeAdmission()
				}
				before := accountStateSnapshot(p.Account)
				w.handle(command{s: p.Session, pkt: criticalSkillCraftRequest(p, req, artisan)})
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || fallback.saves != 0 ||
					len(queue.pending) != 0 || len(backend.entered) != 0 || w.criticalPlayerReserved(p) {
					t.Fatal("rejected skill request changed inventory, saved inline, or retained ownership")
				}
				if reason == "unsupported-batch" && p.Trade == nil {
					t.Fatal("unsupported storage canceled the trade before admission")
				}
			})
		}
	}
}

func TestCriticalSkillCraftAllAlchemyRecipesAndSecondarySkill(t *testing.T) {
	recipes := [][]uint16{
		{413, 2441, 2442}, {413, 2443, 2442}, {4127, 4127, 4127}, {4127, 4127, 697},
		{412, 2441, 2444}, {412, 2444, 2443}, {612, 2441, 2442}, {612, 613, 614, 615},
		{614, 2443, 2444}, {615, 697, 697, 697},
	}
	for recipe, indices := range recipes {
		for _, secondary := range []bool{false, true} {
			t.Run(fmt.Sprintf("recipe-%d/secondary-%t", recipe, secondary), func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalSkillCraftFixture(t, "Alquimia", 39)
				if !secondary {
					p.Char.SecondaryLearnedSkill = 0
				}
				for i, index := range indices {
					req.Items[i].Index = index
					if req.Items[i].UID == "" {
						req.Items[i].UID = fmt.Sprintf("alchemy-recipe-%d-material-%d", recipe, i)
					}
				}
				if recipe == 6 || recipe == 8 || recipe == 9 {
					req.Items[0].Eff = [6]byte{43, 9}
				}
				for i, item := range req.Items {
					p.Char.Inv[int(req.Pos[i])] = item
				}
				want := accountStateSnapshot(p.Account)
				for i, item := range req.Items {
					if item.Index != 0 {
						want.Chars[p.CharSlot].Inv[int(req.Pos[i])] = model.Item{}
					}
				}
				result := model.Item{Index: uint16(3200 + recipe), UID: req.Items[0].UID}
				if secondary {
					result.Eff = [6]byte{effectAmount, 5}
				}
				want.Chars[p.CharSlot].Inv[int(req.Pos[0])] = result
				w.handle(command{s: p.Session, pkt: criticalSkillCraftRequest(p, req, "Alquimia")})
				proposal := awaitCriticalWrite(t, backend.entered)
				if !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(want)) {
					t.Fatal("queued alchemy changed a recipe, stack size, or consumed ingredient set")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || fallback.saves != 0 || len(queue.pending) != 0 {
					t.Fatal("alchemy recipe failed to apply its committed result")
				}
			})
		}
	}
}

func TestCriticalSkillCraftDisconnectPersistsCommittedState(t *testing.T) {
	for _, artisan := range []string{"Extracao", "Alquimia"} {
		for _, closing := range []string{"owner", "peer", "both"} {
			t.Run(artisan+"/"+closing, func(t *testing.T) {
				w, p, fallback, backend, queue, req, _, release := criticalSkillCraftFixture(t, artisan, 39)
				peer := criticalTinyTradePeer(t, w, p)
				before := accountStateSnapshot(p.Account)
				_, positions, roll, code := criticalSkillCraftExpected(p, req, artisan, 39)
				w.handle(command{s: p.Session, pkt: criticalSkillCraftRequest(p, req, artisan)})
				proposal := awaitCriticalWrite(t, backend.entered)
				closed := []*Player{p}
				if closing == "peer" {
					closed = []*Player{peer}
				} else if closing == "both" {
					closed = append(closed, peer)
				}
				for _, player := range closed {
					w.handle(command{s: player.Session})
				}
				if fallback.saves != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
					p.Trade == nil || peer.Trade == nil || len(backend.entered) != 0 {
					t.Fatal("disconnect persisted or exposed an uncommitted skill result")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				seen := make(map[string]bool)
				for range closed {
					final := awaitCriticalWrite(t, backend.entered)
					var player *Player
					var want *model.Account
					for i, participant := range []*Player{p, peer} {
						if final.accounts[0].Name == participant.Account.Name {
							player, want = participant, proposal.accounts[i]
						}
					}
					if player == nil || seen[player.Account.Name] || !reflect.DeepEqual(final.accounts[0], want) ||
						final.character == nil || final.character.uid != player.Char.UID ||
						!reflect.DeepEqual(final.character.state, buildCharState(player, w.now())) {
						t.Fatal("final disconnect save lost committed skill inventory or character sidecar")
					}
					seen[player.Account.Name] = true
				}
				awaitReadyCriticalResults(t, queue, len(closed))
				for _, player := range closed {
					if !w.criticalPlayerReserved(player) || player.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("final disconnect published or released ownership before acknowledgement")
					}
				}
				w.processCriticalCompletions(len(closed))
				for _, player := range closed {
					if w.players[player.Session] != nil || w.criticalPlayerReserved(player) {
						t.Fatal("acknowledged skill disconnect retained the live player")
					}
				}
				if p.Trade != nil || peer.Trade != nil || len(queue.pending) != 0 || fallback.saves != 0 || w.shuttingDown {
					t.Fatal("skill disconnect did not settle once")
				}
				if closing == "peer" {
					assertCriticalSkillCraftPublication(t, p, artisan, positions, roll, code, true)
				} else if closing == "owner" {
					assertCriticalPickupPacket(t, peer, wire.CloseTrade(peer.ID))
				}
			})
		}
	}
}
