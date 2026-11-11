package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestEnterWorldRejectsMissingPlayerAccountAndMalformedPacket(t *testing.T) {
	for _, tc := range []struct {
		name   string
		change func(*World, *Player)
		packet func() []byte
	}{
		{name: "missing player", change: func(w *World, p *Player) { delete(w.players, p.Session) }},
		{name: "missing account", change: func(_ *World, p *Player) { p.Account = nil }},
		{name: "short packet", packet: func() []byte {
			return enterWorldPacket(0)[:characterLoginPacketSize-1]
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, session := newEnterWorldPlayer(t, 742, 1000)
			if tc.change != nil {
				tc.change(w, p)
			}
			packet := enterWorldPacket(0)
			if tc.packet != nil {
				packet = tc.packet()
			}

			w.onEnterWorld(session, packet)

			if p.InWorld || p.Char != nil || p.CharSlot != -1 || p.ID != 0 ||
				session.QueuedPacketsForTest() != 0 {
				t.Fatal("invalid character entry request changed player state or published packets")
			}
		})
	}
}

func TestEnterWorldRestoresExpiredItemsWhenPersistenceFails(t *testing.T) {
	w, p, session := newEnterWorldPlayer(t, 742, 1000)
	now := time.Unix(2_000_000_000, 0)
	w.clock = newFakeClock(now)
	expired := model.Item{
		Index: 4300, UID: "expired-entry-item",
		ActivatedUnix: now.Add(-time.Hour).Unix(), ExpiresUnix: now.Unix(),
	}
	p.Account.Chars[0].Inv[3] = expired
	st := &craftStore{err: errors.New("timed item save failed")}
	w.store = st

	w.onEnterWorld(session, enterWorldPacket(0))

	if p.InWorld || p.Char != nil || p.CharSlot != -1 || p.Account.Chars[0].Inv[3] != expired ||
		st.saves != 1 || session.QueuedPacketsForTest() == 0 {
		t.Fatal("failed timed-item persistence consumed the expired item or admitted the player")
	}
}

func TestEnterWorldLegacyCapeEmblemPersistsBeforeEntry(t *testing.T) {
	for _, tc := range []struct {
		name    string
		failure error
	}{
		{name: "commit"},
		{name: "save rejection", failure: errors.New("cape emblem save failed")},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, session := newEnterWorldPlayer(t, 742, 1000)
			cape := model.Item{Index: 545, UID: "legacy-entry-cape"}
			p.Account.Chars[0].Equip[model.CapeSlot] = cape
			w.items = map[uint16]model.ItemDef{cape.Index: {Index: cape.Index}}
			rng := &capeEmblemRNG{value: 4}
			w.rng = rng
			st := &craftStore{err: tc.failure}
			w.store = st

			w.onEnterWorld(session, enterWorldPacket(0))

			if tc.failure != nil {
				if p.InWorld || p.Char != nil || p.CharSlot != -1 ||
					p.Account.Chars[0].CapeEmblem != 0 || p.Account.Chars[0].Equip[model.CapeSlot] != cape ||
					rng.calls != 1 || st.saves != 1 || session.QueuedPacketsForTest() == 0 {
					t.Fatal("failed cape migration changed character state or entered the world")
				}
				return
			}
			if !p.InWorld || p.Char.CapeEmblem != 5 || p.Char.Equip[model.CapeSlot] != cape ||
				rng.calls != 1 || st.saves != 1 || session.QueuedPacketsForTest() == 0 {
				t.Fatal("successful cape migration did not persist its one-time draw before entry")
			}
		})
	}
}
