package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestClientProtocolPacketBuildersHandleMissingRecipients(t *testing.T) {
	char := &model.Char{Score: &model.Score{Version: model.ScoreVersion}}
	session := &gameNet.Session{}
	player := &Player{ID: 1, Session: session, Char: char}

	if playerScorePacketForChar(nil, char) != nil || playerScorePacketForChar(&Player{}, char) != nil ||
		playerScorePacketForChar(player, nil) != nil {
		t.Fatal("score builder accepted an incomplete recipient")
	}
	if len(playerScorePacketForChar(player, char)) == 0 {
		t.Fatal("score builder returned no packet for a live player")
	}
	if playerScorePacket(nil) != nil || playerScorePacket(&Player{Session: session}) != nil ||
		playerScorePacket(&Player{Char: char}) != nil {
		t.Fatal("private score builder accepted an incomplete player")
	}
	if len(playerScorePacket(player)) == 0 {
		t.Fatal("private score builder returned no packet for a live player")
	}
	if playerAffectsPacketAt(nil, time.Time{}) != nil ||
		playerAffectsPacketAt(&Player{Session: session}, time.Time{}) != nil ||
		playerAffectsPacketAt(&Player{Char: char}, time.Time{}) != nil {
		t.Fatal("affect builder accepted an incomplete player")
	}
	if len(playerAffectsPacketAt(player, time.Time{})) == 0 {
		t.Fatal("affect builder returned no packet for a live player")
	}

	if observedPlayerScorePacket(nil, player) != nil ||
		observedPlayerScorePacket(&Player{}, player) != nil ||
		observedPlayerScorePacket(player, nil) != nil ||
		observedPlayerScorePacket(player, &Player{Session: session}) != nil {
		t.Fatal("observer score builder accepted an incomplete observer or subject")
	}
	if len(observedPlayerScorePacket(player, player)) == 0 {
		t.Fatal("observer score builder returned no packet for live players")
	}
}

func TestCharacterSelectionPacketBuildersHandleMissingAccounts(t *testing.T) {
	session := &gameNet.Session{}
	player := &Player{Account: &model.Account{Name: "account"}}
	if selectionUpdatePacket(nil, 1, 2, player) != nil || selectionUpdatePacket(session, 1, 2, nil) != nil ||
		selectionUpdatePacket(session, 1, 2, &Player{}) != nil {
		t.Fatal("selection update accepted a missing session, player, or account")
	}
	if len(selectionUpdatePacket(session, 1, 2, player)) == 0 {
		t.Fatal("selection update returned no packet for an account")
	}
	if characterListPacket(nil, player) != nil || characterListPacket(session, nil) != nil ||
		characterListPacket(session, &Player{}) != nil {
		t.Fatal("character list accepted a missing session, player, or account")
	}
	if len(characterListPacket(session, player)) == 0 {
		t.Fatal("character list returned no packet for an account")
	}
}
