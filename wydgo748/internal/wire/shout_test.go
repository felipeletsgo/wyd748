package wire

import (
	"bytes"
	"strings"
	"testing"
)

func TestPlayerShoutNativeChatContract(t *testing.T) {
	for _, text := range []string{"hello", "!literal", "=literal", "--literal", "@literal", strings.Repeat("x", 120)} {
		pkt := PlayerShout("Rider", text)
		h := ParseHeader(pkt)
		if len(pkt) != 108 || h.Type != OpMessageShout || h.Type == OpMessagePanel || h.ID != 0 {
			t.Fatalf("invalid native green chat envelope: %x", pkt)
		}
		want := "[Rider]> " + text
		if len(want) > 94 {
			want = want[:94]
		}
		if got := string(bytes.TrimRight(pkt[12:], "\x00")); got != want || pkt[106] != 0 || pkt[107] != 0 {
			t.Fatalf("literal terminated shout = %q, want %q", got, want)
		}
	}
}
