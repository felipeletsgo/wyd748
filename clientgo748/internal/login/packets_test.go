package login

import (
	"encoding/binary"
	"errors"
	"reflect"
	"testing"

	"wydclient748/internal/protocol"
)

func TestBuildAccountLoginContract(t *testing.T) {
	request := AccountLoginRequest{
		Account:  "account-sixteen!",
		Password: []byte("password-12!"),
		Adapter:  [4]uint32{0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00},
		ID:       0x1234,
		Tick:     0x89ABCDEF,
	}
	packet, err := BuildAccountLogin(request)
	if err != nil {
		t.Fatalf("BuildAccountLogin: %v", err)
	}
	if len(packet) != AccountLoginPacketSize {
		t.Fatalf("size=%d want=%d", len(packet), AccountLoginPacketSize)
	}
	if got := binary.LittleEndian.Uint16(packet[4:6]); got != OpcodeAccountLogin {
		t.Fatalf("opcode=0x%03X", got)
	}
	if got := binary.LittleEndian.Uint16(packet[6:8]); got != request.ID {
		t.Fatalf("id=0x%04X", got)
	}
	if got := binary.LittleEndian.Uint32(packet[8:12]); got != request.Tick {
		t.Fatalf("tick=0x%08X", got)
	}
	if got := string(packet[accountOffset : accountOffset+accountNameSize]); got != request.Account {
		t.Fatalf("account=%q", got)
	}
	if got := string(packet[passwordOffset : passwordOffset+passwordSize]); got != string(request.Password) {
		t.Fatalf("password=%q", got)
	}
	if got := binary.LittleEndian.Uint32(packet[clientVersionOffset : clientVersionOffset+4]); got != ClientVersion {
		t.Fatalf("version=%d", got)
	}
	if got := binary.LittleEndian.Uint32(packet[dbNeedSaveOffset : dbNeedSaveOffset+4]); got != 0 {
		t.Fatalf("DBNeedSave=%d", got)
	}
	for i, value := range packet[loginZeroOffset : loginZeroOffset+loginZeroSize] {
		if value != 0 {
			t.Fatalf("reserved[%d]=%d", i, value)
		}
	}
	for i, want := range request.Adapter {
		if got := binary.LittleEndian.Uint32(packet[adapterOffset+i*4 : adapterOffset+(i+1)*4]); got != want {
			t.Fatalf("adapter[%d]=0x%08X want=0x%08X", i, got, want)
		}
	}
}

func TestBuildAccountLoginRejectsInvalidFixedText(t *testing.T) {
	valid := AccountLoginRequest{Account: "account", Password: []byte("password")}
	tests := []struct {
		name   string
		mutate func(*AccountLoginRequest)
	}{
		{name: "empty account", mutate: func(r *AccountLoginRequest) { r.Account = "" }},
		{name: "long account", mutate: func(r *AccountLoginRequest) { r.Account = "12345678901234567" }},
		{name: "nul account", mutate: func(r *AccountLoginRequest) { r.Account = "acc\x00ount" }},
		{name: "empty password", mutate: func(r *AccountLoginRequest) { r.Password = nil }},
		{name: "long password", mutate: func(r *AccountLoginRequest) { r.Password = []byte("1234567890123") }},
		{name: "nul password", mutate: func(r *AccountLoginRequest) { r.Password = []byte("pass\x00word") }},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			request := valid
			test.mutate(&request)
			if _, err := BuildAccountLogin(request); !errors.Is(err, ErrInvalidFixedText) {
				t.Fatalf("error=%v want=%v", err, ErrInvalidFixedText)
			}
		})
	}
}

func TestBuildCharacterLoginContract(t *testing.T) {
	var secret [secretCodeSize]byte
	for i := range secret {
		secret[i] = byte(0xA0 + i)
	}
	for slot := int32(0); slot < SelectionCharacterCount; slot++ {
		packet, err := BuildCharacterLogin(CharacterLoginRequest{
			Slot: slot, Force: -7, SecretCode: secret, ID: 0x2244, Tick: 0x12345678,
		})
		if err != nil {
			t.Fatalf("slot %d: %v", slot, err)
		}
		if len(packet) != CharacterLoginPacketSize ||
			binary.LittleEndian.Uint16(packet[4:6]) != OpcodeCharacterLogin ||
			int32(binary.LittleEndian.Uint32(packet[characterSlotOffset:characterSlotOffset+4])) != slot ||
			int32(binary.LittleEndian.Uint32(packet[characterForceOffset:characterForceOffset+4])) != -7 ||
			!reflect.DeepEqual(packet[characterSecretOffset:characterSecretOffset+secretCodeSize], secret[:]) {
			t.Fatalf("invalid character login for slot %d: % X", slot, packet)
		}
	}
	for _, slot := range []int32{-1, SelectionCharacterCount, 99} {
		if _, err := BuildCharacterLogin(CharacterLoginRequest{Slot: slot}); !errors.Is(err, ErrInvalidSlot) {
			t.Fatalf("slot %d error=%v", slot, err)
		}
	}
}

func TestBuildCharacterLogoutContract(t *testing.T) {
	packet := BuildCharacterLogout(0x1234, 0x55667788)
	if len(packet) != CharacterLogoutPacketSize ||
		binary.LittleEndian.Uint16(packet[4:6]) != OpcodeCharacterLogout ||
		binary.LittleEndian.Uint16(packet[6:8]) != 0x1234 ||
		binary.LittleEndian.Uint32(packet[8:12]) != 0x55667788 {
		t.Fatalf("logout packet: % X", packet)
	}
}

func TestParseScoreAndItemContracts(t *testing.T) {
	rawScore := make([]byte, ScoreSize)
	for i := 0; i < 35; i++ {
		binary.LittleEndian.PutUint32(rawScore[i*4:(i+1)*4], uint32(1000+i))
	}
	score, err := ParseScore(rawScore)
	if err != nil {
		t.Fatalf("ParseScore: %v", err)
	}
	if score.Version != 1000 || score.Level != 1001 || score.MagicAttack != 1003 ||
		score.Accuracy != 1013 || score.Evasion != 1014 || score.Critical != 1016 ||
		score.SkillPts != 1028 || score.Mastery != [4]uint32{1029, 1030, 1031, 1032} ||
		score.AttackRun != 1033 || score.Merchant != 1034 {
		t.Fatalf("score mapping=%+v", score)
	}
	for _, size := range []int{ScoreSize - 1, ScoreSize + 1} {
		if _, err := ParseScore(make([]byte, size)); !errors.Is(err, ErrInvalidPacketSize) {
			t.Fatalf("score size %d error=%v", size, err)
		}
	}

	rawItem := []byte{0x34, 0x12, 1, 2, 3, 4, 5, 6}
	item, err := ParseItem(rawItem)
	if err != nil || item.Index != 0x1234 || item.Effects != [6]byte{1, 2, 3, 4, 5, 6} {
		t.Fatalf("item=%+v error=%v", item, err)
	}
	if _, err := ParseItem(rawItem[:7]); !errors.Is(err, ErrInvalidPacketSize) {
		t.Fatalf("short item error=%v", err)
	}
}

func TestParseCharacterListContract(t *testing.T) {
	packet := characterListFixture()
	list, err := ParseCharacterList(packet)
	if err != nil {
		t.Fatalf("ParseCharacterList: %v", err)
	}
	if list.SecretCode[0] != 0x80 || list.SecretCode[15] != 0x8F ||
		list.CargoGold != 0xA1B2C3D4 || list.AccountName != "account-name" ||
		list.SSN1 != 0x11223344 || list.SSN2 != 0x55667788 {
		t.Fatalf("list header=%+v", list)
	}
	character := list.Characters[2]
	if character.HomeTownX != 2102 || character.HomeTownY != 2202 || character.Name != "CharacterTwo" ||
		character.Score.Version != 2000 || character.Score.SkillPts != 2028 ||
		character.Equip[17].Index != 3017 || character.Equip[17].Effects[5] != 24 ||
		character.Guild != 0x4402 || character.Coin != 0x55000002 || character.Exp != 0x6677889900000002 {
		t.Fatalf("character=%+v", character)
	}
	if list.Characters[0].Occupied() {
		t.Fatal("empty slot reported as occupied")
	}
	if list.Cargo[127].Index != 4127 || list.Cargo[127].Effects[0] != 127 {
		t.Fatalf("cargo[127]=%+v", list.Cargo[127])
	}
}

func TestParseEnterWorldContract(t *testing.T) {
	packet := enterWorldFixture(2, 0x3456)
	snapshot, err := ParseEnterWorld(packet)
	if err != nil {
		t.Fatalf("ParseEnterWorld: %v", err)
	}
	if snapshot.PosX != 2450 || snapshot.PosY != 1777 || snapshot.Slot != 2 ||
		snapshot.ClientID != 0x3456 || snapshot.Weather != 9 || snapshot.Hold != 0x10203040 {
		t.Fatalf("snapshot header=%+v", snapshot)
	}
	if snapshot.Mob.Name != "WorldHero" || snapshot.Mob.Chaos != 7 || snapshot.Mob.Clan != 8 ||
		snapshot.Mob.Guild != 0x1234 || snapshot.Mob.Class != 2 || snapshot.Mob.Quest != 0x3344 ||
		snapshot.Mob.Coin != 7654321 || snapshot.Mob.Exp != 0x1122334455667788 ||
		snapshot.Mob.HomeTownX != 2100 || snapshot.Mob.HomeTownY != 2101 ||
		snapshot.Mob.BaseScore.Attack != 3002 || snapshot.Mob.CurrentScore.SkillPts != 4028 ||
		snapshot.Mob.Equip[17].Index != 5017 || snapshot.Mob.Carry[63].Index != 6063 ||
		snapshot.Mob.LearnedSkill != 0xAABBCCDD || snapshot.Mob.SecondaryLearnedSkill != 0x11224488 ||
		snapshot.Mob.ShortSkill != [4]byte{1, 3, 5, 7} || snapshot.Mob.GuildLevel != 6 ||
		snapshot.Mob.CurrentKill != 123 || snapshot.Mob.TotalKill != 456 {
		t.Fatalf("mob=%+v", snapshot.Mob)
	}
	if snapshot.ShortSkill[15] != 15 || snapshot.ExtensionData[6] != 0x70000006 ||
		snapshot.Affects[15] != (Affect{Type: 25, Level: 35, Value: -115, Time: 1015}) ||
		snapshot.ExtensionReserved[679] != 0xEE {
		t.Fatalf("extension=%+v affect=%+v tail=%d", snapshot.ExtensionData, snapshot.Affects[15], snapshot.ExtensionReserved[679])
	}
}

func TestPacketParsersRejectDivergentFrames(t *testing.T) {
	tests := []struct {
		name string
		edit func(protocol.Packet) protocol.Packet
		want error
	}{
		{name: "truncated", edit: func(p protocol.Packet) protocol.Packet {
			p.Raw = p.Raw[:len(p.Raw)-1]
			p.Body = p.Raw[protocol.HeaderSize:]
			return p
		}, want: ErrInvalidPacketSize},
		{name: "oversized", edit: func(p protocol.Packet) protocol.Packet {
			p.Raw = append(p.Raw, 0)
			p.Body = p.Raw[protocol.HeaderSize:]
			return p
		}, want: ErrInvalidPacketSize},
		{name: "header size", edit: func(p protocol.Packet) protocol.Packet { p.Header.Size--; return p }, want: ErrInvalidPacketSize},
		{name: "raw size", edit: func(p protocol.Packet) protocol.Packet {
			binary.LittleEndian.PutUint16(p.Raw[:2], p.Header.Size-1)
			return p
		}, want: ErrInvalidPacketSize},
		{name: "header opcode", edit: func(p protocol.Packet) protocol.Packet { p.Header.Type++; return p }, want: ErrUnexpectedOpcode},
		{name: "raw opcode", edit: func(p protocol.Packet) protocol.Packet {
			binary.LittleEndian.PutUint16(p.Raw[4:6], p.Header.Type+1)
			return p
		}, want: ErrUnexpectedOpcode},
		{name: "header fields", edit: func(p protocol.Packet) protocol.Packet { p.Header.ID++; return p }, want: nil},
		{name: "body copy", edit: func(p protocol.Packet) protocol.Packet {
			p.Body = append([]byte(nil), p.Body...)
			p.Body[0] ^= 0xFF
			return p
		}, want: nil},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			packet := test.edit(characterListFixture())
			_, err := ParseCharacterList(packet)
			if err == nil {
				t.Fatal("expected rejection")
			}
			if test.want != nil && !errors.Is(err, test.want) {
				t.Fatalf("error=%v want=%v", err, test.want)
			}
		})
	}

	invalidSlot := enterWorldFixture(SelectionCharacterCount, 1)
	if _, err := ParseEnterWorld(invalidSlot); !errors.Is(err, ErrInvalidSlot) {
		t.Fatalf("invalid world slot error=%v", err)
	}
	logout := decodedPacket(OpcodeCharacterLogoutConfirmed, CharacterLogoutConfirmedPacketSize, 9, 10)
	if err := ParseCharacterLogoutConfirmation(logout); err != nil {
		t.Fatalf("valid logout: %v", err)
	}
	logout.Raw = append(logout.Raw, 0)
	logout.Body = logout.Raw[protocol.HeaderSize:]
	if err := ParseCharacterLogoutConfirmation(logout); !errors.Is(err, ErrInvalidPacketSize) {
		t.Fatalf("oversized logout error=%v", err)
	}
}

func decodedPacket(opcode uint16, size int, id uint16, tick uint32) protocol.Packet {
	raw := make([]byte, size)
	binary.LittleEndian.PutUint16(raw[0:2], uint16(size))
	raw[2] = 0x5A
	raw[3] = 0xA5
	binary.LittleEndian.PutUint16(raw[4:6], opcode)
	binary.LittleEndian.PutUint16(raw[6:8], id)
	binary.LittleEndian.PutUint32(raw[8:12], tick)
	return protocol.Packet{
		Header: protocol.Header{Size: uint16(size), KeyWord: raw[2], CheckSum: raw[3], Type: opcode, ID: id, Tick: tick},
		Raw:    raw, Body: raw[protocol.HeaderSize:],
	}
}

func characterListFixture() protocol.Packet {
	packet := decodedPacket(OpcodeCharacterList, CharacterListPacketSize, 0, 77)
	raw := packet.Raw
	for i := 0; i < secretCodeSize; i++ {
		raw[secretCodeOffset+i] = byte(0x80 + i)
	}
	selection := raw[selectionOffset : selectionOffset+SelectionCharacterSize]
	for slot := 0; slot < SelectionCharacterCount; slot++ {
		binary.LittleEndian.PutUint16(selection[slot*2:slot*2+2], uint16(2100+slot))
		binary.LittleEndian.PutUint16(selection[8+slot*2:10+slot*2], uint16(2200+slot))
		if slot > 0 {
			copy(selection[16+slot*characterNameSize:], []byte([]string{"", "CharacterOne", "CharacterTwo", "CharacterThree"}[slot]))
		}
		putScoreWords(selection[80+slot*ScoreSize:80+(slot+1)*ScoreSize], uint32(slot*1000))
		for equip := 0; equip < SelectionEquipmentCount; equip++ {
			offset := 640 + (slot*SelectionEquipmentCount+equip)*ItemSize
			putItem(selection[offset:offset+ItemSize], uint16(1000+slot*1000+equip), byte(slot+equip))
		}
		binary.LittleEndian.PutUint16(selection[1216+slot*2:1218+slot*2], uint16(0x4400+slot))
		binary.LittleEndian.PutUint32(selection[1224+slot*4:1228+slot*4], uint32(0x55000000+slot))
		binary.LittleEndian.PutUint64(selection[1240+slot*8:1248+slot*8], uint64(0x6677889900000000+slot))
	}
	for i := 0; i < CargoItemCount; i++ {
		putItem(raw[cargoOffset+i*ItemSize:cargoOffset+(i+1)*ItemSize], uint16(4000+i), byte(i))
	}
	binary.LittleEndian.PutUint32(raw[cargoGoldOffset:cargoGoldOffset+4], 0xA1B2C3D4)
	copy(raw[accountCopyOffset:accountCopyOffset+accountNameSize], "account-name")
	binary.LittleEndian.PutUint32(raw[ssn1Offset:ssn1Offset+4], 0x11223344)
	binary.LittleEndian.PutUint32(raw[ssn2Offset:ssn2Offset+4], 0x55667788)
	return packet
}

func enterWorldFixture(slot uint16, clientID uint16) protocol.Packet {
	packet := decodedPacket(OpcodeEnterWorld, EnterWorldPacketSize, clientID, 88)
	raw := packet.Raw
	binary.LittleEndian.PutUint16(raw[worldPositionOffset:worldPositionOffset+2], 2450)
	binary.LittleEndian.PutUint16(raw[worldPositionOffset+2:worldPositionOffset+4], 1777)
	mob := raw[worldMobOffset : worldMobOffset+MobSize]
	copy(mob[:mobNameSize], "WorldHero")
	mob[12] = 7
	mob[16] = 8
	binary.LittleEndian.PutUint16(mob[18:20], 0x1234)
	mob[20] = 2
	binary.LittleEndian.PutUint16(mob[22:24], 0x3344)
	binary.LittleEndian.PutUint32(mob[24:28], 7654321)
	binary.LittleEndian.PutUint64(mob[32:40], 0x1122334455667788)
	binary.LittleEndian.PutUint16(mob[40:42], 2100)
	binary.LittleEndian.PutUint16(mob[42:44], 2101)
	putScoreWords(mob[44:184], 3000)
	putScoreWords(mob[184:324], 4000)
	for i := 0; i < MobEquipmentCount; i++ {
		putItem(mob[324+i*ItemSize:324+(i+1)*ItemSize], uint16(5000+i), byte(i))
	}
	for i := 0; i < MobCarryCount; i++ {
		putItem(mob[468+i*ItemSize:468+(i+1)*ItemSize], uint16(6000+i), byte(i))
	}
	binary.LittleEndian.PutUint32(mob[980:984], 0xAABBCCDD)
	binary.LittleEndian.PutUint32(mob[984:988], 0x11224488)
	copy(mob[988:992], []byte{1, 3, 5, 7})
	mob[992] = 6
	binary.LittleEndian.PutUint16(mob[1220:1222], 123)
	binary.LittleEndian.PutUint16(mob[1222:1224], 456)
	binary.LittleEndian.PutUint16(raw[worldSlotOffset:worldSlotOffset+2], slot)
	binary.LittleEndian.PutUint16(raw[worldClientIDOffset:worldClientIDOffset+2], clientID)
	binary.LittleEndian.PutUint16(raw[worldWeatherOffset:worldWeatherOffset+2], 9)
	for i := 0; i < ShortSkillCount; i++ {
		raw[worldShortSkillOffset+i] = byte(i)
	}
	binary.LittleEndian.PutUint32(raw[worldExtOffset:worldExtOffset+4], 0x10203040)
	for i := 0; i < 7; i++ {
		binary.LittleEndian.PutUint32(raw[worldExtOffset+4+i*4:worldExtOffset+8+i*4], uint32(0x70000000+i))
	}
	for i := 0; i < AffectCount; i++ {
		offset := worldAffectOffset + i*AffectSize
		raw[offset] = byte(10 + i)
		raw[offset+1] = byte(20 + i)
		binary.LittleEndian.PutUint16(raw[offset+2:offset+4], uint16(int16(-100-i)))
		binary.LittleEndian.PutUint32(raw[offset+4:offset+8], uint32(1000+i))
	}
	for i := worldReservedOffset; i < EnterWorldPacketSize; i++ {
		raw[i] = 0xEE
	}
	return packet
}

func putScoreWords(raw []byte, base uint32) {
	for i := 0; i < 35; i++ {
		binary.LittleEndian.PutUint32(raw[i*4:(i+1)*4], base+uint32(i))
	}
}

func putItem(raw []byte, index uint16, seed byte) {
	binary.LittleEndian.PutUint16(raw[:2], index)
	for i := 0; i < 6; i++ {
		raw[2+i] = seed + byte(i)
	}
}
