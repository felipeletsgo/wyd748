package model

import "encoding/json"

// The pre-channel registry belonged exclusively to channel 1. Upgrade once,
// preserving an explicitly stored citizenship and every subsequent neutral 0.
func (r *GuildRegistry) UpgradeChannelCitizenship() {
	if r == nil || r.CitizenshipVersion != 0 {
		return
	}
	for i := range r.Guilds {
		if r.Guilds[i].Citizenship == 0 {
			r.Guilds[i].Citizenship = 1
		}
	}
	r.CitizenshipVersion = 1
}

// SelectWarChannel belongs to the single cluster actor. Persisted legacy Wars
// always represents channel 1; global identity and fame remain shared.
func (r *GuildRegistry) SelectWarChannel(channel byte) {
	if r.ActiveWarChannel == 0 {
		r.ActiveWarChannel = 1
	}
	if r.ActiveWarChannel == channel && r.ChannelWars != nil {
		return
	}
	if r.ChannelWars == nil {
		r.ChannelWars = make(map[byte]GuildWarState)
	}
	if r.Wars.FameResetDay > r.FameResetDay {
		r.FameResetDay = r.Wars.FameResetDay
	}
	r.ChannelWars[r.ActiveWarChannel] = r.Wars
	r.ActiveWarChannel = channel
	r.Wars = r.ChannelWars[channel]
	r.Wars.FameResetDay = r.FameResetDay
}

func (r GuildRegistry) Clone() *GuildRegistry {
	r.Guilds = append([]Guild(nil), r.Guilds...)
	for i := range r.Guilds {
		r.Guilds[i].Members = append([]GuildMember(nil), r.Guilds[i].Members...)
	}
	r.Wars = r.Wars.Clone()
	if r.ChannelWars != nil {
		wars := make(map[byte]GuildWarState, len(r.ChannelWars))
		for channel, state := range r.ChannelWars {
			wars[channel] = state.Clone()
		}
		r.ChannelWars = wars
	}
	if r.ActiveWarChannel != 0 {
		if r.ChannelWars == nil {
			r.ChannelWars = make(map[byte]GuildWarState)
		}
		r.ChannelWars[r.ActiveWarChannel] = r.Wars.Clone()
	}
	return &r
}

func (r GuildRegistry) MarshalJSON() ([]byte, error) {
	type plain GuildRegistry
	if r.ActiveWarChannel == 0 {
		return json.Marshal(plain(r))
	}
	copy := r.Clone()
	copy.ChannelWars[copy.ActiveWarChannel] = copy.Wars
	copy.Wars = copy.ChannelWars[1]
	return json.Marshal(plain(*copy))
}

// Clone detaches every instance and its membership before worker publication.
// Channels contains flat snapshots; nested channel maps are never produced.
func (s *InstanceStateSnapshot) Clone() *InstanceStateSnapshot {
	if s == nil {
		return nil
	}
	copy := *s
	copy.Instances = append([]InstanceRuntimeState(nil), s.Instances...)
	for i := range copy.Instances {
		copy.Instances[i].MemberCharacterUIDs = append([]string(nil), copy.Instances[i].MemberCharacterUIDs...)
	}
	copy.NightmarePartyRuns = make(map[string]int, len(s.NightmarePartyRuns))
	for key, count := range s.NightmarePartyRuns {
		copy.NightmarePartyRuns[key] = count
	}
	if s.Channels != nil {
		copy.Channels = make(map[byte]*InstanceStateSnapshot, len(s.Channels))
		for channel, state := range s.Channels {
			copy.Channels[channel] = state.Clone()
		}
	}
	return &copy
}
