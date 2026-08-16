package game

import (
	"testing"

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
