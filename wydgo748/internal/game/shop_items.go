package game

import (
	"fmt"

	"wydgo/internal/model"
	"wydgo/internal/npccontent"
)

// itemEffectValue devolve o valor de um efeito dinamico do STRUCT_ITEM. O
// segundo retorno distingue "efeito ausente" de um efeito presente com valor
// zero, que e um estoque malformado e precisa ser reparado pelo catalogo.
func itemEffectValue(item model.Item, effect byte) (byte, bool) {
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effect {
			return item.Eff[i*2+1], true
		}
	}
	return 0, false
}

// seedShopItemAmount uses the same blueprint preparation as live content edits.
func seedShopItemAmount(item *model.Item, def model.ItemDef) error {
	return npccontent.SeedShopItemAmount(item, def)
}

// initShopItemDefaults prepara os blueprints enviados no ShopList e copiados
// na compra. O item comprado recebe um UID novo em addToInv; o blueprint do NPC
// permanece reutilizavel e sem UID.
func (w *World) initShopItemDefaults() error {
	for i := range w.npcs {
		for j := range w.npcs[i].Vende {
			item := &w.npcs[i].Vende[j]
			if item.Index == 0 {
				continue
			}
			def, ok := w.items[item.Index]
			if !ok {
				// Mestres convertidos preservam entradas sinteticas que o fluxo
				// de skills resolve sem consultar o catalogo de itens. Elas nao
				// possuem defaults de instancia para materializar aqui.
				continue
			}
			if err := seedShopItemAmount(item, def); err != nil {
				return fmt.Errorf("NPC %q: %w", w.npcs[i].Name, err)
			}
		}
	}
	return nil
}
