package game

import (
	"fmt"
	"net/netip"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestChannelAdmissionLimitsAcrossWorlds(t *testing.T) {
	for _, first := range []byte{1, 2} {
		for _, kind := range []string{"ipv4", "ipv6-prefix", "network-rule"} {
			t.Run(fmt.Sprintf("channel-%d/%s", first, kind), func(t *testing.T) {
				c, _, _, _ := channelIsolationFixture(t)
				a, b := c.world(first), c.world(3-first)
				ip, secondIP := "192.0.2.40", "::ffff:192.0.2.40"
				if kind == "ipv6-prefix" {
					ip, secondIP = "2001:db8:1:2::1", "2001:db8:1:2::abcd"
				}
				for _, w := range c.worlds {
					w.operational.MaxAuthenticatedClientsPerIP = 1
					if kind == "network-rule" {
						w.operational.MaxAuthenticatedClientsPerIP = 4
						WithNetworkAdmission(model.NetworkAdmissionFile{
							Version: model.NetworkAdmissionVersion,
							Rules:   []model.NetworkAdmissionRule{{CIDR: netip.MustParsePrefix("192.0.2.0/24"), Action: model.NetworkAdmissionLimit, MaxClients: 1}},
						})(w)
						secondIP = "192.0.2.41"
					}
				}
				source := gameNet.NewTestSessionWithRemoteIP(20, 8, ip)
				// The same numeric socket ID must not alias admission ownership.
				remote := gameNet.NewTestSessionWithRemoteIP(20, 8, secondIP)
				if !a.claimAuthenticatedClientSlot(source) || b.claimAuthenticatedClientSlot(remote) {
					t.Fatal("changing channels bypassed the authenticated connection limit")
				}
				b.releaseAuthenticatedClientSlot(remote)
				if b.claimAuthenticatedClientSlot(remote) {
					t.Fatal("foreign cleanup released another channel's live socket")
				}
				source.Close()
				if !b.claimAuthenticatedClientSlot(remote) {
					t.Fatal("closed source socket retained a shared admission slot")
				}
				a.releaseAuthenticatedClientSlot(source)
				third := gameNet.NewTestSessionWithRemoteIP(21, 8, ip)
				if a.claimAuthenticatedClientSlot(third) {
					t.Fatal("late source cleanup removed the destination's admission slot")
				}
				b.releaseAuthenticatedClientSlot(remote)
				if !a.claimAuthenticatedClientSlot(third) {
					t.Fatal("destination logout did not release shared capacity")
				}
			})
		}
	}
}

func TestChannelAdmissionLoginRateCannotResetByChangingWorlds(t *testing.T) {
	for _, first := range []byte{1, 2} {
		for _, kind := range []string{"account", "origin"} {
			t.Run(fmt.Sprintf("channel-%d/%s", first, kind), func(t *testing.T) {
				c, _, _, _ := channelIsolationFixture(t)
				a, b := c.world(first), c.world(3-first)
				for _, w := range c.worlds {
					w.operational.AuthAttemptsPerMinuteIP = 10
					w.operational.AuthAttemptsPerMinuteAccount = 10
					if kind == "account" {
						w.operational.AuthAttemptsPerMinuteAccount = 1
					} else {
						w.operational.AuthAttemptsPerMinuteIP = 1
					}
				}
				now := time.Date(2026, 10, 26, 12, 0, 0, 0, time.UTC)
				ip, account := "192.0.2.40", "shared"
				if !a.allowLoginAttempt(ip, account, now) {
					t.Fatal("initial attempt rejected")
				}
				if kind == "account" {
					ip = "192.0.2.41"
				} else {
					account = "another"
				}
				if b.allowLoginAttempt(ip, account, now) {
					t.Fatal("changing channels reset the shared login rate limit")
				}
				if !b.allowLoginAttempt(ip, account, now.Add(time.Minute)) {
					t.Fatal("expired shared login window did not renew")
				}
			})
		}
	}
}

func TestChannelAdmissionChatRateSurvivesWorldChange(t *testing.T) {
	for _, first := range []byte{1, 2} {
		for _, kind := range []string{"local", chatChannelWhisper, chatChannelGlobal, chatChannelServer} {
			t.Run(fmt.Sprintf("channel-%d/%s", first, kind), func(t *testing.T) {
				c, _, _, players := channelIsolationFixture(t)
				a, b := c.world(first), c.world(3-first)
				p, destination := players[int(first-1)*2], players[int(2-first)*2]
				// Model the same account reloaded into a distinct destination object.
				destination.Account.Name = p.Account.Name
				for _, w := range c.worlds {
					w.operational.ChatLocalPer10Seconds = 1
					w.operational.ChatWhisperPer10Seconds = 1
					w.operational.ChatGlobalPer10Seconds = 1
				}
				now := time.Date(2026, 10, 26, 12, 0, 0, 0, time.UTC)
				if !a.allowChat(p, kind, now) || b.allowChat(destination, kind, now) {
					t.Fatal("changing channels reset the shared chat rate limit")
				}
				if !b.allowChat(destination, kind, now.Add(10*time.Second)) {
					t.Fatal("expired shared chat window did not renew")
				}
			})
		}
	}
}
