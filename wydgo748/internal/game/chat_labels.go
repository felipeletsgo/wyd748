package game

import (
	"strings"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// tabCommandColor accepts only /tab and /tab2..9; /tab1 is not an alias.
func tabCommandColor(name string) (byte, bool) {
	if name == "tab" {
		return 0, true
	}
	if len(name) == 4 && strings.HasPrefix(name, "tab") && name[3] >= '2' && name[3] <= '9' {
		return name[3] - '1', true
	}
	return 0, false
}

// Chat packets and labels are legacy byte strings, not UTF-8 protocol fields.
func boundedChatLabel(text string, limit int) string {
	if end := strings.IndexByte(text, 0); end >= 0 {
		text = text[:end]
	}
	text = strings.TrimSpace(text)
	for i := 0; i < len(text); i++ {
		if text[i] < 32 || text[i] == 127 {
			text = text[:i] + " " + text[i+1:]
		}
	}
	if len(text) > limit {
		text = text[:limit]
	}
	return text
}

func (w *World) setPlayerTab(s *net.Session, p *Player, text string, color byte) {
	if s == nil || p == nil || p.Char == nil || !p.InWorld {
		return
	}
	if !w.allowChat(p, "labels", w.now()) {
		return
	}
	// W2PP's server rule: the displayed Mortal level must be at least 70.
	// Clearing an existing label is always permitted.
	text = boundedChatLabel(text, 25)
	mortal := p.Char.Evolution == "" || strings.EqualFold(p.Char.Evolution, "mortal")
	if text != "" && mortal && displayLevel(p.Char) < 70 {
		s.Send(wire.MessagePanel("Mortal characters need level 70 to set a tab."))
		return
	}
	if text == "" || color > 8 {
		color = 0
	}
	if p.TabText == text && p.TabColor == color {
		return
	}
	p.TabText, p.TabColor = text, color
	// The native command republishes metadata. The source client handles an
	// existing actor in place: no InitObject, position reset or action packet.
	w.syncPlayerChaos(p)
}

func (w *World) setPlayerStatus(s *net.Session, p *Player, text string) {
	if s == nil || p == nil || p.Char == nil || !p.InWorld || !w.allowChat(p, "labels", w.now()) {
		return
	}
	// Reserve eight bytes for the "Status: " prefix. Apart from identifying
	// the notice, it prevents user text from invoking !!/!# panel controls.
	p.StatusMessage = boundedChatLabel(text, 87)
	if p.StatusMessage == "" {
		s.Send(wire.MessagePanel("Status message cleared."))
	} else {
		s.Send(wire.MessagePanel("Status message saved. Others can read it with /" + p.Char.Name + "."))
	}
}
