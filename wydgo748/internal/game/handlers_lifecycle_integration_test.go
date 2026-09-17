package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
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

func physicalAttackTwoPacket(tick uint32, targetID, targetX, targetY, secondaryID uint16) []byte {
	pkt := make([]byte, 52)
	copy(pkt, physicalAttackPacket(tick, targetID, targetX, targetY))
	binary.LittleEndian.PutUint16(pkt[0:2], uint16(len(pkt)))
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpAttackTwo)
	binary.LittleEndian.PutUint16(pkt[48:50], secondaryID)
	return pkt
}

func TestIronSpearPrimaryMissStillResolvesSecondary(t *testing.T) {
	for _, primaryPlayer := range []bool{false, true} {
		for _, lethal := range []bool{false, true} {
			name := "mob"
			if primaryPlayer {
				name = "player"
			}
			if lethal {
				name += "_lethal_secondary"
			}
			t.Run(name, func(t *testing.T) {
				attacker, _ := networkedTestPlayer(1, "Huntress", 2100, 2100)
				attacker.Char.Class, attacker.Char.LearnedSkill = 3, 0x40
				attacker.Char.Score.Accuracy = 0
				attacker.Char.Score.AttackRun, attacker.Char.Score.Critical = 0xF0, 255
				applyScore(attacker.Char)
				attacker.PKMode = true
				w := worldWithNetworkedPlayers(attacker)
				w.rng = fixedRNG{value: 99} // Primary evades; zero-DEX secondary cannot evade.
				primary := &Mob{ID: 1000, X: 2101, Y: 2100, HP: 10_000,
					Def: testNPCDef(model.Score{MaxHP: 10_000, CurHP: 10_000, Dex: 4000})}
				primaryHP := func() uint32 { return primary.HP }
				if primaryPlayer {
					target, _ := networkedTestPlayer(2, "Evader", primary.X, primary.Y)
					target.Char.Score.Dex = 4000
					applyScore(target.Char)
					w = worldWithNetworkedPlayers(attacker, target)
					w.rng = fixedRNG{value: 99}
					w.showPlayerPair(attacker, target)
					primary.ID = target.ID
					primaryHP = func() uint32 { return playerCurHP(target.Char) }
				} else {
					w.mobs = append(w.mobs, primary)
					w.registerMobSpatial(primary)
					attacker.show(primary.ID)
				}
				w.store = &craftStore{}
				secondary := &Mob{ID: 1001, X: 2102, Y: 2100, HP: 10_000,
					Def: testNPCDef(model.Score{MaxHP: 10_000, CurHP: 10_000})}
				if lethal {
					secondary.HP = 1
				}
				w.mobs = append(w.mobs, secondary)
				w.registerMobSpatial(secondary)
				attacker.show(secondary.ID)
				beforePrimary, beforeSecondary := primaryHP(), secondary.HP
				drainPlayerPackets(attacker)
				w.onAttack(attacker.Session, physicalAttackTwoPacket(1000, primary.ID, primary.X, primary.Y, secondary.ID))
				if primaryHP() != beforePrimary || secondary.HP >= beforeSecondary || secondary.Dead != lethal {
					t.Fatalf("primary=%d/%d secondary=%d/%d dead=%v want lethal=%v",
						primaryHP(), beforePrimary, secondary.HP, beforeSecondary, secondary.Dead, lethal)
				}
				if attacker.AttackProgress != 1 {
					t.Fatalf("one action advanced double-hit progression %d times", attacker.AttackProgress)
				}
				for {
					pkt, ok := attacker.Session.DequeuePacketForTest()
					if !ok {
						t.Fatal("missing two-target result")
					}
					if !wire.Decrypt(pkt) {
						t.Fatal("invalid result checksum")
					}
					if wire.ParseHeader(pkt).Type != wire.OpAttackTwo {
						continue
					}
					if len(pkt) != 68 || pkt[30] != 0 || pkt[31] != 3 ||
						binary.LittleEndian.Uint16(pkt[46:48]) != 0xFFFD ||
						binary.LittleEndian.Uint16(pkt[48:50]) != secondary.ID ||
						binary.LittleEndian.Uint32(pkt[60:64]) != 0 ||
						binary.LittleEndian.Uint32(pkt[64:68]) == 0 {
						t.Fatalf("invalid MISS + double/critical secondary result: %x", pkt)
					}
					break
				}
			})
		}
	}
}

func TestRestartRevivesAfterNativeDelayAndSysQuitPersists(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Score.Merchant = uint32(1 << playerHomeCityShift) // Azran.
	setPlayerCurHP(p.Char, 0)
	p.DeadAt = time.Now().Add(-5 * time.Second)
	p.X, p.Y = 2200, 2200
	w.updatePlayerSpatial(p)

	w.onRestart(p.Session)
	if playerCurHP(p.Char) == 0 || p.DeadAt.IsZero() == false {
		t.Fatal("restart nao reviveu/limpou deadline de morte")
	}
	if chebyshev(p.X, p.Y, cityWarZones[1].exitX, cityWarZones[1].exitY) > 8 || st.saves != 1 {
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

func TestHuntressIronSpearSecondaryPhysicalHit(t *testing.T) {
	setup := func(t *testing.T, class byte, learned uint32, secondaryX, secondaryY uint16) (*World, *Player, *Mob, *Mob) {
		t.Helper()
		attacker, _ := networkedTestPlayer(1, "Huntress", 2100, 2100)
		attacker.Char.Class = class
		attacker.Char.LearnedSkill = learned
		attacker.Char.Score.Attack = 1000
		attacker.Char.Score.Dex = 1000
		attacker.Char.Score.Accuracy = 1000
		applyScore(attacker.Char)

		primary := &Mob{ID: 1000, X: 2101, Y: 2100, HP: 10_000,
			Def: testNPCDef(model.Score{MaxHP: 10_000, CurHP: 10_000, Defense: 0})}
		secondary := &Mob{ID: 1001, X: secondaryX, Y: secondaryY, HP: 10_000,
			Def: testNPCDef(model.Score{MaxHP: 10_000, CurHP: 10_000, Defense: 0})}

		w := worldWithNetworkedPlayers(attacker)
		w.rng = fixedRNG{value: 0}
		w.mobs = append(w.mobs, primary, secondary)
		w.registerMobSpatial(primary)
		w.registerMobSpatial(secondary)
		return w, attacker, primary, secondary
	}

	t.Run("valid target exactly behind primary is hit", func(t *testing.T) {
		w, attacker, primary, secondary := setup(t, 3, 0x40, 2102, 2100)
		primaryBefore, secondaryBefore := primary.HP, secondary.HP

		w.onAttack(attacker.Session, physicalAttackTwoPacket(1000, primary.ID, primary.X, primary.Y, secondary.ID))

		if primary.HP >= primaryBefore || secondary.HP >= secondaryBefore {
			t.Fatalf("Lanca de Ferro nao atingiu os dois alvos: primary=%d/%d secondary=%d/%d",
				primary.HP, primaryBefore, secondary.HP, secondaryBefore)
		}
	})

	t.Run("secondary outside exact behind tile is ignored", func(t *testing.T) {
		w, attacker, primary, secondary := setup(t, 3, 0x40, 2102, 2101)
		primaryBefore, secondaryBefore := primary.HP, secondary.HP

		w.onAttack(attacker.Session, physicalAttackTwoPacket(1000, primary.ID, primary.X, primary.Y, secondary.ID))

		if primary.HP >= primaryBefore {
			t.Fatal("ataque primario valido foi perdido")
		}
		if secondary.HP != secondaryBefore {
			t.Fatalf("Lanca de Ferro aceitou geometria invalida: hp=%d/%d", secondary.HP, secondaryBefore)
		}
	})

	for _, tc := range []struct {
		name    string
		class   byte
		learned uint32
	}{
		{name: "non Huntress", class: 2, learned: 0x40},
		{name: "skill bit absent", class: 3, learned: 0},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, attacker, primary, secondary := setup(t, tc.class, tc.learned, 2102, 2100)
			secondaryBefore := secondary.HP

			w.onAttack(attacker.Session, physicalAttackTwoPacket(1000, primary.ID, primary.X, primary.Y, secondary.ID))

			if secondary.HP != secondaryBefore {
				t.Fatalf("alvo secundario foi atingido sem requisito da passiva: hp=%d/%d",
					secondary.HP, secondaryBefore)
			}
		})
	}

	t.Run("non 0x39E packet cannot nominate secondary", func(t *testing.T) {
		w, attacker, primary, secondary := setup(t, 3, 0x40, 2102, 2100)
		pkt := physicalAttackTwoPacket(1000, primary.ID, primary.X, primary.Y, secondary.ID)
		binary.LittleEndian.PutUint16(pkt[4:6], wire.OpAttackOne)
		secondaryBefore := secondary.HP

		w.onAttack(attacker.Session, pkt)

		if secondary.HP != secondaryBefore {
			t.Fatalf("pacote fisico que nao e 0x39E ativou Lanca de Ferro: hp=%d/%d",
				secondary.HP, secondaryBefore)
		}
	})

	t.Run("same party player secondary is ignored", func(t *testing.T) {
		attacker, _ := networkedTestPlayer(1, "Huntress", 2100, 2100)
		secondary, _ := networkedTestPlayer(2, "Party", 2102, 2100)
		attacker.Char.Class = 3
		attacker.Char.LearnedSkill = 0x40
		attacker.Char.Score.Attack = 1000
		attacker.Char.Score.Dex = 1000
		attacker.Char.Score.Accuracy = 1000
		applyScore(attacker.Char)
		attacker.PKMode = true
		party := &Party{Members: []*Player{attacker, secondary}}
		attacker.Party, secondary.Party = party, party

		primary := &Mob{ID: 1000, X: 2101, Y: 2100, HP: 10_000,
			Def: testNPCDef(model.Score{MaxHP: 10_000, CurHP: 10_000, Defense: 0})}
		w := worldWithNetworkedPlayers(attacker, secondary)
		w.rng = fixedRNG{value: 0}
		w.mobs = append(w.mobs, primary)
		w.registerMobSpatial(primary)
		secondaryBefore := playerCurHP(secondary.Char)

		w.onAttack(attacker.Session, physicalAttackTwoPacket(1000, primary.ID, primary.X, primary.Y, secondary.ID))

		if playerCurHP(secondary.Char) != secondaryBefore {
			t.Fatalf("Lanca de Ferro atingiu membro da mesma party: hp=%d/%d",
				playerCurHP(secondary.Char), secondaryBefore)
		}
	})
}
