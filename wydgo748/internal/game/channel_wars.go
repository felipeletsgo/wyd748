package game

// The shared cluster applies the owner's strict citizenship policy. The
// standalone legacy world retains its existing admission rules.
func (w *World) channelWarEligible(p *Player) bool {
	if p == nil || p.Char == nil {
		return false
	}
	if w.cluster == nil {
		return true
	}
	guild, member := w.guildOf(p.Char)
	return guild != nil && member != nil && guild.Citizenship == w.channel &&
		p.Char.Citizenship == w.channel && w.channel != 0
}
