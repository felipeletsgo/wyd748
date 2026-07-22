package model

import "testing"

// Regras portadas de CMob.cpp:1526 e SendFunc.cpp:3126.
func TestCitizenDropBonus(t *testing.T) {
	const canal = byte(1)

	cases := []struct {
		nome        string
		citizenship byte
		exp         uint32
		want        int
	}{
		{"sem cidadania nao ganha nada", 0, 0, 0},
		{"sem cidadania ignora ate a exp alta", 0, 5_000_000_000 % 4_000_000_000, 0},
		{"cidadao do canal atual ganha 10", canal, 0, 10},
		{"cidadao de OUTRO canal nao ganha os 10", 2, 0, 0},
		{"exp abaixo de 100kk nao soma", canal, 99_999_999, 10},
		{"exp de 100kk soma 1", canal, 100_000_000, 11},
		{"exp de 500kk soma 5", canal, 500_000_000, 15},
		// A parte por EXP nao exige o canal atual, so cidadania != 0.
		{"cidadao de outro canal ainda soma por exp", 2, 300_000_000, 3},
	}
	for _, tc := range cases {
		if got := CitizenDropBonus(tc.citizenship, canal, tc.exp); got != tc.want {
			t.Fatalf("%s: bonus=%d, quer %d", tc.nome, got, tc.want)
		}
	}
}

// O teto de +80% existe no nativo para a parte por EXP.
func TestCitizenDropBonusRespeitaOTetoPorExp(t *testing.T) {
	const canal = byte(1)
	// 4 bilhoes de EXP dariam +40; para estourar o teto seria preciso mais que
	// o uint32 comporta, entao o teto e verificado direto na constante.
	if got := CitizenDropBonus(canal, canal, 4_000_000_000); got != 10+40 {
		t.Fatalf("4kkk de exp deveria dar 10+40, deu %d", got)
	}
	if CitizenExpDropMax != 80 {
		t.Fatalf("teto nativo e 80, esta %d", CitizenExpDropMax)
	}
}
