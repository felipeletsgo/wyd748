package playerportal

import (
	"context"
	"encoding/json"
	"errors"
	"net/http"
	"regexp"
	"sort"
	"strings"
	"time"

	"wydgo/internal/model"
)

type PublicGuildMember struct {
	Name string `json:"name"`
	Rank byte   `json:"rank"`
}

// Only this projection is readable by the portal database role. Account keys,
// join timestamps and private war state are deliberately absent.
type SavedPublicGuild struct {
	ID        uint16              `json:"id"`
	Name      string              `json:"name"`
	Kingdom   byte                `json:"kingdom"`
	Fame      uint32              `json:"fame"`
	Ally      uint16              `json:"ally"`
	WarTarget uint16              `json:"warTarget"`
	CreatedAt time.Time           `json:"createdAt"`
	Members   []PublicGuildMember `json:"members"`
}

type PublicGuildSnapshot struct {
	Version int                `json:"version"`
	Guilds  []SavedPublicGuild `json:"guilds"`
	AsOf    time.Time          `json:"-"`
}

type PublicGuildProfile struct {
	Name      string              `json:"name"`
	Kingdom   byte                `json:"kingdom"`
	Fame      uint32              `json:"fame"`
	CreatedAt time.Time           `json:"createdAt"`
	Members   []PublicGuildMember `json:"members"`
	Ally      *string             `json:"ally"`
	WarTarget *string             `json:"warTarget"`
}

var publicGuildName = regexp.MustCompile(`^[A-Za-z0-9](?:[A-Za-z0-9 ]{0,10}[A-Za-z0-9])?$`)

func validatePublicGuilds(snapshot PublicGuildSnapshot) error {
	invalid := errors.New("portal: invalid public guild snapshot")
	if snapshot.Version != model.GuildRegistryVersion || snapshot.Guilds == nil || len(snapshot.Guilds) > model.MaxGuildID || snapshot.AsOf.IsZero() {
		return invalid
	}
	ids, names, characters := map[uint16]bool{}, map[string]bool{}, map[string]bool{}
	for _, g := range snapshot.Guilds {
		key := strings.ToLower(g.Name)
		if g.ID < model.MinGuildID || g.ID > model.MaxGuildID || ids[g.ID] || names[key] || !publicGuildName.MatchString(g.Name) || len(g.Members) == 0 ||
			(g.Kingdom != model.KingdomNeutral && g.Kingdom != model.KingdomHekalotia && g.Kingdom != model.KingdomAkelonia) {
			return invalid
		}
		ids[g.ID], names[key] = true, true
		leaders, ranks := 0, map[byte]bool{}
		for _, member := range g.Members {
			key := strings.ToLower(member.Name)
			// Character names are public. Accept the established 16-byte name
			// boundary without inventing a new gameplay name policy.
			if member.Name == "" || len(member.Name) > 16 || strings.ContainsAny(member.Name, "\x00\r\n") || characters[key] {
				return invalid
			}
			characters[key] = true
			if member.Rank == model.GuildRankLeader {
				leaders++
			} else if model.IsSubLeader(member.Rank) {
				if ranks[member.Rank] {
					return invalid
				}
				ranks[member.Rank] = true
			} else if member.Rank != model.GuildRankMember {
				return invalid
			}
		}
		if leaders != 1 {
			return invalid
		}
	}
	for _, g := range snapshot.Guilds {
		for _, id := range []uint16{g.Ally, g.WarTarget} {
			if id != 0 && (id == g.ID || !ids[id]) {
				return invalid
			}
		}
	}
	return nil
}

func (d *Database) PublicGuilds(ctx context.Context) (PublicGuildSnapshot, error) {
	ctx, cancel := context.WithTimeout(ctx, 3*time.Second)
	defer cancel()
	var snapshot PublicGuildSnapshot
	var projection []byte
	err := d.pool.QueryRow(ctx, `SELECT CASE WHEN g.updated_at IS NULL THEN '{"version":1,"guilds":[]}'::jsonb ELSE g.projection END,
 COALESCE(g.updated_at,statement_timestamp()) FROM (SELECT 1) seed LEFT JOIN public.player_public_guilds g ON true`).Scan(&projection, &snapshot.AsOf)
	if err != nil {
		return PublicGuildSnapshot{}, err
	}
	if err := json.Unmarshal(projection, &snapshot); err != nil {
		return PublicGuildSnapshot{}, err
	}
	if err := validatePublicGuilds(snapshot); err != nil {
		return PublicGuildSnapshot{}, err
	}
	return snapshot, nil
}

// All names share one bounded, coalesced SQL snapshot. Public visitors cannot
// fan out live World queries or grow a per-name cache. Failures have a short
// cooldown and never serve an old roster as a fresh successful response.
func (h *Handler) publicGuildSnapshot(ctx context.Context) (PublicGuildSnapshot, error) {
	for {
		h.guildMu.Lock()
		if !h.guildAt.IsZero() && h.now().Before(h.guildAt) {
			cached, err := h.guildSnapshot, h.guildError
			h.guildMu.Unlock()
			return cached, err
		}
		if flight := h.guildFlight; flight != nil {
			h.guildMu.Unlock()
			select {
			case <-flight:
				continue
			case <-ctx.Done():
				return PublicGuildSnapshot{}, ctx.Err()
			}
		}
		flight := make(chan struct{})
		h.guildFlight = flight
		h.guildMu.Unlock()
		snapshot, err := h.cfg.Backend.PublicGuilds(ctx)
		if err == nil {
			err = validatePublicGuilds(snapshot)
		}
		ttl := time.Minute
		if err != nil {
			snapshot, ttl = PublicGuildSnapshot{}, 5*time.Second
		}
		h.guildMu.Lock()
		h.guildSnapshot, h.guildError, h.guildAt = snapshot, err, h.now().Add(ttl)
		h.guildFlight = nil
		close(flight)
		h.guildMu.Unlock()
		return snapshot, err
	}
}

func (h *Handler) guildProfile(w http.ResponseWriter, r *http.Request) {
	name := r.PathValue("name")
	if !publicGuildName.MatchString(name) {
		failure(w, 400, "Invalid guild name")
		return
	}
	snapshot, err := h.publicGuildSnapshot(r.Context())
	if err != nil {
		failure(w, 503, "Guild unavailable")
		return
	}
	byID := make(map[uint16]string, len(snapshot.Guilds))
	var selected *SavedPublicGuild
	for i := range snapshot.Guilds {
		guild := &snapshot.Guilds[i]
		byID[guild.ID] = guild.Name
		if strings.EqualFold(guild.Name, name) {
			selected = guild
		}
	}
	if selected == nil {
		failure(w, 404, "Guild not found")
		return
	}
	members := append([]PublicGuildMember(nil), selected.Members...)
	sort.Slice(members, func(i, j int) bool {
		if members[i].Rank != members[j].Rank {
			return members[i].Rank > members[j].Rank
		}
		return strings.ToLower(members[i].Name) < strings.ToLower(members[j].Name)
	})
	profile := PublicGuildProfile{Name: selected.Name, Kingdom: selected.Kingdom, Fame: selected.Fame, CreatedAt: selected.CreatedAt, Members: members}
	if value, ok := byID[selected.Ally]; ok {
		profile.Ally = &value
	}
	if value, ok := byID[selected.WarTarget]; ok {
		profile.WarTarget = &value
	}
	reply(w, 200, map[string]any{"version": 1, "guild": profile, "asOf": snapshot.AsOf.UTC(), "source": "persisted snapshot"})
}
