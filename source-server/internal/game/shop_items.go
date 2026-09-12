package game

import (
	"fmt"

	"wydgo/internal/model"
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

// seedShopItemAmount materializa no estoque do NPC o EF_AMOUNT definido no
// itemlist.csv. Efeitos estaticos de combate NAO sao copiados: itemAbility ja os
// le do catalogo e duplica-los no STRUCT_ITEM somaria o bonus duas vezes.
//
// Um EF_AMOUNT positivo configurado no proprio NPC e preservado. O estoque
// nativo pode vender pacotes especiais (por exemplo, 100 pocoes) diferentes do
// tamanho padrao do itemlist. Quando o JSON omite a quantidade, o itemlist e a
// fonte autoritativa do valor inicial.
func seedShopItemAmount(item *model.Item, def model.ItemDef) error {
	if item == nil || item.Index == 0 {
		return nil
	}
	amount := staticEffect(def, "EF_AMOUNT")
	if amount == 0 {
		return nil
	}
	if amount < 0 || amount > 255 {
		return fmt.Errorf("item %d possui EF_AMOUNT fora do wire: %d", item.Index, amount)
	}
	if current, present := itemEffectValue(*item, effectAmount); present && current > 0 {
		return nil
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effectAmount || item.Eff[i*2] == 0 {
			item.Eff[i*2] = effectAmount
			item.Eff[i*2+1] = byte(amount)
			return nil
		}
	}
	return fmt.Errorf("item %d sem par de efeito livre para EF_AMOUNT=%d", item.Index, amount)
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
