package wire

import (
	"encoding/binary"
	"errors"
	"fmt"
)

const ClientIntegrityPacketSize = 24

var ErrInvalidClientIntegrityResponse = errors.New("wire: invalid client integrity response")

type ClientIntegrityResponse struct {
	ID         uint16
	Category   int32
	ByteOffset int32
	Value      int32
}

// ClientIntegrityChallenge builds the native 24-byte layout. Value is zero in
// the challenge; the client returns the byte it read, sign-extended, in this field.
func ClientIntegrityChallenge(id uint16, category, byteOffset int32) []byte {
	b := Build(OpClientIntegrityChallenge, id, ClientIntegrityPacketSize)
	binary.LittleEndian.PutUint32(b[12:16], uint32(category))
	binary.LittleEndian.PutUint32(b[16:20], uint32(byteOffset))
	return b
}

func ParseClientIntegrityResponse(pkt []byte) (ClientIntegrityResponse, error) {
	if len(pkt) != ClientIntegrityPacketSize {
		return ClientIntegrityResponse{}, fmt.Errorf("%w: size %d", ErrInvalidClientIntegrityResponse, len(pkt))
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
