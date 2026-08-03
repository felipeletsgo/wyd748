package loadtest

import (
	"context"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	mathrand "math/rand"
	stdnet "net"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// RunConfig controla a carga de sessoes. O cliente de observacao (felipe) nao
// passa por este pacote e pode permanecer conectado normalmente.
type RunConfig struct {
	Address       string
	BotCount      int
	AccountPrefix string
	Password      string
	Ramp          time.Duration
	Duration      time.Duration
	Skill         int
	MovePercent   int
	SpawnX        uint16
	SpawnY        uint16
	// Terrain deve ser o mesmo HeightMap/AttributeMap usado pelo servidor.
	// Quando carregado, o bot nunca envia deliberadamente um destino bloqueado.
	Terrain model.TerrainMap
}

type RunReport struct {
	Started       int64
	Connected     int64
	Entered       int64
	Errors        int64
	Attacks       int64
	Moves         int64
	Packets       int64
	BytesSent     int64
	BytesReceived int64
	Elapsed       time.Duration
}

type loadCounters struct {
	started   atomic.Int64
	connected atomic.Int64
	entered   atomic.Int64
	errors    atomic.Int64
	attacks   atomic.Int64
	moves     atomic.Int64
	packets   atomic.Int64
	bytesSent atomic.Int64
	bytesRecv atomic.Int64
}

type bot struct {
	config      RunConfig
	index       int
	conn        stdnet.Conn
	mu          sync.Mutex
	key         byte
	tick        atomic.Uint32
	id          atomic.Uint32
	x, y        atomic.Uint32
	targetMu    sync.RWMutex
	targets     map[uint16]targetPosition
	entered     chan struct{}
	enteredOnce sync.Once
	counters    *loadCounters
	rng         *mathrand.Rand
}

type targetPosition struct {
	x uint16
	y uint16
}

// RunBots inicia as sessoes em rampa e aguarda o contexto. Cada bot usa uma
// conexao TCP propria, leitura continua e um escritor serializado.
func RunBots(ctx context.Context, cfg RunConfig, logger *log.Logger) RunReport {
	started := time.Now()
	if cfg.BotCount <= 0 {
		cfg.BotCount = 990
	}
	if cfg.BotCount > 990 {
		if logger != nil {
			logger.Printf("loadtest: limitando bots de %d para 990", cfg.BotCount)
		}
		cfg.BotCount = 990
	}
	if cfg.AccountPrefix == "" {
		cfg.AccountPrefix = "bot"
	}
	if cfg.Ramp <= 0 {
		cfg.Ramp = time.Minute
	}
	if cfg.Duration <= 0 {
		cfg.Duration = 5 * time.Minute
	}
	cfg.MovePercent = clampMovePercent(cfg.MovePercent)
	if cfg.SpawnX == 0 {
		cfg.SpawnX = 1162
	}
	if cfg.SpawnY == 0 {
		cfg.SpawnY = 1700
	}
	ctx, cancel := context.WithTimeout(ctx, cfg.Duration)
	defer cancel()
	counters := &loadCounters{}
	var wg sync.WaitGroup
	for i := 1; i <= cfg.BotCount; i++ {
		i := i
		wg.Add(1)
		go func() {
			defer wg.Done()
			delay := time.Duration(int64(cfg.Ramp) * int64(i-1) / int64(cfg.BotCount))
			timer := time.NewTimer(delay)
			select {
			case <-ctx.Done():
				timer.Stop()
				return
			case <-timer.C:
			}
			counters.started.Add(1)
			b := &bot{
				config: cfg, index: i, counters: counters,
				entered: make(chan struct{}), targets: make(map[uint16]targetPosition),
				rng: mathrand.New(mathrand.NewSource(int64(i) * 7919)),
			}
			if err := b.run(ctx); err != nil {
				counters.errors.Add(1)
				if logger != nil {
					logger.Printf("bot%04d: %v", i, err)
				}
			}
		}()
	}
	<-ctx.Done()
	wg.Wait()
	return RunReport{
		Started: counters.started.Load(), Connected: counters.connected.Load(),
		Entered: counters.entered.Load(), Errors: counters.errors.Load(),
		Attacks: counters.attacks.Load(), Moves: counters.moves.Load(),
		Packets: counters.packets.Load(), BytesSent: counters.bytesSent.Load(),
		BytesReceived: counters.bytesRecv.Load(), Elapsed: time.Since(started),
	}
}

func clampMovePercent(value int) int {
	if value < 0 {
		return 0
	}
	if value > 100 {
		return 100
	}
	return value
}

func (b *bot) run(ctx context.Context) error {
	conn, err := stdnet.DialTimeout("tcp", b.config.Address, 10*time.Second)
	if err != nil {
		return fmt.Errorf("conectar %s: %w", b.config.Address, err)
	}
	b.conn = conn
	defer conn.Close()
	b.counters.connected.Add(1)
	init := initCodeBytes()
	if _, err := conn.Write(init); err != nil {
		return fmt.Errorf("enviar InitCode: %w", err)
	}
	b.counters.bytesSent.Add(int64(len(init)))
	readErr := make(chan error, 1)
	go func() { readErr <- b.readLoop(ctx) }()
	if err := b.send(loginPacket(b.accountName(), b.config.Password)); err != nil {
		return err
	}
	select {
	case <-b.entered:
	case <-ctx.Done():
		return nil
	case err := <-readErr:
		return err
	case <-time.After(30 * time.Second):
		return fmt.Errorf("timeout para entrar no mundo")
	}
	b.counters.entered.Add(1)
	activityCtx, cancel := context.WithCancel(ctx)
	defer cancel()
	go b.activityLoop(activityCtx)
	select {
	case <-ctx.Done():
		return nil
	case err := <-readErr:
		return err
	}
}

func (b *bot) readLoop(ctx context.Context) error {
	for {
		if deadline, ok := ctx.Deadline(); ok {
			_ = b.conn.SetReadDeadline(deadline)
		}
		pkt, ok, err := wire.ReadPacket(b.conn)
		if err != nil {
			if ctx.Err() != nil || err == io.EOF {
				return nil
			}
			return err
		}
		if !ok || len(pkt) < wire.HeaderSize {
			return fmt.Errorf("pacote cifrado/checksum invalido")
		}
		b.counters.packets.Add(1)
		b.counters.bytesRecv.Add(int64(len(pkt)))
		h := wire.ParseHeader(pkt)
		switch h.Type {
		case wire.OpCharList:
			if err := b.send(characterLoginPacket(0)); err != nil {
				return err
			}
		case wire.OpEnterWorld:
			b.id.Store(uint32(h.ID))
			// EnterWorld carrega a posicao autoritativa em PosX/PosY@12.
			// O servidor pode ter empurrado o bot para uma celula livre; usar
			// sempre a coordenada configurada aqui faria o proximo movimento
			// parecer um teleport e seria recusado pela validacao de rota.
			if len(pkt) >= 16 {
				b.x.Store(uint32(binary.LittleEndian.Uint16(pkt[12:14])))
				b.y.Store(uint32(binary.LittleEndian.Uint16(pkt[14:16])))
			} else {
				b.x.Store(uint32(b.config.SpawnX))
				b.y.Store(uint32(b.config.SpawnY))
			}
			b.enteredOnce.Do(func() { close(b.entered) })
		case wire.OpCreateMob:
			if h.ID >= 1000 {
				b.targetMu.Lock()
				position := targetPosition{}
				if len(pkt) >= 16 {
					position.x = binary.LittleEndian.Uint16(pkt[12:14])
					position.y = binary.LittleEndian.Uint16(pkt[14:16])
				}
				b.targets[h.ID] = position
				b.targetMu.Unlock()
			}
		case wire.OpRemoveMob:
			b.targetMu.Lock()
			delete(b.targets, h.ID)
			b.targetMu.Unlock()
		}
	}
}

func (b *bot) activityLoop(ctx context.Context) {
	ticker := time.NewTicker(900 * time.Millisecond)
	defer ticker.Stop()
	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			if b.config.MovePercent > 0 && b.rng.Intn(100) < b.config.MovePercent {
				if b.sendMove() == nil {
					b.counters.moves.Add(1)
				}
			}
			if b.sendAttack() == nil {
				b.counters.attacks.Add(1)
			}
		}
	}
}

func (b *bot) sendMove() error {
	x, y := uint16(b.x.Load()), uint16(b.y.Load())
	_, target := b.nearestTarget()
	if target.x == 0 || target.y == 0 {
		// Sem uma entidade visível não inventa uma rota aleatória: isso só gera
		// recusas de terreno e não representa uma decisão de um jogador.
		return nil
	}
	dx := signLoadtest(int(target.x) - int(x))
	dy := signLoadtest(int(target.y) - int(y))
	if dx == 0 && dy == 0 {
		return nil
	}
	// Tenta primeiro a direção do alvo e depois as oito vizinhas. A ordem
	// rotativa evita que centenas de bots escolham sempre o mesmo canto quando
	// há um pequeno obstáculo. Com o mapa carregado, cada candidato segue as
	// mesmas travas de terreno/altura da validação server-side.
	candidates := orderedMoveCandidates(dx, dy)
	for _, candidate := range candidates {
		nx, ny := int(x)+candidate[0], int(y)+candidate[1]
		if nx <= 0 || ny <= 0 || nx >= 4096 || ny >= 4096 {
			continue
		}
		ux, uy := uint16(nx), uint16(ny)
		if b.config.Terrain.Loaded() &&
			(!b.config.Terrain.Walkable(ux, uy) ||
				!b.config.Terrain.RouteHeightCompatible(x, y, ux, uy) ||
				!b.config.Terrain.LineOfSight(x, y, ux, uy)) {
			continue
		}
		if err := b.send(wire.PlayerMove(uint16(b.id.Load()), x, y, ux, uy, 1, nil)); err != nil {
			return err
		}
		// Só avança a posição local depois de transmitir o pacote. Se a rota for
		// recusada pelo servidor, o próximo pacote continua partindo da posição
		// autoritativa conhecida e não cria uma cascata de teletransportes.
		b.x.Store(uint32(nx))
		b.y.Store(uint32(ny))
		return nil
	}
	return nil
}

func orderedMoveCandidates(dx, dy int) [][2]int {
	// Direção principal, diagonais que preservam um componente e as demais
	// alternativas. Duplicatas são removidas para manter no máximo oito testes.
	all := [][2]int{{dx, dy}, {dx, 0}, {0, dy}, {dy, -dx}, {-dy, dx},
		{-dx, -dy}, {-dx, 0}, {0, -dy}}
	seen := make(map[[2]int]struct{}, len(all))
	result := make([][2]int, 0, len(all))
	for _, candidate := range all {
		if candidate[0] == 0 && candidate[1] == 0 {
			continue
		}
		if _, ok := seen[candidate]; ok {
			continue
		}
		seen[candidate] = struct{}{}
		result = append(result, candidate)
	}
	return result
}

func (b *bot) sendAttack() error {
	target, targetPos := b.nearestTarget()
	if target == 0 {
		return nil
	}
	id := uint16(b.id.Load())
	x, y := uint16(b.x.Load()), uint16(b.y.Load())
	var pkt []byte
	if b.config.Skill >= 0 {
		pkt = wire.SkillHit(id, target, x, y, targetPos.x, targetPos.y, 0, 0, 0, 0,
			int16(b.config.Skill), 5, 1)
	} else {
		pkt = wire.AttackHit(id, target, x, y, targetPos.x, targetPos.y, 0, 0, 0, 0)
	}
	return b.send(pkt)
}

func (b *bot) nearestTarget() (uint16, targetPosition) {
	b.targetMu.RLock()
	var target uint16
	var targetPos targetPosition
	x, y := uint16(b.x.Load()), uint16(b.y.Load())
	bestDistance := uint16(^uint16(0))
	for id, position := range b.targets {
		distance := chebyshevDistance(x, y, position.x, position.y)
		if target == 0 || distance < bestDistance {
			target, targetPos, bestDistance = id, position, distance
		}
	}
	b.targetMu.RUnlock()
	return target, targetPos
}

func chebyshevDistance(ax, ay, bx, by uint16) uint16 {
	dx, dy := absLoadtest(int(ax)-int(bx)), absLoadtest(int(ay)-int(by))
	if dx > dy {
		return uint16(dx)
	}
	return uint16(dy)
}

func absLoadtest(value int) int {
	if value < 0 {
		return -value
	}
	return value
}

func signLoadtest(value int) int {
	if value < 0 {
		return -1
	}
	if value > 0 {
		return 1
	}
	return 0
}

func (b *bot) send(pkt []byte) error {
	b.mu.Lock()
	defer b.mu.Unlock()
	if b.conn == nil {
		return fmt.Errorf("conexao ausente")
	}
	key := b.key + 1
	if key == 0 {
		key = 1
	}
	b.key = key
	stamp := uint32(time.Now().UnixMilli())
	b.tick.Store(stamp)
	binary.LittleEndian.PutUint32(pkt[8:12], stamp)
	wire.FinishPacket(pkt, key)
	written, err := b.conn.Write(pkt)
	if written > 0 {
		b.counters.bytesSent.Add(int64(written))
	}
	return err
}

func (b *bot) accountName() string {
	return fmt.Sprintf("%s%04d", strings.ToLower(b.config.AccountPrefix), b.index)
}

func initCodeBytes() []byte {
	b := make([]byte, 4)
	binary.LittleEndian.PutUint32(b, uint32(wire.InitCode))
	return b
}

func loginPacket(name, password string) []byte {
	// O client 7.48 envia o bloco completo de MSG_ConnectAccount (116
	// bytes). O servidor usa os campos legados em @12/@28/@40, mas a cauda
	// faz parte do contrato de framing e nao pode ser omitida.
	b := wire.Build(wire.OpConnectAccount, wire.SceneCharList, 116)
	copy(b[12:28], name)
	copy(b[28:40], password)
	binary.LittleEndian.PutUint32(b[40:44], 748)
	return b
}

func characterLoginPacket(slot uint32) []byte {
	b := wire.Build(wire.OpCharacterLogin, wire.SceneCharList, 36)
	binary.LittleEndian.PutUint32(b[12:16], slot)
	return b
}
