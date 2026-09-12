package game

import (
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestInboundCommandMetricLabelsCoverAllowlist(t *testing.T) {
	for raw := 0; raw <= int(^uint16(0)); raw++ {
		opcode := uint16(raw)
		label, labeled := inboundCommandMetricLabel(opcode)
		known := knownInboundOpcode(opcode)
		if known != labeled {
			t.Fatalf("opcode 0x%X known=%v labeled=%v label=%q", opcode, known, labeled, label)
		}
		if known {
			want := fmt.Sprintf("0x%X", opcode)
			if label != want {
				t.Fatalf("opcode 0x%X label=%q; esperado=%q", opcode, label, want)
			}
		}
	}
}

func TestCommandLabelKnownOpcodeDoesNotAllocate(t *testing.T) {
	pkt := make([]byte, wire.HeaderSize)
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpAttackOne)
	cmd := command{pkt: pkt}
	if got := commandLabel(cmd); got != "0x39D" {
		t.Fatalf("label=%q; esperado=0x39D", got)
	}
	allocs := testing.AllocsPerRun(1000, func() {
		if commandLabel(cmd) != "0x39D" {
			panic("label mudou")
		}
	})
	if allocs != 0 {
		t.Fatalf("commandLabel alocou %.2f objeto(s) por comando", allocs)
	}
}

func TestDurationBucketLabelBoundaries(t *testing.T) {
	cases := []struct {
		duration time.Duration
		want     string
	}{
		{0, "<=1ms"},
		{time.Millisecond, "<=1ms"},
		{time.Millisecond + 1, "<=5ms"},
		{5 * time.Millisecond, "<=5ms"},
		{10 * time.Millisecond, "<=10ms"},
		{25 * time.Millisecond, "<=25ms"},
		{50 * time.Millisecond, "<=50ms"},
		{100 * time.Millisecond, "<=100ms"},
		{250 * time.Millisecond, "<=250ms"},
		{time.Second, "<=1s"},
		{time.Second + 1, "+Inf"},
	}
	for _, tc := range cases {
		if got := durationBucketLabel(tc.duration); got != tc.want {
			t.Fatalf("duration=%s label=%q; esperado=%q", tc.duration, got, tc.want)
		}
	}
}

func TestDurationBucketLabelDoesNotAllocate(t *testing.T) {
	allocs := testing.AllocsPerRun(1000, func() {
		if durationBucketLabel(17*time.Millisecond) != "<=25ms" {
			panic("bucket mudou")
		}
	})
	if allocs != 0 {
		t.Fatalf("durationBucketLabel alocou %.2f objeto(s) por observacao", allocs)
	}
}

func BenchmarkCommandMetricLabels(b *testing.B) {
	pkt := make([]byte, wire.HeaderSize)
	binary.LittleEndian.PutUint16(pkt[4:6], wire.OpAttackOne)
	cmd := command{pkt: pkt}
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = commandLabel(cmd)
		_ = durationBucketLabel(17 * time.Millisecond)
	}
}
