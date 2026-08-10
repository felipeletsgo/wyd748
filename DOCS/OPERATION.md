# Operação e diagnóstico — WYD-Go 7.48

Atualizado em 10/08/2026. Cobre a fundação operacional: relógio/RNG injetáveis,
métricas, profiling e desligamento controlado.

Contexto: o alvo declarado do servidor é **operação pública com centenas de
jogadores por canal**. A avaliação dos planos em `DOCS/PLANO_MELHORIA.md`
concluiu que a maior parte daquele documento é otimização prematura, mas que
estes quatro itens eram lacunas reais — nenhuma delas existia antes.

## Fronteira TCP e admissão

`data/server.txt` concentra os limites operacionais: conexões globais/por IP,
timeouts do InitCode, idle e frame parcial, pacotes/bytes por segundo, tentativas
de autenticação e chat. O idle padrão é 600 s porque o heartbeat do client 7.48
é espaçado; o frame parcial tem orçamento separado de 10 s, fechando Slowloris
sem derrubar jogadores parados.

Opcode C→S desconhecido é recusado antes do dispatcher, contabilizado sob o
rótulo fixo `unknown` e reincidência fecha a sessão. Isso evita DoS de log e
cardinalidade de métricas controlada pelo atacante.

## Account API

`/healthz` informa vida do processo e `/readyz` consulta PostgreSQL com timeout
curto. `X-Forwarded-For` só é honrado quando o peer pertence a `trusted-proxies`;
limite por IP e concorrência de PBKDF2 são flags. SIGINT/SIGTERM encerram o HTTP
graciosamente. O proxy declarado deve sobrescrever/remover o header recebido do
usuário antes de encaminhar.

## Relógio e RNG injetáveis

`internal/game/clock.go` define `Clock{Now}` e `RNG{Intn}`, com campos no
`World` e as opções `WithClock`/`WithRNG` (padrão `WorldOption`).

Produção usa o relógio e o RNG reais. O ganho é **testabilidade determinística**:
reset de área de quest, cooldown de mob, respawn e ação pendente de boss passaram
a ser verificáveis **sem `time.Sleep`**.

Dentro do game loop prefira `w.now()` a `time.Now()`. A migração é incremental —
código ainda não migrado continua correto, porque o default é exatamente o
comportamento anterior. `w.now()` tolera `World` com clock zerado (construído em
teste sem `NewWorld`).

**Cuidado de ordem:** os deadlines nascem **depois** das options em `NewWorld`.
Com o cálculo no literal do struct, um teste com relógio falso partiria de um
instante diferente do mundo.

Instrumentação (`observeTick`) usa o relógio **real** de propósito: mede custo de
execução, não tempo de jogo.

## Métricas (`expvar`)

`internal/game/metrics.go`. Usa **stdlib** — não Prometheus. O módulo tinha zero
dependências externas e isso é um ativo; a única dependência hoje é o gopher-lua
do sistema de boss.

| Métrica | O que é |
|---|---|
| `world_loop_lag_micros` | Atraso do último tick vs. os 500 ms nominais |
| `world_loop_lag_max_micros` | Pior atraso desde o boot |
| `world_tick_duration_micros` | Quanto o último tick levou |
| `world_command_queue_depth` | Fila de entrada no último tick |
| `postgres_async_queue_depth` | Fila FIFO de persistência assíncrona |
| `postgres_async_pending_coalesced` | Snapshots mantidos fora da fila cheia |
| `postgres_async_snapshots_coalesced_total` | Autosaves antigos substituídos por snapshots mais novos |
| `postgres_async_failures_total` | Lotes/retries assíncronos que falharam |
| `world_commands_total` | Comandos processados |
| `world_commands_by_type` | Por opcode |
| `world_command_duration_micros` | Tempo acumulado por opcode |

As capacidades `world_command_queue_capacity` e `auth_hash_concurrency` ficam
em `data/server.txt`; não existem mais valores invisíveis fixados em `World`.
O perfil de load test pode aumentá-las sem alterar o binário de produção.
| `world_active_players` / `world_active_mobs` | Gauges |
| `world_panics_total` | Panics contidos por `safeHandle` |

`world_panics_total` diferente de zero merece investigação: significa que **um
comando deixou estado potencialmente parcial**.

Cardinalidade: o rótulo de comando é o opcode, um conjunto fechado. **Nunca**
indexar por sessão, jogador ou mob.

Referência de saúde medida em máquina de desenvolvimento com 6.183 mobs e 4
bosses: lag de 290–770 µs, tick ~0, sem panic.

## Diagnóstico HTTP (`debug_address`)

`expvar` em `/debug/vars` e pprof em `/debug/pprof`. **Desligado por padrão.**

```ini
# data/server.txt
debug_address=127.0.0.1:6060
```

**O host precisa ser loopback.** Esses endpoints expõem estado interno e
permitem disparar profiling; um endereço público é risco de vazamento e de DoS.
A validação **derruba o boot** em vez de subir aberto:

```text
debug_address: host "0.0.0.0" nao e loopback; o diagnostico so pode
escutar em 127.0.0.1 (use tunel SSH)
```

A flag `-debug_address` sobrescreve o arquivo e passa pela **mesma** checagem —
sem isso, `-debug_address 0.0.0.0:6060` furaria a validação do boot.

Acesso remoto por túnel SSH:

```bash
ssh -L 6060:127.0.0.1:6060 usuario@servidor
```

Verificado: inacessível pelo IP da LAN; flag pública derruba o boot.

## Desligamento controlado

SIGTERM (systemd, deploy) e SIGINT (Ctrl+C) persistem antes de sair. Sem isso, o
que estiver na fila de autosave é descartado e o jogador volta com estado velho.

O desligamento roda **como um comando do game loop**. O `World` é dono único do
estado; gravar de fora exigiria trava ou correria com um handler em curso. Como
comando, o drain vê um estado quieto e consistente.

Sequência: marca `shuttingDown` (novos logins recusados com aviso de manutenção)
→ save **síncrono** por conta, deduplicado (dois personagens da mesma conta
compartilham o ponteiro) → `Flush()` da fila assíncrona do store → libera quem
chamou. Timeout de 20 s; falha de disco numa conta não impede as demais nem
trava o desligamento.

> **Não validado em processo real.** No Windows `Stop-Process` é kill forçado,
> não sinal. A lógica tem seis testes (incluindo o fluxo pelo game loop, falha
> de disco e idempotência), mas a verificação do sinal em si só acontece no alvo
> Ubuntu/systemd.

## Alocação de IDs de mob

`allocMobID` pula IDs presentes em `mobsByID`. Dois bugs reais motivaram isso:

1. O clamp vinha **depois** da reserva: um `World` de contador zerado devolvia
   `0` (que significa "sem ID") e em seguida `1000`.
2. O contador **dá a volta** em 65535. Como cada respawn consome um ID novo, um
   servidor de longa duração inevitavelmente retorna ao início da faixa; sem
   verificar ocupação, o ID de um mob **vivo** seria reusado e a entrada dele em
   `mobsByID`, sobrescrita — corrompendo alvo, visibilidade e affects **em
   silêncio**. Foi assim que um escudo de boss chegou a ser removido do add
   errado.

`mobsByID` é registro fiel dos mobs vivos (`removeMobInstance` limpa), então
basta pular o que já está lá.

## Verificação

```powershell
cd wyd-go
go test ./...
go vet ./...
go build -o tm.exe ./cmd/server
```

Código morto:

```powershell
go run golang.org/x/tools/cmd/deadcode@latest -test ./...
```

A flag `-test` evita falso-positivo de helper usado só em teste. Separe **morto
real** de **scaffolding** — felipe deixa stubs de propósito. Símbolos
intencionalmente não usados hoje: `magicDamage` (`combat.go`, superado por
`skillFinalDamage`), `mobHasActivePlayer` (`mob_combat.go`, aguarda chamador) e
`cstr`/`indexByte` (`cmd/npcconvert`, ferramenta pontual).

`go test -race` exige cgo e não roda no ambiente Windows atual.
