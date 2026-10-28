package game

import (
	"encoding/binary"
	"fmt"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

// Entity IDs and physical coordinates deliberately overlap between worlds.
// Tests must exercise recipients and canonical objects, not just map lengths.
func channelIsolationFixture(t *testing.T) (*ChannelCluster, *World, *World, []*Player) {
	t.Helper()
	players := make([]*Player, 4)
	for i, name := range []string{"OneSender", "OnePeer", "TwoSender", "TwoPeer"} {
		players[i], _ = networkedTestPlayer(uint16(i%2+1), name, 2100, 2100)
	}
	one := worldWithNetworkedPlayers(players[0], players[1])
	two := worldWithNetworkedPlayers(players[2], players[3])
	backend := &craftStore{}
	c, err := NewChannelCluster(backend)
	if err != nil {
		t.Fatal(err)
	}
	for i, w := range []*World{one, two} {
		w.store, w.channel = backend, byte(i+1)
		// Production initializes these before attaching worlds.
		w.accountSessions = make(map[string]*gameNet.Session)
		w.chatRateByAccount = make(map[string]*fixedWindowRate)
		w.authClientsByIP = make(map[string]map[*gameNet.Session]struct{})
		w.authClientsByNetwork = make(map[string]map[*gameNet.Session]struct{})
		w.authRateByIP = make(map[string]*fixedWindowRate)
		w.authRateByAccount = make(map[string]*fixedWindowRate)
		if err := c.attach(w); err != nil {
			t.Fatal(err)
		}
	}
	c.enter(one)
	return c, one, two, players
}

func channelIsolationPackets(t *testing.T, p *Player) [][]byte {
	t.Helper()
	var packets [][]byte
	for p.Session.QueuedPacketsForTest() > 0 {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) {
			t.Fatal("invalid outbound packet")
		}
		packets = append(packets, packet)
	}
	return packets
}

func TestChannelIsolationChatAudience(t *testing.T) {
	for _, source := range []int{0, 2} {
		for _, tc := range []struct {
			name, message string
			want          [4]int
		}{
			{"local", "nearby", [4]int{0, 1, 0, 0}},
			{"channel", "@channel", [4]int{0, 1, 0, 0}},
			{"global", "--global", [4]int{0, 1, 1, 1}},
			{"citizenship", "@@citizens", [4]int{0, 1, 1, 0}},
			{"neutral", "@@neutral", [4]int{0, 1, 1, 0}},
			{"guild", "-guild", [4]int{0, 1, 1, 0}},
		} {
			t.Run(fmt.Sprintf("channel-%d/%s", source/2+1, tc.name), func(t *testing.T) {
				c, one, two, players := channelIsolationFixture(t)
				w := []*World{one, two}[source/2]
				// Rotate fixture order so the same audience matrix tests both directions.
				if source == 2 {
					players = append(players[2:], players[:2]...)
				}
				for i, p := range players {
					p.Char.Citizenship = 1
					p.Char.GuildID = 7
					if i == 3 {
						p.Char.Citizenship, p.Char.GuildID = 2, 8
					} else if tc.name == "neutral" {
						p.Char.Citizenship = 0
					}
				}
				guild := model.Guild{ID: 7, Name: "SharedGuild"}
				for _, p := range players[:3] {
					guild.Members = append(guild.Members, model.GuildMember{Character: p.Char.Name, Account: p.Account.Name})
				}
				w.guilds.Guilds = []model.Guild{guild}
				c.enter(w)
				if tc.name == "local" {
					w.onMessageChat(players[0].Session, chatPacket(tc.message))
				} else {
					w.onMessageWhisper(players[0].Session, whisperPacket("", tc.message))
				}
				for i, p := range players {
					packets := channelIsolationPackets(t, p)
					if len(packets) != tc.want[i] {
						t.Errorf("recipient %s got %d packets, want %d", p.Char.Name, len(packets), tc.want[i])
					}
				}
			})
		}
	}
}

func TestChannelIsolationWhisperNeverUsesRemoteEntityID(t *testing.T) {
	for _, source := range []int{0, 2} {
		for _, remote := range []bool{false, true} {
			t.Run(fmt.Sprintf("channel-%d/remote-%t", source/2+1, remote), func(t *testing.T) {
				c, one, two, players := channelIsolationFixture(t)
				w := []*World{one, two}[source/2]
				sender, recipient := players[source], players[source+1]
				if remote {
					recipient = players[2-source]
				}
				c.enter(w)
				w.onMessageWhisper(sender.Session, whisperPacket(recipient.Char.Name, "private payload"))
				for _, p := range players {
					packets := channelIsolationPackets(t, p)
					if p != recipient {
						if len(packets) != 0 {
							t.Errorf("whisper leaked to %s", p.Char.Name)
						}
						continue
					}
					if len(packets) != 1 {
						t.Fatalf("recipient got %d packets", len(packets))
					}
					wantID := sender.ID
					if remote {
						wantID = 0
					}
					if header := wire.ParseHeader(packets[0]); header.Type != 0x334 || header.ID != wantID {
						t.Errorf("whisper header=%+v, want entity ID %d", header, wantID)
					}
					if cstr(packets[0][12:28]) != sender.Char.Name || cstr(packets[0][28:124]) != "private payload" {
						t.Error("whisper sender or payload changed")
					}
				}
			})
		}
	}
}

func TestChannelIsolationInstanceTeardownWithReusedIDs(t *testing.T) {
	for _, channel := range []byte{1, 2} {
		t.Run(fmt.Sprintf("channel-%d", channel), func(t *testing.T) {
			c, one, two, players := channelIsolationFixture(t)
			worlds := []*World{one, two}
			instances := make([]*ItemInstance, 2)
			mobs := make([]*Mob, 2)
			for i, w := range worlds {
				inst := &ItemInstance{RuntimeID: "same-runtime", MobIDs: map[uint16]struct{}{1000: {}}, NPCIDs: map[uint16]struct{}{1001: {}}}
				instances[i] = inst
				w.itemInstances = map[string]*ItemInstance{inst.RuntimeID: inst}
				for _, id := range []uint16{1000, 1001} {
					m := &Mob{ID: id, X: 2100, Y: 2100, HP: 100, InstanceID: inst.RuntimeID}
					w.mobs = append(w.mobs, m)
					w.registerMobSpatial(m)
					if id == 1000 {
						mobs[i] = m
					}
					for _, p := range players[i*2 : i*2+2] {
						p.show(id)
						// Retained viewers have already left the event's spatial window.
						p.X, p.Y = 100, 100
						w.updatePlayerSpatial(p)
					}
				}
				for _, m := range w.mobs {
					w.activeMobs[m.ID] = m
				}
			}
			idx := int(channel - 1)
			c.enter(worlds[idx])
			worlds[idx].retireItemInstanceEntities(instances[idx])
			// Repeated terminal cleanup must not send duplicates or affect another world.
			worlds[idx].retireItemInstanceEntities(instances[idx])
			for i, w := range worlds {
				for _, id := range []uint16{1000, 1001} {
					_, cell := w.mobCell[id]
					if i == idx {
						if w.mobsByID[id] != nil || w.activeMobs[id] != nil || cell {
							t.Error("retired entity remained in a canonical or spatial index")
						}
					} else if w.mobsByID[id] == nil || w.activeMobs[id] == nil || !cell {
						t.Error("teardown removed the other channel's entity")
					}
				}
				if mobs[i].Dead != (i == idx) {
					t.Error("entity death state leaked across channels")
				}
				for _, p := range players[i*2 : i*2+2] {
					packets := channelIsolationPackets(t, p)
					if i != idx {
						if len(packets) != 0 || !p.hasVisible(1000) || !p.hasVisible(1001) {
							t.Error("another channel's client lost its visible entities")
						}
						continue
					}
					if len(packets) != 2 || p.hasVisible(1000) || p.hasVisible(1001) {
						t.Fatalf("terminal teardown packets=%d visible=%v", len(packets), p.Visible)
					}
					removed := map[uint16]bool{}
					for _, packet := range packets {
						if wire.ParseHeader(packet).Type != wire.OpRemoveMob || binary.LittleEndian.Uint32(packet[12:16]) != 3 {
							t.Fatal("teardown did not send immediate removal")
						}
						removed[wire.ParseHeader(packet).ID] = true
					}
					if !removed[1000] || !removed[1001] {
						t.Fatal("teardown removed the wrong IDs")
					}
				}
			}
		})
	}
}

func TestChannelIsolationDisconnectPreservesOtherWorld(t *testing.T) {
	_, one, two, players := channelIsolationFixture(t)
	local, observer, remote := players[0], players[1], players[2]
	local.Account.Name, remote.Account.Name = "shared-account", "shared-account"
	// The source is already released during migration and the destination owns it.
	if !two.claimAccountSession(remote.Session, remote.Account.Name) {
		t.Fatal("destination claim failed")
	}
	observer.show(local.ID)
	remote.show(local.ID)
	one.finishDisconnectAfterInstances(local.Session, local)
	if one.playersByID[local.ID] != nil || two.playersByID[remote.ID] != remote ||
		one.accountSessions[accountSessionKey(remote.Account.Name)] != remote.Session {
		t.Fatal("late source disconnect removed destination identity or account ownership")
	}
	if observer.hasVisible(local.ID) || len(channelIsolationPackets(t, observer)) != 1 ||
		!remote.hasVisible(local.ID) || len(channelIsolationPackets(t, remote)) != 0 {
		t.Fatal("disconnect visibility escaped its owning world")
	}
	if one.playerWorld(remote) != two || one.playerWorld(local) != nil || len(two.globalPlayers()) != 3 {
		t.Fatal("global ownership changed after disconnect")
	}
}
