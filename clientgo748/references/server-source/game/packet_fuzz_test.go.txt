package game

import (
	"testing"

	"wydgo/internal/model"
)

// Estes fuzz targets rodam os seeds em todo `go test` e podem ser ampliados
// manualmente com `go test -fuzz`. O objetivo e garantir que bytes hostis nunca
// escapem dos parsers de borda para um panic ou indice fora do array.
func FuzzParseTradeRequestNeverPanics(f *testing.F) {
	f.Add(make([]byte, 0))
	f.Add(make([]byte, 156))
	f.Add([]byte{0xFF, 0x00, 0x7F})
	character := &model.Char{Gold: 1000}
	f.Fuzz(func(t *testing.T, packet []byte) {
		_, _ = parseTradeRequest(packet, character)
	})
}

func FuzzItemPacketParsersNeverPanic(f *testing.F) {
	f.Add(make([]byte, 0))
	f.Add(make([]byte, 28))
	f.Add(make([]byte, 36))
	f.Fuzz(func(t *testing.T, packet []byte) {
		_, _ = parseDropItemRequest(packet)
		_, _ = parseGetItemRequest(packet)
		_ = parseAttackSkill(packet)
	})
}
