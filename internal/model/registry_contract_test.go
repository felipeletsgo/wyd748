package model

import (
	"strings"
	"testing"
)

func testGuild(id uint16, name, leader string) Guild {
	return Guild{
		ID: id, Name: name,
		Members: []GuildMember{{Character: leader, Account: strings.ToLower(leader), Rank: GuildRankLeader}},
	}
}

func TestGuildRegistryLookupSerializationAndLeaderContracts(t *testing.T) {
	first := testGuild(1, "Alpha Guild", "LeaderA")
	second := testGuild(2, "Beta", "LeaderB")
	registry := GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{first, second}}
	if err := registry.Validate(); err != nil {
		t.Fatalf("fixture de guild valida foi recusada: %v", err)
	}
	if leader := registry.Guilds[0].Leader(); leader == nil || leader.Character != "LeaderA" {
		t.Fatalf("Leader() nao resolveu mestre: %+v", leader)
	}
	if got := registry.FindByID(2); got == nil || got.Name != "Beta" {
		t.Fatalf("FindByID falhou: %+v", got)
	}
	if registry.FindByID(0) != nil || registry.FindByID(999) != nil {
		t.Fatal("FindByID resolveu ID ausente/reservado")
	}
	if got := registry.FindByName("  ALPHA GUILD "); got == nil || got.ID != 1 {
		t.Fatalf("FindByName nao normalizou nome: %+v", got)
	}
	if registry.FindByName("missing") != nil {
		t.Fatal("FindByName resolveu nome inexistente")
	}
	guild, member := registry.FindByCharacter("leaderb")
	if guild == nil || member == nil || guild.ID != 2 || member.Character != "LeaderB" {
		t.Fatalf("FindByCharacter falhou: guild=%+v member=%+v", guild, member)
	}
	if g, m := registry.FindByCharacter("nobody"); g != nil || m != nil {
		t.Fatal("FindByCharacter resolveu personagem inexistente")
	}

	text := string(registry.GuildsTxt(3, 4))
	if text != "3 4 1 Alpha Guild\n3 4 2 Beta\n" {
		t.Fatalf("Guilds.txt divergente: %q", text)
	}
}

func TestQuestValidationCoversCountersRechargeAndFileRelationships(t *testing.T) {
	q := QuestDef{
		ID: 1, NPC: "Survivor",
		Consumes:        []QuestItem{{Index: 4011}},
		ConsumeCounters: map[string]uint32{"kefra_ticket": 1},
		Recharge:        &QuestRecharge{Item: 4127, Counter: "kefra_ticket", Amount: 100},
		Rewards: QuestRewards{
			Items:    []QuestItem{{Index: 4012, Amount: 2}},
			Counters: map[string]uint32{"fame": 10},
		},
		Requires: QuestRequirements{Counters: map[string]uint32{"kefra_ticket": 1}},
		Messages: QuestMessages{Offer: "Offer", Success: "Done"},
		Dialogue: []string{"Welcome."},
	}
	if q.Consumes[0].Quantity() != 1 || q.Rewards.Items[0].Quantity() != 2 {
		t.Fatal("QuestItem.Quantity nao respeitou default/amount")
	}
	if err := q.Validate(); err != nil {
		t.Fatalf("quest completa valida foi recusada: %v", err)
	}
	if NormalizeQuestNPC("  SurVIVor ") != "survivor" {
		t.Fatal("NormalizeQuestNPC nao normalizou caixa/espacos")
	}

	dependent := QuestDef{
		ID: 2, NPC: "Next", Requires: QuestRequirements{AfterQuest: 1},
		Messages: QuestMessages{Offer: "Offer", Success: "Done"},
	}
	file := QuestFile{Version: QuestFileVersion, Quests: []QuestDef{q, dependent}}
	if err := file.Validate(); err != nil {
		t.Fatalf("arquivo de quests valido foi recusado: %v", err)
	}

	badRecharge := q
	badRecharge.ConsumeCounters = nil
	if err := badRecharge.Validate(); err == nil {
		t.Fatal("recharge que nao consome o contador foi aceito")
	}
	badCounter := q
	badCounter.Requires.Counters = map[string]uint32{"": 1}
	if err := badCounter.Validate(); err == nil {
		t.Fatal("contador sem nome foi aceito")
	}
	badDialogue := q
	badDialogue.Dialogue = []string{strings.Repeat("x", MaxQuestLine+1)}
	if err := badDialogue.Validate(); err == nil {
		t.Fatal("fala maior que o wire foi aceita")
	}
	badFile := QuestFile{Version: QuestFileVersion, Quests: []QuestDef{dependent}}
	if err := badFile.Validate(); err == nil {
		t.Fatal("AfterQuest inexistente foi aceito")
	}
}

func TestWorldObjectAndKingdomSmallContracts(t *testing.T) {
	if err := (InitItem{Index: 746, X: 100, Y: 100}).Validate(); err != nil {
		t.Fatalf("InitItem valido recusado: %v", err)
	}
	for _, invalid := range []InitItem{{}, {Index: 746, X: 0, Y: 100}, {Index: 746, X: 100, Y: 0}} {
		if err := invalid.Validate(); err == nil {
			t.Fatalf("InitItem invalido aceito: %+v", invalid)
		}
	}
	if KingdomName(KingdomHekalotia) != "Hekalotia" || KingdomName(KingdomAkelonia) != "Akelonia" ||
		KingdomName(99) != "Neutro" {
		t.Fatal("nomes de reino divergiram do contrato")
	}
}
