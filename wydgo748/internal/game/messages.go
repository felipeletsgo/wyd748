package game

import (
	"wydgo/internal/lang"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// WithMessages selects the server message catalog. Without it the world uses
// lang.Default(), the catalog found from the working directory.
func WithMessages(catalog *lang.Catalog) WorldOption {
	return func(w *World) { w.messages = catalog }
}

// WithLanguage selects the language of every server message this world sends.
// A world built without it sends English.
func WithLanguage(language lang.Language) WorldOption {
	return func(w *World) { w.language = language }
}

func (w *World) messageCatalog() *lang.Catalog {
	if w.messages != nil {
		return w.messages
	}
	return lang.Default()
}

// msg returns the Windows-1252 text of a server message in this world's
// language. Player-authored text must not pass through it.
func (w *World) msg(key string, args ...any) string {
	return w.messageCatalog().Text(w.language, key, args...)
}

// tr localizes a fixed English catalog text that reached a send point through
// a variable, such as a rejection or an error text. Other text is unchanged.
func (w *World) tr(text string) string {
	return w.messageCatalog().Translate(w.language, text)
}

// recipientWorld returns the channel world of p; guild and whisper delivery
// can reach a player in another channel, whose language may differ.
func (w *World) recipientWorld(p *Player) *World {
	if world := w.playerWorld(p); world != nil {
		return world
	}
	return w
}

// msgFor formats a catalog message in the language of p's channel.
func (w *World) msgFor(p *Player, key string, args ...any) string {
	return w.recipientWorld(p).msg(key, args...)
}

// trFor localizes a fixed English catalog text for p's channel.
func (w *World) trFor(p *Player, text string) string {
	return w.recipientWorld(p).tr(text)
}

// kingdomName returns the display name of a kingdom in this world's language.
func (w *World) kingdomName(kingdom byte) string {
	switch kingdom {
	case model.KingdomHekalotia:
		return w.msg("_NN_KINGDOM_RED")
	case model.KingdomAkelonia:
		return w.msg("_NN_KINGDOM_BLUE")
	}
	return w.msg("_WG_Kingdom_Neutral")
}

// panel sends a catalog message as the native top notice.
func (w *World) panel(s *net.Session, key string, args ...any) {
	s.Send(wire.MessagePanel(w.msg(key, args...)))
}
