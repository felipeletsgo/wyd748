package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// These fixtures characterize the existing payment policy, not economic
// parity. Native quotes are retained as explicit migration gaps; changing the
// authoritative base price must update these expectations deliberately.
type saleEconomyCase struct {
	name               string
	def                model.ItemDef
	currentBase, quote uint32
}

func saleEconomyCases(t *testing.T) []saleEconomyCase {
	t.Helper()
	catalog, err := data.LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	return []saleEconomyCase{
		{"quarter_5000", model.ItemDef{Index: 400, Price: 20000}, 5000, 5000},
		{"quarter_5001", model.ItemDef{Index: 400, Price: 20004}, 5001, 3334},
		{"quarter_10000", model.ItemDef{Index: 400, Price: 40000}, 10000, 6666},
		{"quarter_10001", model.ItemDef{Index: 400, Price: 40004}, 10001, 5000},
		{"powder_412", catalog.Items[412], 250000, 800000},
		{"powder_413", catalog.Items[413], 225, 225},
		{"ability_185", model.ItemDef{Index: 400, Price: 20004,
			StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 185}}}, 5001, 20004},
	}
}

type saleEconomyContractStore struct {
	guildFlowStore
	t        *testing.T
	player   *Player
	account  *model.Account
	treasury uint64
	taxed    bool
}

func (s *saleEconomyContractStore) checkAccount(account *model.Account) {
	s.t.Helper()
	if s.player.Session.QueuedPacketsForTest() != 0 {
		s.t.Fatal("sale snapshots were published before persistence")
	}
	if account == s.player.Account || !reflect.DeepEqual(account, s.account) {
		s.t.Fatal("persistence did not receive the complete projected sale snapshot")
	}
}

func (s *saleEconomyContractStore) SaveAccount(account *model.Account) error {
	s.t.Helper()
	if s.taxed {
		s.t.Fatal("taxed sale bypassed the atomic guild/account transaction")
	}
	s.checkAccount(account)
	return s.craftStore.SaveAccount(account)
}

func (s *saleEconomyContractStore) SaveGameState(guilds *model.GuildRegistry, accounts ...*model.Account) error {
	s.t.Helper()
	if !s.taxed || guilds == nil || len(accounts) != 1 ||
		guilds.Wars.Cities.Territories[0].Treasury != s.treasury {
		s.t.Fatal("sale persistence omitted or changed the expected treasury/account pair")
	}
	s.checkAccount(accounts[0])
	return s.guildFlowStore.SaveGameState(guilds, accounts...)
}

func assertSaleEconomyPublication(t *testing.T, p *Player, slot uint16, gold uint32) {
	t.Helper()
	for sequence, expected := range []struct {
		opcode uint16
		size   int
	}{{wire.OpSendItem, 24}, {wire.OpUpdateEtc, 36}} {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || len(packet) != expected.size || !wire.Decrypt(packet) {
			t.Fatalf("sale response %d is not the expected encrypted frame", sequence)
		}
		header := wire.ParseHeader(packet)
		if header.Type != expected.opcode || int(header.Size) != expected.size || header.ID != p.ID {
			t.Fatalf("sale response %d has the wrong envelope: % x", sequence, packet)
		}
		if sequence == 0 {
			if binary.LittleEndian.Uint16(packet[12:14]) != placeInv ||
				binary.LittleEndian.Uint16(packet[14:16]) != slot ||
				!reflect.DeepEqual(packet[16:24], make([]byte, 8)) {
				t.Fatalf("sale did not clear exactly the committed carry slot: % x", packet)
			}
		} else if binary.LittleEndian.Uint32(packet[32:36]) != gold {
			t.Fatalf("sale gold snapshot differs from the committed balance: % x", packet)
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("sale emitted an extra response, including an unsafe legacy 0x37A confirmation")
	}
}

func TestSaleEconomyCurrentPaymentLifecycle(t *testing.T) {
	for _, fixture := range saleEconomyCases(t) {
		for _, taxed := range []bool{false, true} {
			for _, passive := range []bool{false, true} {
				for _, mode := range []string{"success_replay", "save_failure_retry", "gold_cap", "treasury_cap"} {
					if mode == "treasury_cap" && !taxed {
						continue
					}
					name := fmt.Sprintf("%s/tax_%t/passive_%t/%s", fixture.name, taxed, passive, mode)
					t.Run(name, func(t *testing.T) {
						w, _, p, shop := cityEconomyFixture(t, fixture.def)
						territory := &w.guilds.Wars.Cities.Territories[0]
						territory.Treasury = 1234
						if !taxed {
							territory.Owner = 0
						}
						p.Char.Gold = 1000
						if passive {
							p.Char.Class, p.Char.LearnedSkill = 3, 1<<9
							p.Char.RuntimeScore = testScore(*p.Char.Score)
							p.Char.RuntimeScore.Mastery[2] = 100
						}
						base := uint64(fixture.currentBase)
						if passive && base > 1000 {
							base = base * 116 / 100
						}
						net, revenue := base, uint64(0)
						if taxed {
							tax := base * 10 / 100
							net, revenue = base-tax, tax/4
						}
						if mode == "gold_cap" {
							p.Char.Gold = maxCharacterGold - uint32(net) + 1
						} else if mode == "treasury_cap" {
							territory.Treasury = model.GuildWarTreasuryCap - revenue + 1
						}
						const slot = model.PlayerCarrySlots - 1
						p.Char.Inv[slot] = model.Item{Index: fixture.def.Index,
							UID: "11111111111141118111111111110400", Eff: [6]byte{43, 9, 7, 5, 0, 0},
							ActivatedUnix: 1700000000, ExpiresUnix: 1800000000}
						p.Char.Inv[0] = model.Item{Index: 401, UID: "11111111111141118111111111110401"}
						before := accountStateSnapshot(p.Account)
						warsBefore := w.guilds.Wars.Clone()
						want := accountPersistenceSnapshot(p.Account)
						want.Chars[p.CharSlot].Inv[slot] = model.Item{}
						want.Chars[p.CharSlot].Gold += uint32(net)
						st := &saleEconomyContractStore{t: t, player: p, account: want,
							taxed: taxed, treasury: territory.Treasury + revenue}
						w.store = st
						if mode == "save_failure_retry" {
							st.err = errors.New("sale transaction unavailable")
						}
						packet := saleIngressPacket(shop.ID, placeInv, slot)
						packetBefore := append([]byte(nil), packet...)
						w.handle(command{s: p.Session, pkt: packet})
						if !reflect.DeepEqual(packet, packetBefore) {
							t.Fatal("sale modified its borrowed intent")
						}
						if mode != "success_replay" {
							if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.guilds.Wars, warsBefore) ||
								p.Char != &p.Account.Chars[p.CharSlot] || p.ShopNPC != shop.ID {
								t.Fatal("rejected sale did not preserve the complete account, treasury, or player binding")
							}
							if mode == "gold_cap" {
								if p.Session.QueuedPacketsForTest() != 2 || st.saves+st.gameSaves != 0 {
									t.Fatal("gold-cap rejection persisted or omitted the unchanged item response")
								}
								return
							}
							if p.Session.QueuedPacketsForTest() != 0 {
								t.Fatal("failed sale published a success snapshot")
							}
							if mode == "treasury_cap" {
								if st.saves+st.gameSaves != 0 {
									t.Fatal("treasury-cap rejection reached persistence")
								}
								return
							}
							if st.saves+st.gameSaves != 1 {
								t.Fatal("failed sale did not attempt exactly one transaction")
							}
							st.err = nil
							w.handle(command{s: p.Session, pkt: packet})
						}
						attempts := 1
						if mode == "save_failure_retry" {
							attempts = 2
						}
						if !reflect.DeepEqual(accountPersistenceSnapshot(p.Account), want) ||
							w.guilds.Wars.Cities.Territories[0].Treasury != st.treasury || st.saves+st.gameSaves != attempts {
							t.Fatal("committed sale differs from the expected payment/account/treasury transition")
						}
						assertSaleEconomyPublication(t, p, slot, want.Chars[p.CharSlot].Gold)
						w.handle(command{s: p.Session, pkt: packet})
						if st.saves+st.gameSaves != attempts || p.Session.QueuedPacketsForTest() != 0 ||
							!reflect.DeepEqual(accountPersistenceSnapshot(p.Account), want) ||
							w.guilds.Wars.Cities.Territories[0].Treasury != st.treasury {
							t.Fatal("replay credited, persisted, or published the already sold slot")
						}
						if !passive && !taxed && fixture.currentBase != fixture.quote {
							t.Logf("Unresolved native quote/payment gap: quote=%d payment=%d", fixture.quote, fixture.currentBase)
						}
					})
				}
			}
		}
	}
}
