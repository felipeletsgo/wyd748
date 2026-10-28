package game

// Called only on the cluster actor. Global services use every channel while
// spatial broadcasts, combat, parties and world IDs stay local to their World.
func (w *World) channelWorlds() []*World {
	if w.cluster != nil {
		return w.cluster.worlds
	}
	return []*World{w}
}

func (w *World) globalPlayerByName(name string) (*World, *Player) {
	for _, channel := range w.channelWorlds() {
		if p := channel.playerByCharacterName(name); p != nil {
			return channel, p
		}
	}
	return nil, nil
}

func (w *World) guildPlayerByName(name string) *Player {
	_, player := w.globalPlayerByName(name)
	return player
}

// World IDs are local. Resolve session and pointer identity before inspecting
// spatial indices or publishing an update for a shared guild transaction.
func (w *World) playerWorld(player *Player) *World {
	if player == nil || player.Session == nil {
		return nil
	}
	for _, channel := range w.channelWorlds() {
		if channel.players[player.Session] == player {
			return channel
		}
	}
	return nil
}

func (w *World) globalPlayers() []*Player {
	var players []*Player
	for _, channel := range w.channelWorlds() {
		for _, p := range channel.players {
			if p != nil && p.InWorld && p.Char != nil && p.Session != nil && !p.Session.IsClosed() {
				players = append(players, p)
			}
		}
	}
	return players
}

// Administrative snapshots include authenticated players before field entry.
func (w *World) channelPlayers() []*Player {
	var players []*Player
	for _, channel := range w.channelWorlds() {
		for _, player := range channel.players {
			players = append(players, player)
		}
	}
	return players
}
