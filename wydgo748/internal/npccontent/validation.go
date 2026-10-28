package npccontent

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

const MaxShopItems = 27
const ShopAmountEffect = 61

func Decode(raw []byte) (model.NPCDef, error) {
	var def model.NPCDef
	decoder := json.NewDecoder(bytes.NewReader(raw))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&def); err != nil {
		return model.NPCDef{}, err
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			return model.NPCDef{}, errors.New("trailing JSON content")
		}
		return model.NPCDef{}, err
	}
	return def, nil
}

func immutableNPCDefinitionChanged(old, next model.NPCDef) bool {
	if old.Name != next.Name || old.Tipo != next.Tipo || old.ServiceIdentity != next.ServiceIdentity ||
		old.CombatRole != next.CombatRole || old.Score == nil || next.Score == nil ||
		old.Score.Merchant != next.Score.Merchant || old.ClientMerchant != nil && next.ClientMerchant == nil ||
		old.ClientMerchant == nil && next.ClientMerchant != nil {
		return true
	}
	if old.ClientMerchant != nil && *old.ClientMerchant != *next.ClientMerchant {
		return true
	}
	return old.ServiceFaceChanged(next)
}

func ValidateEditable(raw []byte, source model.NPCDef, items map[uint16]model.ItemDef) (model.NPCDef, error) {
	def, err := Decode(raw)
	if err != nil {
		return model.NPCDef{}, err
	}
	validated, err := data.ValidateNPCDefinition(def, items)
	if err != nil {
		return model.NPCDef{}, err
	}
	visibleStock := 0
	for _, item := range validated.Vende {
		if item.Index != 0 {
			visibleStock++
		}
	}
	if validated.Name != source.Name || visibleStock > MaxShopItems {
		return model.NPCDef{}, errors.New("NPC identity or shop size is not editable")
	}
	if immutableNPCDefinitionChanged(source, validated) {
		return model.NPCDef{}, errors.New("NPC identity, type, service, merchant selectors, and service faces cannot change live")
	}
	equipment := validated.Equip.Items()
	blueprints := append(append([]model.Item(nil), equipment[:]...), validated.Carry...)
	blueprints = append(blueprints, validated.Vende...)
	for _, item := range blueprints {
		if item.UID != "" || item.ActivatedUnix != 0 || item.ExpiresUnix != 0 || item.PausedUnix != 0 {
			return model.NPCDef{}, errors.New("NPC definitions accept item blueprints only")
		}
	}
	for i := range validated.Vende {
		if validated.Vende[i].Index == 0 {
			continue
		}
		if err := SeedShopItemAmount(&validated.Vende[i], items[validated.Vende[i].Index]); err != nil {
			return model.NPCDef{}, err
		}
	}
	return validated, nil
}

// SeedShopItemAmount fills omitted stock amounts without changing explicit packs.
func SeedShopItemAmount(item *model.Item, def model.ItemDef) error {
	if item == nil || item.Index == 0 {
		return nil
	}
	amount := 0
	for _, effect := range def.StaticEffects {
		if effect.Name == "EF_AMOUNT" {
			amount = effect.Value
			break
		}
	}
	if amount == 0 {
		return nil
	}
	if amount < 0 || amount > 255 {
		return fmt.Errorf("item %d has EF_AMOUNT outside the wire range: %d", item.Index, amount)
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == ShopAmountEffect && item.Eff[i*2+1] > 0 {
			return nil
		}
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == ShopAmountEffect || item.Eff[i*2] == 0 {
			item.Eff[i*2] = ShopAmountEffect
			item.Eff[i*2+1] = byte(amount)
			return nil
		}
	}
	return fmt.Errorf("item %d has no free effect pair for EF_AMOUNT=%d", item.Index, amount)
}
