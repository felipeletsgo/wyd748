package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func bossAreaWorld(t *testing.T) (*World, *Mob) {
	t.Helper()
	w := newZoneTestWorld()
	w.players = map[*net.Session]*Player{}
	w.groundItems = map[uint16]*GroundItem{}
	w.items = map[uint16]model.ItemDef{420: {Index: 420, Name: "Laktorerium_Scrap"}}
	w.nextItemID = 10000
	return w, &Mob{ID: 3000, X: 2100, Y: 2100, Def: &model.NPCDef{Name: "Demon Lord"}}
}

func poeJogador(w *World, id uint16, x, y uint16) *net.Session {
	s := net.NewTestSession(int64(id), 64)
	acc := &model.Account{Name: "c", Chars: []model.Char{{Name: "n",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 10, CurHP: 10}}}}
	p := &Player{ID: id, Session: s, Account: acc, Char: &acc.Chars[0], InWorld: true, X: x, Y: y}
	w.players[s] = p
	w.updatePlayerSpatial(p)
	return s
}

// TestAvisoDeBossNaoSaiDoRaio: o anuncio e informacao do ENCONTRO. Quem esta do
// outro lado do mapa nao tem o que fazer com ele, e um broadcast global viraria
// spam para o servidor inteiro a cada respawn.
func TestAvisoDeBossNaoSaiDoRaio(t *testing.T) {
	w, m := bossAreaWorld(t)
	dentro := poeJogador(w, 1, m.X, m.Y)
	borda := poeJogador(w, 2, m.X+bossAnnounceRadius, m.Y)
	fora := poeJogador(w, 3, m.X+bossAnnounceRadius+1, m.Y)

	w.announceBoss(m.X, m.Y, "The Demon Lord emerges!")

	if dentro.QueuedPacketsForTest() == 0 {
		t.Error("quem estava junto do boss nao recebeu o aviso")
	}
	if borda.QueuedPacketsForTest() == 0 {
		t.Errorf("quem estava a exatos %d tiles nao recebeu o aviso", bossAnnounceRadius)
	}
	if fora.QueuedPacketsForTest() != 0 {
		t.Errorf("o aviso vazou para %d tiles, alem do raio %d",
			bossAnnounceRadius+1, bossAnnounceRadius)
	}
}

func TestAvisoVazioNaoEnviaNada(t *testing.T) {
	w, m := bossAreaWorld(t)
	s := poeJogador(w, 1, m.X, m.Y)
	w.announceBoss(m.X, m.Y, "")
	if s.QueuedPacketsForTest() != 0 {
		t.Error("mensagem vazia gerou pacote")
	}
}

// TestPremiacaoDeAreaEspalhaNoChao: 50 unidades, uma por celula. Empilhar tudo
// numa celula so daria a premiacao inteira a quem estivesse em cima do corpo.
func TestPremiacaoDeAreaEspalhaNoChao(t *testing.T) {
	w, m := bossAreaWorld(t)
	w.spawnBossAreaReward(m, model.BossAreaReward{Item: 420, Amount: 50})

	if len(w.groundItems) != 50 {
		t.Fatalf("%d itens no chao, quer 50", len(w.groundItems))
	}
	celulas := make(map[uint32]bool, 50)
	for _, g := range w.groundItems {
		if g.Item.Index != 420 {
			t.Fatalf("item errado no chao: %d", g.Item.Index)
		}
		if g.Permanent {
			t.Error("premiacao de area nasceu permanente; ela precisa expirar")
		}
		chave := uint32(g.X)<<16 | uint32(g.Y)
		if celulas[chave] {
			t.Errorf("duas unidades na mesma celula (%d,%d)", g.X, g.Y)
		}
		celulas[chave] = true
		if d := chebyshev(m.X, m.Y, g.X, g.Y); d > bossAnnounceRadius {
			t.Errorf("unidade caiu a %d tiles do corpo, alem do raio", d)
		}
	}
}

func TestPremiacaoDesligadaNaoDropaNada(t *testing.T) {
	w, m := bossAreaWorld(t)
	w.spawnBossAreaReward(m, model.BossAreaReward{})
	w.spawnBossAreaReward(m, model.BossAreaReward{Item: 420, Amount: 0})
	w.spawnBossAreaReward(m, model.BossAreaReward{Item: 9999, Amount: 10}) // fora do catalogo
	if len(w.groundItems) != 0 {
		t.Fatalf("%d itens no chao, quer 0", len(w.groundItems))
	}
}
