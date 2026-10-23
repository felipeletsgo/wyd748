package wire

import (
	"encoding/binary"
	"os"
	"regexp"
	"strconv"
	"testing"
)

func TestClientBuildAdmissionContract(t *testing.T) {
	pkt := Build(OpConnectAccount, 0, AccountLoginSize)
	binary.LittleEndian.PutUint32(pkt[40:44], ClientProtocolVersion)
	for revision := uint32(0); revision <= 0xFFFF; revision++ {
		binary.LittleEndian.PutUint32(pkt[44:48], ClientBuildPrefix|revision)
		if got, want := CompatibleClientLogin(pkt), revision >= MinimumClientBuildRevision; got != want {
			t.Fatalf("revision=%d accepted=%v want=%v", revision, got, want)
		}
	}
	for _, build := range []uint32{0, 1, 748, 0xFFFFFFFF, 0x57460001, 0x57480001} {
		binary.LittleEndian.PutUint32(pkt[44:48], build)
		if CompatibleClientLogin(pkt) {
			t.Fatalf("legacy or unknown identity 0x%X accepted", build)
		}
	}
	binary.LittleEndian.PutUint32(pkt[44:48], CurrentClientBuild)
	for _, protocol := range []uint32{0, 747, 749, 769, 0xFFFFFFFF} {
		binary.LittleEndian.PutUint32(pkt[40:44], protocol)
		if CompatibleClientLogin(pkt) {
			t.Fatalf("incompatible protocol=%d accepted", protocol)
		}
	}
	binary.LittleEndian.PutUint32(pkt[40:44], ClientProtocolVersion)
	for length := 0; length < AccountLoginSize; length++ {
		if CompatibleClientLogin(pkt[:length]) {
			t.Fatalf("truncated packet length=%d accepted", length)
		}
	}
	if CompatibleClientLogin(append(pkt, 0)) {
		t.Fatal("extra login tail accepted")
	}
}

func TestClientBuildConstantsMatchCompiledClient(t *testing.T) {
	header, err := os.ReadFile("../../../tmproject/TMProject748/internal/wire/ClientBuildContract.h")
	if err != nil {
		t.Fatal(err)
	}
	for name, want := range map[string]uint32{
		"Protocol": ClientProtocolVersion, "Prefix": ClientBuildPrefix, "Revision": MinimumClientBuildRevision,
	} {
		match := regexp.MustCompile(`constexpr unsigned int ` + name + ` = (0x[0-9A-Fa-f]+|[0-9]+);`).FindSubmatch(header)
		if len(match) != 2 {
			t.Fatalf("missing compiled client constant %s", name)
		}
		got, err := strconv.ParseUint(string(match[1]), 0, 32)
		if err != nil || uint32(got) != want {
			t.Fatalf("%s=%s server=%d error=%v", name, match[1], want, err)
		}
	}
}
