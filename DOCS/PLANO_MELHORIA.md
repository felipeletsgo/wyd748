# Plano de evolução arquitetural, desempenho e confiabilidade — WYD 7.48

> **NOTA DE ESTADO (24/07/2026).** Avaliado e parcialmente adotado. Os
> princípios estão certos; o sequenciamento e algumas premissas factuais não.
>
> **Premissas que já não valiam quando o plano foi escrito:** comandos NÃO
> esperam o tick (o `select` de `Run()` processa assim que chegam); a ativação
> de mob dirigida pelo jogador via spatial grid (§13.1, a proposta central) **já
> existia**, e sem `MobGroupController`; cooldowns já eram deadline absoluto
> (§7.5); a fila de saída por sessão já era limitada com desconexão de slow
> client.
>
> **Lacunas reais, todas corrigidas:** `allocPlayerID` devolvia `999` em vez de
> recusar login; não havia métricas, pprof, desligamento controlado nem
> `Clock`/`RNG` injetáveis. Ver `DOCS/OPERATION.md`.
>
> **Implementado em 26/07/2026:** PostgreSQL (§11) como store autoritativo,
> com pool limitado, transações serializáveis, UID global de item e JSON apenas
> como adaptador explícito de desenvolvimento.
>
> **Adiado conscientemente:** reestruturação hexagonal (§5 —
> invalidaria os testes de uma vez), Lua para gameplay (§16 — hoje só configura
> boss, sem executar comportamento), multicanal (§21), AOI incremental (§14) e
> `cmd/loadbot` (§20).
>
> **Substituído:** Prometheus (§25) por `expvar` da stdlib. O módulo tinha zero
> dependências externas e isso é um ativo; a única hoje é o gopher-lua do
> sistema de boss.
>
> Princípio adotado: **medir antes de reestruturar**.

**Projeto:** `felipeletsgo/wyd748`
**Objetivo:** preparar o servidor para operar como um MMORPG público, com foco em integridade econômica, justiça no PvP, responsividade de combate, estabilidade de eventos e capacidade para centenas de jogadores por canal em uma VPS Ubuntu Server.
**Status:** proposta técnica e roteiro de implementação.
**Princípio central:** **nenhuma ação confirmada ao jogador pode ser perdida, e nenhuma operação lenta pode bloquear o mundo inteiro.**

> **Decisão de simplificação da IA:** mobs adormecidos serão acordados diretamente pelos jogadores que entrarem em seu raio de ativação. O sistema não dependerá de `MobGroupController`, líder fazendo varredura, área especial de grupo ou hierarquia de ativação. Grupos e líderes do `NPCGenerator` continuam existindo somente para spawn e comportamento de gameplay.

---

## 1. Contexto e diagnóstico atual

O projeto já possui boas decisões fundamentais:

- servidor autoritativo;
- um único actor `World` como proprietário do estado mutável;
- comandos e ticks processados linearmente;
- grid espacial para jogadores e mobs;
- conjunto de `activeMobs`, evitando processar todos os NPCs igualmente;
- persistência atômica em JSON;
- autosave assíncrono;
- operações críticas com persistência antes da confirmação;
- conteúdo orientado a dados;
- testes por sistema;
- validações no boot.

A arquitetura atual usa um timer global de **500 ms**. Dentro desse timer, diversas mecânicas são executadas ou distribuídas por contadores. Os comandos de rede também entram no mesmo loop do `World`.

Essa base é correta para consistência, mas há riscos importantes para uma operação pública:

1. uma fila congestionada pode atrasar golpes, magias, movimento e IA;
2. operações críticas de disco podem bloquear a única goroutine do mundo;
3. o autosave periódico pode gerar rajadas de trabalho;
4. muitas mecânicas compartilham o mesmo tick, mesmo tendo necessidades diferentes;
5. a camada de gameplay conhece diretamente sessão, protocolo e persistência;
6. um panic pode deixar uma mutação parcialmente aplicada e o servidor continuar;
7. áreas densas podem provocar fan-out próximo de O(n²);
8. o cliente usa IDs de jogadores abaixo de 1000, impondo limite funcional por canal;
9. aumentar indiscriminadamente a frequência do tick pode multiplicar o custo sem melhorar a jogabilidade;
10. PostgreSQL ou Lua mal integrados podem piorar a latência em vez de melhorá-la.

Referências da implementação atual:

- [Repositório](https://github.com/felipeletsgo/wyd748)
- [`internal/game/world.go`](https://github.com/felipeletsgo/wyd748/blob/main/internal/game/world.go)
- [`internal/net/session.go`](https://github.com/felipeletsgo/wyd748/blob/main/internal/net/session.go)
- [`internal/store/store.go`](https://github.com/felipeletsgo/wyd748/blob/main/internal/store/store.go)
- [`internal/game/visibility.go`](https://github.com/felipeletsgo/wyd748/blob/main/internal/game/visibility.go)

---

# 2. Garantias obrigatórias do servidor

Estas garantias devem orientar todas as decisões futuras.

## 2.1. Integridade econômica

Para itens, gold e progressão crítica:

- nenhum item confirmado pode desaparecer após desconexão ou crash;
- nenhuma operação pode criar duas cópias do mesmo item;
- trade deve ser atômico entre os dois jogadores;
- crafting deve consumir materiais e conceder resultado na mesma transação;
- refino deve consumir custo e aplicar resultado na mesma transação;
- compra deve retirar gold e entregar item na mesma transação;
- drop raro, recompensa de boss e recompensa de evento devem ser recuperáveis;
- reprocessar uma operação após crash não pode duplicar seu efeito;
- snapshots antigos nunca podem sobrescrever transações mais recentes.

## 2.2. Justiça de gameplay

- nenhum hit ou magia deve ser descartado silenciosamente;
- ações devem ser avaliadas pelo instante em que chegaram ao servidor, não pelo atraso eventual da fila;
- um congestionamento não pode fazer um jogador perder cooldown sem executar a habilidade;
- o servidor não deve executar uma rajada de vários ataques de boss para “compensar” atraso;
- IA ativa não pode ficar paralisada por processamento de autosave, logs ou scripts;
- movimento visual pode ser consolidado, mas eventos de combate não;
- quando o servidor não conseguir manter a qualidade, deve degradar funcionalidades secundárias antes das críticas;
- em sobrecarga severa, deve recusar novos logins em vez de degradar silenciosamente todos os jogadores.

## 2.3. Fonte única de verdade

Continuam proibidos:

- confiar em dano informado pelo cliente;
- confiar em preço informado pelo cliente;
- confiar em cooldown informado pelo cliente;
- confiar em posição final sem validação;
- confiar em item, quantidade, slot ou alvo sem revalidação;
- permitir que Lua modifique estruturas internas diretamente;
- permitir que handlers de rede escrevam estado do jogo fora do `World`.

---

# 3. Metas iniciais de qualidade

Os números abaixo são metas de engenharia iniciais. Devem ser ajustados após testes com o cliente real.

## 3.1. Latência interna

Medir apenas o tempo dentro do servidor, excluindo internet do jogador.

| Métrica | Meta normal | Alerta |
|---|---:|---:|
| Comando recebido → início do processamento, p95 | < 30 ms | > 75 ms |
| Comando recebido → início do processamento, p99 | < 75 ms | > 150 ms |
| Resolução de ataque/magia, p99 | < 100 ms | > 200 ms |
| Atraso de decisão de boss em combate, p99 | < 100 ms | > 250 ms |
| Atraso de movimento de mob ativo, p99 | < 150 ms | > 300 ms |
| Commit econômico crítico no PostgreSQL local, p99 | < 40 ms | > 150 ms |
| Fila de saída por sessão | próxima de zero | crescimento contínuo |
| Pausa de GC, p99 | < 10 ms | > 30 ms |

## 3.2. Integridade

- **zero perda** de item/gold confirmado;
- **zero duplicação** em testes de crash e repetição;
- trade sempre totalmente aplicado ou totalmente revertido;
- crafting sempre totalmente aplicado ou totalmente revertido;
- recompensa rara sempre concedida, pendente ou recuperável;
- nenhuma confirmação econômica enviada antes da durabilidade;
- nenhuma mutação parcial após panic.

## 3.3. Capacidade

O primeiro objetivo de teste deve ser:

- 999 jogadores conectados em um canal;
- centenas de jogadores espalhados;
- 300 jogadores em uma área densa;
- 100–300 jogadores em evento de boss;
- tempestade de login/relogin;
- combate PvP concentrado;
- autosave e commits críticos concorrentes.

O número total de conexões não basta. A densidade em uma única área é normalmente o cenário mais caro.

---

# 4. Decisão arquitetural principal

## 4.1. Manter o modelo de actor

O `World` deve continuar sendo o proprietário exclusivo do estado mutável do canal.

Não adicionar mutexes em todos os sistemas e não criar goroutines que modifiquem diretamente jogadores, mobs, inventários e guildas.

A concorrência deve existir **ao redor** do actor:

- leitura de socket;
- decodificação preliminar;
- autenticação;
- codificação/finalização de pacotes;
- escrita em socket;
- persistência;
- métricas;
- scripts sem acesso direto ao estado;
- geração de relatórios;
- backups.

Toda mutação de gameplay volta ao `World` por mensagens tipadas.

## 4.2. Transformar `World` em coordenador

O `World` atual armazena e manipula praticamente todos os sistemas. A evolução recomendada é:

```go
type World struct {
    ingress     *IngressQueue
    scheduler   *Scheduler
    players     *PlayerRegistry
    mobs        *MobSystem
    combat      *CombatSystem
    inventory   *InventorySystem
    economy     *EconomySystem
    trades      *TradeSystem
    guilds      *GuildSystem
    quests      *QuestSystem
    events      *EventSystem
    visibility  *VisibilitySystem
    persistence PersistenceGateway
    clients     ClientGateway
    clock       Clock
    rng         RNG
}
```

Esses componentes não precisam de goroutines próprias. Eles continuam sendo chamados somente pelo actor.

## 4.3. Fluxo desejado

```text
TCP
  ↓
Session Reader
  ↓
Validação de framing/checksum/limites
  ↓
Decoder WYD 7.48
  ↓
Command tipado + ReceivedAt + Sequence
  ↓
World actor
  ↓
Sistemas de domínio
  ↓
Domain Events / Client Messages / Persistence Requests
  ↓
Encoder WYD 7.48
  ↓
Session Writer
```

Persistência:

```text
World
  ↓
Persistence Request imutável
  ↓
Persistence Workers
  ↓
PostgreSQL / WAL
  ↓
Persistence Result
  ↓
World finaliza ou reverte o estado pendente
```

---

# 5. Separar protocolo, aplicação, domínio e infraestrutura

## 5.1. Estrutura recomendada

```text
cmd/
  server/
  account-api/
  account-create/
  loadbot/

internal/
  domain/
    character/
    combat/
    inventory/
    economy/
    trade/
    guild/
    quest/
    event/
    mob/

  application/
    world/
      world.go
      loop.go
      commands.go
      events.go
      scheduler.go
      overload.go
    ports/
      client_gateway.go
      persistence.go
      clock.go
      rng.go
      scripts.go

  adapters/
    protocol/
      wyd748/
        decoder/
        encoder/
        packets/
        router.go
    transport/
      tcp/
        session.go
        listener.go
        rate_limit.go
    persistence/
      jsonstore/
      postgres/
        migrations/
        repository.go
        transactions.go
        recovery.go
    scripting/
      lua/
    gamedata/

  bootstrap/
    config.go
    server.go
    observability.go
```

## 5.2. Remover `net.Session` do domínio

Evitar:

```go
type Player struct {
    Session *net.Session
}
```

Preferir:

```go
type SessionID uint64
type PlayerID uint16

type Player struct {
    ID        PlayerID
    SessionID SessionID
}
```

Saída:

```go
type ClientGateway interface {
    Send(SessionID, ClientMessage)
    Disconnect(SessionID, DisconnectReason)
}
```

Benefícios:

- testes sem socket;
- bots internos;
- replay;
- simulação;
- reconexão;
- troca futura de transporte;
- domínio independente do pacote `net`.

## 5.3. Commands tipados

Exemplo:

```go
type UseSkillCommand struct {
    SessionID  SessionID
    PlayerID   PlayerID
    SkillID    SkillID
    TargetID   EntityID
    ReceivedAt time.Time
    Sequence   uint64
}
```

O decoder conhece offsets e opcodes. O `CombatSystem` não.

## 5.4. Separar modelos

O domínio não deve depender de:

- tags JSON;
- layouts binários;
- `WireScore`;
- tamanho fixo de pacote;
- detalhes de endianness;
- campos exclusivos de banco.

Criar conversões explícitas:

```go
func CharacterFromRecord(record CharacterRecord) (*domain.Character, error)
func CharacterToRecord(character *domain.Character) CharacterRecord
func CharacterToWire(character *domain.Character) wire.CharacterInfo
```

A migração pode ser gradual. Começar pelos tipos `Wire...` e pelas estruturas de persistência.

---

# 6. Redesenho do loop principal

## 6.1. Não reduzir simplesmente o tick global

Trocar o tick de 500 ms por 50 ms faria o servidor acordar dez vezes mais e poderia multiplicar:

- varreduras de jogadores;
- varreduras de mobs;
- expiração de buffs;
- regen;
- verificações de trade;
- atualizações de visibilidade;
- garbage gerado;
- pacotes enviados.

Isso não garante que um hit será processado em 50 ms, pois a fila ainda pode estar congestionada.

A solução correta é separar:

1. **latência de comandos**;
2. **resolução do scheduler**;
3. **cadência de cada mecânica**.

## 6.2. Comandos devem ser processados imediatamente

Ataques, skills, movimento, trade e interações não devem aguardar um tick global.

Ao chegar:

1. registrar `ReceivedAt` com relógio monotônico;
2. atribuir uma sequência;
3. validar;
4. processar assim que o actor estiver disponível;
5. agendar efeitos futuros por deadline absoluto.

## 6.3. Scheduler por deadline

Substituir a dependência de um único ticker por um scheduler que desperta no próximo deadline.

Pode ser implementado inicialmente com:

- min-heap;
- timing wheel;
- buckets circulares.

Interface conceitual:

```go
type ScheduledAction struct {
    At       time.Time
    Kind     ActionKind
    EntityID EntityID
    Token    uint64
}
```

O loop:

```go
for {
    nextDeadline := scheduler.NextDeadline()
    timer.Reset(time.Until(nextDeadline))

    select {
    case cmd := <-ingress:
        processCommand(cmd)
    case result := <-persistenceResults:
        processPersistenceResult(result)
    case <-timer.C:
        runDueActions(clock.Now())
    }
}
```

## 6.4. Orçamento para não deixar timers morrerem na fila

Se comandos chegam continuamente, um `select` simples pode atrasar ações programadas.

Aplicar orçamento:

- processar no máximo N comandos;
- ou processar comandos por no máximo 2–5 ms;
- depois verificar deadlines vencidos;
- repetir.

Exemplo:

```go
for processed := 0; processed < maxBatch; processed++ {
    if scheduler.HasOverdue(clock.Now()) {
        break
    }

    select {
    case cmd := <-ingress:
        process(cmd)
    default:
        break
    }
}

runDueActions(clock.Now())
```

Os valores devem ser definidos por benchmark.

## 6.5. Cadências iniciais recomendadas

Não são valores finais. Validar contra animações e protocolo do cliente.

| Mecânica | Modelo recomendado |
|---|---|
| Ataque/skill iniciado pelo jogador | imediato ao comando |
| Validação de cooldown | deadline absoluto |
| Cast time | ação agendada para o instante de conclusão |
| Mob/boss em combate — decisão | 100–250 ms |
| Mob comum ativo — aquisição de alvo | 250–500 ms |
| Mob ativo — movimento | 100–250 ms |
| Mob adormecido | sem tick ou 1–5 s |
| Patrulha distante | 500 ms–2 s |
| Atualização de visibilidade | orientada a movimento + consolidação de 50–100 ms |
| Regeneração | 1 s ou regra nativa |
| DoT/HoT | cadência definida pela skill, por deadline |
| Expiração de buff | deadline, sem varrer todos |
| Trade timeout | deadline |
| Item no chão | heap/bucket por expiração |
| Spawn | deadline absoluto |
| Reset de quest/evento | deadline absoluto |
| Autosave | sujo + agendamento individual |
| Métricas | 1–10 s, fora do caminho quente |

O timer de 500 ms pode continuar existindo apenas para mecânicas que realmente dependem da semântica nativa.

---

# 7. Semântica correta quando o servidor atrasa

Este ponto é essencial para evitar injustiça.

## 7.1. Ataques de mobs e bosses

Se um boss deveria atacar às 12:00:00.000 e o loop só consegue processar às 12:00:00.300:

- executar no máximo um ataque;
- não executar vários ataques acumulados;
- atualizar o próximo deadline pulando intervalos vencidos;
- registrar a métrica de atraso.

Exemplo:

```go
func advanceDeadline(previous, now time.Time, interval time.Duration) time.Time {
    next := previous.Add(interval)
    for !next.After(now) {
        next = next.Add(interval)
    }
    return next
}
```

Isso evita:

- rajada injusta após lag;
- boss permanentemente mais lento;
- múltiplos danos instantâneos.

## 7.2. DoT e HoT

Definir por habilidade:

- habilidades que representam dano contínuo podem acumular tempo decorrido;
- aplicar limite máximo de catch-up, por exemplo dois pulsos;
- nunca aplicar dezenas de pulsos de uma vez após stall;
- registrar perda de cadência como erro operacional.

## 7.3. Regeneração

Pode usar tempo decorrido:

```text
regen = taxa_por_segundo × tempo_decorrido
```

Aplicar um limite para evitar recuperação enorme após pausa longa.

## 7.4. Movimento

- validar deslocamento pelo tempo decorrido;
- manter velocidade máxima autoritativa;
- não depender de contagem de ticks;
- consolidar atualizações visuais sem perder a posição autoritativa;
- usar deadlines monotônicos.

## 7.5. Cooldowns

Nunca decrementar uma variável “a cada tick”.

Guardar:

```go
ReadyAt time.Time
```

Validar:

```go
if receivedAt.Before(ReadyAt) {
    reject()
}
```

Assim, atrasos do loop não alongam ou encurtam o cooldown.

## 7.6. Casts

Ao iniciar:

- validar imediatamente;
- reservar mana/estado conforme regra;
- guardar `CompleteAt`;
- concluir por scheduler;
- cancelar de forma determinística se movimento, morte ou stun ocorrer.

## 7.7. Eventos

Fases devem possuir deadlines absolutos:

```text
RegistrationEndsAt
StartsAt
BossSpawnsAt
EndsAt
RewardsAt
```

Nunca depender apenas de `tickCounter++`.

---

# 8. Justiça no PvP

## 8.1. Timestamp de entrada no servidor

Toda ação deve receber um timestamp na goroutine da sessão logo após o pacote completo ser validado.

Não usar timestamp enviado pelo cliente como verdade.

Campos:

```go
type IngressCommand struct {
    SessionID  SessionID
    Sequence   uint64
    ReceivedAt time.Time
    Kind       CommandKind
    Payload    any
}
```

A validação de cooldown, ordem e alcance deve considerar `ReceivedAt`, desde que dentro de limites seguros.

## 8.2. Ordem

Manter ordem por sessão.

Para conflitos entre sessões:

- usar sequência global de ingresso;
- preservar ordem de chegada ao servidor;
- não deixar filas de prioridade reordenarem ações críticas arbitrariamente.

É possível ter filas distintas para tarefas administrativas e de baixa prioridade, mas comandos de gameplay que interagem entre si devem manter ordenação consistente.

## 8.3. Movimento e ataque intercalados

Pode consolidar movimentos consecutivos:

```text
Move A
Move B
Move C
```

em:

```text
Move C
```

Mas não atravessar eventos:

```text
Move A
Attack
Move B
```

Não pode virar:

```text
Attack
Move B
```

A consolidação deve respeitar a ordem por jogador.

## 8.4. Histórico curto de posições

Para skills de alcance e PvP competitivo, considerar manter histórico do estado autoritativo:

```go
type PositionSample struct {
    At time.Time
    X  uint16
    Y  uint16
}
```

Janela inicial sugerida: 250–500 ms.

Uso:

- avaliar a posição no instante de ingresso do comando;
- não aceitar posição enviada pelo cliente sem validação;
- limitar qualquer compensação a uma janela pequena;
- impedir que alta latência conceda vantagem excessiva.

Essa compensação deve ser implementada somente após testes, pois pode alterar as regras do cliente original.

## 8.5. Nunca descartar silenciosamente

Caso a fila esteja cheia:

- não descartar attack/skill/trade/craft;
- identificar a sessão que excedeu limite;
- aplicar rate limit;
- desconectar cliente abusivo;
- recusar novos logins se o servidor inteiro estiver saturado.

Movimento visual redundante pode ser substituído pelo estado mais recente. Combate e economia não.

---

# 9. Backpressure e proteção contra congestionamento

## 9.1. Limites por sessão

Criar token buckets separados:

| Categoria | Tratamento |
|---|---|
| movimento | alto limite, coalescível |
| ataque/skill | limite baseado em velocidade/cooldown + margem |
| chat | limite baixo |
| trade/craft/economia | limite baixo, nunca repetição rápida |
| pacotes inválidos | penalização imediata |
| login | limite por IP e global |

Um cliente não deve conseguir ocupar sozinho a fila global.

## 9.2. Fila global

A fila deve ser limitada e observável.

Métricas:

- profundidade atual;
- profundidade máxima;
- tempo do comando mais antigo;
- comandos por tipo;
- sessões que mais enviam;
- descartes/coalescências;
- bloqueios no enqueue.

## 9.3. Estado de sobrecarga

Criar estados explícitos:

### Normal

Tudo opera na cadência nominal.

### Pressão moderada

- reduzir logs;
- consolidar mais movimento;
- reduzir atualização de patrulha distante;
- reduzir refresh de elementos cosméticos;
- manter combate, boss e economia.

### Pressão alta

- recusar novos logins;
- impedir entrada em eventos;
- reduzir AOI de elementos não críticos;
- manter jogadores atuais estáveis.

### Emergência

- colocar economia em modo seguro se PostgreSQL estiver indisponível;
- interromper crafting/trade/lojas que exigem commit;
- manter combate apenas se puder preservar estado;
- iniciar shutdown controlado caso invariantes estejam ameaçadas.

Nunca tornar bosses passivos ou skills aleatoriamente inválidas como mecanismo de degradação.

---

# 10. Persistência sem perda e sem bloquear o mundo

## 10.1. Problema a resolver

Salvar criticamente dentro da goroutine do `World` garante durabilidade, mas um `fsync` lento pode congelar:

- PvP;
- boss;
- IA;
- movimento;
- todos os jogadores.

Salvar de forma assíncrona e confirmar antes do commit evita freeze, mas permite perda após crash.

É necessário um modelo intermediário: **commit assíncrono para o actor, confirmação somente após durabilidade**.

## 10.2. Operação pendente

Fluxo recomendado:

```text
1. World recebe comando
2. Valida tudo
3. Cria OperationID idempotente
4. Reserva recursos em memória
5. Marca slots/itens como Pending
6. Envia PersistenceRequest
7. World continua processando outras entidades
8. Worker abre transação PostgreSQL
9. Commit
10. Worker devolve Committed/Failed
11. World finaliza estado e envia confirmação
```

Enquanto pendente:

- o item não pode ser vendido, trocado, movido ou usado;
- o gold reservado não pode ser gasto;
- a operação não pode ser reenviada como nova;
- desconectar não cancela o commit;
- reconectar deve consultar o estado persistido.

## 10.3. Estados de operação

```go
type OperationState byte

const (
    OperationPrepared OperationState = iota
    OperationCommitting
    OperationCommitted
    OperationRejected
)
```

Cada operação deve ter:

- `OperationID`;
- tipo;
- participantes;
- versão esperada;
- recursos consumidos;
- recursos concedidos;
- data;
- estado;
- erro, se houver.

## 10.4. Classes de durabilidade

### Classe A — zero perda

Persistir antes de confirmar:

- item dropado e coletado;
- item craftado;
- resultado de refino;
- trade;
- compra/venda;
- gold;
- quest com recompensa;
- recompensa rara;
- recompensa de boss/evento;
- skill point/stat point gasto;
- criação ou remoção de item;
- mudanças de guilda que envolvam custo;
- pontuação crítica de guerra/evento.

### Classe B — pequena janela aceitável, configurável

Pode usar journal consolidado de 1–3 s:

- EXP comum;
- mastery;
- progresso repetitivo;
- contadores não econômicos.

Para um servidor que promete máxima confiabilidade, também pode registrar esses eventos imediatamente em lote.

### Classe C — snapshot eventual

- posição de reentrada;
- configuração de interface;
- estado cosmético;
- métricas;
- dados temporários não valiosos.

## 10.5. Crafting

Transação:

```text
BEGIN
  validar versão do personagem
  validar materiais e slots
  retirar materiais
  criar item com ItemInstanceID único
  gravar ledger
  incrementar versão
COMMIT
```

Somente após `COMMIT`:

- atualizar estado final no cliente;
- enviar sucesso;
- liberar item para uso.

Em falha:

- remover reserva;
- informar erro;
- nenhum material consumido.

## 10.6. Trade

Transação única:

```text
BEGIN
  bloquear personagens em ordem determinística
  validar versões
  validar ownership dos itens
  validar gold
  transferir itens
  transferir gold
  gravar ledger dos dois lados
  atualizar versões
COMMIT
```

Regras:

- ordenar locks por `CharacterID` para evitar deadlock;
- `OperationID` único;
- repetir a mesma operação deve retornar o mesmo resultado;
- confirmação aos dois jogadores somente após commit;
- desconexão durante commit não reverte operação já confirmada no banco.

## 10.7. Drops e bosses

Há três momentos diferentes:

### Geração do loot

Para loot comum, pode ser criado em memória e persistido quando coletado, desde que não tenha sido prometido nominalmente ao jogador.

Para loot raro, boss ou evento:

1. gerar `RewardID`;
2. persistir a recompensa devida;
3. depois materializar no chão, inventário ou caixa de recompensa.

Uma abordagem mais segura para eventos é uma **reward inbox**:

```text
reward_claims
  reward_id
  character_id
  source_type
  source_id
  payload
  status
```

Se o jogador cair:

- recompensa continua devida;
- no próximo login ela é entregue;
- não depende de o pacote de confirmação ter chegado.

## 10.8. Pickup

O pickup precisa ser transacional:

- validar que o item existe;
- validar distância;
- validar ownership/reserva;
- validar espaço;
- remover item do mundo;
- adicionar ao inventário;
- persistir;
- confirmar.

Durante commit, o item fica reservado e não pode ser coletado por outro jogador.

## 10.9. Desconexão

Na desconexão:

- marcar sessão como fechando;
- impedir novos comandos;
- manter operações pendentes;
- não cancelar commit crítico;
- gravar snapshot final não crítico de maneira assíncrona;
- remover jogador do mundo somente após estabilizar referências necessárias;
- permitir reconexão após reconciliar versão no banco.

Nunca usar um snapshot antigo do objeto `Account` para sobrescrever operações já gravadas.

## 10.10. Crash do processo

Testar estes pontos:

1. crash antes de enviar request;
2. crash após reservar em RAM;
3. crash durante transação;
4. crash após commit e antes do ack ao `World`;
5. crash após ack ao `World` e antes do pacote ao cliente;
6. crash após pacote;
7. repetição da operação após reboot.

A combinação de `OperationID`, ledger e constraints deve tornar todos os casos idempotentes.

---

# 11. PostgreSQL

## 11.1. O que PostgreSQL deve melhorar

- transações;
- consistência;
- recuperação;
- múltiplos canais;
- auditoria;
- ranking;
- guildas;
- operações entre contas;
- consultas administrativas;
- backup;
- constraints contra duplicação.

PostgreSQL não deve participar de:

- cada movimento;
- cada ataque;
- cálculo de dano;
- cooldown;
- busca de alvo;
- pathfinding;
- visibilidade;
- tick comum;
- cada execução Lua.

## 11.2. Modelo híbrido recomendado

Em RAM:

- estado ativo do personagem;
- mobs;
- combate;
- cooldowns;
- buffs temporários;
- posição;
- AOI;
- eventos ativos.

No banco:

- conta;
- personagem;
- inventário;
- cargo;
- item instances;
- gold/moedas;
- progressão;
- guilda;
- ledger;
- operações idempotentes;
- reward inbox;
- snapshots;
- outbox intercanal.

## 11.3. Tabelas sugeridas

```text
accounts
characters
character_snapshots
item_instances
inventory_slots
cargo_slots
currencies
guilds
guild_members
economy_operations
economy_ledger
reward_claims
event_scores
outbox_events
schema_migrations
```

## 11.4. ItemInstanceID

Todo item relevante deve ter identificador único persistente:

```text
BIGINT, UUID ou UUIDv7
```

Não usar apenas índice/slot como identidade.

Constraints:

- um `item_instance` pertence a no máximo um local;
- um slot possui no máximo um item;
- operação não pode inserir o mesmo item duas vezes;
- item removido mantém histórico no ledger.

## 11.5. Versão do agregado

Cada personagem:

```text
version BIGINT
```

Atualização:

```sql
UPDATE characters
SET version = version + 1
WHERE id = $1 AND version = $expected;
```

Se nenhuma linha for alterada:

- houve conflito;
- recarregar/reconciliar;
- não sobrescrever.

## 11.6. Pools

Usar `pgxpool`, com pool limitado.

Na mesma VPS:

- começar com 4–8 conexões;
- medir espera;
- não usar uma conexão por jogador;
- separar workers críticos e de snapshot logicamente;
- evitar centenas de transações simultâneas.

## 11.7. Filas distintas

### Fila crítica

- craft;
- trade;
- gold;
- pickup;
- rewards;
- refino.

Prioridade alta, baixa latência, sem descarte.

### Fila de snapshot

- autosave;
- posição;
- estado consolidado.

Pode:

- agrupar;
- substituir snapshot antigo pelo novo;
- aplicar backpressure;
- atrasar sob carga.

## 11.8. Banco indisponível

Fail closed para economia:

- crafting temporariamente indisponível;
- trade temporariamente indisponível;
- compras críticas indisponíveis;
- não consumir item;
- não confirmar recompensa não durável.

Para bosses/eventos, implementar reward claim pendente.

Opcionalmente, criar um emergency journal local fsynced para registrar recompensas enquanto o banco está fora, mas isso adiciona um segundo mecanismo de recuperação e deve ser feito somente com testes rigorosos.

## 11.9. PostgreSQL na mesma VPS

Riscos:

- competição por CPU;
- competição por RAM;
- competição por NVMe;
- checkpoint/WAL causando picos;
- backup afetando latência.

Medidas:

- reservar memória;
- limitar conexões;
- monitorar I/O wait;
- monitorar p95/p99 de commit;
- configurar backups fora do horário de pico;
- considerar VPS separada quando houver crescimento;
- usar NVMe confiável.

---

# 12. Autosave

## 12.1. Remover rajada global

Atualmente o autosave periódico percorre jogadores em um mesmo momento.

Substituir por:

- dirty flags;
- deadline individual;
- jitter;
- coalescência;
- versão;
- snapshot imutável.

## 12.2. Dirty flags

Exemplo:

```go
type DirtyFlags uint32

const (
    DirtyProgress DirtyFlags = 1 << iota
    DirtyPosition
    DirtyAffects
    DirtyMount
    DirtyMetadata
)
```

Quando o personagem muda:

```go
player.Dirty |= DirtyProgress
```

## 12.3. Agendamento distribuído

Ao entrar:

```text
NextSnapshotAt = now + intervalo + jitter
```

Com 999 jogadores, distribuir os snapshots ao longo de 30–60 segundos.

## 12.4. Coalescência

Se existem três snapshots pendentes do mesmo personagem:

- descartar os dois antigos;
- manter o mais recente;
- nunca substituir uma operação crítica no ledger.

---

# 13. IA de mobs e bosses

## 13.1. Estratégia principal: o jogador acorda o mob

A otimização inicial deve ser simples:

> Mobs adormecidos não executam IA. Quando um jogador entra no mundo, teleporta ou se movimenta para uma nova região, o spatial grid localiza os mobs adormecidos dentro do raio de ativação e os acorda individualmente.

Fluxo:

```text
Mob dormindo
    ↓
Jogador entra no raio
    ↓
Movimento do jogador consulta o spatial grid
    ↓
Mob é inserido em activeMobs
    ↓
Mob passa a executar a IA existente
```

Não é necessário que:

- o líder de um grupo procure jogadores;
- um controlador de grupo acorde os membros;
- grupos adormecidos tenham uma área de ativação própria;
- todos os membros compartilhem target acquisition;
- exista uma hierarquia adicional de controllers.

O `NPCGenerator`, os grupos e o campo de líder continuam sendo usados para:

- spawn;
- identificação de grupo;
- formação;
- comportamento específico de gameplay;
- respawn;
- eventuais bônus ou regras de liderança.

Eles não devem ser uma dependência da otimização básica de ativação.

## 13.2. Quando verificar proximidade

Executar a busca de mobs próximos quando o jogador:

- entra no mapa;
- termina o login;
- teleporta;
- troca de célula espacial;
- é movido autoritativamente para outra região;
- retorna de estado em que não interagia com o mundo.

Com célula espacial de tamanho semelhante ao raio de ativação, a troca de célula é um bom gatilho. Caso o raio seja menor que a célula, pode ser necessário verificar também ao atravessar uma distância mínima dentro da própria célula.

Exemplo:

```go
func (w *World) onPlayerChangedCell(
    player *Player,
    oldCell CellID,
    newCell CellID,
) {
    w.forEachSleepingMobNear(
        player.X,
        player.Y,
        mobActivationRange,
        func(mob *Mob) {
            w.wakeMob(mob, player)
        },
    )
}
```

A consulta deve:

- usar o spatial grid;
- evitar criar slices temporários;
- interromper cedo quando possível;
- verificar a distância exata apenas nos candidatos das células próximas;
- ser executada uma vez por movimentação relevante do jogador, não uma vez por mob.

## 13.3. Estruturas mínimas

Manter:

```go
mobsByID    map[MobID]*Mob
activeMobs  map[MobID]*Mob
mobCells    map[CellID]map[MobID]*Mob
```

O spatial index pode conter mobs ativos e adormecidos. O estado do mob define se ele participa dos ciclos de IA.

```go
type MobActivityState byte

const (
    MobSleeping MobActivityState = iota
    MobActive
)
```

Acordar:

```go
func (w *World) wakeMob(mob *Mob, player *Player) {
    if mob.ActivityState == MobActive || mob.Dead {
        return
    }

    mob.ActivityState = MobActive
    mob.TargetID = player.ID
    mob.NextThinkAt = w.clock.Now()
    w.activeMobs[mob.ID] = mob
}
```

A seleção imediata do jogador como alvo depende da regra de aggro. O evento pode apenas acordar o mob e deixar a IA validar o alvo.

## 13.4. Quando o mob volta a dormir

Não é necessário criar um sistema complexo de contagem por grupo.

Um mob ativo pode verificar a possibilidade de dormir somente quando:

- perdeu o alvo;
- concluiu retorno ao ponto de origem;
- saiu de combate;
- terminou uma ação e não encontrou alvo;
- passou por uma avaliação ociosa de baixa frequência.

Nesse momento, executar **uma consulta espacial**:

```go
if !w.hasLivingPlayerNear(mob.X, mob.Y, mobActivationRange) {
    w.sleepMob(mob)
}
```

Dormir:

```go
func (w *World) sleepMob(mob *Mob) {
    delete(w.activeMobs, mob.ID)

    mob.ActivityState = MobSleeping
    mob.TargetID = 0
    mob.Path = nil
    mob.NextThinkAt = time.Time{}
    mob.NextMoveAt = time.Time{}
    mob.NextAttackAt = time.Time{}
}
```

Para evitar acordar e dormir repetidamente na borda do raio, usar uma regra simples:

- raio para acordar: por exemplo 16;
- raio para dormir: por exemplo 20–24;

ou:

- dormir somente após alguns segundos sem alvo.

Isso é uma pequena histerese, não uma nova arquitetura.

## 13.5. Estados da IA ativa

Depois de acordado, o mob pode continuar usando a máquina de estados:

```text
Idle
AcquireTarget
Chase
Attack
Cast
Recover
Returning
Dead
Respawning
```

Campos úteis:

```go
NextThinkAt
NextMoveAt
NextAttackAt
StateSince
TargetID
PathFailureCount
LastProgressAt
```

O ganho principal não vem de executar esses estados em paralelo, mas de garantir que somente mobs acordados participem da IA.

## 13.6. Cadência por estado

| Estado | Cadência inicial |
|---|---:|
| Sleeping | nenhuma |
| Idle ativo | 500 ms–1 s |
| AcquireTarget | ao acordar e depois 250–500 ms |
| Chase | 100–250 ms |
| Attack | conforme cooldown, checagem 50–150 ms |
| Returning | 250–500 ms |
| Boss cast | deadline exato |
| Dead | somente respawn |

Não é necessário implementar timing wheel imediatamente.

Primeira implementação:

- manter `activeMobs`;
- distribuir avaliações em buckets;
- usar deadlines para ataques, casts e respawn;
- medir o custo.

Implementar timing wheel ou scheduler mais sofisticado somente se profiling demonstrar que percorrer `activeMobs` ainda é gargalo após a ativação orientada pelo jogador.

## 13.7. Evitar alocações nas consultas

Evitar:

```go
players := w.nearbyPlayers(...)
```

quando a pergunta é apenas:

- existe algum jogador próximo?
- qual é o jogador mais próximo?
- quais mobs adormecidos estão dentro do raio?

Preferir:

```go
func (w *World) hasLivingPlayerNear(
    x, y uint16,
    radius int,
) bool
```

e:

```go
func (w *World) forEachSleepingMobNear(
    x, y uint16,
    radius int,
    visit func(*Mob) bool,
)
```

O callback pode retornar `true` para interromper a busca.

## 13.8. Bosses

Boss continua sendo um mob ativo com prioridade maior.

Criar métricas:

- atraso de decisão;
- tempo sem atacar enquanto possui alvo válido;
- tempo sem progresso de movimento;
- falhas de pathfinding;
- tempo preso no mesmo tile;
- mudança de estado;
- jogadores na arena.

Watchdog:

- se está em combate, vivo, não stunado e com alvo válido;
- e não move/ataca/casta por um período anormal;
- registrar alerta;
- tentar reacquire/repath;
- somente usar reposicionamento automático se fizer parte das regras e não puder ser explorado.

Boss de evento pode permanecer acordado enquanto o evento estiver ativo, mesmo sem jogador momentaneamente dentro do raio.

## 13.9. Pathfinding

Manter a solução simples até existir evidência de gargalo:

1. movimento direto;
2. tentativa local de contorno;
3. recalcular somente quando alvo ou bloqueio mudar;
4. limitar o trabalho por tentativa;
5. medir falhas e duração.

Pathfinding em worker separado só deve ser implementado se profiling mostrar buscas realmente caras. Nesse caso:

1. o actor cria snapshot imutável;
2. o worker calcula;
3. devolve rota versionada;
4. o actor valida se ainda é aplicável;
5. aplica.

Não criar worker por mob.

## 13.10. Dificuldade consistente

Sob carga, reduzir:

- patrulha decorativa;
- atualização de mobs distantes;
- efeitos cosméticos;
- frequência de avaliação de mobs ativos sem alvo.

Não reduzir:

- frequência de ataques de mobs engajados;
- escolha de skill de boss;
- colisão;
- dano;
- detecção de morte;
- mecânicas de arena.

## 13.11. Métricas específicas

Medir:

- mobs totais;
- mobs em `activeMobs`;
- mobs acordados por segundo;
- mobs colocados para dormir por segundo;
- duração da consulta de wake por movimento;
- candidatos espaciais examinados;
- duração da IA por estado;
- ataques vencidos e atrasados;
- pathfinding;
- pacotes gerados por movimento de mob.

Essas métricas mostrarão se os 8 mil NPCs são realmente o gargalo ou se o custo está concentrado em visibilidade e rede.

---

# 14. Visibilidade e áreas densas

## 14.1. Indexar todas as entidades espaciais

Além de players e mobs:

- itens no chão;
- ghost shops;
- objetos de evento;
- summons;
- paredes/objetos de skill;
- NPCs temporários.

Estruturas:

```go
groundItemCells map[CellID]map[EntityID]*GroundItem
shopCells       map[CellID]map[EntityID]*GhostShop
eventCells      map[CellID]map[EntityID]*EventObject
```

Evitar varrer todos os itens ou lojas em cada refresh.

## 14.2. AOI incremental

Para cada jogador:

- célula atual;
- células assinadas;
- entidades visíveis;
- entidades adicionadas;
- entidades removidas;
- entidades alteradas.

Ao mudar de célula:

- comparar conjunto antigo e novo;
- materializar/desmaterializar apenas diferenças.

## 14.3. Dirty sets por célula

Quando entidade muda:

```text
cell.MarkDirty(entityID)
```

No flush:

- gerar atualização consolidada;
- enviar somente a observadores relevantes.

## 14.4. Fan-out em área cheia

Se 300 jogadores estão no mesmo espaço, cada movimento pode gerar centenas de envios.

Mitigações:

- consolidar movimento por janela de 50–100 ms;
- enviar posição mais recente;
- não enviar estados idênticos;
- separar atualização crítica de cosmética;
- usar pacotes de estado quando o protocolo permitir;
- construir payload base uma vez e finalizar por sessão no writer;
- limitar frequência visual sem alterar posição autoritativa;
- reduzir raio de elementos não críticos em sobrecarga;
- manter jogadores, boss e ataques com maior relevância.

## 14.5. Ocupação

`positionOccupied` deve consultar a célula, não varrer todas as lojas ou objetos globais.

---

# 15. Rede

## 15.1. Session reader

Responsabilidades:

- read deadline;
- handshake deadline;
- tamanho máximo;
- checksum;
- framing;
- rate limit;
- identificação de opcode;
- timestamp;
- sequência;
- decoder;
- enqueue.

Nunca aplicar regra de jogo.

## 15.2. Session writer

Mover para o writer:

- tick/header final;
- checksum;
- criptografia;
- write deadline;
- escrita;
- métricas.

O `World` deve enfileirar uma mensagem ou pacote não finalizado e continuar.

## 15.3. Ownership de buffers

`Send` não deve modificar um slice que o chamador pode reutilizar sem contrato explícito.

Alternativas:

- clonar na fronteira;
- buffer pool;
- tipo `OwnedPacket`;
- builder por sessão.

Começar pela segurança. Otimizar após pprof.

## 15.4. Fila de saída

Fila limitada por sessão.

Se um cliente não consegue consumir:

- medir backlog;
- consolidar atualizações substituíveis;
- não deixar o writer bloquear o actor;
- desconectar slow client após limite;
- nunca crescer memória sem limite.

## 15.5. TCP

- `TCP_NODELAY` explicitamente configurado e medido;
- read/write deadlines;
- keepalive;
- limite por IP;
- limite de conexões em handshake;
- fila de autenticação limitada;
- proteção contra slowloris;
- mensagens inválidas causam penalização/desconexão.

## 15.6. Logs de pacote

Em produção:

- não logar cada pacote;
- usar amostragem;
- logar erro com opcode, sessão e contexto;
- mascarar credenciais;
- usar níveis configuráveis.

---

# 16. Lua

## 16.1. Casos adequados

- diálogos;
- NPCs;
- quests;
- eventos;
- fases de boss;
- spawns;
- recompensas declarativas;
- timers;
- regras de mapa.

## 16.2. Casos inadequados

Não executar Lua:

- para cada mob a cada tick;
- para cada cálculo básico de dano;
- para cada movimento;
- diretamente no banco;
- diretamente em sockets;
- com ponteiro para `World`;
- com acesso a filesystem/rede do sistema.

## 16.3. Modelo orientado a eventos

Eventos:

```text
OnNPCInteract
OnPlayerEnterArea
OnPlayerLeaveArea
OnMobSpawn
OnMobDamaged
OnMobDeath
OnBossPhase
OnTimer
OnEventStart
OnEventEnd
OnQuestCompleted
```

Lua recebe DTO somente leitura e retorna intents:

```lua
return {
  { type = "broadcast", text = "O boss despertou!" },
  { type = "spawn", npc = "Boss", x = 2100, y = 2100 },
  { type = "schedule", event = "phase_2", delay_ms = 30000 }
}
```

O Go valida cada intent.

## 16.4. Sandbox

- remover `os`;
- remover `io`;
- remover loaders externos;
- sem rede;
- sem shell;
- limite de instruções;
- limite de memória;
- timeout;
- limite de intents;
- limite de spawns;
- limite de profundidade;
- erro isolado por script.

## 16.5. Determinismo

- RNG fornecido pelo servidor;
- seed registrada;
- relógio fornecido;
- scripts não usam horário do sistema diretamente;
- resultados importantes entram no ledger.

## 16.6. Carregamento

- compilar no boot;
- validar API;
- cachear;
- versão por script;
- hot reload atômico;
- manter versão antiga até a nova ser validada;
- não recarregar no meio de uma execução.

## 16.7. Scripts lentos

Scripts rápidos podem executar no actor com limite muito baixo.

Scripts mais complexos:

1. receber snapshot;
2. executar fora;
3. retornar intents;
4. actor valida se o contexto ainda é válido;
5. aplicar.

---

# 17. Tratamento de panic e transações em memória

## 17.1. Problema atual

Recuperar panic e continuar pode preservar disponibilidade, mas também pode deixar:

- item removido sem recompensa;
- gold adicionado sem consumo;
- trade parcial;
- referência quebrada;
- mob em estado inválido.

## 17.2. Padrão de handler

```text
decode
validate
calculate
build mutation plan
apply atomically
emit events
```

Nenhuma mutação durante validação.

## 17.3. Mutation plan

```go
type MutationPlan struct {
    Preconditions []Precondition
    Changes       []Change
    Events        []DomainEvent
}
```

Aplicar somente depois que todas as precondições forem satisfeitas.

## 17.4. Após panic

Se o panic ocorrer antes do commit do plano:

- nenhuma mutação.

Se não for possível garantir:

- colocar jogador/agregado em quarentena;
- desconectar sessão;
- recarregar estado persistido;
- bloquear economia daquela conta até reconciliar;
- emitir alerta crítico.

Não continuar silenciosamente com estado desconhecido.

---

# 18. Relógio e aleatoriedade

Criar:

```go
type Clock interface {
    Now() time.Time
    NewTimer(time.Duration) Timer
}

type RNG interface {
    Intn(int) int
    Uint64() uint64
}
```

Benefícios:

- testes determinísticos;
- replay;
- simular atrasos;
- validar cooldown;
- testar boss;
- testar drop;
- reproduzir bugs.

Usar tempo monotônico para deadlines dentro do processo.

---

# 19. Observabilidade

## 19.1. Métricas mínimas

### World

- `world_loop_lag_seconds`;
- `world_command_queue_depth`;
- `world_oldest_command_age_seconds`;
- `world_commands_total{type}`;
- `world_command_duration_seconds{type}`;
- `world_scheduled_action_lag_seconds{kind}`;
- `world_active_players`;
- `world_active_mobs`;
- `world_overload_state`.

### Combate

- `combat_resolution_seconds`;
- `combat_rejections_total{reason}`;
- `skills_cast_total`;
- `boss_decision_lag_seconds`;
- `mob_stuck_total`;
- `mob_path_failures_total`.

### Rede

- `network_connections`;
- `network_packets_in_total{type}`;
- `network_packets_out_total{type}`;
- `network_bytes_in_total`;
- `network_bytes_out_total`;
- `session_out_queue_depth`;
- `rate_limit_events_total`;
- `invalid_checksum_total`;
- `slow_client_disconnects_total`.

### Persistência

- `persistence_critical_queue_depth`;
- `persistence_snapshot_queue_depth`;
- `persistence_commit_seconds{type}`;
- `persistence_failures_total{type}`;
- `persistence_pending_operations`;
- `persistence_conflicts_total`;
- `reward_claims_pending`.

### Runtime

- CPU;
- heap;
- alocações;
- GC;
- goroutines;
- file descriptors;
- CPU steal;
- I/O wait;
- latência do disco;
- conexões PostgreSQL;
- espera no pool.

## 19.2. pprof

Expor somente:

```text
127.0.0.1:<porta>
```

Acessar com túnel SSH.

Coletar:

- CPU;
- heap;
- allocs;
- goroutine;
- block;
- mutex;
- trace.

## 19.3. Logs estruturados

Campos:

- timestamp;
- channel;
- session_id;
- player_id;
- account_id;
- opcode;
- operation_id;
- event_id;
- duration;
- error;
- queue_depth.

## 19.4. Flight recorder

Manter ring buffer pequeno por jogador ou global:

- últimos comandos;
- timestamps;
- decisões de combate;
- mudanças de posição;
- operações econômicas;
- RNG relevante;
- atrasos.

Útil para investigar denúncias de:

- hit que falhou;
- item perdido;
- boss travado;
- trade duplicado;
- jogador morto durante lag.

Não armazenar senhas ou conteúdo sensível.

## 19.5. Auditoria de itens raros

Registrar:

- item instance;
- origem;
- dono anterior;
- dono novo;
- operação;
- timestamp;
- canal;
- script/quest/evento;
- resultado.

Isso protege a reputação do servidor e ajuda suporte.

---

# 20. Testes de carga e falha

## 20.1. Criar `cmd/loadbot`

Bot sem interface que execute o protocolo real.

Capacidades:

- login;
- seleção de personagem;
- movimento;
- ataque;
- skill;
- chat;
- pickup;
- trade;
- craft;
- reconexão;
- participação em evento.

## 20.2. Cenários

### Distribuído

- 999 conectados;
- jogadores em mapas/células diferentes;
- baseline de memória e CPU.

### Cidade cheia

- 300–999 próximos;
- movimento;
- lojas;
- chat;
- teleporte.

### PvP

- 100–300 jogadores;
- skills em área;
- cura;
- buffs;
- morte/ressurreição;
- latência simulada.

### Boss

- 100–300 jogadores;
- summons;
- DoT;
- múltiplas fases;
- drops;
- recompensa;
- pathfinding.

### Economia

- crafts simultâneos;
- trades simultâneos;
- pickup;
- compras;
- refinos;
- desconexão durante operação.

### Reconnect storm

- centenas desconectam e retornam;
- carregamento do banco;
- limite de auth;
- preservação de estado.

## 20.3. Falhas injetadas

- `kill -9` do servidor;
- reinício do PostgreSQL;
- latência artificial no banco;
- fsync lento;
- disco cheio;
- conexão perdida durante commit;
- resposta do banco perdida após commit;
- panic em cada etapa de craft/trade;
- fila de saída cheia;
- cliente que não lê;
- flood de movimento;
- flood de pacotes inválidos.

## 20.4. Rede simulada

No Ubuntu, usar `tc netem` no ambiente de teste para:

- 50–200 ms de latência;
- jitter;
- perda;
- reorder;
- conexão instável.

Validar que latência do jogador não causa:

- duplicação;
- cooldown inconsistente;
- ataque fora de ordem;
- movimento impossível;
- recompensa perdida.

## 20.5. Testes determinísticos

Com fake clock e RNG:

- cooldown;
- cast;
- regen;
- DoT;
- boss;
- respawn;
- drop;
- timers Lua;
- expiração de trade.

## 20.6. Fuzzing

Fuzz tests para:

- framing;
- packet decoder;
- offsets;
- tamanho;
- checksum;
- strings;
- índices;
- item slots;
- comandos Lua;
- dados de configuração.

---

# 21. Escala por canais e mapas

## 21.1. Limite de IDs

O código atual reserva IDs de jogadores abaixo de 1000 e mobs a partir de 1000.

O alocador deve retornar erro quando o canal estiver cheio:

```go
func (w *World) allocPlayerID() (uint16, bool) {
    for id := uint16(1); id < 1000; id++ {
        if _, used := w.playersByID[id]; !used {
            return id, true
        }
    }
    return 0, false
}
```

Nunca retornar 999 ocupado.

## 21.2. Vários canais

Arquitetura natural:

```text
Login/Channel Directory
  ├── Channel 1 — World actor
  ├── Channel 2 — World actor
  ├── Channel 3 — World actor
  └── Event channel — World actor
```

Cada canal:

- processo próprio, preferencialmente;
- IDs próprios;
- estado ativo próprio;
- PostgreSQL compartilhado;
- mensagens intercanal por outbox/event bus.

## 21.3. Por que processo próprio

- isolamento de crash;
- uso de múltiplos núcleos;
- deploy gradual;
- limites claros;
- pprof por canal;
- overload de um canal não trava todos.

## 21.4. Instâncias

Criar Worlds separados para:

- dungeons;
- arenas;
- eventos;
- guerras;
- bosses especiais.

Transferência:

1. persistir/selar estado;
2. remover do World origem;
3. emitir token de transferência;
4. inserir no destino;
5. confirmar;
6. recuperar em caso de falha.

---

# 22. Ubuntu Server e VPS

## 22.1. Recursos iniciais

Para teste sério:

- 4 vCPUs dedicadas com bom desempenho por núcleo;
- 8 GB de RAM sem banco pesado;
- 16 GB se PostgreSQL estiver na mesma VPS;
- NVMe;
- Ubuntu Server LTS;
- baixa porcentagem de CPU steal.

Mais vCPUs compartilhadas não compensam necessariamente um núcleo lento.

## 22.2. Go runtime

- não definir `GOMAXPROCS=1`;
- rede, banco e writers precisam usar outros núcleos;
- configurar `GOMEMLIMIT` abaixo da RAM;
- monitorar GC;
- usar PGO somente após perfil representativo;
- compilar release sem instrumentações excessivas.

## 22.3. systemd

Exemplo de base:

```ini
[Unit]
Description=WYD 7.48 Server
After=network-online.target postgresql.service
Wants=network-online.target

[Service]
Type=simple
User=wyd
Group=wyd
WorkingDirectory=/opt/wyd748
ExecStart=/opt/wyd748/bin/server
Restart=on-failure
RestartSec=3
LimitNOFILE=65535
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ReadWritePaths=/var/lib/wyd748 /var/log/wyd748
Environment=GOMEMLIMIT=6GiB

[Install]
WantedBy=multi-user.target
```

Adaptar diretórios e permissões.

## 22.4. Graceful shutdown

Ao receber SIGTERM:

1. parar novos logins;
2. informar manutenção, quando possível;
3. parar criação de operações econômicas;
4. drenar commits críticos;
5. snapshot final;
6. fechar writers;
7. fechar pool;
8. terminar.

Usar `context.Context`.

## 22.5. File descriptors

- `LimitNOFILE`;
- verificar `ulimit -n`;
- monitorar FDs;
- fechar conexões e arquivos corretamente.

## 22.6. Firewall

Expor somente:

- porta do jogo;
- API estritamente necessária;
- SSH restrito.

Não expor:

- PostgreSQL publicamente;
- pprof;
- Prometheus sem proteção;
- interfaces administrativas.

## 22.7. Backups

- backup automatizado;
- retenção;
- cópia fora da VPS;
- teste de restore;
- backup de scripts/configurações;
- migrations versionadas;
- nunca considerar backup válido sem teste de recuperação.

## 22.8. Evitar tuning cego

Não aplicar sysctls copiados sem benchmark.

Primeiro medir:

- conexões;
- backlog;
- retransmissões;
- latência;
- CPU steal;
- I/O wait;
- FDs;
- memória;
- disco.

---

# 23. Mudanças por área do código atual

## `internal/game/world.go`

- adicionar `ReceivedAt` e `Sequence` aos comandos;
- trocar ticker monolítico por scheduler;
- orçamento de comandos;
- tratar sobrecarga;
- extrair sistemas;
- remover `net.Session`;
- remover persistência direta;
- corrigir alocação de ID;
- injetar `Clock` e `RNG`;
- substituir varreduras por deadlines/buckets;
- remover autosave global;
- não continuar após panic com estado desconhecido.

## `internal/net/session.go`

- validar checksum;
- deadlines;
- rate limit;
- timestamp de ingresso;
- sequence;
- decoder;
- fila de saída limitada;
- mover finalização/criptografia para writer;
- ownership seguro de buffers;
- métricas;
- slow-client policy;
- keepalive;
- `TCP_NODELAY`.

## `internal/store`

- separar interface de snapshot e transação crítica;
- criar `PersistenceService`;
- criar requests/resultados;
- idempotência;
- versionamento;
- recovery;
- PostgreSQL;
- migrations;
- ledger;
- reward inbox;
- outbox;
- `Close()` com drain.

## `internal/game/visibility.go`

- índices espaciais para itens/lojas/eventos;
- AOI incremental;
- dirty cells;
- consolidação;
- métricas de fan-out;
- políticas para áreas densas.

## `internal/model`

- mover wire DTOs;
- mover persistence records;
- manter domínio livre de JSON/protocolo;
- IDs tipados;
- invariantes.

## Novos pacotes

- `application/world`;
- `application/ports`;
- `domain/*`;
- `adapters/protocol/wyd748`;
- `adapters/transport/tcp`;
- `adapters/persistence/postgres`;
- `adapters/scripting/lua`;
- `cmd/loadbot`.

---

# 24. Roteiro de implementação

## Fase 0 — baseline e segurança imediata

### Implementar

- métricas do loop;
- pprof local;
- tempo de fila dos comandos;
- profundidade de filas;
- duração dos handlers;
- duração de saves;
- validar checksum;
- read/write deadline;
- slow-client policy;
- corrigir ID 999;
- `go test -race ./...`;
- fuzzing básico;
- shutdown controlado.

### Aceitação

- dashboards mostram p50/p95/p99;
- nenhum pacote inválido chega ao gameplay;
- canal cheio é recusado corretamente;
- conexão lenta não bloqueia o World;
- shutdown não perde commit pendente.

## Fase 1 — cliente de carga

### Implementar

- `cmd/loadbot`;
- cenários distribuído, cidade, PvP, boss e economia;
- métricas automáticas;
- relatório por execução.

### Aceitação

- teste reproduzível;
- baseline com 100, 300, 500 e 999 conexões;
- gargalos identificados por perfil, não por suposição.

## Fase 2 — scheduler e fairness

### Implementar

- timestamp de ingresso;
- sequência;
- ativação de mobs orientada pelo movimento do jogador;
- `activeMobs` contendo somente mobs acordados;
- buckets/deadlines simples para mobs ativos;
- scheduler por deadline para combate, casts e timers críticos;
- cooldown absoluto;
- cast agendado;
- IA por estado;
- orçamento do loop;
- coalescência de movimento;
- overload states.

### Aceitação

- hit/skill não espera tick de 500 ms;
- mobs adormecidos não executam IA;
- jogador entrando no raio acorda o mob corretamente;
- grupos e líderes não são necessários para ativação;
- boss não congela sob fluxo alto;
- boss não executa rajada após atraso;
- regen/DoT têm semântica documentada;
- p99 de comando dentro da meta.

## Fase 3 — persistência pendente

### Implementar

- operation state;
- persistence workers;
- filas crítica/snapshot;
- reservas em memória;
- idempotência;
- ledger;
- crash tests;
- reward claims.

### Aceitação

- craft, trade, pickup e reward passam em kill tests;
- item confirmado sempre reaparece;
- operação repetida não duplica;
- banco lento não congela combate;
- banco fora deixa economia em modo seguro.

## Fase 4 — PostgreSQL

### Implementar

- migrations;
- schema;
- item instances;
- versionamento;
- transações;
- pool;
- recovery;
- outbox;
- backup/restore.

### Aceitação

- migração de JSON validada;
- reconciliação de versões;
- restore testado;
- p99 de commits dentro da meta;
- snapshots não sobrescrevem ledger.

## Fase 5 — protocolo e arquitetura limpa

### Implementar

- Commands tipados;
- ClientGateway;
- SessionID;
- domínio separado;
- adapters;
- sistemas extraídos do World.

### Aceitação

- testes de combate/economia sem socket;
- protocolo pode ser fuzzado isoladamente;
- store pode ser trocado;
- `World` atua como coordenador;
- nenhum pacote bruto dentro do domínio.

## Fase 6 — AOI e áreas densas

### Implementar

- índices para ground items/shops;
- dirty cells;
- AOI incremental;
- fan-out consolidado;
- perfis de evento.

### Aceitação

- 300 jogadores densos sem crescimento descontrolado;
- movimento visual consolidado;
- combate não descartado;
- filas de saída estáveis.

## Fase 7 — Lua

### Implementar

- runtime sandbox;
- API de intents;
- scripts compilados;
- budgets;
- versionamento;
- testes;
- métricas.

### Aceitação

- script não acessa sistema operacional;
- script lento é interrompido;
- falha de script não trava World;
- item/reward de Lua passa pelo sistema transacional;
- nenhum script faz query direta de gameplay.

## Fase 8 — múltiplos canais/instâncias

### Implementar

- channel directory;
- processos;
- transferência;
- outbox intercanal;
- eventos instanciados.

### Aceitação

- falha de um canal não derruba os outros;
- conta não entra duplicada;
- transferência é recuperável;
- escala usa múltiplos núcleos.

---

# 25. Prioridades

## P0 — antes de servidor público

- durabilidade de item/gold confirmado;
- operação crítica sem bloquear World;
- timestamp de ingresso;
- nenhuma perda silenciosa de hit/skill;
- rate limit;
- checksum;
- deadlines;
- slow-client handling;
- correção do limite de IDs;
- métricas;
- loadbot;
- crash tests;
- panic sem mutação parcial;
- backups restauráveis.

## P1 — antes de eventos grandes

- ativação de mobs pelo jogador;
- `activeMobs` restrito aos mobs acordados;
- scheduler simples/deadlines;
- IA por estado;
- watchdog de boss;
- AOI de itens/lojas;
- overload state;
- dirty autosave;
- reward inbox;
- PostgreSQL;
- flight recorder;
- auditoria de itens raros.

## P2 — evolução

- Lua;
- separação completa de pacotes;
- múltiplos canais;
- instâncias;
- pathfinding worker;
- PGO;
- otimizações avançadas de buffer.

---

# 26. O que não fazer

- não trocar todo o servidor para tick de 50 ms;
- não adicionar goroutine por mob;
- não criar `MobGroupController` ou líder scanner apenas para acordar mobs;
- não adicionar área especial de ativação por grupo sem necessidade comprovada;
- não adicionar mutex em todas as entidades;
- não consultar PostgreSQL por movimento/ataque;
- não confirmar item antes de commit;
- não confiar somente no autosave de três segundos;
- não executar Lua em todo mob a cada tick;
- não permitir Lua direto no banco;
- não deixar fila crescer ilimitadamente;
- não descartar combate silenciosamente;
- não reduzir IA de mobs engajados para sobreviver à carga;
- não continuar após panic com agregado possivelmente corrompido;
- não otimizar buffers antes de medir;
- não usar número de jogadores conectados como único benchmark;
- não operar sem testes de crash;
- não manter PostgreSQL exposto à internet.

---

# 27. Critérios finais de prontidão

O servidor só deve ser considerado pronto para uma operação pública mais séria quando:

- [ ] 999 conexões são suportadas funcionalmente por canal;
- [ ] 300 jogadores densos foram testados;
- [ ] boss com 100–300 jogadores foi testado;
- [ ] mobs adormecidos não aparecem no ciclo de IA;
- [ ] jogador acorda corretamente os mobs ao entrar no raio;
- [ ] grupos e líderes do `NPCGenerator` não são necessários para ativação;
- [ ] p99 de resolução de combate está dentro da meta;
- [ ] nenhum hit/skill crítico é descartado silenciosamente;
- [ ] AI overdue possui métricas e alertas;
- [ ] boss stuck possui watchdog;
- [ ] crafting passa em crash tests;
- [ ] trade passa em crash tests;
- [ ] pickup passa em crash tests;
- [ ] recompensa rara passa em crash tests;
- [ ] item confirmado sobrevive a `kill -9`;
- [ ] operação repetida não duplica;
- [ ] banco lento não congela o World;
- [ ] banco indisponível ativa modo econômico seguro;
- [ ] autosave é distribuído e baseado em dirty flags;
- [ ] filas possuem limites e métricas;
- [ ] slow clients são isolados;
- [ ] pprof não está público;
- [ ] backups foram restaurados em ambiente limpo;
- [ ] scripts Lua possuem sandbox e budget;
- [ ] logs permitem investigar disputa de item/PvP;
- [ ] deploy e shutdown via systemd foram testados;
- [ ] alertas operacionais estão configurados.

---

# 28. Resumo executivo

A arquitetura atual possui uma base forte: actor único, estado autoritativo, grid espacial e preocupação anti-dupe. A evolução não deve abandonar esses princípios.

As mudanças mais importantes são:

1. processar ações de jogador imediatamente, sem depender do tick global;
2. deixar mobs adormecidos sem IA e acordá-los diretamente pelo jogador dentro do raio;
3. usar `activeMobs`, buckets e deadlines simples antes de adotar schedulers mais complexos;
4. registrar o instante de ingresso para preservar justiça;
5. nunca descartar silenciosamente combate ou economia;
6. persistir operações críticas antes da confirmação, mas fora do actor;
7. usar estados pendentes e reservas durante commits;
8. criar ledger, idempotência e reward inbox;
9. migrar para PostgreSQL sem colocá-lo no caminho quente;
10. manter IA engajada responsiva e reduzir apenas trabalho secundário;
11. tratar áreas densas e fan-out como principal cenário de escala;
12. integrar Lua por eventos e intents, com sandbox;
13. escalar por canais/Worlds, não por mutação concorrente do mesmo estado;
14. medir p99, filas e atraso do scheduler;
15. provar a confiabilidade por testes de carga, crash e rede degradada.

A meta não é apenas “suportar 1.000 sockets”. A meta é preservar **economia, combate e mecânicas de boss** quando o servidor está sob a pior carga realista. A reputação de um MMORPG depende mais dessas garantias do que do número nominal de conexões.
