package game

import (
	"bytes"
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestShoutPublishesNativeNamedChatOnlyAfterPayment(t *testing.T) {
	for _, scenario := range []string{"success", "save failure", "missing item", "empty message"} {
		t.Run(scenario, func(t *testing.T) {
			sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
			// Shouts are global, not limited to the sender's local visibility.
			recipient, _ := networkedTestPlayer(2, "Recipient", 1000, 1000)
			w := worldWithNetworkedPlayers(sender, recipient)
			st := &craftStore{}
			w.store = st
			item := model.Item{Index: magicTrumpet}
			if scenario != "missing item" {
				sender.Char.Inv[0] = item
			}
			if scenario == "save failure" {
				st.err = errors.New("injected shout save failure")
			}
			text := "!literal announcement"
			if scenario == "empty message" {
				text = ""
			}
			w.executeShout(sender.Session, sender, text)
			if scenario != "success" {
				if recipient.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("unpaid/rejected shout reached a recipient")
				}
				if scenario != "missing item" && sender.Char.Inv[0] != item {
					t.Fatal("rejected shout consumed the item")
				}
				return
			}
			if st.saves != 1 || sender.Char.Inv[0].Index != 0 {
				t.Fatal("successful shout did not persist payment once")
			}
			pkt, ok := recipient.Session.DequeuePacketForTest()
			if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessageShout ||
				wire.ParseHeader(pkt).ID != 0 || len(pkt) != 108 ||
				string(bytes.TrimRight(pkt[12:], "\x00")) != "[Sender]> "+text {
				t.Fatalf("recipient did not receive native green literal named chat: %x", pkt)
			}
			if recipient.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("shout broadcast duplicated")
			}
			// Item is gone: a repeated request must not publish again.
			w.executeShout(sender.Session, sender, text)
			if st.saves != 1 || recipient.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("repeated unpaid shout was accepted")
			}
		})
	}
}
