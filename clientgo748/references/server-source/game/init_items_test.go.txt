package game

import (
	"encoding/binary"
	"path/filepath"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/net"
)

// Objeto de mundo (portao, porta, canhao, torre) e mobilia do mapa: entra no
// chao no boot, nunca expira e nao pode ser recolhido. Um drop comum e o
// oposto nos tres pontos, e e o mesmo GroundItem -- por isso cada um tem teste.

func worldComObjetos(objetos ...model.InitItem) (*World, error) {
	w := &World{
		groundItems: map[uint16]*GroundItem{},
		items:       map[uint16]model.ItemDef{},
		nextItemID:  10000,
		initItems:   objetos,
	}
	for _, obj := range objetos {
		w.items[obj.Index] = model.ItemDef{Index: obj.Index}
	}
	return w, w.spawnInitItems()
}

func TestObjetosDeMundoNascemNoBoot(t *testing.T) {
	w, err := worldComObjetos(
		model.InitItem{Index: 773, X: 2100, Y: 2100, Rotate: 1},
		model.InitItem{Index: 758, X: 1500, Y: 1500, Rotate: 3},
	)
	if err != nil {
		t.Fatal(err)
	}
	if len(w.groundItems) != 2 {
		t.Fatalf("%d objetos no chao, quer 2", len(w.groundItems))
	}
	for _, g := range w.groundItems {
		if !g.Permanent {
			t.Errorf("objeto %d nasceu como drop comum", g.Item.Index)
		}
	}
	// A rotacao vem do arquivo e precisa sobreviver ate o pacote: era 0 fixo
	// nos dois caminhos de publicacao, o que achataria todo portao.
	var achou bool
	for _, g := range w.groundItems {
		if g.Item.Index == 758 {
			achou = true
			if g.Rotate != 3 {
				t.Errorf("rotacao=%d, quer 3", g.Rotate)
			}
		}
	}
	if !achou {
		t.Error("objeto 758 nao foi posto no mapa")
	}
}

// TestObjetosDeMundoNaoExpiram: o tick de decay varre groundItems, e um portao
// com Expire zerado seria removido no PRIMEIRO tick (now.After(zero) e true).
func TestObjetosDeMundoNaoExpiram(t *testing.T) {
	w, err := worldComObjetos(model.InitItem{Index: 773, X: 2100, Y: 2100, Rotate: 1})
	if err != nil {
		t.Fatal(err)
	}
	w.tickGroundItems(time.Now().Add(365 * 24 * time.Hour))
	if len(w.groundItems) != 1 {
		t.Fatal("o portao sumiu do mapa com o passar do tempo")
	}
}

// TestDropComumContinuaExpirando garante que a isencao vale SO para objeto de
// mundo -- senao o chao acumularia lixo para sempre.
func TestDropComumContinuaExpirando(t *testing.T) {
	w, err := worldComObjetos()
	if err != nil {
		t.Fatal(err)
	}
	w.groundItems[1] = &GroundItem{ID: 1, Item: model.Item{Index: 412},
		X: 2100, Y: 2100, Expire: time.Now().Add(time.Minute)}

	w.tickGroundItems(time.Now().Add(2 * time.Minute))

	if len(w.groundItems) != 0 {
		t.Fatal("drop comum nao expirou")
	}
}

// TestObjetoDeMundoNaoPodeSerRecolhido: sem a guarda, o jogador caminha ate um
// portao de castelo e o poe no inventario.
func TestObjetoDeMundoNaoPodeSerRecolhido(t *testing.T) {
	w, err := worldComObjetos(model.InitItem{Index: 773, X: 2100, Y: 2100, Rotate: 1})
	if err != nil {
		t.Fatal(err)
	}
	var portao *GroundItem
	for _, g := range w.groundItems {
		portao = g
	}

	session := net.NewTestSession(1, 64)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{Name: "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100}}}}
	p := &Player{ID: 1, Session: session, Account: acc, Char: &acc.Chars[0],
		InWorld: true, X: 2100, Y: 2100}
	w.players = map[*net.Session]*Player{session: p}
	w.volatiles = model.VolatileCatalog{}

	w.onGetItem(session, getItemPacket(portao.ID))

	if _, ainda := w.groundItems[portao.ID]; !ainda {
		t.Fatal("o portao foi recolhido do mapa")
	}
	for _, it := range p.Char.Inv {
		if it.Index == 773 {
			t.Fatal("o portao foi parar no inventario do jogador")
		}
	}
}

// getItemPacket monta o 0x272: DestType@12, DestPos@16 e ItemID@20.
func getItemPacket(itemID uint16) []byte {
	pkt := make([]byte, 28)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(placeInv))
	binary.LittleEndian.PutUint32(pkt[16:20], 0)
	binary.LittleEndian.PutUint16(pkt[20:22], itemID)
	return pkt
}

// TestInitItemsDeDadosReaisCarregam roda o arquivo de verdade contra o catalogo
// de verdade: indice fora do catalogo ou duas pecas na mesma celula derrubam o
// boot, e este teste pega isso no `go test`.
func TestInitItemsDeDadosReaisCarregam(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	objetos, err := data.LoadInitItems(filepath.Join(root, "init_items.csv"), catalog.Items)
	if err != nil {
		t.Fatalf("data/init_items.csv invalido: %v", err)
	}
	if len(objetos) == 0 {
		t.Fatal("nenhum objeto de mundo carregado")
	}
	for _, obj := range objetos {
		def := catalog.Items[obj.Index]
		if def.Name == "" {
			t.Errorf("objeto %d em (%d,%d) nao tem nome no catalogo",
				obj.Index, obj.X, obj.Y)
		}
	}
}
