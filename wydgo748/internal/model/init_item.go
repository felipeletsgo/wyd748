package model

import "fmt"

// InitItem e um objeto PERMANENTE do mundo: portao de castelo, porta de teste,
// canhao, torre. O nativo os carrega de InitItem.csv antes de abrir o servidor
// e os mantem isentos do decay -- ProcessDecayItem comeca em g_dwInitItem+1,
// justamente para nunca varrer essa faixa (Basedef.cpp:7237, Server.cpp:11759).
//
// Diferente de um drop, ele nunca expira e nao pode ser recolhido.
type InitItem struct {
	Index  uint16
	X, Y   uint16
	Rotate byte
}

func (i InitItem) Validate() error {
	if i.Index == 0 {
		return fmt.Errorf("objeto de mundo sem indice de item")
	}
	if i.X == 0 || i.Y == 0 {
		return fmt.Errorf("objeto %d em posicao invalida (%d,%d)", i.Index, i.X, i.Y)
	}
	return nil
}
