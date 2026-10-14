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

// Storage projections contain display fields only, never persisted identities.
// Slots are zero-based selectors within this saved snapshot, not item IDs.
type StoredItem struct {
	Slot     int     `json:"slot"`
	ItemID   uint16  `json:"itemId"`
	Name     string  `json:"name"`
	Effects  [6]byte `json:"effects"`
	Quantity uint16  `json:"quantity"`
}

type StorageSnapshot struct {
	AsOf          time.Time    `json:"asOf"`
	Capacity      int          `json:"capacity"`
	CharacterSlot *int         `json:"characterSlot,omitempty"`
	Gold          *uint32      `json:"gold,omitempty"`
	Items         []StoredItem `json:"items"`
}

func (h *Handler) inventory(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.current(r)
	if !ok || s.Account == "" {
		failure(w, 401, "Sign in to view your inventory")
		return
	}
	selector := r.PathValue("slot")
	if len(selector) != 1 || selector[0] < '0' || selector[0] > '3' {
		failure(w, 400, "Invalid character slot")
		return
	}
	snapshot, err := h.cfg.Backend.Inventory(r.Context(), s.Account, int(selector[0]-'0'))
	h.storageReply(w, snapshot, err)
}

func (h *Handler) cargo(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.current(r)
	if !ok || s.Account == "" {
		failure(w, 401, "Sign in to view your Cargo")
		return
	}
	snapshot, err := h.cfg.Backend.Cargo(r.Context(), s.Account)
	h.storageReply(w, snapshot, err)
}

func (h *Handler) storageReply(w http.ResponseWriter, snapshot StorageSnapshot, err error) {
	if errors.Is(err, os.ErrNotExist) {
		failure(w, 404, "Saved storage not found")
		return
	}
	if err != nil {
		failure(w, 503, "Storage unavailable")
		return
	}
	for i := range snapshot.Items {
		item := &snapshot.Items[i]
		item.Name = strings.ReplaceAll(h.cfg.ItemNames[item.ItemID], "_", " ")
		item.Quantity = storedQuantity(*item)
	}
	reply(w, 200, map[string]any{"version": 1, "storage": snapshot, "source": "persisted snapshot"})
}

// Effect 61 is the instance quantity, not a static catalogue default. Mounts
// 2330..2389 reuse that byte for HP and must always display one item.
func storedQuantity(item StoredItem) uint16 {
	if item.ItemID >= 2330 && item.ItemID <= 2389 {
		return 1
	}
	for pair := 0; pair < 3; pair++ {
		if item.Effects[pair*2] == 61 {
			return max(1, uint16(item.Effects[pair*2+1]))
		}
	}
	return 1
}

func (d *Database) Inventory(ctx context.Context, name string, slot int) (StorageSnapshot, error) {
	if slot < 0 || slot > 3 {
		return StorageSnapshot{}, os.ErrNotExist
	}
	return d.readStorage(ctx, inventorySQL, strings.ToLower(name), slot, model.PlayerCarrySlots)
}

func (d *Database) Cargo(ctx context.Context, name string) (StorageSnapshot, error) {
	return d.readStorage(ctx, cargoSQL, strings.ToLower(name), model.PlayerCargoSlots)
}

func (d *Database) readStorage(ctx context.Context, query string, args ...any) (StorageSnapshot, error) {
	ctx, cancel := context.WithTimeout(ctx, 3*time.Second)
	defer cancel()
	var result StorageSnapshot
	var projection []byte
	err := d.pool.QueryRow(ctx, query, args...).Scan(&projection, &result.AsOf)
	if errors.Is(err, pgx.ErrNoRows) {
		return StorageSnapshot{}, os.ErrNotExist
	}
	if err != nil {
		return StorageSnapshot{}, err
	}
	if err := json.Unmarshal(projection, &result); err != nil {
		return StorageSnapshot{}, err
	}
	return result, nil
}

// Filtering happens inside SQL: reserved ABI cells and internal item data
// never leave PostgreSQL. Each collection and timestamp share one statement.
const inventorySQL = `SELECT jsonb_build_object(
 'capacity',$3::integer,'characterSlot',$2::integer,
 'items',COALESCE((SELECT jsonb_agg(jsonb_build_object(
   'slot',ordinal-1,'itemId',(item->>'index')::integer,
   'effects',COALESCE(item->'eff','[0,0,0,0,0,0]'::jsonb)) ORDER BY ordinal)
 FROM jsonb_array_elements(c->'inv') WITH ORDINALITY AS cells(item,ordinal)
 WHERE ordinal<=$3::integer AND COALESCE((item->>'index')::integer,0)>0),'[]'::jsonb)),updated_at
FROM accounts CROSS JOIN LATERAL (SELECT payload->'chars'->$2::integer AS c) AS selected
WHERE name_key=$1 AND COALESCE(c->>'name','')<>'' AND jsonb_typeof(c->'score')='object'`

const cargoSQL = `SELECT jsonb_build_object(
 'capacity',$2::integer,'gold',COALESCE((payload->>'cargoGold')::bigint,0),
 'items',COALESCE((SELECT jsonb_agg(jsonb_build_object(
   'slot',ordinal-1,'itemId',(item->>'index')::integer,
   'effects',COALESCE(item->'eff','[0,0,0,0,0,0]'::jsonb)) ORDER BY ordinal)
 FROM jsonb_array_elements(payload->'cargo') WITH ORDINALITY AS cells(item,ordinal)
 WHERE ordinal<=$2::integer AND COALESCE((item->>'index')::integer,0)>0),'[]'::jsonb)),updated_at
FROM accounts WHERE name_key=$1`
