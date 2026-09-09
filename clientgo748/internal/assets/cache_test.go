package assets

import (
	"bytes"
	"crypto/ed25519"
	"errors"
	"testing"
)

func TestProtectedAssetCacheLoadsCanonicalTexture(t *testing.T) {
	privateKey, publicKey := testPackageKeyPair()
	contentKey := testContentKey(0x61)
	canonical, err := EncodeTexture(Texture{Width: 1, Height: 1, SourceBits: 32, Pixels: []byte{9, 8, 7, 6}})
	if err != nil {
		t.Fatal(err)
	}
	packageData, err := BuildProtectedPackage([]PackageInput{{
		ID: "ui-logo", Kind: KindTexture, Path: "UI/logo1.wydasset", Revision: 4, Data: canonical,
	}}, contentKey, privateKey, PackageOptions{
		AssetVersion: "test-assets",
		KeyID:        "assets-key-1",
		Random:       bytes.NewReader(bytes.Repeat([]byte{0x73}, protectedNonceSize)),
	})
	if err != nil {
		t.Fatal(err)
	}
	cache, err := NewProtectedAssetCache(packageData, contentKey, publicKey)
	if err != nil {
		t.Fatal(err)
	}
	texture, err := cache.LoadTexture("UI/logo1.wydasset")
	if err != nil {
		t.Fatal(err)
	}
	if texture.Width != 1 || texture.Height != 1 || !bytes.Equal(texture.Pixels, []byte{9, 8, 7, 6}) {
		t.Fatalf("texture = %#v", texture)
	}
	texture.Pixels[0] = 0
	again, err := cache.LoadTexture("UI/logo1.wydasset")
	if err != nil {
		t.Fatal(err)
	}
	if again.Pixels[0] != 9 {
		t.Fatal("LoadTexture returned aliased pixel data")
	}
	if cache.AssetVersion() != "test-assets" || cache.KeyID() != "assets-key-1" {
		t.Fatalf("metadata = %q/%q", cache.AssetVersion(), cache.KeyID())
	}
	if err := cache.Close(); err != nil {
		t.Fatal(err)
	}
	if _, err := cache.LoadTexture("UI/logo1.wydasset"); !errors.Is(err, ErrAssetCacheClosed) {
		t.Fatalf("LoadTexture after Close() = %v, want closed error", err)
	}
	if err := cache.Close(); err != nil {
		t.Fatal(err)
	}
}

func TestProtectedAssetCacheRejectsInvalidPackageBeforeConstruction(t *testing.T) {
	_, err := NewProtectedAssetCache([]byte("not a package"), testContentKey(0x72), testPackagePublicKey())
	if !errors.Is(err, ErrInvalidProtectedPackage) {
		t.Fatalf("NewProtectedAssetCache() error = %v, want invalid package", err)
	}
}

func TestProtectedAssetCacheRejectsMissingAndWrongKind(t *testing.T) {
	privateKey, publicKey := testPackageKeyPair()
	contentKey := testContentKey(0x73)
	packageData, err := BuildProtectedPackage([]PackageInput{
		{ID: "data", Kind: KindData, Path: "UI/data.bin", Revision: 1, Data: []byte("data")},
	}, contentKey, privateKey, PackageOptions{
		AssetVersion: "test", KeyID: "key", Random: bytes.NewReader(bytes.Repeat([]byte{0x74}, protectedNonceSize)),
	})
	if err != nil {
		t.Fatal(err)
	}
	cache, err := NewProtectedAssetCache(packageData, contentKey, publicKey)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := cache.LoadTexture("UI/missing.wydasset"); !errors.Is(err, ErrAssetNotFound) {
		t.Fatalf("missing texture error = %v", err)
	}
	if _, err := cache.LoadTexture("UI/data.bin"); !errors.Is(err, ErrAssetKind) {
		t.Fatalf("wrong kind error = %v", err)
	}
}

func testPackagePublicKey() ed25519.PublicKey {
	_, publicKey := testPackageKeyPair()
	return publicKey
}
