package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestKingdomJoinPromoteLeaveLifecycle(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Extended.Level = 255
	p.Char.Inv[0] = model.Item{Index: model.SapphireItem, Eff: [6]byte{effectAmount, 25}}
	applyExtendedScore(p.Char)

	w.joinKingdom(p.Session, p, model.KingdomHekalotia)
	if characterKingdom(p.Char) != model.KingdomHekalotia ||
		p.Char.Equip[model.CapeSlot].Index != 545 || sapphireCount(p.Char) != 21 {
		t.Fatalf("adesao incorreta: cape=%d kingdom=%d sapphires=%d",
			p.Char.Equip[model.CapeSlot].Index, characterKingdom(p.Char), sapphireCount(p.Char))
	}

	w.joinKingdom(p.Session, p, model.KingdomHekalotia)
	if p.Char.Equip[model.CapeSlot].Index != 543 || sapphireCount(p.Char) != 17 {
		t.Fatalf("promocao Knight incorreta: cape=%d sapphires=%d",
			p.Char.Equip[model.CapeSlot].Index, sapphireCount(p.Char))
	}

	monday := time.Date(2026, 7, 27, 12, 0, 0, 0, time.Local)
	w.leaveKingdom(p.Session, p, monday)
	if characterKingdom(p.Char) != model.KingdomNeutral ||
		p.Char.Equip[model.CapeSlot].Index != 549 || sapphireCount(p.Char) != 1 {
		t.Fatalf("saida incorreta: cape=%d kingdom=%d sapphires=%d",
			p.Char.Equip[model.CapeSlot].Index, characterKingdom(p.Char), sapphireCount(p.Char))
	}
	if st.saves != 3 {
		t.Fatalf("join/promote/leave deveriam persistir tres vezes: %d", st.saves)
	}
}

func TestKingdomJoinWithEmblemAndRollback(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Extended.Level = kingdomJoinMinLevel
	p.Char.Equip[13] = model.Item{Index: model.KingdomEmblem}
	applyExtendedScore(p.Char)

	w.joinKingdom(p.Session, p, model.KingdomAkelonia)
	if p.Char.Equip[13].Index != 0 || p.Char.Equip[model.CapeSlot].Index != 546 {
		t.Fatalf("pagamento por emblema falhou: emblem=%d cape=%d",
			p.Char.Equip[13].Index, p.Char.Equip[model.CapeSlot].Index)
	}

	p.Char.Equip[model.CapeSlot] = model.Item{}
	p.Char.Equip[13] = model.Item{Index: model.KingdomEmblem}
	w.recalcPlayer(p.Char)
	st.err = errors.New("disk full")
	before := cloneCharacterState(p.Char)
	w.joinKingdom(p.Session, p, model.KingdomAkelonia)
	if !reflect.DeepEqual(*p.Char, before) {
		t.Fatal("falha de save nao restaurou emblema, capa e score")
	}
}

func TestHandleKingdomNPCAndTeleportCommands(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Extended.Level = kingdomJoinMinLevel
	p.Char.Inv[0] = model.Item{Index: model.SapphireItem, Eff: [6]byte{effectAmount, 20}}
	applyExtendedScore(p.Char)
	king := &Mob{ID: 1600, Def: &model.NPCDef{Name: "King_Glantuar"}}
	if !w.handleKingdomNPC(p.Session, p, king) ||
		characterKingdom(p.Char) != model.KingdomAkelonia {
		t.Fatal("rei nao roteou a adesao ao reino")
	}

	p.NextKingdomTeleport = time.Time{}
	w.kingdomCommandTeleport(p.Session, p, true)
	if p.X != 1748 || p.Y != 1880 || p.NextKingdomTeleport.IsZero() {
		t.Fatalf("teleporte do rei incorreto: (%d,%d) cooldown=%v", p.X, p.Y, p.NextKingdomTeleport)
	}
	x, y := p.X, p.Y
	w.kingdomCommandTeleport(p.Session, p, true)
	if p.X != x || p.Y != y {
		t.Fatal("cooldown nao bloqueou segundo teleporte")
	}

	p.NextKingdomTeleport = time.Time{}
	w.kingdomCommandTeleport(p.Session, p, false)
	if p.X != 1690 || p.Y != 1842 {
		t.Fatalf("teleporte de reino incorreto: (%d,%d)", p.X, p.Y)
	}
	if w.handleKingdomNPC(p.Session, p, &Mob{Def: &model.NPCDef{Name: "Ordinary"}}) {
		t.Fatal("NPC comum foi capturado pelo sistema de reino")
	}
}

func TestKingdomRejectsDeadInsufficientAndSundayLeave(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Extended.Level = kingdomJoinMinLevel
	applyExtendedScore(p.Char)

	w.joinKingdom(p.Session, p, model.KingdomHekalotia)
	if characterKingdom(p.Char) != model.KingdomNeutral || st.saves != 0 {
		t.Fatal("adesao sem safiras alterou estado")
	}

	p.Char.Equip[model.CapeSlot] = model.Item{Index: 545}
	p.Char.Inv[0] = model.Item{Index: model.SapphireItem, Eff: [6]byte{effectAmount, 20}}
	w.recalcPlayer(p.Char)
	sunday := time.Date(2026, 7, 26, 12, 0, 0, 0, time.Local)
	w.leaveKingdom(p.Session, p, sunday)
	if p.Char.Equip[model.CapeSlot].Index != 545 || st.saves != 0 {
		t.Fatal("broker permitiu saida no domingo")
	}

	setPlayerCurHP(p.Char, 0)
	w.joinKingdom(p.Session, p, model.KingdomHekalotia)
	w.leaveKingdom(p.Session, p, sunday.Add(24*time.Hour))
	if st.saves != 0 {
		t.Fatal("personagem morto alterou o reino")
	}
}
