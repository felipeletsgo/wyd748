package wire

import (
	"encoding/binary"
	"errors"
	"fmt"
)

const ClientIntegrityPacketSize = 24

var ErrInvalidClientIntegrityResponse = errors.New("wire: resposta de integridade invalida")

type ClientIntegrityResponse struct {
	ID         uint16
	Category   int32
	ByteOffset int32
	Value      int32
}

// ClientIntegrityChallenge monta o layout nativo de 24 bytes. Value permanece
// zero no desafio e o client devolve nesse campo o byte lido, promovido com sinal.
func ClientIntegrityChallenge(id uint16, category, byteOffset int32) []byte {
	b := Build(OpClientIntegrityChallenge, id, ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint32(b[12:16], uint32(category))
	binary.LittleEndian.PutUint32(b[16:20], uint32(byteOffset))
	return b
}

func ParseClientIntegrityResponse(pkt []byte) (ClientIntegrityResponse, error) {
	if len(pkt) != ClientIntegrityPacketSize {
		return ClientIntegrityResponse{}, fmt.Errorf("%w: tamanho %d", ErrInvalidClientIntegrityResponse, len(pkt))
	}
	header := ParseHeader(pkt)
	if header.Size != ClientIntegrityPacketSize {
		return ClientIntegrityResponse{}, fmt.Errorf("%w: Header.Size=%d", ErrInvalidClientIntegrityResponse, header.Size)
	}
	if header.Type != OpClientIntegrityResponse {
		return ClientIntegrityResponse{}, fmt.Errorf("%w: opcode=0x%X", ErrInvalidClientIntegrityResponse, header.Type)
	}
	return ClientIntegrityResponse{
		ID:         header.ID,
		Category:   int32(binary.LittleEndian.Uint32(pkt[12:16])),
		ByteOffset: int32(binary.LittleEndian.Uint32(pkt[16:20])),
		Value:      int32(binary.LittleEndian.Uint32(pkt[20:24])),
	}, nil
}
