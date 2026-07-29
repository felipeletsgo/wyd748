package model

// CharState e o estado de SESSAO persistido de um personagem, guardado num
// registro separado da ficha da conta. Em producao vive em character_states,
// identificado pelo Char.UID; o adaptador JSON usa charstate/<UID>.json. Serve
// para buffs ativos e contadores que precisam sobreviver ao relog.
//
// Ficar num sidecar mantem o contrato estrito do account JSON intacto: affects
// tem tempo absoluto de expiracao e sao um estado volatil, nao parte da ficha.
type CharState struct {
	Version int `json:"version"`
	// Affects sao os buffs/debuffs ativos com expiracao ABSOLUTA (unix segundos).
	// No load, os ja expirados sao descartados e o restante recomposto com o
	// tempo que sobrou.
	Affects []PersistedAffect `json:"affects,omitempty"`
	// SpecialCoins guarda moedas especiais por nome. Ainda nao ha logica de moeda;
	// o campo ja persiste em round-trip para quando ela existir.
	SpecialCoins map[string]uint32 `json:"specialCoins,omitempty"`
}

// PersistedAffect e a forma serializavel de um Affect: os campos de dominio mais
// o instante absoluto de expiracao.
type PersistedAffect struct {
	Type        byte   `json:"type"`
	ClientType  byte   `json:"clientType,omitempty"`
	Value       int    `json:"value,omitempty"`
	Level       int    `json:"level,omitempty"`
	OwnerID     uint16 `json:"ownerId,omitempty"`
	ExpiresUnix int64  `json:"expiresUnix"`
}

// CharStateVersion e a versao do contrato do sidecar. Sobe quando o formato muda.
const CharStateVersion = 1
