package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func TestInViewUsesExpanded65By65Window(t *testing.T) {
	if !inView(100, 100, 132, 132) {
		t.Fatal("a borda inclusiva de 32 tiles deveria estar visivel")
	}
	if inView(100, 100, 133, 100) || inView(100, 100, 100, 133) {
		t.Fatal("entidade fora da janela 65x65 nao deveria estar visivel")
	}
}

func TestPlayerVisibilitySet(t *testing.T) {
	p := &Player{}
	if p.hasVisible(1000) {
		t.Fatal("mapa vazio marcou entidade visivel")
	}
	p.show(1000)
	if !p.hasVisible(1000) {
		t.Fatal("show nao registrou entidade")
	}
	p.hide(1000)
	if p.hasVisible(1000) {
		t.Fatal("hide nao removeu entidade")
	}
}

func TestPlayerEnterViewRestoresAliveStateBeforeMovement(t *testing.T) {
	p := &Player{
		ID: 77,
		X:  2200,
		Y:  2100,
		Char: &model.Char{
			Name: "Revived",
			Score: testScore(model.Score{
				MaxHP: 123,
				CurHP: 123,
				MaxMP: 45,
				CurMP: 45,
			}),
		},
	}
	p.Char.Affects[0] = model.Affect{Type: 13, ExpiresAt: time.Now().Add(80 * time.Second)}

	packets := playerEnterViewPackets(p)
	if len(packets) != 3 {
		t.Fatalf("entrada na visibilidade gerou %d pacotes, esperado 3", len(packets))
	}
	wantType := []uint16{wire.OpCreateMob, wire.OpSetHpMp, wire.OpAction}
	wantID := []uint16{wire.SceneField, p.ID, p.ID}
	for i, pkt := range packets {
		h := wire.ParseHeader(pkt)
		if h.Type != wantType[i] || h.ID != wantID[i] {
			t.Fatalf("pacote %d: Type=0x%X ID=%d, esperado Type=0x%X ID=%d",
				i, h.Type, h.ID, wantType[i], wantID[i])
		}
	}
	if got := binary.LittleEndian.Uint16(packets[0][16:18]); got != p.ID {
		t.Fatalf("CreateMob materializou ID=%d, esperado %d", got, p.ID)
	}
	if packets[0][66] != 10 || packets[0][67] != 13 {
		t.Fatalf("CreateMob nao restaurou affect visual: % X", packets[0][66:70])
	}
}

func TestAppearanceUpdateUsesPositionlessUpdateEquip(t *testing.T) {
	ch := model.Char{Score: &model.Score{Version: model.ScoreVersion}}
	ch.Equip[6] = model.Item{Index: 700, Eff: [6]byte{116, 4}}
	p := &Player{ID: 9, Char: &ch}
	mesh := bodyMesh(p.Char)
	pkt := playerAppearancePacket(p)
	header := wire.ParseHeader(pkt)
	if header.Type != wire.OpUpdateEquip || len(pkt) != 60 {
		t.Fatalf("aparencia type=0x%X size=%d, esperado UpdateEquip/60", header.Type, len(pkt))
	}
	if got := binary.LittleEndian.Uint16(pkt[12+6*2:]); got != mesh[6] {
		t.Fatalf("mesh da arma=%d, esperado %d", got, mesh[6])
	}
	if got := pkt[44+6]; got != model.AncientCode(ch.Equip[6]) {
		t.Fatalf("AnctCode da arma=%d, esperado %d", got, model.AncientCode(ch.Equip[6]))
	}
}

func TestPlayerMovePublishesBeforeNewObserverEntersView(t *testing.T) {
	newPlayer := func(id, x, y uint16) *Player {
		s := net.NewTestSession(int64(id), 16)
		ch := &model.Char{Name: "Player", X: x, Y: y,
			Score: testScore(model.Score{MaxHP: 100, CurHP: 100})}
		return &Player{ID: id, Session: s, Char: ch, InWorld: true, X: x, Y: y,
			Visible: make(map[uint16]struct{})}
	}

	mover := newPlayer(1, 100, 100)
	existing := newPlayer(2, 100, 101)
	entering := newPlayer(3, 156, 100) // fora da origem; na borda do destino.
	mover.show(existing.ID)
	existing.show(mover.ID)

	w := &World{
		players:     map[*net.Session]*Player{mover.Session: mover, existing.Session: existing, entering.Session: entering},
		playersByID: map[uint16]*Player{mover.ID: mover, existing.ID: existing, entering.ID: entering},
	}
	w.updatePlayerSpatial(mover)
	w.updatePlayerSpatial(existing)
	w.updatePlayerSpatial(entering)

	// A rota visual e publicada imediatamente, mas o destino ainda nao e a
	// posicao autoritativa usada pelo indice espacial.
	w.publishPlayerMove(mover, 100, 100, 124, 100, []byte("666666666666666666666666"))

	if got := existing.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("observador existente recebeu %d pacotes, esperado somente o movimento", got)
	}
	if got := entering.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("observador do destino recebeu %d pacotes antes da chegada autoritativa", got)
	}
	mover.X, mover.Y = 124, 100
	mover.Char.X, mover.Char.Y = mover.X, mover.Y
	w.refreshPlayerVisibility(mover)
	if got := entering.Session.QueuedPacketsForTest(); got != 3 {
		t.Fatalf("novo observador recebeu %d pacotes, esperado CreateMob+HP+Stop sem rota retroativa", got)
	}
}

func TestPlayerStopUsesSingleSoftAdjustmentOnlyWhenInterrupted(t *testing.T) {
	makePlayer := func(id, x uint16) *Player {
		s := net.NewTestSession(int64(id), 8)
		ch := &model.Char{Name: "Walker", X: x, Y: 100,
			Score: testScore(model.Score{AttackRun: 4, MaxHP: 100, CurHP: 100})}
		return &Player{ID: id, Session: s, Char: ch, InWorld: true, X: x, Y: 100,
			Visible: make(map[uint16]struct{})}
	}
	mover, observer := makePlayer(1, 105), makePlayer(2, 104)
	mover.MovePublished = true
	mover.MovePublishedStartX, mover.MovePublishedStartY = 100, 100
	mover.MovePublishedTargetX, mover.MovePublishedTargetY = 110, 100
	copy(mover.MovePublishedRoute[:], []byte("6666666666"))
	mover.show(observer.ID)
	observer.show(mover.ID)
	w := &World{}
	w.updatePlayerSpatial(mover)
	w.updatePlayerSpatial(observer)

	w.publishPlayerStop(mover)
	if got := observer.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("parada intermediaria publicou %d pacotes, esperado um ajuste suave", got)
	}
	if mover.MovePublished {
		t.Fatal("rota continuou ativa depois da parada")
	}
	w.publishPlayerStop(mover)
	if got := observer.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("parada repetida publicou outro ajuste: %d pacotes", got)
	}

	// Chegar naturalmente ao destino nao precisa de nenhum pacote: a Action
	// original ja encerra a animacao exatamente nessa coordenada.
	mover.MovePublished = true
	mover.MovePublishedStartX, mover.MovePublishedStartY = 105, 100
	mover.MovePublishedTargetX, mover.MovePublishedTargetY = mover.X, mover.Y
	w.publishPlayerStop(mover)
	if got := observer.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("chegada natural reiniciou/corrigiu a animacao: %d pacotes", got)
	}
}
