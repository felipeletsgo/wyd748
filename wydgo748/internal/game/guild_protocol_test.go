package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func inviteGuildPacket(actor, target uint16, inviteType uint32) []byte {
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint16(pkt[0:2], uint16(len(pkt)))
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpInviteGuild)
	binary.LittleEndian.PutUint16(pkt[6:8], actor)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(target))
	binary.LittleEndian.PutUint32(pkt[16:20], inviteType)
	return pkt
}

func TestInviteGuildPacketAddsMemberAtomically(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	leader.Char.Gold = 10_000_000
	w, st := guildFlowWorld(leader, member)
	w.clock = newFakeClock(time.Date(2026, 1, 5, 12, 0, 0, 0, time.UTC)) // segunda
	w.guildCommandCreate(leader.Session, leader, "Alpha")
	guild := w.guilds.FindByName("Alpha")
	if guild == nil {
		t.Fatal("guild de teste nao foi criada")
	}
	saves := st.gameSaves
	w.onInviteGuild(leader.Session, inviteGuildPacket(leader.ID, member.ID, 0))
	if member.Char.GuildID != guild.ID || member.Char.GuildRank != model.GuildRankMember ||
		guild.Member(member.Char.Name) == nil || leader.Char.Gold != 6_000_000 || st.gameSaves != saves+1 {
		t.Fatalf("0x3D5 nao incluiu membro: char=%d/%d membros=%+v gold=%d saves=%d",
			member.Char.GuildID, member.Char.GuildRank, guild.Members, leader.Char.Gold, st.gameSaves)
	}
}

func TestInviteGuildPacketRollsBackOnSaveFailure(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	leader.Char.Gold = 10_000_000
	w, st := guildFlowWorld(leader, member)
	w.clock = newFakeClock(time.Date(2026, 1, 5, 12, 0, 0, 0, time.UTC))
	w.guildCommandCreate(leader.Session, leader, "Alpha")
	st.err = errors.New("postgres indisponivel")
	before := w.snapshotGuilds()
	w.onInviteGuild(leader.Session, inviteGuildPacket(leader.ID, member.ID, 0))
	guild := w.guilds.FindByName("Alpha")
	if member.Char.GuildID != 0 || member.Char.GuildRank != 0 || leader.Char.Gold != 10_000_000 ||
		guild == nil || len(guild.Members) != len(before[0].Members) {
		t.Fatalf("rollback 0x3D5 incompleto: guilds=%+v member=%d/%d gold=%d",
			w.guilds.Guilds, member.Char.GuildID, member.Char.GuildRank, leader.Char.Gold)
	}
}

func TestInviteGuildPacketRejectsMemberAndInvalidIdentity(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	w, st := guildFlowWorld(leader, member)
	w.clock = newFakeClock(time.Date(2026, 1, 5, 12, 0, 0, 0, time.UTC))
	w.guildCommandCreate(leader.Session, leader, "Alpha")
	guild := w.guilds.FindByName("Alpha")
	guild.Members = append(guild.Members, model.GuildMember{Character: member.Char.Name, Account: member.Account.Name})
	member.Char.GuildID, member.Char.GuildRank = guild.ID, model.GuildRankMember
	before := st.gameSaves
	w.onInviteGuild(leader.Session, inviteGuildPacket(leader.ID, member.ID, 0))
	if st.gameSaves != before || leader.Char.Gold != 0 {
		t.Fatal("recrutamento de membro alterou estado")
	}

	member.Char.GuildID, member.Char.GuildRank = 0, 0
	member.Account.Chars[0].GuildID, member.Account.Chars[0].GuildRank = 0, 0
	w.onInviteGuild(member.Session, inviteGuildPacket(leader.ID, member.ID, 0))
	if st.gameSaves != before {
		t.Fatal("ID de recrutador forjado foi aceito")
	}
}
