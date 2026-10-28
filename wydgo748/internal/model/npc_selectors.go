package model

// serviceFace identifies appearance that selects a gameplay service.
func (n NPCDef) serviceFace() bool {
	return n.Equip.Rosto.Index == 57 || n.Equip.Rosto.Index == 63 &&
		n.Score != nil && n.Score.Merchant&0xF == 7
}

// ServiceFaceChanged rejects both removing and introducing a face-bound service.
func (n NPCDef) ServiceFaceChanged(next NPCDef) bool {
	return n.Equip.Rosto.Index != next.Equip.Rosto.Index && (n.serviceFace() || next.serviceFace())
}

// WithSourceSelectors rebases a staff override onto the current service contract.
// Mutable stock, cosmetic appearance, labels, and attributes remain staff-owned.
func (n NPCDef) WithSourceSelectors(source NPCDef) NPCDef {
	n.Name, n.Tipo = source.Name, source.Tipo
	n.ServiceIdentity, n.CombatRole = source.ServiceIdentity, source.CombatRole
	n.ClientMerchant = nil
	if source.ClientMerchant != nil {
		selector := *source.ClientMerchant
		n.ClientMerchant = &selector
	}
	if source.Score != nil {
		score := *source.Score
		if n.Score != nil {
			score = *n.Score
		}
		score.Merchant = source.Score.Merchant
		n.Score = &score
	}
	// Restore face-bound services without discarding unrelated staff edits.
	if n.serviceFace() || source.serviceFace() {
		n.Equip.Rosto = source.Equip.Rosto
	}
	return n
}
