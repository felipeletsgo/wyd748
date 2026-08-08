package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

type guildFlowStore struct {
	craftStore
	gameSaves int
}

func (s *guildFlowStore) LoadGuilds() (*model.GuildRegistry, error) {
	return &model.GuildRegistry{Version: model.GuildRegistryVersion}, nil
}

func (s *guildFlowStore) SaveGameState(_ *model.GuildRegistry, _ ...*model.Account) error {
	s.gameSaves++
	return s.err
}

func (s *guildFlowStore) SaveGameStateWithInstanceState(_ *model.GuildRegistry,
	snapshot *model.InstanceStateSnapshot, _ ...*model.Account) error {
	// Keep the historical assertions meaningful: opening/joining a room is a
	// gameplay transaction, while a reward-only commit is counted as the
	// account save. Production still performs one atomic PostgreSQL commit.
	rewardOnly := snapshot != nil && len(snapshot.Instances) > 0
	if rewardOnly {
		for _, inst := range snapshot.Instances {
			if !inst.RewardGranted {
				rewardOnly = false
				break
			}
		}
	}
	if rewardOnly {
		s.saves++
	} else {
		s.gameSaves++
	}
	return s.err
}

func guildFlowWorld(players ...*Player) (*World, *guildFlowStore) {
	w := worldWithNetworkedPlayers(players...)
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	st := &guildFlowStore{}
	w.store = st
	return w, st
}

func TestGuildCreateInviteAcceptChatPromoteAndExpel(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	w, st := guildFlowWorld(leader, member)

	w.guildCommandCreate(leader.Session, leader, "Alpha")
	guild := w.guilds.FindByName("Alpha")
	if guild == nil || leader.Char.GuildID != guild.ID ||
		leader.Char.GuildRank != model.GuildRankLeader || st.gameSaves != 1 {
		t.Fatalf("criacao de guild incorreta: guild=%+v char=%d/%d saves=%d",
			guild, leader.Char.GuildID, leader.Char.GuildRank, st.gameSaves)
	}

	w.guildCommandInvite(leader.Session, leader, member.Char.Name)
	if member.GuildInviteFrom != guild.ID || member.GuildInviteUntil.IsZero() {
		t.Fatal("convite de guild nao foi registrado")
	}
	w.guildCommandAccept(member.Session, member, "")
	if member.Char.GuildID != guild.ID || guild.Member(member.Char.Name) == nil || st.gameSaves != 2 {
		t.Fatalf("aceite incorreto: charGuild=%d members=%d saves=%d",
			member.Char.GuildID, len(guild.Members), st.gameSaves)
	}

	before := member.Session.QueuedPacketsForTest()
	w.sendGuildChat(leader, "-hello")
	if member.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("chat de guild nao chegou ao membro")
	}

	w.guildCommandSubLeader(leader.Session, leader, member.Char.Name)
	if member.Char.GuildRank != model.GuildRankSubFirst ||
		guild.Member(member.Char.Name).Rank != model.GuildRankSubFirst {
		t.Fatalf("promocao nao sincronizou char e registro: %d/%d",
			member.Char.GuildRank, guild.Member(member.Char.Name).Rank)
	}

	deprivate := make([]byte, 16)
	binary.LittleEndian.PutUint32(deprivate[12:16], uint32(member.ID))
	w.onGuildDeprivate(leader.Session, deprivate)
	if member.Char.GuildID != 0 || guild.Member(member.Char.Name) != nil {
		t.Fatal("expulsao nao removeu o membro")
	}
}

func TestGuildCommandExpelUsesAuthoritativeHierarchy(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	w, st := guildFlowWorld(leader, member)
	w.guildCommandCreate(leader.Session, leader, "Alpha")
	guild := w.guilds.FindByName("Alpha")
	guild.Members = append(guild.Members, model.GuildMember{
		Character: member.Char.Name, Rank: model.GuildRankMember,
	})
	member.Char.GuildID = guild.ID
	member.Char.GuildRank = model.GuildRankMember

	w.guildCommandExpel(leader.Session, leader, member.Char.Name)
	if member.Char.GuildID != 0 || guild.Member(member.Char.Name) != nil ||
		st.gameSaves != 2 {
		t.Fatalf("expulsao: guild=%d membro=%+v saves=%d",
			member.Char.GuildID, guild.Member(member.Char.Name), st.gameSaves)
	}
}

func TestGuildLeavePromotesOldestMember(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	w, st := guildFlowWorld(leader, member)
	guild := model.Guild{
		ID: 1, Name: "Alpha",
		Members: []model.GuildMember{
			{Character: leader.Char.Name, Account: leader.Account.Name, Rank: model.GuildRankLeader},
			{Character: member.Char.Name, Account: member.Account.Name, Rank: model.GuildRankMember},
		},
	}
	w.guilds.Guilds = []model.Guild{guild}
	leader.Char.GuildID, leader.Char.GuildRank = 1, model.GuildRankLeader
	member.Char.GuildID, member.Char.GuildRank = 1, model.GuildRankMember

	w.guildCommandLeave(leader.Session, leader, "")
	current := w.guilds.FindByID(1)
	if current == nil || leader.Char.GuildID != 0 ||
		member.Char.GuildRank != model.GuildRankLeader ||
		current.Member(member.Char.Name).Rank != model.GuildRankLeader || st.gameSaves != 1 {
		t.Fatalf("sucessao incorreta: guild=%+v leader=%d memberRank=%d saves=%d",
			current, leader.Char.GuildID, member.Char.GuildRank, st.gameSaves)
	}
}

func TestGuildPersistenceFailureRollsBackCreationAndAlliance(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	w, st := guildFlowWorld(leader)
	st.err = errors.New("disk full")

	w.guildCommandCreate(leader.Session, leader, "Alpha")
	if len(w.guilds.Guilds) != 0 || leader.Char.GuildID != 0 {
		t.Fatal("falha ao criar guild nao restaurou o estado")
	}

	st.err = nil
	w.guilds.Guilds = []model.Guild{
		{ID: 1, Name: "Alpha", Members: []model.GuildMember{{
			Character: leader.Char.Name, Account: leader.Account.Name, Rank: model.GuildRankLeader,
		}}},
		{ID: 2, Name: "Beta", Members: []model.GuildMember{{
			Character: "Other", Account: "OtherAccount", Rank: model.GuildRankLeader,
		}}},
	}
	leader.Char.GuildID, leader.Char.GuildRank = 1, model.GuildRankLeader
	ally := make([]byte, 20)
	binary.LittleEndian.PutUint32(ally[12:16], 1)
	binary.LittleEndian.PutUint32(ally[16:20], 2)
	w.onGuildAlly(leader.Session, ally)
	if w.guilds.FindByID(1).Ally != 2 {
		t.Fatal("alianca valida nao foi aplicada")
	}

	st.err = errors.New("disk full")
	binary.LittleEndian.PutUint32(ally[16:20], 0)
	w.onGuildAlly(leader.Session, ally)
	if w.guilds.FindByID(1).Ally != 2 {
		t.Fatal("falha ao desfazer alianca nao restaurou o snapshot")
	}
}
