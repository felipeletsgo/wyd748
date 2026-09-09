package game

import (
	"fmt"

	"wydgo/internal/model"
)

// materializeItem converte um blueprint de NPC/template numa instancia
// economica. Itens que ja possuem UID estao apenas mudando de local e preservam
// a identidade; um blueprint sem UID recebe uma identidade nova.
func materializeItem(item model.Item) (model.Item, error) {
	if item.Index == 0 {
		if item.UID != "" || item.ActivatedUnix != 0 || item.ExpiresUnix != 0 {
			return model.Item{}, fmt.Errorf("item vazio possui identidade ou prazo")
		}
		return model.Item{}, nil
	}
	if item.UID != "" {
		normalized, err := model.NormalizeItemUID(item.UID)
		if err != nil {
			return model.Item{}, err
		}
		item.UID = normalized
		return item, nil
	}
	uid, err := model.NewItemUID()
	if err != nil {
		return model.Item{}, err
	}
	item.UID = uid
	return item, nil
}
