package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func physicalAttackPacket(tick uint32, targetID, targetX, targetY uint16) []byte {
	pkt := make([]byte, 48)
	binary.LittleEndian.PutUint32(pkt[8:12], tick)
	binary.LittleEndian.PutUint16(pkt[20:22], targetX)
	binary.LittleEndian.PutUint16(pkt[22:24], targetY)
	binary.LittleEndian.PutUint16(pkt[24:26], 0)
	binary.LittleEndian.PutUint16(pkt[44:46], targetID)
	return pkt
}

func TestRestartRevivesAfterNativeDelayAndSysQuitPersists(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	setPlayerCurHP(p.Char, 0)
	p.DeadAt = time.Now().Add(-5 * time.Second)
	p.X, p.Y = 2200, 2200
	w.updatePlayerSpatial(p)

	w.onRestart(p.Session)
	if playerCurHP(p.Char) == 0 || p.DeadAt.IsZero() == false {
		t.Fatal("restart nao reviveu/limpou deadline de morte")
	}
	if chebyshev(p.X, p.Y, recallX, recallY) > 8 || st.saves != 1 {
		t.Fatalf("restart nao chamou recall seguro: pos=(%d,%d) saves=%d", p.X, p.Y, st.saves)
	}

	p.X, p.Y = 2300, 2300
	before := p.Session.QueuedPacketsForTest()
	w.onSysQuit(p.Session)
	if st.saves != 2 || p.Session.QueuedPacketsForTest() <= before {
		t.Fatalf("DelayStart nao persistiu/confirmou: saves=%d packets=%d", st.saves, p.Session.QueuedPacketsForTest())
	}
	if p.Char.X != 2300 || p.Char.Y != 2300 {
		t.Fatalf("save alterou a posicao viva: (%d,%d)", p.Char.X, p.Char.Y)
	}
}

func TestRestartAndSysQuitRejectInvalidOrTooEarlyRequests(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	w.onRestart(nil)
	w.onSysQuit(nil)
	if st.saves != 0 {
		t.Fatal("sessao desconhecida alterou persistencia")
	}

	setPlayerCurHP(p.Char, 0)
	p.DeadAt = time.Now()
	w.onRestart(p.Session)
	if playerCurHP(p.Char) != 0 || st.saves != 0 {
		t.Fatal("restart ignorou bloqueio nativo de quatro segundos")
	}

	st.err = errors.New("disk")
	before := p.Session.QueuedPacketsForTest()
	w.onSysQuit(p.Session)
	if st.saves != 1 || p.Session.QueuedPacketsForTest() != before {
		t.Fatal("falha de save no DelayStart deveria impedir confirmacao")
	}
}

func TestPhysicalAttackHandlerDamagesMobAndTracksAggro(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Score.Attack = 1000
	p.Char.Score.Dex = 1000
	p.Char.Score.Accuracy = 1000
	applyScore(p.Char)
	mob := &Mob{
		ID: 1000, X: 2101, Y: 2100, HP: 10_000,
		Def: &model.NPCDef{
			Name: "TargetMob", Tipo: model.TipoMonstro,
			Score: testScore(model.Score{
				MaxHP: 10_000, CurHP: 10_000, Defense: 0,
			}),
		},
	}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	p.show(mob.ID)

	w.onAttack(p.Session, physicalAttackPacket(1000, mob.ID, mob.X, mob.Y))
	if mob.HP >= 10_000 || mob.TargetID != p.ID || p.CombatTargetID != mob.ID {
		t.Fatalf("ataque nao aplicou dano/aggro: hp=%d mobTarget=%d playerTarget=%d",
			mob.HP, mob.TargetID, p.CombatTargetID)
	}
	after := mob.HP
	w.onAttack(p.Session, physicalAttackPacket(1001, mob.ID, mob.X, mob.Y))
	if mob.HP != after {
		t.Fatal("anti-speed aceitou ataque imediatamente repetido")
	}
}

func TestPhysicalAttackHandlerDamagesEnemyPlayerOnly(t *testing.T) {
	attacker, _ := networkedTestPlayer(1, "Attacker", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	attacker.Char.Score.Attack = 2000
	attacker.Char.Score.Dex = 1000
	attacker.Char.Score.Accuracy = 1000
	target.Char.Score.Defense = 0
	target.Char.Score.Dex = 0
	target.Char.Score.Parry = 0
	applyScore(attacker.Char)
	applyScore(target.Char)
	w := worldWithNetworkedPlayers(attacker, target)
	w.store = &craftStore{}
	w.showPlayerPair(attacker, target)

	hpBefore := playerCurHP(target.Char)
	w.onAttack(attacker.Session, physicalAttackPacket(1000, target.ID, target.X, target.Y))
	if playerCurHP(target.Char) != hpBefore || target.LastAttackerID != 0 ||
		attacker.LastAttackTick != 0 || !attacker.LastAttackAt.IsZero() {
		t.Fatalf("PK desligado alterou combate ou consumiu relogio: hp=%d/%d attacker=%d tick=%d at=%v",
			playerCurHP(target.Char), hpBefore, target.LastAttackerID,
			attacker.LastAttackTick, attacker.LastAttackAt)
	}

	attacker.PKMode = true
	w.onAttack(attacker.Session, physicalAttackPacket(1000, target.ID, target.X, target.Y))
	if playerCurHP(target.Char) >= hpBefore || target.LastAttackerID != attacker.ID {
		t.Fatalf("PvP fisico nao aplicado: hp=%d/%d attacker=%d", playerCurHP(target.Char), hpBefore, target.LastAttackerID)
	}

	target.Party = &Party{Members: []*Player{attacker, target}}
	attacker.Party = target.Party
	attacker.LastAttackAt = time.Time{}
	attacker.LastAttackTick = 0
	hpBefore = playerCurHP(target.Char)
	w.onAttack(attacker.Session, physicalAttackPacket(2000, target.ID, target.X, target.Y))
	if playerCurHP(target.Char) != hpBefore {
		t.Fatal("ataque fisico atingiu membro da mesma party")
	}
}
