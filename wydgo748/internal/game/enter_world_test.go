package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// enterWorldPacket builds 0x213 with the requested slot.
func enterWorldPacket(slot int) []byte {
	pkt := make([]byte, characterLoginPacketSize)
	binary.LittleEndian.PutUint16(pkt[0:2], characterLoginPacketSize)
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpCharacterLogin)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(slot))
	return pkt
}

func TestEnterWorldFailsClosedWhenCharStateLoadFails(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	w.store = &charStateMemoryStore{loadErr: errors.New("postgres unavailable")}
	w.onEnterWorld(s, enterWorldPacket(0))
	if p.InWorld || p.Char != nil || p.CharSlot != -1 || p.ID != 0 {
		t.Fatalf("character entered without authoritative state: %+v", p)
	}
	if s.QueuedPacketsForTest() != 1 {
		t.Fatalf("failure should send only the notice, packets=%d", s.QueuedPacketsForTest())
	}
}

func newEnterWorldPlayer(t *testing.T, curHP, maxHP uint32) (*World, *Player, *net.Session) {
	t.Helper()
	w := newZoneTestWorld()
	w.store = &craftStore{}
	w.questZones = nil
	w.gameplay = model.DefaultGameplayConfig()

	session := net.NewTestSession(1, 256)
	acc := &model.Account{Name: "EntryTestAccount", Chars: []model.Char{{
		Name:  "EntryTestCharacter",
		Class: 0,
		Score: &model.Score{
			Version: model.ScoreVersion,
			Level:   50, MaxHP: maxHP, CurHP: curHP, MaxMP: 100, CurMP: 100,
			Str: 10, Int: 10, Dex: 10, Con: 10,
		},
	}}}
	p := &Player{Session: session, Account: acc, CharSlot: -1, Visible: map[uint16]struct{}{}}
	w.players[session] = p
	return w, p, session
}

// TestDeadCharacterEntersWithOneHP protects a previously observed
// in-game regression: a dead character logs out with persisted CurHP=0;
// entering with zero HP locks the death pose and blocks input, including
// /restart.
func TestDeadCharacterEntersWithOneHP(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 0, 1000)

	w.onEnterWorld(s, enterWorldPacket(0))

	if !p.InWorld {
		t.Fatal("character should have entered the world")
	}
	if got := playerCurHP(p.Char); got != 1 {
		t.Fatalf("entry HP=%d, want 1", got)
	}
	if !p.DeadAt.IsZero() {
		t.Error("DeadAt should be cleared when reviving on entry")
	}
}

// TestLiveCharacterKeepsHPOnEnter ensures the fix does not change a
// character entering alive.
func TestLiveCharacterKeepsHPOnEnter(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)

	w.onEnterWorld(s, enterWorldPacket(0))

	if got := playerCurHP(p.Char); got != 742 {
		t.Fatalf("entry HP=%d, want to preserve 742", got)
	}
}

func TestEnterWorldUsesPersistedHomeCity(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	p.Account.Chars[0].Score.Merchant = uint32(2 << playerHomeCityShift) // Erion.

	w.onEnterWorld(s, enterWorldPacket(0))

	if !p.InWorld {
		t.Fatal("o personagem deveria ter entrado no mundo")
	}
	if p.X != cityWarZones[2].exitX || p.Y != cityWarZones[2].exitY ||
		p.Char.X != p.X || p.Char.Y != p.Y {
		t.Fatalf("login ignored Erion hometown: player=(%d,%d) char=(%d,%d)",
			p.X, p.Y, p.Char.X, p.Char.Y)
	}
}

func TestLegacyHomeCityBitsDefaultToArmia(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	p.Account.Chars[0].Score.Merchant = 0x1F // only the low six bits.

	w.onEnterWorld(s, enterWorldPacket(0))

	// Armia births spread ±5 tiles around 2091,2100 (plus free-tile search).
	if absInt(int(p.X)-int(cityWarZones[0].exitX)) > armiaSpawnSpread+8 ||
		absInt(int(p.Y)-int(cityWarZones[0].exitY)) > armiaSpawnSpread+8 || !cityWarZones[0].city.contains(p.X, p.Y) {
		t.Fatalf("legacy character did not enter Armia: (%d,%d)", p.X, p.Y)
	}
}

func TestEnterWorldKeepsLearnedSkillCostsDeducted(t *testing.T) {
	for _, tc := range []struct {
		name  string
		bonus uint32
		want  uint32
	}{
		{name: "no permanent bonus", want: 0},
		{name: "preserves Magical Pill bonus", bonus: 9, want: 9},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, s := newEnterWorldPlayer(t, 742, 1000)
			p.Account.Chars[0].Score.Level = 54 // 54 * 3 = 162 points.
			p.Account.Chars[0].LearnedSkill = 1<<0 | 1<<1 | 1<<2 | 1<<16
			p.Account.Chars[0].SkillPointBonus = tc.bonus
			w.skills = map[int]model.SkillDef{
				0:  {Index: 0, SkillPoint: 24},
				1:  {Index: 1, SkillPoint: 48},
				2:  {Index: 2, SkillPoint: 66},
				16: {Index: 16, SkillPoint: 24},
			}

			w.onEnterWorld(s, enterWorldPacket(0))

			if got := p.Char.Score.SkillPts; got != tc.want {
				t.Fatalf("entry skill points=%d, want %d", got, tc.want)
			}
			if p.Char.RuntimeScore == nil || p.Char.RuntimeScore.SkillPts != tc.want {
				t.Fatalf("entry runtime points=%v, want %d", p.Char.RuntimeScore, tc.want)
			}
		})
	}
}

func TestEnterWorldMigratesUnsupportedEquipmentToCarry(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	store := p.Account
	legacy := model.Item{Index: 640, UID: "legacy-necklace"}
	store.Chars[0].Equip[9] = legacy

	w.onEnterWorld(s, enterWorldPacket(0))

	if !p.InWorld || p.Char.Equip[9].Index != 0 || p.Char.Inv[0] != legacy {
		t.Fatalf("misplaced equipment did not migrate to Carry: equip9=%+v inv0=%+v",
			p.Char.Equip[9], p.Char.Inv[0])
	}
	if saves := w.store.(*craftStore).saves; saves != 1 {
		t.Fatalf("migration saved %d times, want 1", saves)
	}
}

func TestEnterWorldRollsBackUnsupportedEquipmentMigrationOnSaveFailure(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	legacy := model.Item{Index: 640, UID: "legacy-necklace"}
	p.Account.Chars[0].Equip[9] = legacy
	w.store = &craftStore{err: errors.New("disk")}

	w.onEnterWorld(s, enterWorldPacket(0))

	if !p.InWorld || p.Char.Equip[9] != legacy || p.Char.Inv[0].Index != 0 {
		t.Fatalf("save failure did not restore equipment: equip9=%+v inv0=%+v",
			p.Char.Equip[9], p.Char.Inv[0])
	}
	if clientEquipProjection(p.Char)[9] != legacy {
		t.Fatal("rollback lost the retained equipment in the client projection")
	}
}

func TestEnterWorldMigratesUnsupportedEquipmentToCargoWhenCarryIsFull(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	legacy := model.Item{Index: 640, UID: "legacy-necklace"}
	p.Account.Chars[0].Equip[9] = legacy
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		p.Account.Chars[0].Inv[slot] = model.Item{Index: 400}
	}

	w.onEnterWorld(s, enterWorldPacket(0))

	if !p.InWorld || p.Char.Equip[9].Index != 0 || p.Account.Cargo[0] != legacy {
		t.Fatalf("misplaced equipment did not migrate to Cargo: equip9=%+v cargo0=%+v",
			p.Char.Equip[9], p.Account.Cargo[0])
	}
}

func TestLoadtestAccountUsesConfiguredNoatumSpawn(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	p.Account.Name = "bot0001"
	w.loadtestAccountPrefix = "bot"
	w.loadtestSpawn = model.CharacterSpawn{X: 1162, Y: 1700}

	w.onEnterWorld(s, enterWorldPacket(0))

	if p.X != 1162 || p.Y != 1700 || p.Char.X != 1162 || p.Char.Y != 1700 {
		t.Fatalf("spawn loadtest=(%d,%d) char=(%d,%d)", p.X, p.Y, p.Char.X, p.Char.Y)
	}
}

func TestOnlyProvisionedLoadtestRangeUsesNoatumSpawn(t *testing.T) {
	for _, tc := range []struct {
		name string
		want bool
	}{
		{"bot0001", true}, {"bot0990", true}, {"bot0991", false},
		{"bot9999", false}, {"botabcd", false}, {"felipe", false},
	} {
		if got := isLoadtestAccountName(tc.name, "bot"); got != tc.want {
			t.Errorf("%s: isLoadtest=%v, want %v", tc.name, got, tc.want)
		}
	}
}

func TestEnterWorldRejectsWhenAllPlayerIDsAreOccupied(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	for id := uint16(1); id < 1000; id++ {
		ownerSession := net.NewTestSession(int64(1000+id), 1)
		owner := &Player{Session: ownerSession, ID: id, InWorld: true,
			X: playerEntryX, Y: playerEntryY}
		w.players[ownerSession] = owner
		w.playersByID[id] = owner
	}
	w.onEnterWorld(s, enterWorldPacket(0))
	if p.InWorld || p.ID != 0 || w.playersByID[999] == nil {
		t.Fatal("entry should be rejected without overwriting the last player")
	}
	if s.QueuedPacketsForTest() == 0 {
		t.Fatal("world-full rejection should notify the client")
	}
}

// TestDeadCharacterRevivedBeforePackets: revive must happen before entry
// packets; otherwise the client receives HP 0 in EnterWorld/CreateMob and
// draws the death pose even after the server corrects its state.
func TestDeadCharacterRevivedBeforePackets(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 0, 1000)
	// Stale DeadAt from an earlier death in the same session.
	p.DeadAt = time.Now()

	w.onEnterWorld(s, enterWorldPacket(0))

	if s.QueuedPacketsForTest() == 0 {
		t.Fatal("entry sequence should have been sent")
	}
	// Authoritative HP is already corrected when packets are sent.
	if got := playerCurHP(p.Char); got != 1 {
		t.Fatalf("HP=%d when building packets, want 1", got)
	}
}
