package control

import (
	"context"
	"encoding/json"
	"errors"
	"strings"
	"time"
)

const (
	NPCDefinitionCapability = "content.npc.shop"
)

type NPCDefinitionView struct {
	Name           string          `json:"name"`
	DisplayName    string          `json:"displayName"`
	Channel        uint8           `json:"channel"`
	Version        uint32          `json:"version"`
	AppliedVersion uint32          `json:"appliedVersion"`
	Active         bool            `json:"active"`
	Overridden     bool            `json:"overridden"`
	Definition     json.RawMessage `json:"definition"`
	Source         json.RawMessage `json:"source"`
	UpdatedBy      string          `json:"updatedBy"`
	UpdatedAt      *time.Time      `json:"updatedAt,omitempty"`
}

type NPCDefinitionCatalogView struct {
	Version   int                 `json:"version"`
	Available bool                `json:"available"`
	Channel   uint8               `json:"channel"`
	NPCs      []NPCDefinitionView `json:"npcs"`
}

type NPCDefinitionCommand struct {
	Version         int                   `json:"version"`
	Channel         uint8                 `json:"channel"`
	ExpectedVersion uint32                `json:"expectedVersion"`
	Name            string                `json:"name"`
	Reason          string                `json:"reason"`
	RestoreSource   bool                  `json:"restoreSource"`
	Definition      json.RawMessage       `json:"definition,omitempty"`
	Targets         []NPCDefinitionTarget `json:"targets,omitempty"`
}

// Targets is optional for a single-channel edit and explicit for channels 1 and 2.
type NPCDefinitionTarget struct {
	Channel         uint8  `json:"channel"`
	ExpectedVersion uint32 `json:"expectedVersion"`
}

func (c NPCDefinitionCommand) Validate() error {
	if c.Version != 1 || c.Channel == 0 || c.ExpectedVersion == 0 || c.ExpectedVersion == ^uint32(0) || strings.TrimSpace(c.Name) == "" || len(c.Name) > 64 || !validReason(c.Reason) {
		return errors.New("invalid_command")
	}
	if c.Targets != nil {
		if len(c.Targets) != 2 {
			return errors.New("invalid_command")
		}
		seen := map[uint8]bool{}
		for _, target := range c.Targets {
			if target.Channel < 1 || target.Channel > 2 || seen[target.Channel] || target.ExpectedVersion == 0 || target.ExpectedVersion == ^uint32(0) {
				return errors.New("invalid_command")
			}
			if target.Channel == c.Channel && target.ExpectedVersion != c.ExpectedVersion {
				return errors.New("invalid_command")
			}
			seen[target.Channel] = true
		}
		if !seen[c.Channel] {
			return errors.New("invalid_command")
		}
	}
	if c.RestoreSource {
		if len(c.Definition) != 0 {
			return errors.New("invalid_command")
		}
		return nil
	}
	if len(c.Definition) == 0 || len(c.Definition) > 64*1024 || !json.Valid(c.Definition) {
		return errors.New("invalid_command")
	}
	return nil
}

type NPCDefinitionResult struct {
	Code string              `json:"code"`
	NPC  NPCDefinitionView   `json:"npc"`
	NPCs []NPCDefinitionView `json:"npcs,omitempty"`
}

type NPCDefinitionSource interface {
	NPCDefinitions(context.Context, uint8) (NPCDefinitionCatalogView, error)
	SaveNPCDefinition(context.Context, string, NPCDefinitionCommand, func() bool) (NPCDefinitionResult, error)
}
