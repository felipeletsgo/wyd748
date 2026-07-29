package game

import (
	"encoding/binary"
	"errors"
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

	t.Run("gold", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "gold", Consume: true, Gold: 500})
		p.Char.Gold = 100
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Gold != 600 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("gold: total=%d item=%d saves=%d", p.Char.Gold, p.Char.Inv[0].Index, st.saves)
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
}

func TestOnUseItemBuffSkillAndCosmeticActions(t *testing.T) {
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

	t.Run("refine powder incubates an inventory egg", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "refine", Consume: true, RefineMax: 9,
		})
		eggID := uint16(model.MountEggBase)
		p.Char.Inv[1] = model.Item{Index: eggID}
		w.items[eggID] = model.ItemDef{
			Index:         eggID,
			StaticEffects: []model.StaticEffect{{Name: "EF_INCUBATE", Value: 0}},
		}
		pkt := useItemPacket(0, 1)
		w.onUseItem(p.Session, pkt)
		if p.Char.Inv[0].Index != 0 ||
			p.Char.Inv[1].Index != eggID+model.MountTypeCount || st.saves != 1 {
			t.Fatalf("incubacao: po=%d ovo=%+v saves=%d",
				p.Char.Inv[0].Index, p.Char.Inv[1], st.saves)
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

	t.Run("mount protection persists affect and consumption atomically", func(t *testing.T) {
		w, p, _ := useItemWorld(model.VolatileRule{
			Action: "mount", MountAction: "invuln", Consume: true,
			DurationUnits: 100,
		})
		st := &atomicCharStateMemoryStore{}
		w.store = st
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Inv[0].Index != 0 || activePlayerAffect(p.Char, 51) == nil ||
			st.atomicSaves != 1 {
			t.Fatalf("protecao: item=%d affect=%+v saves=%d",
				p.Char.Inv[0].Index, activePlayerAffect(p.Char, 51), st.atomicSaves)
		}
	})

	t.Run("fairy dust advances exactly one level", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "grant_next_level", Consume: true})
		p.Char.Extended.Level = 10
		p.Char.Exp = mortalNextLevel[10]
		syncProgression(p.Char)
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Extended.Level != 11 || p.Char.Exp != mortalNextLevel[11] ||
			p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("fairy dust: level=%d exp=%d item=%d saves=%d",
				p.Char.Extended.Level, p.Char.Exp, p.Char.Inv[0].Index, st.saves)
		}
	})

	t.Run("fairy dust refuses max level", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "grant_next_level", Consume: true})
		p.Char.Extended.Level = maxMortalLevel
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
		p.Char.Extended.Level = 10
		p.Char.Exp = mortalNextLevel[10]
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Extended.Level != 10 || p.Char.Inv[0].Index != 100 || st.saves != 0 {
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
		p.Char.Extended.Level = 70
		p.Char.Extended.Mastery = [4]uint32{10, 80, 30, 5}
		p.Char.LearnedSkill = 0x00FFFFFF
		p.Char.ShortSkill[0] = 7
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.Extended.Mastery[1] != 30 || p.Char.Extended.Mastery[2] != 0 ||
			p.Char.LearnedSkill != 0 || p.Char.Inv[0].Index != 0 ||
			!questCompleted(p.Char, questFlagOpportunityCompound) || st.saves != 1 {
			t.Fatalf("compound: mastery=%v learned=%08X item=%d done=%v saves=%d",
				p.Char.Extended.Mastery, p.Char.LearnedSkill, p.Char.Inv[0].Index,
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

	t.Run("special skill", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "learn_special_skill", Consume: true, LearnedBit: 25,
		})
		w.onUseItem(p.Session, useItemPacket(0, 0))
		if p.Char.LearnedSkill&(1<<25) == 0 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatal("livro especial nao foi aprendido")
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
		// Os adds antigos devem ser substituidos, mantendo a tintura +5.
		p.Char.Inv[1] = model.Item{Index: 200, Eff: [6]byte{60, 9, 120, 5, 71, 70}}
		w.onUseItem(p.Session, useItemPacket(0, 1))
		if p.Char.Inv[1].Eff != ([6]byte{120, 5, 2, 30, 3, 25}) ||
			p.Char.Inv[0].Index != 0 || st.saves != 1 {
			t.Fatalf("repliction: source=%d target=% X saves=%d", p.Char.Inv[0].Index, p.Char.Inv[1].Eff, st.saves)
		}

		// Persistencia falhando restaura tanto o consumivel quanto todos os
		// efeitos antigos do alvo.
		p.Char.Inv[0] = model.Item{Index: 4016}
		before := model.Item{Index: 200, Eff: [6]byte{43, 4, 60, 8, 71, 50}}
		p.Char.Inv[1] = before
		st.err = errors.New("disk")
		w.onUseItem(p.Session, useItemPacket(0, 1))
		if p.Char.Inv[0].Index != 4016 || p.Char.Inv[1] != before || st.saves != 2 {
			t.Fatalf("rollback repliction: source=%d target=% X saves=%d",
				p.Char.Inv[0].Index, p.Char.Inv[1].Eff, st.saves)
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
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 100 {
		t.Fatal("item fora do catalogo foi consumido")
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
