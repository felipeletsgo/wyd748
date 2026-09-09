package model

// Cidadania, portada do W2PP (CMob.cpp:1526 e SendFunc.cpp:3126).
//
// O valor de Citizenship e o NUMERO DO CANAL (no nativo, ServerIndex+1), nao um
// booleano: "Cidadania 2" significa cidadao do canal 2. Isso importa porque o
// bonus principal so vale no proprio canal -- ser cidadao de outro canal nao
// rende nada ali.
//
// A cidadania NAO da bonus de EXP no nativo. Os bonus de EXP vem de itens,
// evento Kefra e affect 39; a cidadania mexe somente no drop.
const (
	// CitizenChannelDropBonus e o +10% de drop por ser cidadao do canal atual.
	CitizenChannelDropBonus = 10

	// CitizenExpDropStep: a cada 100 milhoes de EXP acumulada o cidadao ganha
	// +1% de drop...
	CitizenExpDropStep = 100_000_000
	// ...ate o teto de +80%.
	CitizenExpDropMax = 80
)

// CitizenDropBonus devolve o bonus de drop em porcentagem, portando as duas
// regras nativas:
//
//  1. +10 se a cidadania for a do canal atual;
//  2. +Exp/100kk (teto 80) para QUALQUER cidadania diferente de zero -- essa
//     parte nao exige que seja o canal corrente, so que exista cidadania.
func CitizenDropBonus(citizenship byte, currentChannel byte, exp uint32) int {
	if citizenship == 0 {
		return 0
	}
	bonus := 0
	if citizenship == currentChannel {
		bonus += CitizenChannelDropBonus
	}
	if exp >= CitizenExpDropStep {
		byExp := int(exp / CitizenExpDropStep)
		if byExp > CitizenExpDropMax {
			byExp = CitizenExpDropMax
		}
		bonus += byExp
	}
	return bonus
}
