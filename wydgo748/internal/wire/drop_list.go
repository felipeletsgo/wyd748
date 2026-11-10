package wire

import (
	"bytes"
	"encoding/binary"
)

const (
	OpDropListQuery     = 0x7F41
	OpDropListResult    = 0x7F42
	DropListQuerySize   = 904
	DropListResultSize  = 716
	DropListPageSize    = 10
	DropListChanceScale = 1_000_000_000
)

type DropListQuery struct {
	Mode       byte
	ID, Offset uint32
	Query      string
	Items      [816]byte
}

func ParseDropListQuery(pkt []byte) (DropListQuery, bool) {
	var q DropListQuery
	if len(pkt) != DropListQuerySize || binary.LittleEndian.Uint16(pkt[0:2]) != DropListQuerySize || binary.LittleEndian.Uint16(pkt[4:6]) != OpDropListQuery || binary.LittleEndian.Uint16(pkt[12:14]) != 1 || pkt[14] > 1 || pkt[15] != 0 {
		return q, false
	}
	q.Mode, q.ID, q.Offset = pkt[14], binary.LittleEndian.Uint32(pkt[16:20]), binary.LittleEndian.Uint32(pkt[20:24])
	end := bytes.IndexByte(pkt[24:88], 0)
	if q.ID == 0 || q.Offset%DropListPageSize != 0 || end < 0 {
		return DropListQuery{}, false
	}
	for _, v := range pkt[24+end : 88] {
		if v != 0 {
			return DropListQuery{}, false
		}
	}
	q.Query = string(pkt[24 : 24+end])
	copy(q.Items[:], pkt[88:])
	for id := 0; id < 6528; id++ {
		if q.Items[id/8]&(1<<uint(id%8)) != 0 && (q.Mode == 0 || id == 0 || id >= 6500) {
			return DropListQuery{}, false
		}
	}
	return q, true
}

type DropListRow struct {
	Item, Flags   uint16
	Base, Current uint32
	Monster       string
}

func DropListResult(q DropListQuery, total uint32, bonus int32, status byte, rows []DropListRow) []byte {
	if len(rows) > DropListPageSize {
		rows = rows[:DropListPageSize]
	}
	b := Build(OpDropListResult, SceneField, DropListResultSize)
	binary.LittleEndian.PutUint16(b[12:14], 1)
	b[14], b[15] = q.Mode, status
	binary.LittleEndian.PutUint32(b[16:20], q.ID)
	binary.LittleEndian.PutUint32(b[20:24], q.Offset)
	binary.LittleEndian.PutUint32(b[24:28], total)
	binary.LittleEndian.PutUint32(b[28:32], uint32(bonus))
	binary.LittleEndian.PutUint16(b[32:34], uint16(len(rows)))
	for i, row := range rows {
		o := 36 + 68*i
		binary.LittleEndian.PutUint16(b[o:o+2], row.Item)
		binary.LittleEndian.PutUint16(b[o+2:o+4], row.Flags)
		binary.LittleEndian.PutUint32(b[o+8:o+12], row.Base)
		binary.LittleEndian.PutUint32(b[o+12:o+16], row.Current)
		name := []byte(row.Monster)
		if len(name) > 47 {
			name = name[:47]
		}
		copy(b[o+20:o+68], name)
	}
	return b
}
