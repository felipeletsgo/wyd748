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
	MinLevel   uint32      `json:"minLevel,omitempty"`
	MaxLevel   uint32      `json:"maxLevel,omitempty"`
	MortalOnly bool        `json:"mortalOnly,omitempty"`
	Gold       uint32      `json:"gold,omitempty"`
	Items      []QuestItem `json:"items,omitempty"`
	// AfterQuest exige uma quest concluida antes desta.
	AfterQuest int `json:"afterQuest,omitempty"`
	// Counters exige um saldo minimo de contadores nomeados do personagem --
	// o KefraTicket do nativo e um deles. Exigir NAO consome; para gastar,
	// use QuestDef.ConsumeCounters.
	Counters map[string]uint32 `json:"counters,omitempty"`
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
	// Refine refina o equipamento VESTIDO. E a recompensa dos guardas do
	// Training Camp: o nativo grava EF_SANC direto no Equip do jogador.
	Refine *QuestRefine `json:"refine,omitempty"`
	// Counters credita contadores nomeados ao personagem.
	Counters map[string]uint32 `json:"counters,omitempty"`
}

// QuestRecharge troca um item por saldo de contador, quando o saldo zera.
type QuestRecharge struct {
	Item    uint16 `json:"item"`
	Counter string `json:"counter"`
	Amount  uint32 `json:"amount"`
}

// QuestRefine e a recompensa em refinacao. Slot negativo refina TODAS as pecas
// vestidas; caso contrario refina so aquele slot (6 = arma).
type QuestRefine struct {
	Slot int `json:"slot"`
	Sanc int `json:"sanc"`
}

// QuestMessages sao os textos exibidos no painel.
type QuestMessages struct {
	Offer   string `json:"offer"`
	Success string `json:"success"`
	Denied  string `json:"denied,omitempty"`
}

// MaxQuestLine e o limite de uma fala de NPC. O 0x333 carrega String[96] e o
// builder copia 95 bytes, entao o que passar disso e cortado no meio da frase.
const MaxQuestLine = 95

// MaxEquipSlots e o tamanho do Char.Equip.
const MaxEquipSlots = 16

// MaxQuestCounter limita um contador nomeado. O saldo mora num uint32, mas um
// numero solto no JSON viraria saldo praticamente infinito -- o nativo concede
// 100 entradas de Kefra por vez, entao esta folga e larga o bastante.
const MaxQuestCounter = 1_000_000

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
	// Recharge porta o comportamento do Sobrevivente nativo: quando o saldo do
	// contador acaba, o NPC procura um item no inventario, consome UMA unidade
	// e credita Amount de entradas. So dispara com saldo ZERO -- nunca gasta o
	// item de quem ainda tem entradas.
	//
	// Existe como campo proprio porque o item nao e consumivel: nem o nosso
	// itemlist nem o do W2PP dao EF_VOLATILE ao selo 4127, entao "usar o item"
	// nao e um caminho valido -- quem o gasta e o NPC.
	Recharge *QuestRecharge `json:"recharge,omitempty"`
	// ConsumeCounters GASTA contadores nomeados. Paralelo ao Consumes de itens,
	// e nao ao Requires: exigir saldo e gastar saldo sao coisas diferentes --
	// a entrada de Kefra gasta uma ficha, o portao de uma area pode so exigir.
	ConsumeCounters map[string]uint32 `json:"consumeCounters,omitempty"`
	Messages        QuestMessages     `json:"messages"`
	// Dialogue sao falas do NPC, ditas em voz alta ao interagir. Diferente de
	// Messages, que e painel privado: isto sai como fala no chat, com o ID do
	// NPC, e quem estiver perto tambem le. Uma linha e escolhida por vez.
	Dialogue []string `json:"dialogue,omitempty"`
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
	for rotulo, tabela := range map[string]map[string]uint32{
		"requires.counters": q.Requires.Counters,
		"consumeCounters":   q.ConsumeCounters,
		"rewards.counters":  q.Rewards.Counters,
	} {
		for nome, quantidade := range tabela {
			if strings.TrimSpace(nome) == "" {
				return fmt.Errorf("quest %d tem contador sem nome em %s", q.ID, rotulo)
			}
			if quantidade == 0 {
				return fmt.Errorf("quest %d: contador %q em %s com quantidade zero",
					q.ID, nome, rotulo)
			}
			if quantidade > MaxQuestCounter {
				return fmt.Errorf("quest %d: contador %q em %s vale %d; o teto e %d",
					q.ID, nome, rotulo, quantidade, MaxQuestCounter)
			}
		}
	}
	if r := q.Recharge; r != nil {
		if r.Item == 0 || strings.TrimSpace(r.Counter) == "" || r.Amount == 0 {
			return fmt.Errorf("quest %d com recharge incompleto: %+v", q.ID, *r)
		}
		if r.Amount > MaxQuestCounter {
			return fmt.Errorf("quest %d: recharge de %d excede o teto %d",
				q.ID, r.Amount, MaxQuestCounter)
		}
		// Recarregar um contador que a quest nunca gasta nao faria nada.
		if q.ConsumeCounters[r.Counter] == 0 {
			return fmt.Errorf("quest %d recarrega %q, mas nao o consome",
				q.ID, r.Counter)
		}
	}
	if r := q.Rewards.Refine; r != nil {
		if r.Sanc < 1 || r.Sanc > 9 {
			return fmt.Errorf("quest %d refina para +%d; use 1..9", q.ID, r.Sanc)
		}
		if r.Slot >= MaxEquipSlots {
			return fmt.Errorf("quest %d refina o slot %d, fora de 0..%d",
				q.ID, r.Slot, MaxEquipSlots-1)
		}
	}
	// Fala longa demais nao da erro no wire: ela chega ao jogador CORTADA no
	// meio. Recusar no boot e melhor que descobrir isso lendo o chat.
	for i, linha := range q.Dialogue {
		if strings.TrimSpace(linha) == "" {
			return fmt.Errorf("quest %d com fala %d vazia", q.ID, i)
		}
		if len(linha) > MaxQuestLine {
			return fmt.Errorf("quest %d fala %d com %d caracteres; o maximo e %d",
				q.ID, i, len(linha), MaxQuestLine)
		}
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
