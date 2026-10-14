package playerportal

import (
	"context"
	"encoding/json"
	"errors"
	"net/http"
	"os"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

// These DTOs are an allowlist, not the persisted character/item models. Base
// score is not effective combat score, and internal identities stay private.
type EquipmentItem struct {
	Slot    int     `json:"slot"`
	ItemID  uint16  `json:"itemId"`
	Name    string  `json:"name"`
	Effects [6]byte `json:"effects"`
}

type CharacterDetail struct {
	Character
	AsOf                  time.Time             `json:"asOf"`
	Exp                   uint32                `json:"exp"`
	Gold                  uint32                `json:"gold"`
	CP                    int16                 `json:"cp"`
	Fame                  uint32                `json:"fame"`
	Equipment             []EquipmentItem       `json:"equipment"`
	Ascension             ArchAscension         `json:"ascension"`
	CelestialAscension    CelestialAscension    `json:"celestialAscension"`
	SubCelestialAscension SubCelestialAscension `json:"subCelestialAscension"`
}

type AscensionCheck struct {
	ID  string `json:"id"`
	Met bool   `json:"met"`
}

type ArchAscension struct {
	Applicable      bool             `json:"applicable"`
	RequirementsMet bool             `json:"requirementsMet"`
	RequiredLevel   uint32           `json:"requiredLevel"`
	Checks          []AscensionCheck `json:"checks"`
}

type CelestialAscension struct {
	Applicable          bool             `json:"applicable"`
	RequirementsMet     bool             `json:"requirementsMet"`
	RequiredLevel       uint32           `json:"requiredLevel"`
	RequiredMortalLevel uint32           `json:"requiredMortalLevel"`
	Checks              []AscensionCheck `json:"checks"`
}

type SubCelestialAscension struct {
	Applicable      bool             `json:"applicable"`
	RequirementsMet bool             `json:"requirementsMet"`
	RequiredLevel   uint32           `json:"requiredLevel"`
	RequiredFame    uint32           `json:"requiredFame"`
	Checks          []AscensionCheck `json:"checks"`
}

func subCelestialAdvice(detail CharacterDetail, evolution string, fame uint32, ideal, noAlternate, face, space bool) SubCelestialAscension {
	var sefirot uint16
	for _, item := range detail.Equipment {
		if item.Slot == model.ArchSefirotSlot {
			sefirot = item.ItemID
		}
	}
	r := model.EvaluateSubCelestialRequirements(evolution, detail.Level, fame, sefirot, ideal, noAlternate, face, space)
	return SubCelestialAscension{Applicable: r.Celestial, RequirementsMet: r.Met(), RequiredLevel: model.SubCelestialMinimumLevel + 1, RequiredFame: model.SubCelestialFameCost,
		Checks: []AscensionCheck{{"celestial", r.Celestial}, {"level", r.Level}, {"fame", r.Fame}, {"sefirot", r.Sefirot}, {"idealStone", r.IdealStone}, {"noAlternate", r.NoAlternate}, {"face", r.Face}, {"rewardSpace", r.RewardSpace}}}
}

func celestialAdvice(rawEvolution string, level, mortalLevel uint32, idealStone bool) CelestialAscension {
	r := model.EvaluateCelestialRequirements(rawEvolution, level, mortalLevel, idealStone)
	return CelestialAscension{Applicable: r.Arch, RequirementsMet: r.Met(),
		RequiredLevel: model.CelestialMinimumArchLevel + 1, RequiredMortalLevel: model.CelestialMinimumMortalLevel + 1,
		Checks: []AscensionCheck{{"arch", r.Arch}, {"level", r.Level}, {"mortalLevel", r.MortalLevel}, {"idealStone", r.IdealStone}}}
}

func archAdvice(detail CharacterDetail, rawEvolution string, freeSlot bool) ArchAscension {
	var stone, sefirot uint16
	for _, item := range detail.Equipment {
		switch item.Slot {
		case model.ArchEternalStoneSlot:
			stone = item.ItemID
		case model.ArchSefirotSlot:
			sefirot = item.ItemID
		}
	}
	r := model.EvaluateArchRequirements(rawEvolution, detail.Level, stone, sefirot, freeSlot)
	return ArchAscension{Applicable: r.Mortal, RequirementsMet: r.Met(), RequiredLevel: model.ArchMinimumLevel + 1,
		Checks: []AscensionCheck{{"mortal", r.Mortal}, {"level", r.Level}, {"eternalStone", r.EternalStone}, {"sefirot", r.Sefirot}, {"freeSlot", r.FreeSlot}}}
}

func (h *Handler) characterDetail(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.current(r)
	if !ok || s.Account == "" {
		failure(w, 401, "Sign in to view your characters")
		return
	}
	selector := r.PathValue("slot")
	if len(selector) != 1 || selector[0] < '0' || selector[0] > '3' {
		failure(w, 400, "Invalid character slot")
		return
	}
	// URL/query account names are never used for ownership. The slot is a
	// selector in this account snapshot, not a durable character identity.
	detail, err := h.cfg.Backend.CharacterDetail(r.Context(), s.Account, int(selector[0]-'0'))
	if errors.Is(err, os.ErrNotExist) {
		failure(w, 404, "Character not found")
		return
	}
	if err != nil {
		failure(w, 503, "Character unavailable")
		return
	}
	for i := range detail.Equipment {
		detail.Equipment[i].Name = strings.ReplaceAll(h.cfg.ItemNames[detail.Equipment[i].ItemID], "_", " ")
	}
	reply(w, 200, map[string]any{"version": 1, "character": detail, "source": "persisted snapshot"})
}

func (d *Database) CharacterDetail(ctx context.Context, name string, slot int) (CharacterDetail, error) {
	if slot < 0 || slot > 3 {
		return CharacterDetail{}, os.ErrNotExist
	}
	ctx, cancel := context.WithTimeout(ctx, 3*time.Second)
	defer cancel()
	var result CharacterDetail
	var projection []byte
	err := d.pool.QueryRow(ctx, characterDetailSQL, strings.ToLower(name), slot, model.ArchCharacterSlots, model.PlayerCarrySlots).Scan(&projection, &result.AsOf)
	if errors.Is(err, pgx.ErrNoRows) {
		return CharacterDetail{}, os.ErrNotExist
	}
	if err != nil {
		return CharacterDetail{}, err
	}
	// The owner's Fame and derived checks are allowlisted. Raw evolution,
	// inventory and account slot occupancy stay inside this adapter.
	var saved struct {
		CharacterDetail
		RawEvolution string       `json:"rawEvolution"`
		FreeSlot     bool         `json:"freeSlot"`
		MortalLevel  uint32       `json:"mortalLevel"`
		Inventory    []model.Item `json:"inventory"`
		Fame         *uint32      `json:"fame"`
		NoAlternate  bool         `json:"noAlternate"`
		ValidFace    bool         `json:"validFace"`
	}
	if err := json.Unmarshal(projection, &saved); err != nil {
		return CharacterDetail{}, err
	}
	saved.AsOf = result.AsOf
	result = saved.CharacterDetail
	if saved.Fame == nil {
		return CharacterDetail{}, errors.New("invalid saved progression counter")
	}
	result.Fame = *saved.Fame
	ideal, space := model.SavedIdealInventoryRequirements(saved.Inventory)
	result.Ascension = archAdvice(result, saved.RawEvolution, saved.FreeSlot)
	result.CelestialAscension = celestialAdvice(saved.RawEvolution, result.Level, saved.MortalLevel, ideal)
	result.SubCelestialAscension = subCelestialAdvice(result, saved.RawEvolution, *saved.Fame, ideal, saved.NoAlternate, saved.ValidFace, space)
	return result, nil
}

// One SQL snapshot with allowlisted private inputs, discarded by the adapter.
// Named equipment keys are the established persistence contract.
const characterDetailSQL = `SELECT jsonb_build_object(
 'slot', $2::integer, 'name', c->>'name', 'class', COALESCE((c->>'class')::integer,0),
 'level', COALESCE((c->'score'->>'level')::bigint,0),
 'evolution', COALESCE(NULLIF(c->>'evolution',''),'mortal'),
 'rawEvolution', COALESCE(c->>'evolution',''),
 'mortalLevel', COALESCE((c->>'archMortalLevel')::bigint,0),
 'inventory', (SELECT jsonb_agg(jsonb_build_object(
   'index', COALESCE((c->'inv'->inventory.slot->>'index')::integer,0),
   'eff', COALESCE(c->'inv'->inventory.slot->'eff','[0,0,0,0,0,0]'::jsonb)) ORDER BY inventory.slot)
   FROM generate_series(0,$4::integer-1) AS inventory(slot)),
 'fame', CASE WHEN progression.character_uid IS NULL THEN 0 ELSE progression.fame END,
 'noAlternate', COALESCE(c->'alternateCelestial','null'::jsonb)='null'::jsonb,
 'validFace', COALESCE((c->'equip'->'rosto'->>'index')::integer,0)>=COALESCE((c->>'class')::integer,0),
 'freeSlot', EXISTS(SELECT 1 FROM generate_series(0,$3::integer-1) AS slots(slot)
   WHERE COALESCE(a.payload->'chars'->slots.slot->>'name','')=''),
 'exp', COALESCE((c->>'exp')::bigint,0), 'gold', COALESCE((c->>'gold')::bigint,0),
 'cp', COALESCE((c->>'cp')::integer,0),
 'equipment', COALESCE((SELECT jsonb_agg(jsonb_build_object(
   'slot', e.slot, 'itemId', (c->'equip'->e.key->>'index')::integer,
   'effects', COALESCE(c->'equip'->e.key->'eff','[0,0,0,0,0,0]'::jsonb)) ORDER BY e.slot)
 FROM (VALUES ('elmo',1),('armadura',2),('calca',3),('luva',4),('bota',5),
   ('arma',6),('escudo',7),('acessorio1',8),('acessorio2',9),('acessorio3',10),
   ('acessorio4',11),('acessorio5',12),('acessorio6',13),('montaria',14),('capa',15)) AS e(key,slot)
 WHERE COALESCE((c->'equip'->e.key->>'index')::integer,0)>0), '[]'::jsonb)), GREATEST(a.updated_at,progression.updated_at)
FROM accounts a CROSS JOIN LATERAL (SELECT a.payload->'chars'->$2::integer AS c) AS selected
LEFT JOIN public.player_character_progression progression
 ON replace(progression.character_uid::text,'-','')=c->>'uid' AND progression.account_key=a.name_key
WHERE a.name_key=$1 AND COALESCE(c->>'name','')<>'' AND jsonb_typeof(c->'score')='object'`
