package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestSameCellPlayerMoveRecomputesMobActivation(t *testing.T) {
	w := &World{clock: realClock{}}
	p := &Player{
		ID:      1,
		InWorld: true,
		X:       0,
		Y:       8,
		Char: &model.Char{Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion,
			MaxHP:   100,
			CurHP:   100,
		}},
	}
	w.updatePlayerSpatial(p)

	m := &Mob{
		ID: 1000,
		X:  31,
		Y:  8,
		Def: &model.NPCDef{
			Tipo: model.TipoMonstro,
			Extended: &model.ExtendedScore{
				Version: model.ExtendedScoreVersion,
				MaxHP:   100,
			},
		},
	}
	w.registerMobSpatial(m)
	if m.Awake {
		t.Fatal("mob acordou com jogador ainda a 31 tiles")
	}

	// 0 -> 15 continua na mesma celula espacial (0), mas cruza a fronteira
	// exata de ativacao do mob: 31 - 15 = 16 tiles.
	p.X = 15
	w.updatePlayerSpatial(p)
	if !m.Awake {
		t.Fatal("movimento dentro da mesma celula nao acordou mob na borda de 16 tiles")
	}
	if w.activeMobs[m.ID] != m {
		t.Fatal("mob acordado nao entrou no indice activeMobs")
	}

	p.X = 0
	w.updatePlayerSpatial(p)
	if m.Awake {
		t.Fatal("movimento dentro da mesma celula nao adormeceu mob ao sair da ativacao")
	}
	if _, active := w.activeMobs[m.ID]; active {
		t.Fatal("mob adormecido permaneceu em activeMobs")
	}
}

func TestNearestLivingPlayerSpatialScanPreservesSelectionRules(t *testing.T) {
	w := &World{clock: realClock{}}
	newTarget := func(id, x, y uint16) *Player {
		p := &Player{
			ID:      id,
			InWorld: true,
			X:       x,
			Y:       y,
			Char: &model.Char{Extended: &model.ExtendedScore{
				Version: model.ExtendedScoreVersion,
				MaxHP:   100,
				CurHP:   100,
			}},
		}
		w.updatePlayerSpatial(p)
		return p
	}

	farther := newTarget(1, 104, 100)
	tieHighID := newTarget(10, 102, 100)
	tieLowID := newTarget(5, 102, 100)

	if got := w.nearestLivingPlayerInGameplaySpace(100, 100, 4, ""); got != tieLowID {
		t.Fatalf("empate de distancia escolheu ID=%v, esperado menor ID=%d", playerIDForTest(got), tieLowID.ID)
	}

	setPlayerCurHP(tieLowID.Char, 0)
	if got := w.nearestLivingPlayerInGameplaySpace(100, 100, 4, ""); got != tieHighID {
		t.Fatalf("alvo morto participou da aquisicao: ID=%v, esperado %d", playerIDForTest(got), tieHighID.ID)
	}

	tieHighID.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: time.Now().Add(time.Hour)}
	if got := w.nearestLivingPlayerInGameplaySpace(100, 100, 4, ""); got != farther {
		t.Fatalf("alvo oculto participou da aquisicao: ID=%v, esperado %d", playerIDForTest(got), farther.ID)
	}

	setPlayerCurHP(farther.Char, 0)
	if got := w.nearestLivingPlayerInGameplaySpace(100, 100, 4, ""); got != nil {
		t.Fatalf("busca sem alvo valido retornou ID=%d", got.ID)
	}
}

func playerIDForTest(p *Player) any {
	if p == nil {
		return nil
	}
	return p.ID
}

func BenchmarkUpdatePlayerSpatialSameCellNoMobs(b *testing.B) {
	w := &World{clock: realClock{}}
	p := &Player{
		ID:      1,
		InWorld: true,
		X:       1,
		Y:       8,
		Char: &model.Char{Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion,
			MaxHP:   100,
			CurHP:   100,
		}},
	}
	w.updatePlayerSpatial(p)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		p.X = uint16(1 + i%14) // permanece sempre na celula 0
		w.updatePlayerSpatial(p)
	}
}

func BenchmarkNearestLivingPlayerSpatialScan(b *testing.B) {
	w := &World{clock: realClock{}}
	for i := 0; i < 16; i++ {
		p := &Player{
			ID:      uint16(i + 1),
			InWorld: true,
			X:       uint16(98 + i%5),
			Y:       uint16(98 + (i/5)%5),
			Char: &model.Char{Extended: &model.ExtendedScore{
				Version: model.ExtendedScoreVersion,
				MaxHP:   100,
				CurHP:   100,
			}},
		}
		w.updatePlayerSpatial(p)
	}
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = w.nearestLivingPlayerInGameplaySpace(100, 100, mobAggroRange, "")
	}
}
