package model

import (
	"fmt"
	"strings"
)

// QuestFileVersion e a unica versao aceita, seguindo o contrato estrito dos
// demais arquivos de dados.
const QuestFileVersion = 1

// QuestItem e uma referencia a item com quantidade, usada tanto em requisito
// quanto em consumo e recompensa.
type QuestItem struct {
	Index  uint16 `json:"index"`
	Amount uint32 `json:"amount,omitempty"` // 0 e tratado como 1
}

func (q QuestItem) Quantity() uint32 {
	if q.Amount == 0 {
		return 1
	}
	return q.Amount
}

// QuestRequirements sao as pre-condicoes verificadas no servidor.
//
// ATENCAO ao nivel: MinLevel/MaxLevel sao comparados contra o nivel INTERNO,
// que e base zero -- o servidor guarda 0 para o que o client exibe como nivel
// 1, e 399 para o nivel 400. Entao "minLevel: 49" libera a partir do nivel 50
// exibido. E a mesma escala usada pela progressao (progression.go), e mante-la
// aqui evita duas convencoes de nivel no mesmo servidor.
type QuestRequirements struct {
	MinLevel uint32      `json:"minLevel,omitempty"`
	MaxLevel uint32      `json:"maxLevel,omitempty"`
	Gold     uint32      `json:"gold,omitempty"`
	Items    []QuestItem `json:"items,omitempty"`
	// AfterQuest exige uma quest concluida antes desta.
	AfterQuest int `json:"afterQuest,omitempty"`
}

// QuestTeleport leva o jogador para uma coordenada ao concluir.
type QuestTeleport struct {
	X uint16 `json:"x"`
	Y uint16 `json:"y"`
}

// QuestRewards e o que o jogador recebe. Tudo entregue server-side.
type QuestRewards struct {
	Gold     uint32         `json:"gold,omitempty"`
	Exp      uint32         `json:"exp,omitempty"`
	Items    []QuestItem    `json:"items,omitempty"`
	Teleport *QuestTeleport `json:"teleport,omitempty"`
	// Citizenship concede a cidadania DESTE canal (o Kibita nativo). O valor
	// gravado e o numero do canal, nao um booleano -- ver citizenship.go.
	Citizenship bool `json:"citizenship,omitempty"`
}

// QuestMessages sao os textos exibidos no painel.
type QuestMessages struct {
	Offer   string `json:"offer"`
	Success string `json:"success"`
	Denied  string `json:"denied,omitempty"`
}

// QuestDef e uma quest. A correspondencia com o NPC e EXPLICITA pelo nome: o
// roteamento por allowlist so aciona quest para NPC configurado aqui, nunca
// por heuristica de tipo.
type QuestDef struct {
	ID         int               `json:"id"`
	NPC        string            `json:"npc"`
	Repeatable bool              `json:"repeatable,omitempty"`
	Requires   QuestRequirements `json:"requires,omitempty"`
	Consumes   []QuestItem       `json:"consumes,omitempty"`
	Rewards    QuestRewards      `json:"rewards,omitempty"`
	Messages   QuestMessages     `json:"messages"`
}

// QuestFile e o conteudo de data/quests.json.
type QuestFile struct {
	Version int        `json:"version"`
	Quests  []QuestDef `json:"quests"`
}

func (q *QuestDef) Validate() error {
	if q == nil {
		return fmt.Errorf("quest ausente")
	}
	if q.ID <= 0 {
		return fmt.Errorf("quest com id %d; use um id estavel positivo", q.ID)
	}
	if strings.TrimSpace(q.NPC) == "" {
		return fmt.Errorf("quest %d sem npc", q.ID)
	}
	if strings.TrimSpace(q.Messages.Offer) == "" {
		return fmt.Errorf("quest %d sem mensagem de oferta", q.ID)
	}
	if strings.TrimSpace(q.Messages.Success) == "" {
		return fmt.Errorf("quest %d sem mensagem de sucesso", q.ID)
	}
	if q.Requires.MaxLevel != 0 && q.Requires.MaxLevel < q.Requires.MinLevel {
		return fmt.Errorf("quest %d com maxLevel abaixo de minLevel", q.ID)
	}
	if q.Requires.AfterQuest == q.ID {
		return fmt.Errorf("quest %d exige a si mesma", q.ID)
	}
	for _, list := range [][]QuestItem{q.Requires.Items, q.Consumes, q.Rewards.Items} {
		for _, item := range list {
			if item.Index == 0 {
				return fmt.Errorf("quest %d referencia item index 0", q.ID)
			}
		}
	}
	if t := q.Rewards.Teleport; t != nil && (t.X == 0 || t.Y == 0) {
		return fmt.Errorf("quest %d com teleporte invalido", q.ID)
	}
	return nil
}

// NormalizeQuestNPC e a forma usada para casar o nome do NPC.
func NormalizeQuestNPC(name string) string {
	return strings.ToLower(strings.TrimSpace(name))
}

func (f *QuestFile) Validate() error {
	if f == nil {
		return fmt.Errorf("arquivo de quests ausente")
	}
	if f.Version != QuestFileVersion {
		return fmt.Errorf("arquivo de quests versao %d; esperado %d", f.Version, QuestFileVersion)
	}
	ids := make(map[int]struct{}, len(f.Quests))
	npcs := make(map[string]struct{}, len(f.Quests))
	for i := range f.Quests {
		quest := &f.Quests[i]
		if err := quest.Validate(); err != nil {
			return err
		}
		if _, duplicate := ids[quest.ID]; duplicate {
			return fmt.Errorf("quest id %d duplicado", quest.ID)
		}
		ids[quest.ID] = struct{}{}
		// Um NPC so pode oferecer uma quest: com duas, o clique seria ambiguo.
		key := NormalizeQuestNPC(quest.NPC)
		if _, duplicate := npcs[key]; duplicate {
			return fmt.Errorf("npc %q aparece em mais de uma quest", quest.NPC)
		}
		npcs[key] = struct{}{}
	}
	// AfterQuest precisa existir; senao a quest nunca ficaria disponivel.
	for i := range f.Quests {
		after := f.Quests[i].Requires.AfterQuest
		if after == 0 {
			continue
		}
		if _, exists := ids[after]; !exists {
			return fmt.Errorf("quest %d exige a quest %d, que nao existe",
				f.Quests[i].ID, after)
		}
	}
	return nil
}
