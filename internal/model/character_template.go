package model

// CharacterEquipment e o layout nomeado dos 16 slots iniciais. O formato
// nomeado deixa o arquivo de criacao editavel sem depender de indices magicos.
type CharacterEquipment struct {
	Rosto      Item `json:"rosto"`
	Elmo       Item `json:"elmo"`
	Armadura   Item `json:"armadura"`
	Calca      Item `json:"calca"`
	Luva       Item `json:"luva"`
	Bota       Item `json:"bota"`
	Arma       Item `json:"arma"`
	Escudo     Item `json:"escudo"`
	Acessorio1 Item `json:"acessorio1"`
	Acessorio2 Item `json:"acessorio2"`
	Acessorio3 Item `json:"acessorio3"`
	Acessorio4 Item `json:"acessorio4"`
	Acessorio5 Item `json:"acessorio5"`
	Acessorio6 Item `json:"acessorio6"`
	Montaria   Item `json:"montaria"`
	Capa       Item `json:"capa"`
}

func (e CharacterEquipment) Items() [16]Item {
	return [16]Item{
		e.Rosto, e.Elmo, e.Armadura, e.Calca, e.Luva, e.Bota,
		e.Arma, e.Escudo, e.Acessorio1, e.Acessorio2, e.Acessorio3,
		e.Acessorio4, e.Acessorio5, e.Acessorio6, e.Montaria, e.Capa,
	}
}

type CharacterSpawn struct {
	X uint16 `json:"x"`
	Y uint16 `json:"y"`
}

// CharacterTemplate contem somente o estado autoritativo concedido na criacao.
// Campos efetivos como RuntimeScore nao pertencem ao arquivo.
type CharacterTemplate struct {
	Name         string             `json:"name"`
	Class        byte               `json:"class"`
	Score        Score              `json:"extendedScore"`
	Gold         uint32             `json:"gold"`
	CP           int16              `json:"cp"`
	Exp          uint32             `json:"exp"`
	LearnedSkill uint32             `json:"learnedSkill"`
	Equip        CharacterEquipment `json:"equip"`
	Inventory    []Item             `json:"inventory,omitempty"`
}

type CharacterTemplateFile struct {
	Spawn   CharacterSpawn      `json:"spawn"`
	Classes []CharacterTemplate `json:"classes"`
}

// NewCharacter materializa uma copia independente do template. Atalhos vazios
// usam 0xFF, convencao do client para "nenhuma habilidade neste slot".
func (t CharacterTemplate) NewCharacter(name string, spawn CharacterSpawn) Char {
	extended := t.Score
	ch := Char{
		Name:         name,
		Class:        t.Class,
		X:            spawn.X,
		Y:            spawn.Y,
		Score:        &extended,
		Equip:        t.Equip.Items(),
		CP:           clampCP(t.CP),
		ChaosVersion: ChaosFormatVersion,
		Gold:         t.Gold,
		Exp:          t.Exp,
		LearnedSkill: t.LearnedSkill,
	}
	for i := range ch.ShortSkill {
		ch.ShortSkill[i] = 0xFF
	}
	for i := 0; i < len(t.Inventory) && i < PlayerCarrySlots; i++ {
		ch.Inv[i] = t.Inventory[i]
	}
	return ch
}
