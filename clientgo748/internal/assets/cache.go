package assets

import (
	"crypto/ed25519"
	"errors"
	"fmt"
	"sync"
)

// TextureSource é a única dependência de assets que a primeira cena precisa
// conhecer. O renderer recebe Texture e não precisa conhecer WYT, o formato
// canônico ou o pacote protegido.
type TextureSource interface {
	LoadTexture(path string) (Texture, error)
	Close() error
}

var (
	ErrAssetCacheClosed = errors.New("clientgo748: asset cache is closed")
	ErrAssetNotFound    = errors.New("clientgo748: asset was not found")
	ErrAssetKind        = errors.New("clientgo748: asset kind is not supported")
)

type cachedAsset struct {
	kind     AssetKind
	revision uint32
	data     []byte
}

// ProtectedAssetCache mantém somente dados já autenticados pelo pacote. O
// cache é imutável durante a cena: LoadTexture devolve uma textura própria e
// Close zera e libera todos os buffers que pertencem ao cache.
type ProtectedAssetCache struct {
	mu           sync.RWMutex
	assetVersion string
	keyID        string
	entries      map[string]cachedAsset
	closed       bool
}

// NewProtectedAssetCache valida o pacote inteiro antes de publicar qualquer
// entrada. As chaves são copiadas apenas para a chamada de abertura; nenhuma
// chave é retida pelo cache depois da autenticação.
func NewProtectedAssetCache(packageData []byte, contentKey [32]byte, publicKey ed25519.PublicKey) (*ProtectedAssetCache, error) {
	protected, err := OpenProtectedPackage(packageData, contentKey, publicKey)
	if err != nil {
		return nil, fmt.Errorf("open protected assets: %w", err)
	}
	entries := protected.Assets()
	cache := &ProtectedAssetCache{
		assetVersion: protected.AssetVersion,
		keyID:        protected.KeyID,
		entries:      make(map[string]cachedAsset, len(entries)),
	}
	for _, entry := range entries {
		if _, exists := cache.entries[entry.Path]; exists {
			return nil, fmt.Errorf("%w: duplicate path %q", ErrInvalidProtectedPackage, entry.Path)
		}
		cache.entries[entry.Path] = cachedAsset{
			kind:     entry.Kind,
			revision: entry.Revision,
			data:     append([]byte(nil), entry.Data...),
		}
	}
	return cache, nil
}

// AssetVersion e KeyID identificam a versão autenticada que está em memória.
// Eles são metadados de observabilidade, não uma autorização de gameplay.
func (c *ProtectedAssetCache) AssetVersion() string {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.assetVersion
}

func (c *ProtectedAssetCache) KeyID() string {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.keyID
}

// LoadTexture decodifica somente um asset KindTexture e devolve um buffer
// independente. Um asset inválido falha antes de chegar ao renderer.
func (c *ProtectedAssetCache) LoadTexture(path string) (Texture, error) {
	if c == nil {
		return Texture{}, ErrAssetCacheClosed
	}
	c.mu.RLock()
	if c.closed {
		c.mu.RUnlock()
		return Texture{}, ErrAssetCacheClosed
	}
	entry, ok := c.entries[path]
	if !ok {
		c.mu.RUnlock()
		return Texture{}, fmt.Errorf("%w: %s", ErrAssetNotFound, path)
	}
	if entry.kind != KindTexture {
		c.mu.RUnlock()
		return Texture{}, fmt.Errorf("%w: %s", ErrAssetKind, path)
	}
	data := append([]byte(nil), entry.data...)
	c.mu.RUnlock()
	texture, err := DecodeTexture(data)
	if err != nil {
		return Texture{}, fmt.Errorf("decode protected texture %q: %w", path, err)
	}
	return texture, nil
}

// Close encerra o ownership do cache. É idempotente e não retém pixels ou
// identificadores depois da primeira chamada.
func (c *ProtectedAssetCache) Close() error {
	if c == nil {
		return nil
	}
	c.mu.Lock()
	defer c.mu.Unlock()
	if c.closed {
		return nil
	}
	for path, entry := range c.entries {
		clear(entry.data)
		delete(c.entries, path)
	}
	c.assetVersion = ""
	c.keyID = ""
	c.closed = true
	return nil
}
