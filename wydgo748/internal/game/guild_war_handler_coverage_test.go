package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestGuildWarHandlerRejectsMalformedUnauthorizedAndOutOfRangeTarget(t *testing.T) {
	w, st, alpha, _ := guildWarTestWorld(t)
	w.onGuildWar(alpha.Session, make([]byte, 19))
	if got := alpha.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("malformed war packet queued %d responses", got)
	}

	// Authorization follows the canonical registry membership, not the claimed
	// guild ID or the character's cached guild field.
	alphaName := alpha.Char.Name
	alpha.Char.Name = "Orphan"
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 2))
	if got := alpha.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("unauthorized war request queued %d responses, want 1", got)
	}
	alpha.Char.Name = alphaName

	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, uint32(model.MaxGuildID)+1))
	if st.gameSaves != 0 || w.guilds.FindByID(1).WarTarget != 0 {
		t.Fatal("out-of-range target changed or persisted guild state")
	}
}

func TestGuildWarHandlerRejectsDuplicatePendingDeclaration(t *testing.T) {
	w, st, alpha, beta := guildWarTestWorld(t)
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 2))
	if w.guilds.FindByID(1).WarTarget != 2 || st.gameSaves != 1 {
		t.Fatal("initial declaration did not persist")
	}
	alphaPackets, betaPackets := alpha.Session.QueuedPacketsForTest(), beta.Session.QueuedPacketsForTest()
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 2))
	if w.guilds.FindByID(1).WarTarget != 2 || st.gameSaves != 1 ||
		alpha.Session.QueuedPacketsForTest() != alphaPackets+1 || beta.Session.QueuedPacketsForTest() != betaPackets {
		t.Fatal("duplicate declaration was persisted or announced as a new war")
	}
}

func TestGuildWarCancellationDoesNothingWithoutDeclarationAndRestoresFailedSave(t *testing.T) {
	w, st, alpha, _ := guildWarTestWorld(t)
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 0))
	if st.gameSaves != 0 {
		t.Fatal("canceling an absent declaration wrote guild state")
	}

	w.guilds.FindByID(1).WarTarget, w.guilds.FindByID(2).WarTarget = 2, 1
	st.err = errors.New("injected war cancellation failure")
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 0))
	if w.guilds.FindByID(1).WarTarget != 2 || w.guilds.FindByID(2).WarTarget != 1 || st.gameSaves != 1 {
		t.Fatalf("failed reciprocal cancellation changed state: alpha=%d beta=%d saves=%d",
			w.guilds.FindByID(1).WarTarget, w.guilds.FindByID(2).WarTarget, st.gameSaves)
	}
}

func TestGuildWarHandlerUsesCriticalPersistenceForDeclarationAndCancellation(t *testing.T) {
	w, leader, _, _, base, q, release := criticalTreasuryFixture(t, 100)
	defer release()
	w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Beta"})

	w.onGuildWar(leader.Session, guildWarTestPacket(leader.ID, 1, 2))
	declaration := awaitCriticalWrite(t, base.entered)
	if declaration.guilds == nil || declaration.guilds.FindByID(1).WarTarget != 2 ||
		w.guilds.FindByID(1).WarTarget != 0 || leader.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("critical declaration was not detached before persistence acknowledgement")
	}
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if w.guilds.FindByID(1).WarTarget != 2 || leader.Session.QueuedPacketsForTest() == 0 || w.shuttingDown {
		t.Fatal("acknowledged critical declaration did not settle")
	}
	packets := leader.Session.QueuedPacketsForTest()

	w.onGuildWar(leader.Session, guildWarTestPacket(leader.ID, 1, 0))
	cancellation := awaitCriticalWrite(t, base.entered)
	if cancellation.guilds == nil || cancellation.guilds.FindByID(1).WarTarget != 0 ||
		w.guilds.FindByID(1).WarTarget != 2 || leader.Session.QueuedPacketsForTest() != packets {
		t.Fatal("critical cancellation was not detached before persistence acknowledgement")
	}
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if w.guilds.FindByID(1).WarTarget != 0 || leader.Session.QueuedPacketsForTest() != packets+1 || w.shuttingDown {
		t.Fatal("acknowledged critical cancellation did not settle")
	}
}
