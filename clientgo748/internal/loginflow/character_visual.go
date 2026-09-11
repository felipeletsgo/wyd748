package loginflow

// characterEquipmentLook mirrors the native/TMProject character projection.
// The returned array is indexed by the visual look slot used by TMHuman.
func characterEquipmentLook(equip [18]uint16, battleMaster uint16) [16]uint16 {
	var look [16]uint16
	look[0] = equip[0]
	if look[0] == 22 || look[0] == 23 || look[0] == 24 || look[0] == 25 || look[0] == 32 {
		look[0] = battleMaster
	}

	helmet := equip[1] % 6500
	if helmet < 3500 || (helmet > 3502 && helmet != 3507) {
		look[1] = equip[1]
	}
	look[2] = equip[2]
	look[3] = equip[3]
	look[4] = equip[4]
	look[5] = equip[5]
	look[6] = equip[6]
	look[7] = equip[7]
	look[13] = equip[13]
	look[14] = equip[14]
	look[15] = equip[15]
	return look
}

func characterSkinExpanded(class int) bool {
	switch class {
	case 4, 8, 36, 39, 40, 60, 63:
		return true
	default:
		return false
	}
}
