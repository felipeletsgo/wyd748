package assets

import (
	"bytes"
	"crypto/ed25519"
	"errors"
	"testing"
)

func TestProtectedPackageRoundTrip(t *testing.T) {
	privateKey, publicKey := testPackageKeyPair()
	contentKey := testContentKey(0x31)
	inputs := []PackageInput{
		{ID: "ui-logo", Kind: KindTexture, Path: "UI/logo1.wydasset", Revision: 7, Data: []byte{1, 2, 3, 4}},
		{ID: "font", Kind: KindData, Path: "UI/font.dat", Revision: 2, Data: []byte("font-payload")},
	}
	packageData, err := BuildProtectedPackage(inputs, contentKey, privateKey, PackageOptions{
		AssetVersion: "2026.09.09",
		KeyID:        "assets-test-1",
		Random:       bytes.NewReader(append(bytes.Repeat([]byte{0x41}, protectedNonceSize), bytes.Repeat([]byte{0x42}, protectedNonceSize)...)),
	})
	if err != nil {
		t.Fatalf("BuildProtectedPackage() error = %v", err)
	}
	opened, err := OpenProtectedPackage(packageData, contentKey, publicKey)
	if err != nil {
		t.Fatalf("OpenProtectedPackage() error = %v", err)
	}
	if opened.AssetVersion != "2026.09.09" || opened.KeyID != "assets-test-1" {
		t.Fatalf("metadata = %q/%q", opened.AssetVersion, opened.KeyID)
	}
	got := opened.Assets()
	if len(got) != 2 || got[0].Path != "UI/font.dat" || got[1].Path != "UI/logo1.wydasset" {
		t.Fatalf("assets were not deterministically ordered: %#v", got)
	}
	if !bytes.Equal(got[0].Data, []byte("font-payload")) || !bytes.Equal(got[1].Data, []byte{1, 2, 3, 4}) {
		t.Fatalf("round-trip payload mismatch: %#v", got)
	}
}

func TestProtectedPackageRejectsInvalidSignature(t *testing.T) {
	packageData, contentKey, publicKey := buildTestProtectedPackage(t)
	packageData[protectedPackageHeaderSize] ^= 0x01
	if _, err := OpenProtectedPackage(packageData, contentKey, publicKey); !errors.Is(err, ErrProtectedIntegrity) && !errors.Is(err, ErrInvalidProtectedPackage) {
		t.Fatalf("OpenProtectedPackage() error = %v, want signature failure", err)
	}
}

func TestProtectedPackageRejectsWrongContentKey(t *testing.T) {
	packageData, _, publicKey := buildTestProtectedPackage(t)
	wrongKey := testContentKey(0x77)
	if _, err := OpenProtectedPackage(packageData, wrongKey, publicKey); !errors.Is(err, ErrProtectedIntegrity) {
		t.Fatalf("OpenProtectedPackage() error = %v, want integrity failure", err)
	}
}

func TestProtectedPackageRejectsManifestAdulteration(t *testing.T) {
	packageData, contentKey, publicKey := buildTestProtectedPackage(t)
	bodyStart := protectedPackageHeaderSize + protectedManifestHeaderSize
	packageData[bodyStart+1] ^= 0x20
	if _, err := OpenProtectedPackage(packageData, contentKey, publicKey); err == nil {
		t.Fatal("OpenProtectedPackage() accepted adulterated manifest")
	}
}

func TestProtectedPackageRejectsUnsafePath(t *testing.T) {
	privateKey, _ := testPackageKeyPair()
	_, err := BuildProtectedPackage([]PackageInput{{
		ID: "bad", Kind: KindData, Path: "../outside", Data: []byte{1},
	}}, testContentKey(0x22), privateKey, PackageOptions{
		AssetVersion: "test", KeyID: "test", Random: bytes.NewReader(bytes.Repeat([]byte{1}, protectedNonceSize)),
	})
	if !errors.Is(err, ErrInvalidProtectedPackage) {
		t.Fatalf("BuildProtectedPackage() error = %v, want unsafe path rejection", err)
	}
}

func TestProtectedPackageRejectsRepeatedNonce(t *testing.T) {
	privateKey, _ := testPackageKeyPair()
	_, err := BuildProtectedPackage([]PackageInput{
		{ID: "one", Kind: KindData, Path: "one", Data: []byte{1}},
		{ID: "two", Kind: KindData, Path: "two", Data: []byte{2}},
	}, testContentKey(0x23), privateKey, PackageOptions{
		AssetVersion: "test", KeyID: "test", Random: bytes.NewReader(bytes.Repeat([]byte{2}, protectedNonceSize*2)),
	})
	if !errors.Is(err, ErrInvalidProtectedPackage) {
		t.Fatalf("BuildProtectedPackage() error = %v, want repeated nonce rejection", err)
	}
}

func TestProtectedPackageRejectsTruncatedChunk(t *testing.T) {
	packageData, contentKey, publicKey := buildTestProtectedPackage(t)
	packageData = packageData[:len(packageData)-1]
	if _, err := OpenProtectedPackage(packageData, contentKey, publicKey); !errors.Is(err, ErrInvalidProtectedPackage) {
		t.Fatalf("OpenProtectedPackage() error = %v, want truncated package rejection", err)
	}
}

func TestProtectedPackageAssetsAreIndependentCopies(t *testing.T) {
	packageData, contentKey, publicKey := buildTestProtectedPackage(t)
	opened, err := OpenProtectedPackage(packageData, contentKey, publicKey)
	if err != nil {
		t.Fatal(err)
	}
	first := opened.Assets()
	first[0].Data[0] ^= 0xff
	second := opened.Assets()
	if first[0].Data[0] == second[0].Data[0] {
		t.Fatal("Assets() returned aliased data")
	}
}

func buildTestProtectedPackage(t *testing.T) ([]byte, [32]byte, ed25519.PublicKey) {
	t.Helper()
	privateKey, publicKey := testPackageKeyPair()
	contentKey := testContentKey(0x55)
	data, err := BuildProtectedPackage([]PackageInput{{
		ID: "test", Kind: KindData, Path: "UI/test.dat", Revision: 1, Data: []byte("payload"),
	}}, contentKey, privateKey, PackageOptions{
		AssetVersion: "test", KeyID: "test-key", Random: bytes.NewReader(bytes.Repeat([]byte{9}, protectedNonceSize)),
	})
	if err != nil {
		t.Fatal(err)
	}
	return data, contentKey, publicKey
}

func testPackageKeyPair() (ed25519.PrivateKey, ed25519.PublicKey) {
	seed := bytes.Repeat([]byte{0x19}, ed25519.SeedSize)
	privateKey := ed25519.NewKeyFromSeed(seed)
	return privateKey, privateKey.Public().(ed25519.PublicKey)
}

func testContentKey(value byte) (key [32]byte) {
	for index := range key {
		key[index] = value + byte(index)
	}
	return key
}
