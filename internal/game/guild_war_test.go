package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func guildWarTestPacket(id uint16, claimed, target uint32) []byte {
	return wire.StandardParm2(wire.OpGuildWar, id, claimed, target)
}

func guildWarTestWorld(t *testing.T) (*World, *guildFlowStore, *Player, *Player) {
	t.Helper()
	alpha, _ := networkedTestPlayer(1, "AlphaLeader", 2100, 2100)
	beta, _ := networkedTestPlayer(2, "BetaLeader", 2101, 2100)
	w, st := guildFlowWorld(alpha, beta)
	w.guilds.Guilds = []model.Guild{
		{ID: 1, Name: "Alpha", Members: []model.GuildMember{{
			Character: alpha.Char.Name, Account: alpha.Account.Name, Rank: model.GuildRankLeader,
		}}},
		{ID: 2, Name: "Beta", Members: []model.GuildMember{{
			Character: beta.Char.Name, Account: beta.Account.Name, Rank: model.GuildRankLeader,
		}}},
	}
	alpha.Char.GuildID, alpha.Char.GuildRank = 1, model.GuildRankLeader
	beta.Char.GuildID, beta.Char.GuildRank = 2, model.GuildRankLeader
	return w, st, alpha, beta
}

func TestGuildWarDeclarationReciprocalStartAndCancel(t *testing.T) {
	w, st, alpha, beta := guildWarTestWorld(t)

	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 2))
	if got := w.guilds.FindByID(1).WarTarget; got != 2 ||
		w.guilds.FindByID(2).WarTarget != 0 || st.gameSaves != 1 {
		t.Fatalf("declaracao unilateral incorreta: alpha=%d beta=%d saves=%d",
			got, w.guilds.FindByID(2).WarTarget, st.gameSaves)
	}

	w.onGuildWar(beta.Session, guildWarTestPacket(beta.ID, 2, 1))
	if w.guilds.FindByID(1).WarTarget != 2 || w.guilds.FindByID(2).WarTarget != 1 ||
		st.gameSaves != 2 {
		t.Fatalf("declaracao reciproca nao iniciou a guerra: alpha=%d beta=%d saves=%d",
			w.guilds.FindByID(1).WarTarget, w.guilds.FindByID(2).WarTarget, st.gameSaves)
	}

	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 0))
	if w.guilds.FindByID(1).WarTarget != 0 || w.guilds.FindByID(2).WarTarget != 0 ||
		st.gameSaves != 3 {
		t.Fatalf("cancelamento nao limpou o par: alpha=%d beta=%d saves=%d",
			w.guilds.FindByID(1).WarTarget, w.guilds.FindByID(2).WarTarget, st.gameSaves)
	}
}

func TestGuildWarRejectsForgedLeaderInvalidTargetsAndSecondDeclaration(t *testing.T) {
	w, st, alpha, beta := guildWarTestWorld(t)
	before := st.gameSaves

	// O ID do header pertence ao remetente real; o primeiro campo nao pode
	// escolher outra guild para conceder autoridade.
	w.onGuildWar(alpha.Session, guildWarTestPacket(beta.ID, 2, 2))
	if st.gameSaves != before || w.guilds.FindByID(1).WarTarget != 0 {
		t.Fatal("ID de sessao forjado foi aceito")
	}
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 2, 1))
	if st.gameSaves != before || w.guilds.FindByID(1).WarTarget != 0 {
		t.Fatal("guild alvo propria foi aceita")
	}
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 999))
	if st.gameSaves != before || w.guilds.FindByID(1).WarTarget != 0 {
		t.Fatal("guild inexistente foi aceita")
	}

	// Alpha ja declarou contra Beta; uma segunda declaracao nao troca o alvo.
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 2))
	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 0))
	if w.guilds.FindByID(1).WarTarget != 0 || st.gameSaves != before+2 {
		t.Fatalf("segunda declaracao/cancelamento alterou estado indevido: target=%d saves=%d",
			w.guilds.FindByID(1).WarTarget, st.gameSaves)
	}
	_ = beta
}

func TestGuildWarRollsBackOnSaveFailure(t *testing.T) {
	w, st, alpha, _ := guildWarTestWorld(t)
	st.err = errors.New("postgres indisponivel")

	w.onGuildWar(alpha.Session, guildWarTestPacket(alpha.ID, 1, 2))
	if w.guilds.FindByID(1).WarTarget != 0 || st.gameSaves != 1 {
		t.Fatalf("rollback de guerra incompleto: target=%d saves=%d",
			w.guilds.FindByID(1).WarTarget, st.gameSaves)
	}
}
