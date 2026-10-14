package webcatalog

import (
	"fmt"
	"sort"

	"wydgo/internal/model"
)

type QuestItem struct {
	ID       uint16 `json:"id"`
	Quantity uint32 `json:"quantity"`
}

type Counter struct {
	Name     string `json:"name"`
	Quantity uint32 `json:"quantity"`
}

type Destination struct {
	X           uint16 `json:"x"`
	Y           uint16 `json:"y"`
	GlobalReset bool   `json:"globalReset"`
}

type Refinement struct {
	Slot  int `json:"slot"`
	Level int `json:"level"`
}

type Recharge struct {
	ItemID   uint16 `json:"itemId"`
	Counter  string `json:"counter"`
	Quantity uint32 `json:"quantity"`
}

// Quest is a public configuration snapshot, not a player's progress or a
// promise that the server will accept an interaction. Levels are display-based.
type Quest struct {
	ID               int          `json:"id"`
	Name             string       `json:"name"`
	Objective        string       `json:"objective"`
	Repeatable       bool         `json:"repeatable"`
	MortalOnly       bool         `json:"mortalOnly"`
	MinLevel         uint64       `json:"minLevel"`
	MaxLevel         *uint64      `json:"maxLevel,omitempty"`
	AfterQuestID     int          `json:"afterQuestId,omitempty"`
	GoldCost         uint32       `json:"goldCost"`
	RequiredItems    []QuestItem  `json:"requiredItems"`
	ConsumedItems    []QuestItem  `json:"consumedItems"`
	RewardItems      []QuestItem  `json:"rewardItems"`
	RequiredCounters []Counter    `json:"requiredCounters"`
	ConsumedCounters []Counter    `json:"consumedCounters"`
	RewardCounters   []Counter    `json:"rewardCounters"`
	RewardGold       uint32       `json:"rewardGold"`
	BaseRewardExp    uint32       `json:"baseRewardExp"`
	Citizenship      bool         `json:"citizenship"`
	Destination      *Destination `json:"destination,omitempty"`
	Refinement       *Refinement  `json:"refinement,omitempty"`
	Recharge         *Recharge    `json:"recharge,omitempty"`
}

func ProjectQuests(file model.QuestFile, zones model.QuestZoneFile, items []Item) ([]Quest, error) {
	if err := file.Validate(); err != nil {
		return nil, err
	}
	if err := zones.Validate(); err != nil {
		return nil, err
	}
	knownItems := map[uint16]bool{}
	for _, item := range items {
		knownItems[item.ID] = true
	}
	projectItems := func(list []model.QuestItem) ([]QuestItem, error) {
		result := []QuestItem{}
		for _, item := range list {
			if !knownItems[item.Index] {
				return nil, fmt.Errorf("quest references unavailable public item %d", item.Index)
			}
			result = append(result, QuestItem{ID: item.Index, Quantity: item.Quantity()})
		}
		return result, nil
	}
	projectCounters := func(values map[string]uint32) ([]Counter, error) {
		result := []Counter{}
		for key, quantity := range values {
			// The only currently verified public counter binding.
			if key != "kefra_ticket" {
				return nil, fmt.Errorf("unsupported public quest counter %q", key)
			}
			result = append(result, Counter{Name: "Kefra entries", Quantity: quantity})
		}
		return result, nil
	}
	result := []Quest{}
	for _, def := range file.Quests {
		name, err := displayName(def.NPC)
		if err != nil {
			return nil, err
		}
		objective, err := displayName(def.Messages.Offer)
		if err != nil {
			return nil, err
		}
		q := Quest{ID: def.ID, Name: name, Objective: objective, Repeatable: def.Repeatable, MortalOnly: def.Requires.MortalOnly,
			MinLevel: uint64(def.Requires.MinLevel) + 1, AfterQuestID: def.Requires.AfterQuest, GoldCost: def.Requires.Gold,
			RewardGold: def.Rewards.Gold, BaseRewardExp: def.Rewards.Exp, Citizenship: def.Rewards.Citizenship}
		if def.Requires.MaxLevel != 0 {
			maximum := uint64(def.Requires.MaxLevel) + 1
			q.MaxLevel = &maximum
		}
		if q.RequiredItems, err = projectItems(def.Requires.Items); err != nil {
			return nil, err
		}
		if q.ConsumedItems, err = projectItems(def.Consumes); err != nil {
			return nil, err
		}
		if q.RewardItems, err = projectItems(def.Rewards.Items); err != nil {
			return nil, err
		}
		if q.RequiredCounters, err = projectCounters(def.Requires.Counters); err != nil {
			return nil, err
		}
		if q.ConsumedCounters, err = projectCounters(def.ConsumeCounters); err != nil {
			return nil, err
		}
		if q.RewardCounters, err = projectCounters(def.Rewards.Counters); err != nil {
			return nil, err
		}
		if target := def.Rewards.Teleport; target != nil {
			q.Destination = &Destination{X: target.X, Y: target.Y}
			for _, zone := range zones.Zones {
				if zone.Contains(target.X, target.Y) {
					q.Destination.GlobalReset = true
				}
			}
		}
		if reward := def.Rewards.Refine; reward != nil {
			q.Refinement = &Refinement{Slot: reward.Slot, Level: reward.Sanc}
		}
		if recharge := def.Recharge; recharge != nil {
			if !knownItems[recharge.Item] {
				return nil, fmt.Errorf("quest recharge references unavailable public item %d", recharge.Item)
			}
			counter, err := projectCounters(map[string]uint32{recharge.Counter: recharge.Amount})
			if err != nil {
				return nil, err
			}
			q.Recharge = &Recharge{ItemID: recharge.Item, Counter: counter[0].Name, Quantity: recharge.Amount}
		}
		result = append(result, q)
	}
	// Reject unreachable prerequisite cycles, including chains through other quests.
	parents := map[int]int{}
	for _, q := range result {
		parents[q.ID] = q.AfterQuestID
	}
	for _, q := range result {
		seen := map[int]bool{}
		for id := q.ID; id != 0; id = parents[id] {
			if seen[id] {
				return nil, fmt.Errorf("quest %d has a prerequisite cycle", q.ID)
			}
			seen[id] = true
		}
	}
	sort.Slice(result, func(i, j int) bool { return result[i].ID < result[j].ID })
	return result, nil
}
