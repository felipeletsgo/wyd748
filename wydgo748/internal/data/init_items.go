package data

import (
	"fmt"
	"strings"

	"wydgo/internal/model"
)

// LoadInitItems le os objetos permanentes do mundo (portoes, portas, canhoes,
// torres) no formato do InitItem.csv nativo: index,x,y,rotacao.
//
// Aceita comentario no fim da linha depois de '#'. O arquivo e 50 linhas de
// numeros crus, e sem o nome do objeto ao lado nao ha como conferir uma posicao
// sem abrir o itemlist em paralelo.
//
// Validacao estrita, como os demais dados: item fora do catalogo ou posicao
// zerada derruba o boot em vez de virar um portao invisivel no mapa.
func LoadInitItems(path string, items map[uint16]model.ItemDef) ([]model.InitItem, error) {
	var out []model.InitItem
	err := records(path, func(row []string) error {
		if len(row) < 4 {
			return fmt.Errorf("esperado index,x,y,rotacao")
		}
		valores := make([]int, 4)
		for i := 0; i < 4; i++ {
			campo := row[i]
			if corte := strings.IndexByte(campo, '#'); corte >= 0 {
				campo = campo[:corte]
			}
			v, err := integer(campo)
			if err != nil {
				return err
			}
			if v < 0 || v > 65535 {
				return fmt.Errorf("valor fora de faixa: %d", v)
			}
			valores[i] = v
		}
		obj := model.InitItem{
			Index:  uint16(valores[0]),
			X:      uint16(valores[1]),
			Y:      uint16(valores[2]),
			Rotate: byte(valores[3]),
		}
		if err := obj.Validate(); err != nil {
			return err
		}
		if _, ok := items[obj.Index]; !ok {
			return fmt.Errorf("objeto %d nao existe no catalogo de itens", obj.Index)
		}
		out = append(out, obj)
		return nil
	})
	if err != nil {
		return nil, err
	}
	// Duas entradas na mesma celula deixariam um objeto tapando o outro, e o
	// client so guarda um item por posicao do grid.
	ocupada := make(map[uint32]int, len(out))
	for i, obj := range out {
		chave := uint32(obj.X)<<16 | uint32(obj.Y)
		if antes, repetida := ocupada[chave]; repetida {
			return nil, fmt.Errorf("data: %s: objetos %d e %d na mesma posicao (%d,%d)",
				path, out[antes].Index, obj.Index, obj.X, obj.Y)
		}
		ocupada[chave] = i
	}
	return out, nil
}
