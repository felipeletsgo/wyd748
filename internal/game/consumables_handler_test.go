package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"log"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func useItemPacket(src, dst uint32) []byte {
	pkt := make([]byte, 36)
	binary.LittleEndian.PutUint32(pkt[12:16], placeInv)
	binary.LittleEndian.PutUint32(pkt[16:20], src)
	binary.LittleEndian.PutUint32(pkt[20:24], placeInv)
	binary.LittleEndian.PutUint32(pkt[24:28], dst)
	return pkt
}

func useItemWorld(rule model.VolatileRule) (*World, *Player, *craftStore) {
	p, _ := networkedTestPlayer(1, "Consumer", 2100, 2100)
	st := &craftStore{}
	w := worldWithNetworkedPlayers(p)
	w.store = st
	w.items = map[uint16]model.ItemDef{
		100: {Index: 100, Price: 500},
		200: {Index: 200, Pos: 4},
	}
	w.volatiles = model.VolatileCatalog{
		Default:   model.VolatileRule{Action: "generic"},
		Items:     map[uint16]model.VolatileRule{100: rule},
		ItemCodes: map[uint16]int{100: 1},
	}
	p.Char.Inv[0] = model.Item{Index: 100}
	return w, p, st
}

func TestBuffConsumablesPersistSourceUIDAndAbsoluteExpiration(t *testing.T) {
	now := time.Unix(2_000_000_000, 0)

	t.Run("experience box consumes last unit and tracks its UID", func(t *testing.T) {
		rule := model.VolatileRule{Action: "buff", Consume: true, AffectType: 39,
			DurationUnits: 900, Accumulate: true, MaxDurationUnits: 10800}
		w, p, _ := useItemWorld(rule)
		w.clock = newFakeClock(now)
		st := &atomicCharStateMemoryStore{}
		w.store = st
		const uid = "11111111111141118111111111104140"
		p.Char.Inv[0] = model.Item{Index: 4140, UID: uid}
		w.items = map[uint16]model.ItemDef{4140: {Index: 4140}}
		w.volatiles.Items = map[uint16]model.VolatileRule{4140: rule}
		w.volatiles.ItemCodes = map[uint16]int{4140: 198}

		w.onUseItem(p.Session, useItemPacket(0, 0))

		a := activePlayerAffectAt(p.Char, 39, now)
		if p.Char.Inv[0] != (model.Item{}) || a == nil ||
			a.SourceItemUID != uid || a.SourceItemIndex != 4140 ||
			a.ExpiresAt.Unix() != now.Add(2*time.Hour).Unix() {
			t.Fatalf("bau EXP incompleto: item=%+v affect=%+v", p.Char.Inv[0], a)
		}
		if st.atomicSaves != 1 || st.state == nil || len(st.state.Affects) != 1 ||
			st.state.Affects[0].SourceItemUID != uid ||
			st.state.Affects[0].ExpiresUnix != now.Add(2*time.Hour).Unix() ||
			st.accountSnapshot.Chars[0].Inv[0] != (model.Item{}) {
			t.Fatalf("snapshot atomico do bau EXP incorreto: saves=%d state=%+v account=%+v",
				st.atomicSaves, st.state, st.accountSnapshot)
		}
		// Persistir o affect nao basta: prove que o pipeline autoritativo de EXP
		// reconhece o mesmo prazo absoluto enquanto ele esta ativo.
		if got := expWithDoubleBuffAt(p.Char, 100, now); got != 200 {
			t.Fatalf("bau EXP persistiu sem dobrar recompensa: got=%d want=200", got)
		}
	})

	t.Run("meat decrements stack without changing UID", func(t *testing.T) {
		rule := model.VolatileRule{Action: "buff", Consume: true, AffectType: 30,
			DurationUnits: 1800, Accumulate: true, MaxDurationUnits: 10800}
		w, p, _ := useItemWorld(rule)
		w.clock = newFakeClock(now)
		st := &atomicCharStateMemoryStore{}
		w.store = st
		const uid = "11111111111141118111111111100498"
		p.Char.Inv[0] = model.Item{Index: 498, UID: uid, Eff: [6]byte{effectAmount, 5}}
		w.items = map[uint16]model.ItemDef{498: {Index: 498}}
		w.volatiles.Items = map[uint16]model.VolatileRule{498: rule}
		w.volatiles.ItemCodes = map[uint16]int{498: 62}

		w.onUseItem(p.Session, useItemPacket(0, 0))

		a := activePlayerAffectAt(p.Char, 30, now)
		if itemStackAmount(p.Char.Inv[0]) != 4 || p.Char.Inv[0].UID != uid ||
			a == nil || a.SourceItemUID != uid || a.SourceItemIndex != 498 ||
			a.ExpiresAt.Unix() != now.Add(4*time.Hour).Unix() {
			t.Fatalf("pilha Meat/affect incorretos: item=%+v affect=%+v", p.Char.Inv[0], a)
		}
		if st.atomicSaves != 1 || st.accountSnapshot.Chars[0].Inv[0].UID != uid ||
			itemStackAmount(st.accountSnapshot.Chars[0].Inv[0]) != 4 ||
			st.state.Affects[0].SourceItemUID != uid {
			t.Fatalf("snapshot atomico da pilha incorreto: state=%+v account=%+v",
				st.state, st.accountSnapshot)
		}
		// Courage e consultado no golpe PvE, fora do RuntimeScore; teste os dois
		// caminhos para evitar um buff salvo que nao produz efeito no combate.
		if got := applyCouragePvEDamageAt(p.Char, 100, false, now); got != 1100 {
			t.Fatalf("Meat nao aplicou Courage fisico: got=%d want=1100", got)
		}
		if got := applyCouragePvEDamageAt(p.Char, 100, true, now); got != 2100 {
			t.Fatalf("Meat nao aplicou Courage magico: got=%d want=2100", got)
		}
	})

	t.Run("love candy applies four affects from one UID", func(t *testing.T) {
		rule := model.VolatileRule{Action: "buff", Consume: true, Affects: []model.VolatileAffect{
			{SkillID: 43, Level: 320, DurationUnits: 320},
			{SkillID: 44, Level: 320, DurationUnits: 320},
			{SkillID: 45, Level: 320, DurationUnits: 320},
			{SkillID: 41, Level: 320, DurationUnits: 320},
		}}
		w, p, _ := useItemWorld(rule)
		w.clock = newFakeClock(now)
		w.skills = map[int]model.SkillDef{
			43: {AffectType: 11, AffectValue: 15}, 44: {AffectType: 9, AffectValue: 90},
			45: {AffectType: 15, AffectValue: 7}, 41: {AffectType: 2, AffectValue: 1},
		}
		st := &atomicCharStateMemoryStore{}
		w.store = st
		const uid = "11111111111141118111111111104145"
		p.Char.Inv[0] = model.Item{Index: 4145, UID: uid, Eff: [6]byte{effectAmount, 10}}
		w.items = map[uint16]model.ItemDef{4145: {Index: 4145}}
		w.volatiles.Items = map[uint16]model.VolatileRule{4145: rule}
		w.volatiles.ItemCodes = map[uint16]int{4145: 67}

		w.onUseItem(p.Session, useItemPacket(0, 0))

		for _, typ := range []byte{11, 9, 15, 2} {
			a := activePlayerAffectAt(p.Char, typ, now)
			if a == nil || a.SourceItemUID != uid || a.SourceItemIndex != 4145 {
				t.Fatalf("affect %d sem origem do doce: %+v", typ, a)
			}
		}
		if itemStackAmount(p.Char.Inv[0]) != 9 || len(st.state.Affects) != 4 {
			t.Fatalf("doce nao confirmou pacote atomico: item=%+v state=%+v", p.Char.Inv[0], st.state)
		}
		for _, persisted := range st.state.Affects {
			if persisted.SourceItemUID != uid || persisted.SourceItemIndex != 4145 {
				t.Fatalf("affect persistido sem origem comum: %+v", persisted)
			}
		}
		// O pacote do Doce do Amor precisa materializar todos os quatro efeitos
		// no score runtime. Recalcule um clone sem affects para isolar o bonus do
		// item da recalculacao normal de atributos/equipamentos do personagem.
		withoutCandy := cloneCharacterState(p.Char)
		withoutCandy.Affects = [16]model.Affect{}
		withoutCandy.RuntimeScore = nil
		w.recalcPlayer(&withoutCandy)
		base := effectiveScore(&withoutCandy)
		got := effectiveScore(p.Char)
		if got.Defense != base.Defense+121 || got.Attack != base.Attack+255 {
			t.Fatalf("doce nao alterou defesa/ataque: base=%+v got=%+v", base, got)
		}
		for branch := range got.Mastery {
			if got.Mastery[branch] != base.Mastery[branch]+39 {
				t.Fatalf("doce nao alterou mastery[%d]: base=%d got=%d",
					branch, base.Mastery[branch], got.Mastery[branch])
			}
		}
		wantRun := uint32(minInt(15, int(base.AttackRun&0x0F)+1))
		if got.AttackRun&0x0F != wantRun || got.AttackRun&0xF0 != base.AttackRun&0xF0 {
			t.Fatalf("doce nao alterou movimento corretamente: base=%#x got=%#x wantRun=%d",
				base.AttackRun, got.AttackRun, wantRun)
		}
	})

	t.Run("failed atomic save restores item and all affects", func(t *testing.T) {
		rule := model.VolatileRule{Action: "buff", Consume: true, AffectType: 39,
			DurationUnits: 900, Accumulate: true, MaxDurationUnits: 10800}
		w, p, _ := useItemWorld(rule)
		w.clock = newFakeClock(now)
		st := &atomicCharStateMemoryStore{atomicErr: errors.New("postgres unavailable")}
		w.store = st
		const uid = "11111111111141118111111111104140"
		p.Char.Inv[0] = model.Item{Index: 4140, UID: uid}
		w.items = map[uint16]model.ItemDef{4140: {Index: 4140}}
		w.volatiles.Items = map[uint16]model.VolatileRule{4140: rule}
		w.volatiles.ItemCodes = map[uint16]int{4140: 198}

		w.onUseItem(p.Session, useItemPacket(0, 0))

		if p.Char.Inv[0].Index != 4140 || p.Char.Inv[0].UID != uid ||
			activePlayerAffectAt(p.Char, 39, now) != nil {
			t.Fatalf("rollback PostgreSQL deixou estado parcial: item=%+v affects=%+v",
				p.Char.Inv[0], p.Char.Affects)
		}
	})
}

func TestLaktoreriumPowderConsumesOnlyCommittedValidAttempts(t *testing.T) {
	newWorld := func(index uint16, powder model.Item) (*World, *Player, *craftStore, []byte) {
		rule := model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9}
		w, p, st := useItemWorld(rule)
		p.Char.Inv[0] = powder
		p.Char.Equip[4] = model.Item{Index: 200, UID: "22222222222242228222222222200200"}
		w.items = map[uint16]model.ItemDef{
			index: {Index: index},
			200:   {Index: 200, Pos: 4},
		}
		w.volatiles.Items = map[uint16]model.VolatileRule{index: rule}
		w.volatiles.ItemCodes = map[uint16]int{index: 5}
		pkt := useItemPacket(0, 4)
		binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)
		return w, p, st, pkt
	}

	t.Run("item 413 last unit disappears after successful commit", func(t *testing.T) {
		const uid = "11111111111141118111111111100413"
		w, p, st, pkt := newWorld(413, model.Item{Index: 413, UID: uid})
		w.rng = fixedRNG{value: 0}
		var logs bytes.Buffer
		oldWriter := log.Writer()
		log.SetOutput(&logs)
		t.Cleanup(func() { log.SetOutput(oldWriter) })

		w.onUseItem(p.Session, pkt)

		if p.Char.Inv[0] != (model.Item{}) || itemSanc(p.Char.Equip[4]) != 1 ||
			p.Char.Equip[4].UID != "22222222222242228222222222200200" || st.saves != 1 {
			t.Fatalf("refino confirmado nao consumiu uma unidade: po=%+v alvo=%+v saves=%d",
				p.Char.Inv[0], p.Char.Equip[4], st.saves)
		}
		if got := logs.String(); !strings.Contains(got, "consumiu=true restante=0") {
			t.Fatalf("auditoria do ultimo po nao registrou pilha vazia: %q", got)
		}
	})

	t.Run("item 4141 failed roll still decrements the committed stack", func(t *testing.T) {
		const uid = "11111111111141118111111111104141"
		powder := model.Item{Index: 4141, UID: uid, Eff: [6]byte{effectAmount, 2}}
		w, p, st, pkt := newWorld(4141, powder)
		// +0 tem chance autoritativa de 100%; partimos de +1 para que a rolagem
		// 100 falhe contra 95 sem tornar o teste dependente de aleatoriedade.
		if !setItemSanc(&p.Char.Equip[4], 1) {
			t.Fatal("fixture sem slot para EF_SANC")
		}
		w.rng = fixedRNG{value: 99}

		w.onUseItem(p.Session, pkt)

		if p.Char.Inv[0].Index != 4141 || p.Char.Inv[0].UID != uid ||
			itemStackAmount(p.Char.Inv[0]) != 1 || itemSanc(p.Char.Equip[4]) != 1 || st.saves != 1 {
			t.Fatalf("falha valida nao consumiu exatamente uma unidade: po=%+v alvo=%+v saves=%d",
				p.Char.Inv[0], p.Char.Equip[4], st.saves)
		}
	})

	t.Run("invalid target and refinement ceiling preserve the powder", func(t *testing.T) {
		const uid = "11111111111141118111111111104141"
		for _, tc := range []struct {
			name    string
			prepare func(*Player, []byte)
		}{
			{name: "inventory target", prepare: func(_ *Player, pkt []byte) {
				binary.LittleEndian.PutUint32(pkt[20:24], placeInv)
			}},
			{name: "plus nine", prepare: func(p *Player, _ []byte) {
				if !setItemSanc(&p.Char.Equip[4], 9) {
					t.Fatal("fixture sem slot para EF_SANC")
				}
			}},
		} {
			t.Run(tc.name, func(t *testing.T) {
				powder := model.Item{Index: 4141, UID: uid}
				w, p, st, pkt := newWorld(4141, powder)
				tc.prepare(p, pkt)

				w.onUseItem(p.Session, pkt)

				if p.Char.Inv[0] != powder || st.saves != 0 {
					t.Fatalf("tentativa invalida consumiu/salvou o po: po=%+v saves=%d",
						p.Char.Inv[0], st.saves)
				}
			})
		}
	})

	t.Run("PostgreSQL failure restores powder UID and target", func(t *testing.T) {
		const uid = "11111111111141118111111111104141"
		powder := model.Item{Index: 4141, UID: uid}
		w, p, st, pkt := newWorld(4141, powder)
		beforeTarget := p.Char.Equip[4]
		w.rng = fixedRNG{value: 0}
		st.err = errors.New("postgres unavailable")

		w.onUseItem(p.Session, pkt)

		if p.Char.Inv[0] != powder || p.Char.Equip[4] != beforeTarget || st.saves != 1 {
			t.Fatalf("rollback do refino perdeu identidade/estado: po=%+v alvo=%+v saves=%d",
				p.Char.Inv[0], p.Char.Equip[4], st.saves)
		}
	})
}

func TestOnUseItemRestoreGoldTeleportAndPositionActions(t *testing.T) {
	t.Run("restore", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{Action: "restore", Consume: true, HP: 100})
		setPlayerCurHP(p.Char, 500)
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if playerCurHP(p.Char) != 600 || p.Char.Inv[0].Index != 0 || p.LastPotion.IsZero() {
			t.Fatalf("restore: hp=%d item=%d last=%v",
				playerCurHP(p.Char), p.Char.Inv[0].Index, p.LastPotion)
		}
	})

	t.Run("restore full", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{Action: "restore", Consume: true, HP: 100})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 {
			t.Fatal("pocao foi consumida com HP cheio")
		}
	})

	t.Run("restore rolls back item and HP when persistence fails", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "restore", Consume: true, HP: 100})
		setPlayerCurHP(p.Char, 500)
		st.err = errors.New("database unavailable")
		beforePackets := p.Session.QueuedPacketsForTest()
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if playerCurHP(p.Char) != 500 || p.Char.Inv[0].Index != 100 || !p.LastPotion.IsZero() {
			t.Fatalf("rollback da pocao incompleto: hp=%d item=%d last=%v",
				playerCurHP(p.Char), p.Char.Inv[0].Index, p.LastPotion)
		}
		if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
			t.Fatalf("rollback da pocao nao republicou o slot: %d -> %d", beforePackets, got)
		}
	})

	t.Run("gold", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "gold", Consume: true, Gold: 500})
		p.Char.Gold = 100
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Gold != 600 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("gold: total=%d item=%d saves=%d", p.Char.Gold, p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("gold rollback resyncs source", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "gold", Consume: true, Gold: 500})
		p.Char.Gold = 100
		st.err = errors.New("database unavailable")
		beforePackets := p.Session.QueuedPacketsForTest()
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Gold != 100 || p.Char.Inv[0].Index != 100 {
			t.Fatalf("rollback gold alterou estado: gold=%d item=%d", p.Char.Gold, p.Char.Inv[0].Index)
		}
		if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
			t.Fatalf("rollback gold nao republicou o slot: %d -> %d", beforePackets, got)
		}
	})

	t.Run("gold cap", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "gold", Consume: true, Gold: 500})
		p.Char.Gold = maxCharacterGold - 100
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Gold != maxCharacterGold-100 || p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatal("barra acima do teto alterou estado")
		}
	})

	t.Run("teleport", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "teleport", Consume: true, X: 2200, Y: 2201})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.X != 2200 || p.Y != 2201 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("teleport: pos=(%d,%d) item=%d saves=%d", p.X, p.Y, p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("save and warp", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "save_position", Consume: true})
		p.X, p.Y = 2300, 2301
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.SavedX != 2300 || p.Char.SavedY != 2301 || st.saves != 1 {
			t.Fatalf("save_position: saved=(%d,%d) saves=%d", p.Char.SavedX, p.Char.SavedY, st.saves)
		}

		p.Char.Inv[0] = model.Item{Index: 100}
		w.volatiles.Items[100] = model.VolatileRule{Action: "warp_saved", Consume: true}
		p.X, p.Y = 2100, 2100
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.X != 2300 || p.Y != 2301 || st.saves != 2 {
			t.Fatalf("warp_saved: pos=(%d,%d) saves=%d", p.X, p.Y, st.saves)
		}
	})

	t.Run("save position rollback resyncs source", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "save_position", Consume: true})
		p.Char.SavedX, p.Char.SavedY = 2000, 2001
		p.X, p.Y = 2300, 2301
		st.err = errors.New("database unavailable")
		beforePackets := p.Session.QueuedPacketsForTest()
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.SavedX != 2000 || p.Char.SavedY != 2001 || p.Char.Inv[0].Index != 100 {
			t.Fatalf("rollback save_position alterou estado: saved=(%d,%d) item=%d",
				p.Char.SavedX, p.Char.SavedY, p.Char.Inv[0].Index)
		}
		if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
			t.Fatalf("rollback save_position nao republicou o slot: %d -> %d", beforePackets, got)
		}
	})
}

func TestOnUseItemBuffSkillAndCosmeticActions(t *testing.T) {
	t.Run("active longer buff warns and keeps secret medicine", func(t *testing.T) {
		rule := model.VolatileRule{
			Action: "buff", Consume: true,
			AffectType: 30, AffectValue: 1, AffectLevel: 500, DurationUnits: 100,
		}
		w, p, st := useItemWorld(rule)
		delete(w.items, 100)
		w.items[646] = model.ItemDef{Index: 646}
		delete(w.volatiles.Items, 100)
		delete(w.volatiles.ItemCodes, 100)
		w.volatiles.Items[646] = rule
		w.volatiles.ItemCodes[646] = 184
		p.Char.Inv[0] = model.Item{Index: 646, UID: "00000000000000000000000000000646"}
		p.Char.Affects[0] = model.Affect{
			Type: 30, ClientType: 30, Value: 1, Level: 2_000,
			ExpiresAt: time.Now().Add(time.Hour),
		}
		beforeItem, beforeAffect := p.Char.Inv[0], p.Char.Affects[0]
		queued := p.Session.QueuedPacketsForTest()

		w.onUseItem(p.Session, useItemPacket(0, 0))

		if p.Char.Inv[0] != beforeItem || p.Char.Affects[0] != beforeAffect || st.saves != 0 {
			t.Fatalf("recusa alterou estado: item=%+v affect=%+v saves=%d",
				p.Char.Inv[0], p.Char.Affects[0], st.saves)
		}
		if got := p.Session.QueuedPacketsForTest() - queued; got != 2 {
			t.Fatalf("recusa deveria enviar slot + aviso, enviou %d pacote(s)", got)
		}
		if buffAlreadyActiveMessage != "This item cannot be used because the buff is already active." {
			t.Fatalf("mensagem em ingles alterada: %q", buffAlreadyActiveMessage)
		}
	})

	t.Run("no direct use never consumes", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "no_direct_use", Consume: false,
		})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatal("item reservado para NPC/comando foi consumido")
		}
	})

	t.Run("counter grant commits item and sidecar atomically", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{
			Action: "grant_counter", Consume: true,
			Counters: map[string]uint32{"kefra_ticket": 100},
		})
		st := &atomicCharStateMemoryStore{}
		w.store = st
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || counterBalance(p, "kefra_ticket") != 100 ||
			st.atomicSaves != 1 || st.state.SpecialCoins["kefra_ticket"] != 100 {
			t.Fatalf("contador: item=%d saldo=%d saves=%d state=%+v",
				p.Char.Inv[0].Index, counterBalance(p, "kefra_ticket"),
				st.atomicSaves, st.state)
		}
	})

	t.Run("refine set changes armor and rolls back on save failure", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "refine_set", Consume: true, RefineMax: 6,
		})
		p.Char.Equip[1] = model.Item{Index: 200}
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || itemSanc(p.Char.Equip[1]) != 0 {
			t.Fatal("refine_set sem persistencia alterou item")
		}
		st.err = nil
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || itemSanc(p.Char.Equip[1]) != 6 {
			t.Fatalf("refine_set: po=%d sanc=%d",
				p.Char.Inv[0].Index, itemSanc(p.Char.Equip[1]))
		}
	})

	t.Run("molar requires mortal range and is one-shot", func(t *testing.T) {
		rule := model.VolatileRule{
			Action: "refine_set", Consume: true, RefineMax: 6, MortalOnly: true,
			MinLevel: 200, MaxLevelExclusive: 256, OnceQuestID: -194,
		}
		w, p, st := useItemWorld(rule)
		p.Char.Score.Level = 200
		p.Char.Equip[1] = model.Item{Index: 200, Eff: [6]byte{43, 0}}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || itemSanc(p.Char.Equip[1]) != 6 ||
			!questCompleted(p.Char, -194) || st.saves != 1 {
			t.Fatalf("molar nao concluiu: item=%d sanc=%d quest=%v saves=%d",
				p.Char.Inv[0].Index, itemSanc(p.Char.Equip[1]), questCompleted(p.Char, -194), st.saves)
		}

		p.Char.Inv[0] = model.Item{Index: 100}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || itemSanc(p.Char.Equip[1]) != 6 || st.saves != 1 {
			t.Fatalf("molar duplicado consumiu ou alterou estado: item=%d sanc=%d saves=%d",
				p.Char.Inv[0].Index, itemSanc(p.Char.Equip[1]), st.saves)
		}
	})

	t.Run("molar rejects arch and levels outside the native range", func(t *testing.T) {
		rule := model.VolatileRule{
			Action: "refine_set", Consume: true, RefineMax: 6, MortalOnly: true,
			MinLevel: 200, MaxLevelExclusive: 256, OnceQuestID: -194,
		}
		for _, tc := range []struct {
			name, evolution string
			level           uint32
		}{
			{name: "below", level: 199},
			{name: "above", level: 256},
			{name: "arch", evolution: "arch", level: 200},
		} {
			t.Run(tc.name, func(t *testing.T) {
				w, p, st := useItemWorld(rule)
				p.Char.Evolution, p.Char.Score.Level = tc.evolution, tc.level
				p.Char.Equip[1] = model.Item{Index: 200, Eff: [6]byte{43, 0}}
				w.onUseItem(p.Session, useItemPacket(0, 0))
				if p.Char.Inv[0].Index != 100 || itemSanc(p.Char.Equip[1]) != 0 ||
					questCompleted(p.Char, -194) || st.saves != 0 {
					t.Fatalf("molar aceito fora da regra: evol=%q level=%d item=%d sanc=%d quest=%v saves=%d",
						p.Char.Evolution, tc.level, p.Char.Inv[0].Index, itemSanc(p.Char.Equip[1]),
						questCompleted(p.Char, -194), st.saves)
				}
			})
		}
	})

	t.Run("refine equipped consumes powder only after commit", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "refine", Consume: true, RefineMax: 9,
		})
		p.Char.Equip[4] = model.Item{Index: 200}
		pkt := useItemPacket(0, 4)
		binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 100 || itemSanc(p.Char.Equip[4]) != 0 {
			t.Fatal("refino sem persistencia alterou equipamento/po")
		}
		st.err = nil
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 0 || itemSanc(p.Char.Equip[4]) != 1 {
			t.Fatalf("refino +0: po=%d sanc=%d",
				p.Char.Inv[0].Index, itemSanc(p.Char.Equip[4]))
		}
	})

	t.Run("refine powder hatches an equipped zero-critical egg", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "refine", Consume: true, RefineMax: 9,
		})
		eggID := uint16(model.MountEggBase)
		p.Char.Equip[mountSlot] = model.Item{Index: eggID}
		w.items[eggID] = model.ItemDef{
			Index:         eggID,
			StaticEffects: []model.StaticEffect{{Name: "EF_INCUBATE", Value: 0}},
		}
		pkt := useItemPacket(0, mountSlot)
		binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 0 ||
			p.Char.Equip[mountSlot].Index != eggID+model.MountTypeCount || st.saves != 1 {
			t.Fatalf("incubacao: po=%d ovo=%+v saves=%d",
				p.Char.Inv[0].Index, p.Char.Equip[mountSlot], st.saves)
		}
	})

	t.Run("mount revive charges gold and restores dead mount", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "mount_revive", Consume: true,
		})
		mount := model.Item{Index: model.MountAdultBase}
		mount.SetMountHP(0)
		mount.SetMountLongev(20)
		p.Char.Equip[mountSlot] = mount
		p.Char.Gold = 1_000
		w.items[mount.Index] = model.ItemDef{Index: mount.Index, Price: 100}
		w.rng = fixedRNG{value: 0}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || p.Char.Gold != 900 ||
			p.Char.Equip[mountSlot].MountHP() <= 0 || st.saves != 1 {
			t.Fatalf("revive: item=%d gold=%d hp=%d saves=%d",
				p.Char.Inv[0].Index, p.Char.Gold,
				p.Char.Equip[mountSlot].MountHP(), st.saves)
		}
	})

	t.Run("mount recovery restores exactly one life point", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "mount", MountAction: "longevity_restore", Amount: 1, Consume: true,
		})
		mount := model.Item{Index: model.MountAdultBase}
		mount.SetMountHP(10_000)
		mount.SetMountLongev(20)
		p.Char.Equip[mountSlot] = mount
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if got := p.Char.Equip[mountSlot].MountLongev(); got != 21 ||
			p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("recovery: LP=%d item=%d saves=%d", got, p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("mount recovery rolls back item and life point on save failure", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "mount", MountAction: "longevity_restore", Amount: 1, Consume: true,
		})
		mount := model.Item{Index: model.MountAdultBase}
		mount.SetMountHP(10_000)
		mount.SetMountLongev(20)
		p.Char.Equip[mountSlot] = mount
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if got := p.Char.Equip[mountSlot].MountLongev(); got != 20 || p.Char.Inv[0].Index != 100 {
			t.Fatalf("rollback recovery: LP=%d item=%d", got, p.Char.Inv[0].Index)
		}
	})

	t.Run("mount recovery does not consume at maximum life points", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "mount", MountAction: "longevity_restore", Amount: 1, Consume: true,
		})
		mount := model.Item{Index: model.MountAdultBase}
		mount.SetMountHP(10_000)
		mount.SetMountLongev(model.MountMaxLongevity)
		p.Char.Equip[mountSlot] = mount
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatalf("recovery no maximo consumiu/salvou: item=%d saves=%d", p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("mount level catalysts enforce their native ranges", func(t *testing.T) {
		tests := []struct {
			name        string
			min, target int
			start       int
			want        int
			consumed    bool
		}{
			{name: "3316 to 100", target: 100, start: 50, want: 100, consumed: true},
			{name: "3317 rejects below 100", min: 100, target: 120, start: 99, want: 99},
			{name: "3317 to 120", min: 100, target: 120, start: 100, want: 120, consumed: true},
		}
		for _, tc := range tests {
			t.Run(tc.name, func(t *testing.T) {
				w, p, st := useItemWorld(model.VolatileRule{
					Action: "mount", MountAction: "level_set", MountMinLevel: tc.min,
					Amount: tc.target, Consume: true,
				})
				mount := model.Item{Index: model.MountAdultBase}
				mount.SetMountHP(10_000)
				mount.SetMountLongev(3)
				mount.SetMountLevel(tc.start)
				p.Char.Equip[mountSlot] = mount
				w.onUseItem(p.Session, useItemPacket(0, 0))
				if got := p.Char.Equip[mountSlot].MountLevel(); got != tc.want {
					t.Fatalf("level=%d, quer %d", got, tc.want)
				}
				if got := p.Char.Inv[0].Index == 0; got != tc.consumed {
					t.Fatalf("consumido=%v, quer %v", got, tc.consumed)
				}
				wantSaves := 0
				if tc.consumed {
					wantSaves = 1
				}
				if st.saves != wantSaves {
					t.Fatalf("saves=%d, quer %d", st.saves, wantSaves)
				}
			})
		}
	})

	t.Run("fairy dust advances exactly one level", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "grant_next_level", Consume: true})
		p.Char.Score.Level = 10
		p.Char.Exp = mortalNextLevel[10]
		syncProgression(p.Char)
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Score.Level != 11 || p.Char.Exp != mortalNextLevel[11] ||
			p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("fairy dust: level=%d exp=%d item=%d saves=%d",
				p.Char.Score.Level, p.Char.Exp, p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("fairy dust refuses max level", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "grant_next_level", Consume: true})
		p.Char.Score.Level = maxMortalLevel
		p.Char.Exp = mortalNextLevel[maxMortalLevel]
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatal("fairy dust consumiu no level maximo")
		}
	})

	t.Run("fairy dust is mortal only", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "grant_next_level", Consume: true, MortalOnly: true,
		})
		p.Char.Evolution = "arch"
		p.Char.Score.Level = 10
		p.Char.Exp = mortalNextLevel[10]
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Score.Level != 10 || p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatal("Poeira de Fada foi aceita por Arch")
		}
	})

	t.Run("firework", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "firework", Consume: true})
		w.rng = fixedRNG{value: 5}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("firework: item=%d saves=%d", p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("chaos remission", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "chaos_remission", Consume: true})
		p.Char.CP = -30
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.CP != 75 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("remission: cp=%d item=%d saves=%d",
				p.Char.CP, p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("native action rollback", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "chaos_remission", Consume: true})
		p.Char.CP = -30
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.CP != -30 || p.Char.Inv[0].Index != 100 {
			t.Fatalf("rollback remission: cp=%d item=%d", p.Char.CP, p.Char.Inv[0].Index)
		}
	})

	t.Run("timed buff persists item and affect atomically", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "buff", Consume: true, AffectType: 35,
			AffectValue: 10, DurationUnits: 450,
		})
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || activePlayerAffect(p.Char, 35) != nil {
			t.Fatal("falha de save deixou consumo/affect parcial")
		}
		st.err = nil
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || activePlayerAffect(p.Char, 35) == nil ||
			st.saves != 2 {
			t.Fatal("buff valido nao foi persistido com o consumo")
		}
	})

	t.Run("timed buff commits account and charstate in one transaction", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{
			Action: "buff", Consume: true, AffectType: 35,
			AffectValue: 10, DurationUnits: 450,
		})
		st := &atomicCharStateMemoryStore{}
		w.store = st
		st.atomicErr = errors.New("database unavailable")
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if st.atomicSaves != 1 || p.Char.Inv[0].Index != 100 ||
			activePlayerAffect(p.Char, 35) != nil {
			t.Fatalf("rollback atomico: saves=%d item=%d affect=%+v",
				st.atomicSaves, p.Char.Inv[0].Index, activePlayerAffect(p.Char, 35))
		}

		st.atomicErr = nil
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if st.atomicSaves != 2 || st.lastUID != p.Char.UID ||
			p.Char.Inv[0].Index != 0 || activePlayerAffect(p.Char, 35) == nil ||
			st.state == nil || len(st.state.Affects) != 1 {
			t.Fatalf("commit atomico incompleto: saves=%d uid=%q item=%d state=%+v",
				st.atomicSaves, st.lastUID, p.Char.Inv[0].Index, st.state)
		}
	})

	t.Run("opportunity compound", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "mastery_reset", Consume: true, MortalOnly: true,
			MinLevel: 69, MaxLevelExclusive: 74, Amount: 50,
		})
		w.volatiles.ItemCodes[100] = 192
		p.Char.Score.Level = 70
		p.Char.Score.Mastery = [4]uint32{10, 80, 30, 5}
		p.Char.LearnedSkill = 0x00FFFFFF
		p.Char.ShortSkill[0] = 7
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Score.Mastery[1] != 30 || p.Char.Score.Mastery[2] != 0 ||
			p.Char.LearnedSkill != 0 || p.Char.Inv[0].Index != 0 ||
			!questCompleted(p.Char, questFlagOpportunityCompound) || st.saves != 1 {
			t.Fatalf("compound: mastery=%v learned=%08X item=%d done=%v saves=%d",
				p.Char.Score.Mastery, p.Char.LearnedSkill, p.Char.Inv[0].Index,
				questCompleted(p.Char, questFlagOpportunityCompound), st.saves)
		}

		p.Char.Inv[0] = model.Item{Index: 100}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 1 {
			t.Fatal("compound foi reutilizado")
		}
	})

	t.Run("territory pass validates evolution", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "territory_pass", Consume: true, RequiredEvolution: "arch",
			X: 2200, Y: 2201,
		})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 0 {
			t.Fatal("Mortal usou passe Arch")
		}
		p.Char.Evolution = "arch"
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || p.X != 2200 || p.Y != 2201 || st.saves != 1 {
			t.Fatalf("passe Arch: item=%d pos=(%d,%d) saves=%d",
				p.Char.Inv[0].Index, p.X, p.Y, st.saves)
		}
	})

	t.Run("fairy blessing water", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "refine_equipped", Consume: true, TargetSlot: 13,
			TargetItems: []uint16{769, 1726}, RefineMax: 6,
		})
		p.Char.Equip[13] = model.Item{Index: 769}
		if !setItemSanc(&p.Char.Equip[13], 5) {
			t.Fatal("fixture sem slot de sanc")
		}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if itemSanc(p.Char.Equip[13]) != 6 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("fairy water: sanc=%d item=%d saves=%d",
				itemSanc(p.Char.Equip[13]), p.Char.Inv[0].Index, st.saves)
		}
		p.Char.Inv[0] = model.Item{Index: 100}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 1 {
			t.Fatal("fairy water ultrapassou +6")
		}
	})

	t.Run("nightmare ticket cooldown and rollback", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "nightmare_ticket", Consume: true, Amount: 13, CooldownSeconds: 43200,
		})
		clock := newFakeClock(time.Unix(2_000_000_000, 0))
		w.clock = clock
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.NightmareTickets != 13 || p.Char.Inv[0].Index != 0 ||
			p.Char.LastNightmareUnix != clock.Now().Unix() || st.saves != 1 {
			t.Fatalf("nightmare: tickets=%d item=%d last=%d saves=%d",
				p.Char.NightmareTickets, p.Char.Inv[0].Index,
				p.Char.LastNightmareUnix, st.saves)
		}

		p.Char.Inv[0] = model.Item{Index: 100}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.NightmareTickets != 13 || p.Char.Inv[0].Index != 100 || st.saves != 1 {
			t.Fatal("nightmare ignorou cooldown")
		}

		clock.Advance(12 * time.Hour)
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.NightmareTickets != 13 || p.Char.Inv[0].Index != 100 ||
			p.Char.LastNightmareUnix != 2_000_000_000 {
			t.Fatal("nightmare nao restaurou estado apos falha")
		}
	})

	t.Run("buff", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{
			Action: "buff", Consume: true, AffectType: 4, AffectValue: 10, DurationUnits: 10,
		})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || !hasActiveAffect(p.Char, 4) {
			t.Fatal("buff nao foi aplicado/consumido")
		}
	})

	t.Run("magical pill", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "magical_pill", Consume: true})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if !p.Char.MagicalPillUsed || p.Char.SkillPointBonus != 9 ||
			p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("pill: used=%v bonus=%d item=%d saves=%d",
				p.Char.MagicalPillUsed, p.Char.SkillPointBonus, p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("magical pill rollback resyncs source", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "magical_pill", Consume: true})
		st.err = errors.New("database unavailable")
		beforePackets := p.Session.QueuedPacketsForTest()
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.MagicalPillUsed || p.Char.SkillPointBonus != 0 || p.Char.Inv[0].Index != 100 {
			t.Fatalf("rollback pill alterou estado: used=%v bonus=%d item=%d",
				p.Char.MagicalPillUsed, p.Char.SkillPointBonus, p.Char.Inv[0].Index)
		}
		if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
			t.Fatalf("rollback pill nao republicou o slot: %d -> %d", beforePackets, got)
		}
	})

	t.Run("special skill", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "learn_special_skill", Consume: true, LearnedBit: 25,
		})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.LearnedSkill&(1<<25) == 0 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatal("livro especial nao foi aprendido")
		}
	})

	t.Run("special skill rollback resyncs source", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "learn_special_skill", Consume: true, LearnedBit: 25,
		})
		st.err = errors.New("database unavailable")
		beforePackets := p.Session.QueuedPacketsForTest()
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.LearnedSkill&(1<<25) != 0 || p.Char.Inv[0].Index != 100 {
			t.Fatalf("rollback skill alterou estado: learned=%08X item=%d",
				p.Char.LearnedSkill, p.Char.Inv[0].Index)
		}
		if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
			t.Fatalf("rollback skill nao republicou o slot: %d -> %d", beforePackets, got)
		}
	})

	t.Run("face transform and restore", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{
			Action: "face_transform", Consume: true, FaceMesh: 202, DurationUnits: 10,
		})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || !hasActiveAffect(p.Char, affectFaceTransform) {
			t.Fatal("transformacao de rosto nao aplicada")
		}
		p.Char.Inv[0] = model.Item{Index: 100}
		w.volatiles.Items[100] = model.VolatileRule{Action: "face_restore", Consume: true}
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || hasActiveAffect(p.Char, affectFaceTransform) {
			t.Fatal("restauracao de rosto nao aplicada")
		}
	})

	t.Run("disabled", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{Action: "disabled", Consume: true})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 {
			t.Fatal("volatile disabled nao consumiu item")
		}
	})
}

func TestOnUseItemTintUntintReplictionAndFallback(t *testing.T) {
	t.Run("tint and untint", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "tint", Consume: true, Color: 120})
		p.Char.Inv[1] = model.Item{Index: 200, Eff: [6]byte{43, 9}}
		w.onUseItem(p.Session, useItemPacket(0, 1))
		if p.Char.Inv[1].Eff[0] != 120 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("tint: source=%d target=% X saves=%d", p.Char.Inv[0].Index, p.Char.Inv[1].Eff, st.saves)
		}

		p.Char.Inv[0] = model.Item{Index: 100}
		w.volatiles.Items[100] = model.VolatileRule{Action: "untint", Consume: true}
		w.onUseItem(p.Session, useItemPacket(0, 1))
		if p.Char.Inv[1].Eff[0] != 43 || p.Char.Inv[0].Index != 0 || st.saves != 2 {
			t.Fatalf("untint: source=%d target=% X saves=%d", p.Char.Inv[0].Index, p.Char.Inv[1].Eff, st.saves)
		}
	})

	t.Run("tint rollback resyncs source and target", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "tint", Consume: true, Color: 120})
		beforeTarget := model.Item{Index: 200, Eff: [6]byte{43, 9}}
		p.Char.Inv[1] = beforeTarget
		st.err = errors.New("database unavailable")
		beforePackets := p.Session.QueuedPacketsForTest()

		w.onUseItem(p.Session, useItemPacket(0, 1))

		if p.Char.Inv[0].Index != 100 || p.Char.Inv[1] != beforeTarget {
			t.Fatalf("rollback tint alterou estado: source=%d target=% X",
				p.Char.Inv[0].Index, p.Char.Inv[1].Eff)
		}
		// MSG_UseItem e otimista nas duas pontas do arraste; o rollback deve
		// devolver exatamente os dois slots autoritativos ao client.
		if got := p.Session.QueuedPacketsForTest(); got != beforePackets+2 {
			t.Fatalf("rollback tint nao republicou origem e alvo: %d -> %d", beforePackets, got)
		}
	})

	t.Run("untint rollback resyncs source and target", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "untint", Consume: true})
		beforeTarget := model.Item{Index: 200, Eff: [6]byte{120, 9}}
		p.Char.Inv[1] = beforeTarget
		st.err = errors.New("database unavailable")
		beforePackets := p.Session.QueuedPacketsForTest()

		w.onUseItem(p.Session, useItemPacket(0, 1))

		if p.Char.Inv[0].Index != 100 || p.Char.Inv[1] != beforeTarget {
			t.Fatalf("rollback untint alterou estado: source=%d target=% X",
				p.Char.Inv[0].Index, p.Char.Inv[1].Eff)
		}
		if got := p.Session.QueuedPacketsForTest(); got != beforePackets+2 {
			t.Fatalf("rollback untint nao republicou origem e alvo: %d -> %d", beforePackets, got)
		}
	})

	t.Run("repliction", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "repliction", Consume: true})
		delete(w.volatiles.Items, 100)
		delete(w.volatiles.ItemCodes, 100)
		w.volatiles.Items[4016] = model.VolatileRule{Action: "repliction", Consume: true}
		w.volatiles.ItemCodes[4016] = 190
		w.items[4016] = model.ItemDef{Index: 4016}
		w.volatiles.Repliction = model.ReplictionCatalog{
			Items: map[uint16]model.ReplictionItem{4016: {ItemLevel: 1, MaxSanc: 6}},
			Pools: map[int][]model.ReplictionBonus{4: {
				{Effect1: 2, Value1: 30, Effect2: 3, Value2: 25},
			}},
		}
		w.items[200] = model.ItemDef{
			Index: 200, Pos: 4,
			StaticEffects:      []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 1}},
			DynamicEffectNames: map[byte]string{43: "EF_SANC"},
		}
		p.Char.Inv[0] = model.Item{Index: 4016}
		// Os adds antigos devem ser substituidos, mantendo a tintura +5. O
		// protocolo nativo aceita Repliction somente no equipamento vestido.
		p.Char.Equip[4] = model.Item{Index: 200, Eff: [6]byte{60, 9, 120, 5, 71, 70}}
		pkt := useItemPacket(0, 4)
		binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)
		w.onUseItem(p.Session, pkt)
		if p.Char.Equip[4].Eff != ([6]byte{120, 5, 2, 30, 3, 25}) ||
			p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("repliction: source=%d target=% X saves=%d", p.Char.Inv[0].Index, p.Char.Equip[4].Eff, st.saves)
		}

		// Persistencia falhando restaura tanto o consumivel quanto todos os
		// efeitos antigos do alvo.
		p.Char.Inv[0] = model.Item{Index: 4016}
		before := model.Item{Index: 200, Eff: [6]byte{43, 4, 60, 8, 71, 50}}
		p.Char.Equip[4] = before
		st.err = errors.New("disk")
		beforePackets := p.Session.QueuedPacketsForTest()
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 4016 || p.Char.Equip[4] != before || st.saves != 2 {
			t.Fatalf("rollback repliction: source=%d target=% X saves=%d",
				p.Char.Inv[0].Index, p.Char.Equip[4].Eff, st.saves)
		}
		if got := p.Session.QueuedPacketsForTest(); got < beforePackets+4 {
			t.Fatalf("rollback repliction nao republicou slots e equipamento: %d -> %d", beforePackets, got)
		}
	})

	t.Run("generic", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "generic"})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || st.saves != 0 || p.Session.QueuedPacketsForTest() != 1 {
			t.Fatal("generic deveria apenas reenviar o slot")
		}
	})

	t.Run("unknown action", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{Action: "future_action"})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 100 || p.Session.QueuedPacketsForTest() != 1 {
			t.Fatal("acao desconhecida deveria preservar e reenviar o item")
		}
	})
}

func TestOnUseItemRejectsInvalidStateAndCooldown(t *testing.T) {
	w, p, _ := useItemWorld(model.VolatileRule{Action: "restore", Consume: true, HP: 100})
	setPlayerCurHP(p.Char, 500)
	w.onUseItem(p.Session, make([]byte, 10))
	if p.Char.Inv[0].Index != 100 {
		t.Fatal("pacote invalido consumiu item")
	}

	p.LastPotion = time.Now()
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 100 || playerCurHP(p.Char) != 500 {
		t.Fatal("cooldown de pocao nao preservou estado")
	}

	delete(w.items, 100)
	p.LastPotion = time.Time{}
	beforePackets := p.Session.QueuedPacketsForTest()
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 100 {
		t.Fatal("item fora do catalogo foi consumido")
	}
	if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
		t.Fatalf("item fora do catalogo nao foi ressincronizado: %d -> %d", beforePackets, got)
	}
}

func TestRefineTargetResolvers(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Refiner", 1, 1)
	p.Char.Equip[6] = model.Item{Index: 200}
	p.Char.Inv[2] = model.Item{Index: 201}
	w := &World{}

	equipReq := useItemRequest{dstType: placeEquip, dstPos: 6}
	if item, typ, pos := w.destRefineTarget(p, equipReq); item != &p.Char.Equip[6] || typ != placeEquip || pos != 6 {
		t.Fatal("destRefineTarget nao resolveu equipamento")
	}
	if item, _, _ := w.destRefineTarget(p, useItemRequest{dstType: placeInv, dstPos: 2}); item != nil {
		t.Fatal("destRefineTarget aceitou inventario")
	}
	if item, typ, pos := w.destItemTarget(p, useItemRequest{dstType: placeInv, dstPos: 2}); item != &p.Char.Inv[2] || typ != placeInv || pos != 2 {
		t.Fatal("destItemTarget nao resolveu inventario")
	}
	if item, _, _ := w.destItemTarget(p, useItemRequest{dstType: placeStorage, dstPos: 0}); item != nil {
		t.Fatal("destItemTarget aceitou cargo")
	}
}
