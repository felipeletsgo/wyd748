package login

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"

	"wydclient748/internal/protocol"
)

var (
	ErrInvalidPacketSize = errors.New("login: invalid packet size")
	ErrUnexpectedOpcode  = errors.New("login: unexpected opcode")
	ErrInvalidFixedText  = errors.New("login: invalid fixed text")
	ErrInvalidSlot       = errors.New("login: invalid character slot")
)

const (
	accountOffset       = 12
	passwordOffset      = 28
	clientVersionOffset = 40
	dbNeedSaveOffset    = 44
	loginZeroOffset     = 48
	loginZeroSize       = 52
	adapterOffset       = 100

	secretCodeOffset  = 12
	selectionOffset   = 32
	cargoOffset       = 1304
	cargoGoldOffset   = 2328
	accountCopyOffset = 2332
	ssn1Offset        = 2348
	ssn2Offset        = 2352

	characterSlotOffset   = 12
	characterForceOffset  = 16
	characterSecretOffset = 20

	worldPositionOffset   = 12
	worldMobOffset        = 16
	worldSlotOffset       = 1240
	worldClientIDOffset   = 1242
	worldWeatherOffset    = 1244
	worldShortSkillOffset = 1246
	worldExtOffset        = 1264
	worldAffectOffset     = 1296
	worldReservedOffset   = 1424
)

// BuildAccountLogin materializa o 0x20D em claro. Size, KeyWord e CheckSum são
// deliberadamente finalizados por protocol.Encode no momento do envio.
func BuildAccountLogin(request AccountLoginRequest) ([]byte, error) {
	account := []byte(request.Account)
	if err := validateFixedText("account", account, accountNameSize); err != nil {
		return nil, err
	}
	if err := validateFixedText("password", request.Password, passwordSize); err != nil {
		return nil, err
	}

	packet := protocol.NewPacket(OpcodeAccountLogin, request.ID, request.Tick,
		make([]byte, AccountLoginPacketSize-protocol.HeaderSize))
	copy(packet[accountOffset:accountOffset+accountNameSize], account)
	copy(packet[passwordOffset:passwordOffset+passwordSize], request.Password)
	binary.LittleEndian.PutUint32(packet[clientVersionOffset:clientVersionOffset+4], ClientVersion)
	binary.LittleEndian.PutUint32(packet[dbNeedSaveOffset:dbNeedSaveOffset+4], 0)
	clear(packet[loginZeroOffset : loginZeroOffset+loginZeroSize])
	for i, value := range request.Adapter {
		binary.LittleEndian.PutUint32(packet[adapterOffset+i*4:adapterOffset+(i+1)*4], value)
	}
	return packet, nil
}

// BuildCharacterLogin materializa o 0x213 somente para slots nativos 0..3.
func BuildCharacterLogin(request CharacterLoginRequest) ([]byte, error) {
	if request.Slot < 0 || request.Slot >= SelectionCharacterCount {
		return nil, fmt.Errorf("%w: %d", ErrInvalidSlot, request.Slot)
	}
	packet := protocol.NewPacket(OpcodeCharacterLogin, request.ID, request.Tick,
		make([]byte, CharacterLoginPacketSize-protocol.HeaderSize))
	binary.LittleEndian.PutUint32(packet[characterSlotOffset:characterSlotOffset+4], uint32(request.Slot))
	binary.LittleEndian.PutUint32(packet[characterForceOffset:characterForceOffset+4], uint32(request.Force))
	copy(packet[characterSecretOffset:characterSecretOffset+secretCodeSize], request.SecretCode[:])
	return packet, nil
}

// BuildCharacterLogout cria o pedido 0x215 sem payload. O ID identifica o
// personagem ativo e é obtido do snapshot aceito, nunca da UI.
func BuildCharacterLogout(id uint16, tick uint32) []byte {
	return protocol.NewPacket(OpcodeCharacterLogout, id, tick, nil)
}

// ParseScore converte exatamente 140 bytes. A função não aceita prefixos nem
// buffers maiores, evitando que um offset incorreto pareça válido.
func ParseScore(raw []byte) (Score, error) {
	if len(raw) != ScoreSize {
		return Score{}, fmt.Errorf("%w: score got %d want %d", ErrInvalidPacketSize, len(raw), ScoreSize)
	}
	var words [35]uint32
	for i := range words {
		words[i] = binary.LittleEndian.Uint32(raw[i*4 : (i+1)*4])
	}
	return Score{
		Version: words[0], Level: words[1], Attack: words[2], MagicAttack: words[3],
		Defense: words[4], MaxHP: words[5], MaxMP: words[6], CurHP: words[7],
		CurMP: words[8], Str: words[9], Int: words[10], Dex: words[11], Con: words[12],
		Accuracy: words[13], Evasion: words[14], Parry: words[15], Critical: words[16],
		Range: words[17], ResistFire: words[18], ResistIce: words[19], ResistHoly: words[20],
		ResistThunder: words[21], SaveMana: words[22], MagicAmp: words[23],
		RegenHP: words[24], RegenMP: words[25], StatusPts: words[26], MasterPts: words[27],
		SkillPts: words[28], Mastery: [4]uint32{words[29], words[30], words[31], words[32]},
		AttackRun: words[33], Merchant: words[34],
	}, nil
}

// ParseItem converte exatamente os oito bytes visíveis de STRUCT_ITEM.
func ParseItem(raw []byte) (Item, error) {
	if len(raw) != ItemSize {
		return Item{}, fmt.Errorf("%w: item got %d want %d", ErrInvalidPacketSize, len(raw), ItemSize)
	}
	var item Item
	item.Index = binary.LittleEndian.Uint16(raw[0:2])
	copy(item.Effects[:], raw[2:8])
	return item, nil
}

// ParseCharacterList valida o frame completo antes de construir o agregado.
func ParseCharacterList(packet protocol.Packet) (CharacterList, error) {
	if err := validatePacket(packet, OpcodeCharacterList, CharacterListPacketSize); err != nil {
		return CharacterList{}, err
	}
	raw := packet.Raw
	var list CharacterList
	copy(list.SecretCode[:], raw[secretCodeOffset:secretCodeOffset+secretCodeSize])

	selection := raw[selectionOffset : selectionOffset+SelectionCharacterSize]
	for slot := 0; slot < SelectionCharacterCount; slot++ {
		character := &list.Characters[slot]
		character.HomeTownX = binary.LittleEndian.Uint16(selection[slot*2 : slot*2+2])
		character.HomeTownY = binary.LittleEndian.Uint16(selection[8+slot*2 : 10+slot*2])
		character.Name = fixedString(selection[16+slot*characterNameSize : 16+(slot+1)*characterNameSize])
		score, err := ParseScore(selection[80+slot*ScoreSize : 80+(slot+1)*ScoreSize])
		if err != nil {
			return CharacterList{}, fmt.Errorf("character %d score: %w", slot, err)
		}
		character.Score = score
		for equipSlot := 0; equipSlot < SelectionEquipmentCount; equipSlot++ {
			offset := 640 + (slot*SelectionEquipmentCount+equipSlot)*ItemSize
			item, err := ParseItem(selection[offset : offset+ItemSize])
			if err != nil {
				return CharacterList{}, fmt.Errorf("character %d equip %d: %w", slot, equipSlot, err)
			}
			character.Equip[equipSlot] = item
		}
		character.Guild = binary.LittleEndian.Uint16(selection[1216+slot*2 : 1218+slot*2])
		character.Coin = binary.LittleEndian.Uint32(selection[1224+slot*4 : 1228+slot*4])
		character.Exp = binary.LittleEndian.Uint64(selection[1240+slot*8 : 1248+slot*8])
	}
	for i := range list.Cargo {
		item, err := ParseItem(raw[cargoOffset+i*ItemSize : cargoOffset+(i+1)*ItemSize])
		if err != nil {
			return CharacterList{}, fmt.Errorf("cargo %d: %w", i, err)
		}
		list.Cargo[i] = item
	}
	list.CargoGold = binary.LittleEndian.Uint32(raw[cargoGoldOffset : cargoGoldOffset+4])
	list.AccountName = fixedString(raw[accountCopyOffset : accountCopyOffset+accountNameSize])
	list.SSN1 = binary.LittleEndian.Uint32(raw[ssn1Offset : ssn1Offset+4])
	list.SSN2 = binary.LittleEndian.Uint32(raw[ssn2Offset : ssn2Offset+4])
	return list, nil
}

// ParseEnterWorld valida e materializa o snapshot 0x114 sem publicar dados
// parciais. O slot externo é validado porque governa o lifecycle de relogin.
func ParseEnterWorld(packet protocol.Packet) (WorldSnapshot, error) {
	if err := validatePacket(packet, OpcodeEnterWorld, EnterWorldPacketSize); err != nil {
		return WorldSnapshot{}, err
	}
	raw := packet.Raw
	mob, err := parseMob(raw[worldMobOffset : worldMobOffset+MobSize])
	if err != nil {
		return WorldSnapshot{}, err
	}
	var snapshot WorldSnapshot
	snapshot.PosX = binary.LittleEndian.Uint16(raw[worldPositionOffset : worldPositionOffset+2])
	snapshot.PosY = binary.LittleEndian.Uint16(raw[worldPositionOffset+2 : worldPositionOffset+4])
	snapshot.Mob = mob
	snapshot.Slot = binary.LittleEndian.Uint16(raw[worldSlotOffset : worldSlotOffset+2])
	if snapshot.Slot >= SelectionCharacterCount {
		return WorldSnapshot{}, fmt.Errorf("%w: %d", ErrInvalidSlot, snapshot.Slot)
	}
	snapshot.ClientID = binary.LittleEndian.Uint16(raw[worldClientIDOffset : worldClientIDOffset+2])
	snapshot.Weather = binary.LittleEndian.Uint16(raw[worldWeatherOffset : worldWeatherOffset+2])
	copy(snapshot.ShortSkill[:], raw[worldShortSkillOffset:worldShortSkillOffset+ShortSkillCount])
	snapshot.Hold = binary.LittleEndian.Uint32(raw[worldExtOffset : worldExtOffset+4])
	for i := range snapshot.ExtensionData {
		offset := worldExtOffset + 4 + i*4
		snapshot.ExtensionData[i] = binary.LittleEndian.Uint32(raw[offset : offset+4])
	}
	for i := range snapshot.Affects {
		offset := worldAffectOffset + i*AffectSize
		snapshot.Affects[i] = parseAffect(raw[offset : offset+AffectSize])
	}
	copy(snapshot.ExtensionReserved[:], raw[worldReservedOffset:EnterWorldPacketSize])
	return snapshot, nil
}

// ParseCharacterLogoutConfirmation aceita somente o header vazio 0x116.
func ParseCharacterLogoutConfirmation(packet protocol.Packet) error {
	return validatePacket(packet, OpcodeCharacterLogoutConfirmed, CharacterLogoutConfirmedPacketSize)
}

func parseMob(raw []byte) (Mob, error) {
	if len(raw) != MobSize {
		return Mob{}, fmt.Errorf("%w: mob got %d want %d", ErrInvalidPacketSize, len(raw), MobSize)
	}
	var mob Mob
	mob.Name = fixedString(raw[:mobNameSize])
	mob.Chaos = raw[12]
	copy(mob.NameReserved[:], raw[13:16])
	mob.Clan = raw[16]
	mob.AlignmentReserved = raw[17]
	mob.Guild = binary.LittleEndian.Uint16(raw[18:20])
	mob.Class = raw[20]
	mob.Reserved = raw[21]
	mob.Quest = binary.LittleEndian.Uint16(raw[22:24])
	mob.Coin = binary.LittleEndian.Uint32(raw[24:28])
	copy(mob.PreExpPadding[:], raw[28:32])
	mob.Exp = binary.LittleEndian.Uint64(raw[32:40])
	mob.HomeTownX = binary.LittleEndian.Uint16(raw[40:42])
	mob.HomeTownY = binary.LittleEndian.Uint16(raw[42:44])
	baseScore, err := ParseScore(raw[44:184])
	if err != nil {
		return Mob{}, fmt.Errorf("base score: %w", err)
	}
	mob.BaseScore = baseScore
	currentScore, err := ParseScore(raw[184:324])
	if err != nil {
		return Mob{}, fmt.Errorf("current score: %w", err)
	}
	mob.CurrentScore = currentScore
	for i := range mob.Equip {
		item, err := ParseItem(raw[324+i*ItemSize : 324+(i+1)*ItemSize])
		if err != nil {
			return Mob{}, fmt.Errorf("equip %d: %w", i, err)
		}
		mob.Equip[i] = item
	}
	for i := range mob.Carry {
		item, err := ParseItem(raw[468+i*ItemSize : 468+(i+1)*ItemSize])
		if err != nil {
			return Mob{}, fmt.Errorf("carry %d: %w", i, err)
		}
		mob.Carry[i] = item
	}
	mob.LearnedSkill = binary.LittleEndian.Uint32(raw[980:984])
	mob.SecondaryLearnedSkill = binary.LittleEndian.Uint32(raw[984:988])
	copy(mob.ShortSkill[:], raw[988:992])
	mob.GuildLevel = raw[992]
	copy(mob.ReservedTail[:], raw[993:1220])
	mob.CurrentKill = binary.LittleEndian.Uint16(raw[1220:1222])
	mob.TotalKill = binary.LittleEndian.Uint16(raw[1222:1224])
	return mob, nil
}

func parseAffect(raw []byte) Affect {
	return Affect{
		Type: raw[0], Level: raw[1],
		Value: int16(binary.LittleEndian.Uint16(raw[2:4])),
		Time:  binary.LittleEndian.Uint32(raw[4:8]),
	}
}

func validatePacket(packet protocol.Packet, opcode uint16, size int) error {
	if len(packet.Raw) != size || len(packet.Body) != size-protocol.HeaderSize ||
		packet.Header.Size != uint16(size) {
		return fmt.Errorf("%w: opcode 0x%03X got raw=%d body=%d header=%d want=%d",
			ErrInvalidPacketSize, opcode, len(packet.Raw), len(packet.Body), packet.Header.Size, size)
	}
	if binary.LittleEndian.Uint16(packet.Raw[0:2]) != uint16(size) {
		return fmt.Errorf("%w: raw size", ErrInvalidPacketSize)
	}
	rawOpcode := binary.LittleEndian.Uint16(packet.Raw[4:6])
	if packet.Header.Type != opcode || rawOpcode != opcode {
		return fmt.Errorf("%w: got header=0x%03X raw=0x%03X want=0x%03X",
			ErrUnexpectedOpcode, packet.Header.Type, rawOpcode, opcode)
	}
	if packet.Header.KeyWord != packet.Raw[2] || packet.Header.CheckSum != packet.Raw[3] ||
		packet.Header.ID != binary.LittleEndian.Uint16(packet.Raw[6:8]) ||
		packet.Header.Tick != binary.LittleEndian.Uint32(packet.Raw[8:12]) ||
		!bytes.Equal(packet.Body, packet.Raw[protocol.HeaderSize:]) {
		return fmt.Errorf("login: inconsistent decoded packet")
	}
	return nil
}

func validateFixedText(name string, value []byte, size int) error {
	if len(value) == 0 || len(value) > size || bytes.IndexByte(value, 0) >= 0 {
		return fmt.Errorf("%w: %s length=%d capacity=%d", ErrInvalidFixedText, name, len(value), size)
	}
	return nil
}

func fixedString(raw []byte) string {
	if end := bytes.IndexByte(raw, 0); end >= 0 {
		raw = raw[:end]
	}
	return string(raw)
}
