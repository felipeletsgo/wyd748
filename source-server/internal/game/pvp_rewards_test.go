package game

import (
	"encoding/binary"
	"testing"
)

func TestPvPKillDoesNotTransferExperienceOrGold(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Killer", 2100, 2100)
	victim, _ := networkedTestPlayer(2, "Victim", 2101, 2100)
	killer.Char.Exp, killer.Char.Gold = 123_456, 700
	victim.Char.Exp, victim.Char.Gold = 9_999_999, 800_000
	wantDebt := heldExperienceDeathDebt(victim.Char)
	w, _ := guildFlowWorld(killer, victim)

	w.applyPvPKills(killer, victim)

	if killer.Char.Exp != 123_456 || killer.Char.Gold != 700 {
		t.Fatalf("killer recebeu recompensa PvP: exp=%d gold=%d", killer.Char.Exp, killer.Char.Gold)
	}
	if victim.Char.Exp != 9_999_999 || victim.Char.Gold != 800_000 {
		t.Fatalf("economia da vitima foi alterada: exp=%d gold=%d", victim.Char.Exp, victim.Char.Gold)
	}
	if victim.Char.Hold != wantDebt || wantDebt == 0 {
		t.Fatalf("morte PvP criou Hold=%d, esperado %d", victim.Char.Hold, wantDebt)
	}
}

func TestPlayerDeathPacketCarriesRecipientOwnExperience(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Killer", 2100, 2100)
	victim, _ := networkedTestPlayer(2, "Victim", 2101, 2100)
	observer, _ := networkedTestPlayer(3, "Observer", 2102, 2100)
	killer.Char.Exp = 123_456
	killer.Char.Hold = 111
	victim.Char.Exp = 9_999_999
	victim.Char.Hold = 222
	observer.Char.Exp = 444_555
	observer.Char.Hold = 333

	for _, recipient := range []*Player{killer, victim, observer} {
		pkt := playerDeathPacket(recipient, victim, killer.ID)
		if got := binary.LittleEndian.Uint32(pkt[20:24]); got != recipient.Char.Exp {
			t.Fatalf("destinatario %q recebeu exp=%d, esperado seu total=%d",
				recipient.Char.Name, got, recipient.Char.Exp)
		}
		if got := binary.LittleEndian.Uint32(pkt[12:16]); got != recipient.Char.Hold {
			t.Fatalf("destinatario %q recebeu Hold=%d, esperado %d",
				recipient.Char.Name, got, recipient.Char.Hold)
		}
		if killed := binary.LittleEndian.Uint16(pkt[16:18]); killed != victim.ID {
			t.Fatalf("KilledMob=%d, esperado %d", killed, victim.ID)
		}
	}
}

func TestMobDeathPacketCarriesRecipientOwnProgress(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Killer", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	killer.Char.Exp, killer.Char.Hold = 123_456, 111
	observer.Char.Exp, observer.Char.Hold = 654_321, 222

	for _, recipient := range []*Player{killer, observer} {
		pkt := mobDeathPacket(recipient, 1400, killer.ID, nil)
		if got := binary.LittleEndian.Uint32(pkt[12:16]); got != recipient.Char.Hold {
			t.Fatalf("destinatario %q recebeu Hold=%d, esperado %d",
				recipient.Char.Name, got, recipient.Char.Hold)
		}
		if got := binary.LittleEndian.Uint32(pkt[20:24]); got != recipient.Char.Exp {
			t.Fatalf("destinatario %q recebeu EXP=%d, esperado %d",
				recipient.Char.Name, got, recipient.Char.Exp)
		}
	}
}

func TestPublishPlayerDeathReachesVictimAndVisibleObservers(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Killer", 2100, 2100)
	victim, _ := networkedTestPlayer(2, "Victim", 2101, 2100)
	observer, _ := networkedTestPlayer(3, "Observer", 2102, 2100)
	w := worldWithNetworkedPlayers(killer, victim, observer)
	killer.show(victim.ID)
	observer.show(victim.ID)
	before := []int{
		killer.Session.QueuedPacketsForTest(),
		victim.Session.QueuedPacketsForTest(),
		observer.Session.QueuedPacketsForTest(),
	}

	w.publishPlayerDeath(victim, killer.ID)

	for i, recipient := range []*Player{killer, victim, observer} {
		if got := recipient.Session.QueuedPacketsForTest(); got != before[i]+1 {
			t.Fatalf("destinatario %q recebeu %d pacote(s), esperado +1",
				recipient.Char.Name, got-before[i])
		}
	}
}
