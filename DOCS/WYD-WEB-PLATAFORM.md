# WYD Web Platform

## Estado e decisões desta revisão — 20/09/2026

Plano arquitetural em implementação. Base inspecionada:
`912a9512942af9e9c96400f361f3bbd4eb929e24` em `main`, acrescida das correções
locais de Cargo/gold, mapa/minimapa e remoção server-side da recompra descritas
nos registros de implementação. As telas, endpoints, tabelas e tipos apresentados
abaixo continuam propostas quando não estiverem acompanhados de evidência de
implementação. A lista completa é o backlog do produto; a ordem de execução e
os gates estão nas seções 161–162.

### Diretório administrativo de contas — 20/09/2026

**MODERNIZACAO_COMPATIVEL v1.** O painel integrado oferece `GET
/api/v1/staff/accounts`, protegido por sessão staff e pela capability
`moderation.player.search`. A tela busca por prefixo de conta ou personagem,
pagina até 50 contas por cursor e exibe criação, última atualização e resumo dos
quatro slots de personagem (UID, slot, nome, classe, nível e evolução).

- A consulta PostgreSQL usa uma projeção estreita: não carrega nem devolve hash
  de senha, payload bruto da conta, inventário ou cargo.
- A presença online é resolvida em uma única leitura enfileirada no `World` para
  todas as contas da página. Se essa fonte não estiver disponível, a API marca
  `presenceAvailable=false`; não inventa estado offline.
- Busca e cursor aceitam apenas chaves ASCII alfanuméricas de até 12 caracteres;
  limite máximo de 50, body ausente, query estrita e limite de 30 consultas por
  minuto por ator.
- O frontend usa paginação keyset, DOM criado com `textContent` e informa
  separadamente o timestamp persistente e a disponibilidade da presença live.

Testes focados de store, projeção, handler e leitura autoritativa do World,
geração OpenAPI, Astro check e build passaram. `STATICALLY VERIFIED` e
`AUTOMATED TESTED`; **não CLIENT_TESTED**. A validação visual no navegador com
PostgreSQL e servidor reais permanece pendente.

### Incremento atual: drop global, quiz, bosses, kick e teleporte para cidades

O painel embarcado no `tm.exe` agora possui comandos de eventos e summon de
bosses, desconexão administrativa e teleporte para cidades, além das consultas existentes. Este incremento altera a prioridade
read-only anterior; teleporte por coordenadas livres, ban e disparo administrativo de guerras
ainda estão pendentes. O restante do documento descreve o produto-alvo, não uma
lista de funções já disponíveis.

- **Drop global — MODERNIZACAO_COMPATIVEL:** ID de item, rate inteira de 1 a
  100%, limite de entregas e/ou duração. Se ambos forem definidos, encerra no
  primeiro limite. Drop adicional nas mortes elegíveis de monstros, independente
  do loot normal; não inclui jogadores, NPCs nem torres. A cota só é consumida
  na entrega efetiva, com reserva para evitar ultrapassagem em mortes em área
  e liberação da reserva em rollback/falha.
- **Quiz — EXTENSAO_COORDENADA v1:** multiplicação de dois algarismos por 2–9
  ou divisão exata de dois algarismos por 2–9, quatro alternativas embaralhadas,
  primeira pergunta imediatamente e depois uma por minuto. Cada rodada dura
  10 segundos; o client fecha por timer e o servidor rejeita respostas vencidas.
  Duração de 1–1440 minutos; recompensa configurável por ID e 1–60 unidades por
  acerto. O item deve existir no catálogo carregado.
- **Boss summon — MODERNIZACAO_COMPATIVEL v1:** consulta todos os bosses
  configurados no lifecycle existente do servidor e permite invocar manualmente
  somente um boss morto. O comando carrega `epoch`, `bossId`, `expectedRevision`,
  `operationId` e motivo. `epoch` invalida requisições de outra execução do
  servidor; a revisão por boss impede duplicação se spawn, morte ou respawn
  avançarem o lifecycle; `operationId`, vinculado ao ator e ao mesmo body, permite
  repetir uma resposta HTTP incerta sem executar outro summon. O World revalida
  a capability imediatamente antes da mutação e chama o mesmo `spawnBoss` usado
  pelo fluxo normal, sem criar um caminho paralelo de gameplay.
- Participam personagens presentes no mundo ao emitir a pergunta. Entrada ou
  relogin durante a rodada espera a seguinte. Encerrar pelo painel fecha a
  pergunta ativa. Reiniciar o servidor encerra ambos os eventos, sem retomada.
- Cada unidade do quiz é materializada com UID próprio e ocupa um slot livre.
  Sem espaço para a recompensa inteira, não entrega nenhuma unidade; registra
  falha e informa o jogador. Não há drop no chão nem resgate posterior neste
  incremento. Falha de persistência restaura a cópia local e bloqueia/desconecta
  a conta para impedir continuação após resultado de commit incerto.

**Uso:** entrar no `/admin/` do servidor embarcado, abrir Eventos ou Quiz de
matemática, consultar estado, preencher configuração e motivo, e confirmar
Iniciar. Consultar evento atualiza contadores e horários; Encerrar interrompe.
Em timeout HTTP, repetir com o botão próprio conserva o ID da operação e não
inicia outro evento. Não recarregar a página enquanto o resultado for incerto.
`felipetr` recebeu as capabilities `game.event.global-drop`, `game.event.quiz` e
`game.boss.summon` em `data/staff.json`; alterações de permissões invalidam
sessões antigas e exigem novo login.

**Fronteira administrativa:** GET/POST `/api/v1/staff/events/global-drop`,
`/api/v1/staff/events/quiz` e `/api/v1/staff/bosses`, sessão staff, CSRF,
limites, body estrito e revalidação da sessão/permissão no momento de executar
na fila do World. O snapshot de bosses expõe `id`, nome, revisão, estado vivo,
Mob ID, HP/max HP, posição atual, posição de spawn e `respawnAt`.
Control API HTTP permanece read-only; o web-api separado não tem a fonte de
comandos e retorna indisponibilidade para eventos. Recibos administrativos são
locais ao processo, limitados a 1024 (mais o encerramento final), vinculados ao
ator, body, epoch e evento. Não constituem o ledger durável geral proposto no
plano; um restart muda epoch e invalida requisições antigas. Recompensas são
persistidas antes de publicar inventário.

**Proteção contra WPE:** novos opcodes `0x7F10` (148 bytes) e `0x7F11` (36 bytes),
versionados, com token criptográfico de 128 bits por participante/rodada. O
servidor exige sessão, conta, personagem, token e prazo ativos. A primeira
resposta consome a tentativa mesmo errada; repetir pacote, trocar alternativa,
usar token de outra sessão ou enviar resposta antiga não concede recompensa.
O caminho legado `0x2C7` não concede prêmio. Forjar abertura local de uma janela
não cria participação no servidor. Bots capazes de resolver perguntas legítimas
e múltiplas contas não são eliminados por esse mecanismo.

**Validação:** suíte Go completa e vet passaram; testes focados de eventos com race e
testes focados de bosses passaram; OpenAPI gerado, Astro check e build passaram.
O cliente C++ foi
compilado com `Build-Client.ps1 -NoDeploy`, incluindo 35.621 verificações de
arquitetura/contrato. `STATICALLY VERIFIED` e `AUTOMATED TESTED`; **não é
CLIENT_TESTED**. Pendentes: janela real, quatro cliques, timeout, logout/relogin
e recompensa com PostgreSQL real no fluxo integrado. O executável instalado e
o servidor em execução não foram substituídos/reiniciados. Candidatos em
`wydgo748/bin/tm-webadmin.exe` e `tmproject/build/TMProject748/Release/WYD.exe`;
atualizar servidor e cliente juntos numa parada planejada. O frontend está
compilado no `web/portal/dist` usado pelo painel embarcado.

### Moderação: desconectar jogador — 14/09/2026

**MODERNIZACAO_COMPATIVEL v1.** Implementado `POST
/api/v1/staff/players/{uid}/kick` no painel integrado, com capability
`moderation.player.kick`. O body inclui versão, `operationId`, motivo (3–200
caracteres sem controles), UID, conta canônica, `expectedSessionId` e `epoch`
obtido de `moderationEpoch` no overview. UID da rota e do body devem coincidir.
Não altera o wire do client e não aplica banimento: o jogador pode reconectar.

- O `World` revalida autorização, contexto, processo, personagem, conta e sessão
  antes de fechar o socket e executar `onDisconnect`. O fluxo normal salva
  conta/charstate e limpa presença, grupo, trocas, summons e reserva da conta.
- Sessão antiga/offline ou epoch antigo retorna `stale_target`, sem escolher
  automaticamente a nova conexão. Recibos são vinculados a ator/body e não são
  descartados durante o processo (máximo 1024); replay recupera o resultado,
  conflito rejeita, e saturação não executa. Não há ledger durável neste lote.
- Falha no save não impede a desconexão física: retorna explicitamente
  `disconnected_persistence_failed` e registra o erro. Não promete rollback da
  conexão nem salvamento bem-sucedido. Recibo e motivo aparecem na auditoria.
- UI: em Jogadores online, clicar **Desconectar**, preencher motivo e confirmar.
  Em falha HTTP, **Consultar / repetir a mesma operação** mantém alvo, body e ID;
  não permite trocar o alvo pendente. Não recarregar a página com resultado
  incerto. Atualizar a lista após a confirmação. Resultados tardios após logout
  não alteram a UI de uma nova sessão administrativa.
- `felipetr` recebeu a nova capability em `data/staff.json`; fazer novo login
  administrativo após a mudança. Web API separada retorna
  `moderation_unavailable`; a Control API HTTP continua somente leitura.

**Validação deste lote:** `go test -count=1 ./...`, `go vet ./...`, testes
focados de kick/overview/bosses com `-race`, geração OpenAPI, Astro check/build e
sete testes de estado DOM em
[`Test-PlayerModeration.mjs`](../tools/web-admin/Test-PlayerModeration.mjs).
Inclui rejeições, fila cheia, cancelamento, revogação/expiração, replay após
relogin, callback duplo de desconexão e falha simulada de persistência.
`STATICALLY VERIFIED` / `AUTOMATED TESTED`; **não CLIENT_TESTED**.
Faltam operação pelo navegador com jogador real e save em PostgreSQL real.
Build candidato: `wydgo748/bin/tm-webadmin.exe`. Nenhum servidor em execução foi
encerrado; `tm.exe` e `project.exe` não foram substituídos neste lote.

### Moderação: teleporte para cidades — 14/09/2026

**MODERNIZACAO_COMPATIVEL v1.** O painel integrado implementa `POST
/api/v1/staff/players/{uid}/teleport`, com capability
`moderation.player.teleport` (incluída para `felipetr`; exige novo login).
Selecionar **Teleportar** na lista online, escolher a cidade, preencher motivo
e confirmar. O servidor resolve os destinos a partir dos pontos públicos já
usados pelas saídas de guerras: Armia (2086, 2093), Azran (2494, 1707),
Erion (2453, 2000) e Nippleheim (3652, 3122).

- Body v1 com UID, conta canônica, sessão observada, `moderationEpoch`,
  `operationId`, motivo e `destination` (`armia`, `azran`, `erion` ou
  `nippleheim`). **Não aceita coordenadas livres nem acesso a instâncias.**
- Revalida sessão staff/CSRF/capability e alvo no World; rejeita morto, troca,
  loja fantasma, persistência bloqueada, membership de instância (incluindo
  saída em andamento), arenas de torre/cidades ou participante de guerra.
- Reutiliza `teleportPlayer`: terreno transitável, busca de célula livre
  em raio 3, limite da cidade e recusa se todas estiverem ocupadas. Salva antes
  de publicar; falha restaura a posição e retorna `persistence_failed`.
  Sucesso limpa a rota anterior de caminhada e contextos de NPC; não altera
  ouro, itens ou o protocolo do client.
- Recibo retorna coordenadas finais em `x/y` somente no sucesso; a UI mostra
  essas coordenadas. Recibos por ator/body, até 1024 operações de teleporte por
  processo, sem descarte. Retry idêntico recupera o resultado original, sem
  teleportar novamente mesmo que o jogador já tenha caminhado ou reconectado.
  Após reinício, o epoch antigo é recusado.
- Em resultado HTTP incerto, **Consultar / repetir a mesma operação** conserva
  alvo, cidade e ID. Não recarregar a página. Respostas tardias após logout são
  descartadas. A API separada continua incapaz de executar a mutação.

**Validação:** testes Go de contrato, World e HTTP, incluindo rollback simulado,
destino cheio, cancelamento/fila, revogação, expiração, replay e caminhada
anterior; testes de estado DOM em
[`Test-PlayerTeleport.mjs`](../tools/web-admin/Test-PlayerTeleport.mjs).
Gates de integração: suíte Go, vet, race focado, geração OpenAPI e Astro
check/build. `STATICALLY VERIFIED` / `AUTOMATED TESTED`; **não CLIENT_TESTED**.
Falta validar pelo navegador com jogador e PostgreSQL reais.
O candidato fica em `wydgo748/bin/tm-webadmin.exe`; nenhum servidor é
reiniciado e `tm.exe`/`project.exe` não são substituídos por este lote.

Próximo lote: banimento administrativo com persistência e revogação no login.
Coordenadas livres e disparo de guerras seguem pendentes, com contratos próprios.

### Base existente e lacunas

| Área | Evidência no repositório | Consequência para o plano |
| --- | --- | --- |
| Autoridade de gameplay | [`World`](../wydgo748/internal/game/world.go) processa comandos e ticks em uma goroutine; snapshots administrativos e os comandos de drop global, quiz e summon de boss entram pela fila autoritativa | Preservar o `World` como autoridade; cada nova mutação administrativa ainda exige comando tipado, rejeições, lifecycle e auditoria próprios |
| Cadastro e credenciais | [`account`](../wydgo748/internal/account/service.go), [`validação`](../wydgo748/internal/account/validation.go) e [`hash`](../wydgo748/internal/account/password.go) | Reutilizar criação, autenticação e PBKDF2; sessão web, recuperação e alteração de senha são trabalho novo |
| HTTP de contas | [`accountapi`](../wydgo748/internal/accountapi/handler.go) oferece cadastro e health/readiness | Preservar `POST /v1/accounts` durante a migração; não assumir login HTTP existente |
| Persistência | [`PostgresStore`](../wydgo748/internal/store/postgres.go) e [`schema`](../wydgo748/internal/store/postgres_schema.sql) mantêm conta em JSONB, identidades de personagens e instâncias de item; o diretório administrativo usa projeção SQL estreita | Separar acesso web de snapshots de gameplay; novas leituras devem preservar projeções mínimas e não expor payload ou credenciais |
| Guerras | [`guild_wars.go`](../wydgo748/internal/game/guild_wars.go) e [estado documentado](guild-wars.md) | Torre e cidades têm implementação e testes registrados, com gate no client pendente; RvR/reinos e castelo têm escopo separado |
| Portal | [`cmd/web-api`](../wydgo748/cmd/web-api/main.go), [`internal/webadmin`](../wydgo748/internal/webadmin), [`api/openapi.yaml`](../wydgo748/api/openapi.yaml) e [`web/portal`](../wydgo748/web/portal) implementam a fundação administrativa, eventos e summon de bosses | O painel administrativo básico e as primeiras mutações autoritativas existem localmente; site público, painel do jogador, exportador e demais comandos administrativos seguem por fases |

O lote administrativo possui testes automatizados de contrato, auth,
Control API e servidor estático, além de build do frontend. A extensão de quiz
altera o client de jogo; os gates e limitações atuais estão registrados acima.

### Decisões para execução

1. Preservar as duas sources: Go e frontend do portal ficam sob `wydgo748/`,
   conforme a seção 135. Documentação permanece em `DOCS/`.
2. A prioridade atual é tornar o painel administrativo operacional. Drop global,
   quiz, summon de bosses, kick, teleporte para cidades e diretório de contas já
   possuem contratos implementados. Banimento e disparo de guerras são os
   próximos lotes. Site público e painel do jogador permanecem posteriores.
3. Astro permanece estático inicialmente. Notícias/guias publicados em Git
   exigem build; CMS sem rebuild fica para uma fase com renderização definida.
4. Navegador usa `/api/*` no mesmo origin; a Function usa um destino HTTPS fixo.
   Conectividade até o Go, credenciais e isolamento de previews são gate de
   implantação, não consequências automáticas de usar `pages.dev`.
5. O processo web escreve apenas domínio web e usa uma operação restrita para
   cadastro. Escritas em contas existentes, guildas e economia dependem de
   contratos autoritativos que protejam login, autosave e estado online.
6. Request ID serve para rastreio. Idempotência precisa de registro durável,
   unicidade, resultado recuperável e commit junto do efeito econômico.
7. Projeções públicas têm versão e `asOf`; dados persistidos não são estado
   live. Processos separados ainda compartilham CPU, disco e PostgreSQL.

## 1. Objetivo

Criar uma plataforma web única para o ecossistema WYD-Go, dividida em quatro superfícies:

```text
Portal público
Área do jogador
Área da guild
/admin — staff e gerenciamento do servidor
```

O portal não será apenas institucional. Ele deverá funcionar como:

* site oficial;
* database/wiki oficial;
* central de download;
* rankings;
* calendário de eventos;
* consulta de personagens e guilds;
* painel da conta;
* painel da guild;
* reward inbox;
* marketplace;
* suporte;
* sistema de doações;
* sistema de moderação;
* control plane administrativo do servidor.

Todas as superfícies compartilham a mesma identidade e o mesmo backend web.

---

# 2. Princípios arquiteturais

A plataforma deve respeitar integralmente os contratos existentes do WYD-Go:

* `World` continua sendo o único escritor do estado de gameplay;
* comandos administrativos que alteram jogador, boss, evento, guerra, item ou economia entram no `World`;
* PostgreSQL continua sendo a persistência autoritativa;
* conteúdo de jogo continua vindo de `data/`;
* `ExtendedScore` continua autoritativo;
* item materializado continua possuindo UID server-side;
* cliente e navegador são não confiáveis;
* operações econômicas continuam seguindo `validar → mutar cópia → persistir → publicar`.

O projeto atual já trabalha com uma única goroutine de `World` alterando gameplay e sessões apenas entregando comandos.

PostgreSQL já mantém contas, personagens, itens materializados, estados de
personagem e guildas. Os caminhos transacionais usam `withSerializableTx`;
isso não transforma duas chamadas independentes em uma transação, nem protege
contra salvar um snapshot antigo de outro processo. UID global também não
substitui o contrato de posse, escrow e entrega exatamente uma vez.

---

# 3. Arquitetura geral

```text
                        INTERNET
                            │
                            ▼
                ┌──────────────────────┐
                │   Cloudflare Pages   │
                │                      │
                │    Astro estático    │
                │                      │
                │ *.pages.dev          │
                └──────────┬───────────┘
                           │
                        /api/*
                           │
                           ▼
                ┌──────────────────────┐
                │       wyd-web        │
                │          Go          │
                │                      │
                │ Public API           │
                │ Account API          │
                │ Guild API            │
                │ Staff API            │
                │ Auth / RBAC          │
                │ Support              │
                │ Payments             │
                │ Marketplace          │
                └──────┬────────┬──────┘
                       │        │
                       │        ▼
                       │   PostgreSQL
                       │
                       ▼
              PRIVATE CONTROL API
                       │
                       ▼
        ┌────────────────────────────────┐
        │          wyd-server            │
        │                                │
        │ Game listener                  │
        │ Scheduler                      │
        │ Control API                    │
        │ Metrics                        │
        │                                │
        │          World                 │
        └────────────────────────────────┘
```

---

# 4. Três componentes executáveis

## 4.1 `wyd-server`

Continua sendo o servidor de jogo.

Passa também a possuir a **Control API interna**.

Responsabilidades:

```text
Game protocol
World
Sessions
Gameplay
Events
Bosses
Instances
Wars
Scheduler
Control API
Metrics
Graceful shutdown
```

---

## 4.2 `wyd-web`

Processo Go independente.

Responsabilidades:

```text
Login web
Cadastro
Conta
Guild
Rankings
News
Support
Payments
Marketplace
Staff authorization
Public API
Admin API
```

O objetivo é reiniciar ou perder o processo web sem encerrar o jogo. Essa
isolação precisa ser demonstrada com limites de CPU/memória, pools PostgreSQL
independentes, limites de consulta e carga concorrente; o banco continua sendo
uma dependência compartilhada.

---

## 4.3 Astro

Somente frontend.

```text
Astro
↓
npm run build
↓
HTML + CSS + JS
↓
Cloudflare Pages
```

Não existe Node rodando junto ao servidor.

Cloudflare suporta diretamente build e publicação de projetos Astro em `*.pages.dev`; o nome do projeto determina esse subdomínio.

---

# 5. Proxy `/api` no Cloudflare

Como não teremos domínio próprio inicialmente, uma solução interessante é manter o navegador no mesmo origin:

```text
https://wyd-go.pages.dev/
https://wyd-go.pages.dev/api/...
```

Uma Pages Function mínima pode funcionar apenas como proxy:

```text
Browser
   ↓
wyd-go.pages.dev/api/*
   ↓
Cloudflare Function
   ↓
wyd-web Go
```

A Function **não contém regra de negócio**.

Ela apenas:

* encaminha request;
* adiciona segredo interno para autenticar o proxy;
* devolve response.

O proxy terá upstream HTTPS configurado no servidor, sem URL escolhida pelo
browser; removerá headers internos fornecidos pelo cliente antes de adicionar
sua própria autenticação. Preservará corretamente `Set-Cookie`, status e
headers de cache, com limites de corpo, duração e métodos. Somente a cadeia
de proxies explicitamente confiável pode informar o IP usado pelo rate limit.

Uma Function na Cloudflare não alcança o `127.0.0.1` do host Go nem o socket
privado de controle. Antes da implantação, definir o endpoint HTTPS alcançável
do `wyd-web` e sua autenticação de origem. A Control API permanece local.
Ocultar o endereço no JavaScript não é controle de acesso ao upstream.

Configurar `_routes.json` para executar Functions apenas em `/api/*`; arquivos
estáticos continuam fora dessa rota. Previews devem usar backend, banco e
segredos de homologação; não conceder acesso ao backend de produção por
wildcard de `*.pages.dev`. Referência: [roteamento de Pages Functions](https://developers.cloudflare.com/pages/functions/routing/).

Pages Functions suportam rotas dinâmicas e podem inclusive funcionar como proxy de serviços externos.

Isso evita:

```text
CORS complexo
cookies third-party
origens diferentes para login
expor o endereço real da API no frontend
```

Se posteriormente tivermos domínio próprio, a topologia pode mudar sem alterar o frontend.

---

# 6. Control API dentro do `wyd-server`

Esta é a parte fundamental.

Ela deve iniciar junto do servidor:

```text
cmd/server
├── World
├── listener WYD
├── scheduler
├── metrics
└── control API
```

Mas a Control API não muta estruturas diretamente.

Fluxo:

```text
HTTP/control goroutine
        │
        │ AdminCommand
        ▼
World command queue
        │
        ▼
      World
        │
        ├── validate
        ├── mutate
        ├── persist
        └── publish
```

Portanto:

```go
world.Players[id].X = 2100
```

é proibido fora do `World`.

A Control API envia algo conceitualmente equivalente a:

```go
AdminTeleportPlayer{
    CharacterUID: uid,
    MapID:        mapID,
    X:            x,
    Y:            y,
}
```

e aguarda o resultado.

---

# 7. Transporte da Control API

Produção Linux:

```text
/run/wyd/control.sock
```

Unix Domain Socket.

Isso é preferível a abrir uma porta TCP.

Permissões:

```text
wyd-server → owner
wyd-web    → grupo autorizado
```

Desenvolvimento Windows:

```text
127.0.0.1:8081
```

com segredo interno obrigatório.

Autenticar o serviço também no socket Unix; permissões do sistema operacional
e loopback não substituem autorização por ação. O servidor deve distinguir a
identidade do serviço do ator final, validar a credencial interna e aceitar o
contexto do ator somente desse canal autenticado. Nunca confiar em `Actor`,
role ou account ID enviados diretamente pelo browser. Revalidar permissões e
ownership no momento da execução; registrar rotação e revogação do segredo.

Nunca:

```text
0.0.0.0:8081
```

---

# 8. Consultas live

Consulta administrativa também passa pelo `World`.

Exemplo:

```text
GET player live state
        ↓
Control API
        ↓
World Query
        ↓
AdminPlayerSnapshot
        ↓
Control API
```

O `World` devolve uma **cópia imutável**.

Nunca:

```text
*game.Player
*game.Mob
map interno do World
```

para outra goroutine.

---

# 9. Dados que NÃO devem passar pelo World

O `World` não será usado para:

```text
notícias
guias
ranking
login web
support tickets
pagamentos
histórico
market listings read-only
```

Esses caminhos usam:

```text
wyd-web
↓
PostgreSQL
```

Isso mantém tráfego web completamente fora do game loop.

---

# 10. Atualização de dados

Regra geral:

> nada é realtime por padrão.

Páginas operacionais exibem:

```text
Last updated: 19:32:51

[ Refresh ]
```

Aplicável a:

```text
Server Status
Players Online
Player Inspector
Bosses
Events
Instances
World
Economy
```

Nenhum polling automático.

Nenhum WebSocket global.

Nenhum SSE global.

---

# 11. Comandos sob demanda e confirmação de resultado

Não haver realtime visual não significa controle atrasado.

Exemplo:

```text
Admin
↓
Start Big Cube
↓
POST
↓
wyd-web
↓
Control API
↓
World
↓
Big Cube iniciado
↓
response
```

A ação é submetida no POST e executada quando o `World` puder processá-la.
Resposta de sucesso significa efeito confirmado; aceite em fila e timeout
precisam dos estados e consulta de operação definidos na seção 157.

---

# 12. Identidade e autenticação

A conta do site deve ser a própria conta WYD.

O sistema atual já possui validação compartilhada de usuário/senha e PBKDF2-HMAC-SHA256 versionado.

O `cmd/account-api` atual será gradualmente absorvido por:

```text
cmd/web-api
```

sem duplicar as regras existentes de conta.

Hoje o HTTP expõe somente cadastro e saúde. Migrar por compatibilidade de
rota, payload, validação e erros, mantendo `POST /v1/accounts` até atualizar
seus consumidores. Reutilizar os limites de corpo, proxy confiável, rate limit
e concorrência de hash já existentes.

Cadastro aceita usuário alfanumérico com 4–12 caracteres e senha ASCII visível
sem espaços com 4–10 caracteres. A autenticação aceita o intervalo legado de
1–10 observado no fluxo do client 7.48. Preservar essa diferença, normalizar a
chave da conta como no store e nunca truncar ou alterar a senha no frontend.
O campo wire continua fixo em 12 bytes, mas o client nativo rejeita mais de dez
caracteres antes de conectar.

---

# 13. Dados exclusivamente web

Informações que não pertencem ao gameplay não serão adicionadas ao JSON autoritativo da conta.

Criar:

```text
web_account_profiles
```

Exemplo:

```text
account_key
email
email_verified_at
locale
created_at
updated_at
```

Assim:

```text
Account
```

continua sendo domínio do jogo.

E:

```text
WebProfile
```

é domínio do portal.

---

# 14. Sessão web

Usar sessão server-side:

```text
web_sessions
```

Cookie:

```text
Secure
HttpOnly
SameSite=Lax
```

A sessão identifica:

```text
account_key
session_id
created_at
expires_at
last_seen
```

Não confiar em role enviada pelo navegador.

Gerar token opaco aleatório, persistir apenas seu hash e rotacioná-lo após
login/reautenticação. Usar cookie `__Host-wyd_session`, `Path=/`, sem `Domain`,
com expiração absoluta e por inatividade. Logout/revogação invalidam a sessão
no servidor. Credenciais, tokens e hashes não aparecem em DTOs ou logs.

Validar Origin e token CSRF nas mutações autenticadas por cookie, incluindo
login/logout; GET não altera estado. Cada consulta por UID verifica a posse
da conta autenticada. Aplicar `Cache-Control: no-store` a respostas privadas
e de autenticação, inclusive erros. Roles/capabilities devem refletir revogação
sem depender de um valor fixado no momento do login.

---

# 15. Roles de staff

Não colocar:

```text
role = admin
```

dentro do JSON da conta.

Criar:

```text
web_staff_roles
```

Com:

```text
account_key
role
granted_by
granted_at
revoked_at
```

Roles:

```text
Player
Moderator
GM
Admin
SuperAdmin
```

---

# 16. Capabilities

A autorização real será por capability.

Exemplos:

```text
account.self.read
account.password.change

guild.read
guild.members.remove
guild.members.promote
guild.applications.manage

moderation.player.search
moderation.player.message
moderation.player.kick
moderation.player.mute
moderation.player.ban

game.player.teleport
game.player.reward

game.event.start
game.event.stop
game.boss.control
game.instance.control
game.war.control

economy.inspect
economy.reward
economy.direct_grant

server.status
server.maintenance
server.shutdown
server.restart

staff.audit.read
staff.manage
```

Roles são apenas conjuntos dessas capabilities.

---

# 17. Guild role é independente

Um usuário pode ser:

```text
GlobalRole = Player
GuildRole  = Leader
```

Isso não dá nenhum poder de staff.

Guild roles:

```text
Member
Officer
Leader
```

---

# 18. Segurança de staff

Para Moderator+:

* 2FA/TOTP obrigatório antes de disponibilizar acesso de staff em produção;
* durante a fase local inicial, o painel usa temporariamente um PIN numérico
  adicional de 6–12 dígitos definido por `admin_access_pin` no `data/server.txt`;
* sessões mais curtas;
* reautenticação para ações críticas;
* audit log;
* CSRF obrigatório;
* rate limiting;
* confirmação de ações destrutivas.

SuperAdmin:

* poucas contas;
* acesso a staff management;
* economia avançada;
* restart/shutdown;
* direct item grant.

---

# 19. Estrutura pública principal

Navbar:

```text
Home
Play
Database
Events
Rankings
World
Community
Market

[ Account ]
```

Se staff:

```text
[ Admin ]
```

---

# 20. Home `/`

A home deve ser simples e concentrada.

## Hero

```text
WYD-Go

WYD 7.48 rebuilt around a modern server-authoritative architecture.

[ Create Account ]
[ Download ]
```

---

## Server summary

```text
Server status
Players online
Current version

[ Check Status ]
```

Só consulta quando solicitado.

---

## Latest News

3–4 notícias recentes.

---

## Next Events

Máximo de 3–5 ocorrências.

```text
19:00 Royal Arena
20:00 Big Cube
21:00 Boss Event
```

---

## Rankings resumidos

Tabs:

```text
Level
PvP
Guild
Arena
```

Top 5.

---

## World Control

Para cada modalidade de guerra integrada ao painel:

```text
Armia      Guild A
Azran      Guild B
Erion      Guild A
Noatun     Guild C
```

---

## Featured Guides

Guias relevantes:

```text
Getting Started
Mortal → Arch
Celestial
Crafting
Guilds
```

---

# 21. `/play`

Central de entrada no jogo.

```text
/play
/play/server
/play/getting-started
/play/classes
/play/rules
/play/faq
```

---

# 22. Server Information

Mostrar:

```text
Version
Rates
Level caps
Progression
PvP rules
Kingdoms
Guild rules
Main systems
Instances
Events
```

---

# 23. Classes

Página por classe:

```text
TransKnight
Foema
BeastMaster
Huntress
```

Conteúdo:

```text
Role
Strengths
Weaknesses
Skill trees
Progression
Main mechanics
```

Valores concretos de skill devem vir do catálogo real.

`SkillData.csv` já fornece custos, mana, delay, alcance, alvo e efeitos ao catálogo do servidor.

---

# 24. Download Center `/download`

O client completo deve ficar em armazenamento de artefatos ou mirror HTTPS,
com URL versionada e imutável; o Pages hospeda a página e o manifest. O limite
consultado é de 25 MiB por asset do Pages, insuficiente para assumir que o
pacote do jogo cabe ali. Revalidar limites antes de publicar: [Cloudflare Pages](https://developers.cloudflare.com/pages/platform/limits/).

Mostrar:

```text
Full Client
Launcher
Patch
Alternative mirrors

Version
Date
Size
SHA-256
```

Também:

```text
Minimum requirements
Recommended requirements
Installation
Common problems
Repair instructions
```

---

# 25. Client manifest

Criar um arquivo público:

```json
{
  "clientVersion": "7.48.3",
  "launcherVersion": "1.0.0",
  "minimumLauncherVersion": "1.0.0",
  "publishedAt": "...",
  "sha256": "..."
}
```

O mesmo manifest pode ser consumido por:

```text
site
launcher
/admin
```

---

# 26. News `/news`

No MVP, notícias em conteúdo editorial versionado no Git, publicado por build.

Categorias:

```text
News
Event
Maintenance
Community
Announcement
```

CMS para staff é evolução posterior. Publicação sem rebuild exige leitura
dinâmica e uma decisão de renderização/SEO, conforme a seção 152.

---

# 27. Changelog `/changelog`

Separado de notícias.

Categorias:

```text
Server
Client
Balance
Content
Bug Fix
Security
```

---

# 28. Roadmap `/roadmap`

Mostrar sistemas:

```text
Implemented
Testing
In Development
Planned
```

Sem necessidade de prometer datas.

Publicar o estado a partir de código e validações atuais. Torre e cidades já
têm implementação de servidor; o teste no client permanece pendente em
[Guerras de guilda](guild-wars.md). Separar essa integração web de RvR/reinos
e castelo, sem inferir conclusão ou ausência a partir de um roadmap histórico.

---

# 29. Database oficial `/database`

Essa deve ser uma das áreas mais fortes do portal.

```text
Items
Skills
Monsters
NPCs
Maps
Drops
Crafting
Quests
Bosses
Classes
```

---

# 30. Fonte da database

Não cadastrar esses dados manualmente.

Criar:

```text
cmd/web-export
```

Fluxo:

```text
data/
↓
loaders Go existentes
↓
web-export
↓
normalized JSON
↓
Astro build
```

O servidor já centraliza itens, nomes, efeitos e skills nos loaders de `data/`.

O exportador reutiliza loaders sem iniciar `World`, listeners ou store de
produção. Exporta somente campos aprovados para publicação, com
`schemaVersion`, `contentHash`, origem e IDs estáveis. Fórmulas ou regras
codificadas em Go precisam de extrator/contrato próprio; nem toda informação
de quests, drops e crafting está automaticamente disponível em CSV.

Validar referências e gerar saída determinística. Campo sem fonte verificada
fica indisponível, sem inventar valor. Assets visuais precisam de identidade,
formato compatível com web e autorização de distribuição. Mudança apenas
documental deste plano não autoriza converter ou publicar assets do client.

---

# 31. Item Database

Rota:

```text
/database/items
/database/items/{slug}
```

Mostrar:

```text
Name
Index
Category
Effects
Requirements
Allowed slots
Allowed classes
Price
Stack behavior
Trade restrictions
```

Relacionamentos:

```text
Dropped by
Sold by
Used in crafting
Used in quests
Related NPCs
Related items
```

---

# 32. Monster Database

Mostrar:

```text
Name
Level
HP
Attack
Defense
EXP
Map
Spawn areas
Drops
Boss flag
```

---

# 33. NPC Database

Mostrar:

```text
Name
Map
Coordinates
Function
Shop
Quest
Craft
Teleport
```

---

# 34. Map Database

Cada mapa mostra:

```text
Monsters
NPCs
Bosses
Quests
Event areas
Relevant drops
Progression recommendations
```

---

# 35. Droplist

A droplist será apenas outra visão da mesma database.

Permitir:

```text
Where does this item drop?
```

e:

```text
What does this monster drop?
```

---

# 36. Crafting Database

Mostrar:

```text
Recipe
NPC
Location
Inputs
Requirements
Cost
Chance
Result
Failure result
```

O projeto já possui os sistemas de composição separados do combate e diversos NPCs de craft implementados.

---

# 37. Crafting Calculator

Ferramenta pública:

```text
/tools/crafting
```

Pode calcular:

```text
materials
expected attempts
estimated cost
requirements
```

Somente informativa.

Nunca executa crafting.

---

# 38. Quests

```text
/database/quests
```

Categorias:

```text
Mortal
Arch
Celestial
SubCelestial
Daily
Instance
Event
```

Mostrar:

```text
Requirements
NPC
Map
Objectives
Rewards
Cooldown
Next quest
```

---

# 39. Progression Guide

```text
/progression
```

Visual:

```text
Mortal
  ↓
Arch
  ↓
Celestial
  ↓
SubCelestial
```

Cada estágio mostra:

```text
level requirements
items
quests
fame
NPCs
caps
unlock conditions
```

---

# 40. Events `/events`

Visualização:

```text
Today
Week
Month
```

Categorias:

```text
PvP
Boss
Guild
War
Instance
Seasonal
Bonus
```

---

# 41. Event details

Cada evento:

```text
/events/{slug}
```

Mostrar:

```text
Description
Schedule
Entry requirements
How to enter
Rules
Stages
Rewards
Related items
Historical winners
Ranking
```

---

# 42. Schedule authority

A agenda verdadeira pertence ao `wyd-server`.

O site apenas consulta.

```text
Server scheduler
↓
PostgreSQL
↓
wyd-web
↓
calendar
```

O browser nunca executa o evento.

Essa projeção de agenda para PostgreSQL é trabalho novo. Derivá-la da
configuração efetivamente carregada pelo scheduler, com versão, fuso IANA,
instantes UTC e `asOf`; o frontend pode apresentar horário local. Definir
reconciliação após reinício e tratar alteração de agenda sem disparar eventos
duplicados. Para torre/cidades, respeitar `America/Sao_Paulo` já documentado.

---

# 43. Rankings `/rankings`

Categorias:

```text
Level
EXP
PvP
Kills
Arena
Guilds
Events
Bosses
Season
```

Períodos:

```text
Today
Week
Month
Season
All Time
```

---

# 44. Ranking read model

Não fazer consultas pesadas sobre JSONB a cada visita.

Criar projeções:

```text
ranking_current
ranking_history
```

Essas tabelas são reconstruíveis e não autoritativas.

Um projetor limitado processa snapshots persistidos por watermark/versão,
fora do loop do jogo. Registrar atraso, retry, reconstrução e publicação
atômica de uma versão completa. TTL HTTP não atualiza essas tabelas. Rankings
e históricos só existem para métricas realmente coletadas; não reconstruir
um placar antigo a partir do estado atual. Toda resposta expõe `asOf` e
`contentVersion` quando aplicável.

---

# 45. Hall of Fame

```text
/rankings/hall-of-fame
```

Preservar resultados de seasons encerradas.

---

# 46. Public Character Profile

```text
/characters/{name}
```

Mostrar somente:

```text
Name
Class
Evolution
Level
Guild
Kingdom
Rankings
Achievements
```

Nunca:

```text
Account
IP
ItemUID
Private inventory
Security data
```

---

# 47. Public Guild Profile

```text
/guilds/{name}
```

Mostrar:

```text
Guildmark
Name
Leader
Officers
Kingdom
Members
Ranking
Cities
War history
Achievements
```

---

# 48. World `/world`

Central pública de estado do universo.

```text
/world
/world/bosses
/world/territories
```

---

# 49. Public World Status

Atender com resumo sanitizado e cache compartilhado, agregando pedidos
concorrentes em uma coleta limitada. Uma visita não pode enfileirar consultas
live arbitrárias. Distinguir `online`, `offline`, `unknown` e `stale`: falha do
web/proxy/banco não prova que o jogo caiu. Exibir o instante da última coleta.

Sob demanda:

```text
Server online/offline
Players online
Active event
```

Não expor:

```text
goroutines
World queue
DB queue
panic count
profiling
```

Esses dados ficam no `/admin`.

---

# 50. Boss Board

Mostrar:

```text
Boss
Alive / Dead
Map
Last kill
Killed by
Next respawn
```

Não necessariamente HP atual.

---

# 51. Boss History

Registrar:

```text
Boss
Killed at
Character
Guild
Participants
```

Permite rankings de boss posteriormente.

---

# 52. Territories

Para cada modalidade de guerra integrada ao painel:

```text
Armia
Azran
Erion
Nippleheim
Noatun
```

Mostrar:

```text
Owner
Kingdom
Owned since
Tax
Next contest
```

---

# 53. Public Economy

```text
/economy
```

Mostrar indicadores agregados:

```text
Total gold
Gold change 7d
Trade volume
Marketplace volume
Items created
Items destroyed
```

Também índices de itens importantes.

Não expor informações que auxiliem exploração/abuso.

---

# 54. Economy history

Períodos:

```text
7 days
30 days
Season
```

Permite identificar inflação publicamente.

---

# 55. Marketplace `/market`

Roadmap importante.

Funções:

```text
Browse listings
Search
Filters
Sell
Buy
My listings
Purchase history
Sales history
Price history
```

---

# 56. Marketplace server-authoritative

Item listado continua sendo item real com UID.

Fluxo:

```text
Player
↓
List ItemUID
↓
wyd-web
↓
Control API
↓
World
↓
validate ownership
↓
move to market escrow
↓
persist
↓
publish listing
```

Os UIDs de item já são globais e exclusivamente server-side.

---

# 57. Market escrow

Ao listar:

```text
inventory
→ escrow
```

Nunca:

```text
inventory
+
market
```

simultaneamente.

O schema atual exige `item_instances.account_key` e localização única por
conta; não oferece escrow pronto. Definir custódia explícita e migrar schema,
serialização e persistência juntos, preservando unicidade de UID e a
consistência com o inventário no JSONB. Não representar escrow por edição
isolada de `item_instances` ou por uma conta fictícia.

---

# 58. Market transaction

Compra:

```text
validate buyer
validate listing
validate currency
validate seller
↓
mutate copy
↓
persist transaction
↓
publish
```

A transação inclui débito, crédito, taxa, mudança de custódia/posse, estado da
listagem, entrega pendente, auditoria e resultado idempotente. Não transferir
posse em uma segunda gravação após o commit. Duas compras concorrentes devem
ter um único vencedor; cancelamento, trade e login concorrem sob a mesma
autoridade. Revalidar preço, moeda, quantidade e dono no servidor.

Quando comprador/vendedor estiver offline, coordenar carregamento e reserva
da conta com login/autosave. Começar exigindo `wyd-server` disponível; a web
não assume a escrita de gameplay quando ele cai. Testar inventário cheio,
reinício entre commit e resposta, cancelamento e recuperação de entrega.

---

# 59. Market filters

```text
Item
Category
Upgrade
Effects
Quantity
Price
Currency
Seller
```

---

# 60. Price history

Exemplo:

```text
Laktorerium Powder

7d avg
30d avg
Last transactions
Volume
```

---

# 61. Community

```text
/community
```

Pode reunir:

```text
Discord
Streamers
Videos
Screenshots
Polls
Guild recruitment
```

---

# 62. Pesquisa global pública

Uma única pesquisa deve encontrar:

```text
item
skill
monster
NPC
map
quest
guide
event
character
guild
```

Conteúdo estático pode usar índice gerado durante o build.

---

# 63. `/account`

Ao autenticar:

```text
/account
├── overview
├── characters
├── guild
├── market
├── rewards
├── donations
├── notifications
├── security
└── support
```

---

# 64. Account Overview

Mostrar:

```text
Account status
Created at
Last login
Characters
Guild
Pending rewards
Market listings
Support tickets
```

---

# 65. Character list

Cards:

```text
Name
Class
Evolution
Level
Kingdom
Guild
Last online
```

---

# 66. Character details

```text
/account/characters/{uid}
```

Mostrar:

```text
Class
Evolution
Level
EXP
Guild
Kingdom
Fame
Equipment
Progression
Achievements
```

---

# 67. Score exibido

Nunca calcular score no browser.

API entrega DTO resolvido pelo servidor.

Consultas normais usam a última projeção persistida do servidor e seu `asOf`.
Se ainda não houver projeção confiável de um score derivado, omitir esse
campo até implementá-la; não chamar o `World` a cada abertura de personagem.

Como `ExtendedScore` é autoritativo e `WireScore` é apenas projeção do client 7.48, o portal deve usar valores reais wide, não a projeção wire.

---

# 68. Equipment Viewer

Mostrar visualmente:

```text
Helmet
Armor
Pants
Gloves
Boots
Weapon
Shield
Accessories
Mount
Cape
```

Read-only inicialmente.

---

# 69. Inventory/Cargo Viewer

Podemos oferecer:

```text
Inventory
Cargo
Equipment
```

read-only.

Não mostrar UID interno ao jogador.

O inventário estrutural é 64/63 jogável e o Cargo 128/120 visível.

---

# 70. Personal Progression Advisor

Um diferencial importante.

Exemplo:

```text
ARCH ASCENSION

✓ Required level
✓ Eternal Stone
✗ Sephirot
✓ Fame

Missing:
Sephirot
```

Usa estado real do personagem + regras reais do servidor.

---

# 71. Security Center

```text
/account/security
```

Funções:

```text
Change password
Active web sessions
Login history
Revoke session
Logout all devices
```

Futuro:

```text
Email verification
TOTP
Recovery codes
```

---

# 72. Password change

Fluxo:

```text
current password
new password
confirmation
```

Reutilizar `internal/account`.

Não criar um segundo algoritmo de senha.

O pacote atual oferece criação/autenticação/hash; ainda não implementa uma
troca segura de senha. Como a credencial participa do snapshot da conta,
uma gravação web concorrente pode ser revertida por autosave/relogin.

Primeira implementação: comando autoritativo serializado com login e saves,
incluindo conta offline, barreira de persistência e atualização da cópia em
memória. O hash é calculado fora do loop; o commit revalida identidade,
credencial/versão esperada e autorização. Testar troca concorrente com
autosave, login e restart, além de revogar sessões web após confirmação.
`SERIALIZABLE` isoladamente não resolve snapshot obsoleto entre processos.

Recuperação depende de canal previamente verificado, token de uso único com
expiração e resposta sem enumeração. Não habilitar `/auth/recover` nem prometer
troca de senha no MVP enquanto esses contratos não estiverem prontos.

---

# 73. Reward Inbox

```text
/account/rewards
```

Central para:

```text
Donation
Event
Compensation
Promotion
GM reward
Code
```

---

# 74. Reward Claim

Fluxo:

```text
RewardID
↓
claim
↓
server validates
↓
idempotency
↓
validar capacidade e preparar cópia
↓
persistir entrega + claim + resultado idempotente na mesma transação
↓
publicar estado confirmado
```

Separar direito à recompensa de entrega: `pending → claimed` ou `expired`.
Falha de capacidade não consome o direito; retry devolve o mesmo resultado.
O servidor valida destinatário, prazo e conteúdo, incluindo concorrência
entre dispositivos. Webhook apenas registra o direito; não insere inventário.

Melhor que inserir diretamente item em inventário.

---

# 75. Donations

```text
/account/donations
```

Mostrar:

```text
Packages
Payment
Payment status
History
Rewards
```

---

# 76. Payment flow

```text
checkout
↓
payment provider
↓
webhook
↓
wyd-web
↓
PaymentID validated
↓
Reward created
↓
Reward Inbox
```

Nunca confiar no frontend para confirmar pagamento.

Validar assinatura do corpo original, identificador do provedor, valor,
moeda, conta e estado final antes de criar o direito à recompensa. Registrar
evento e direito com unicidade durável e transação/outbox; responder sucesso
ao provedor somente após persistência. Duplicação e eventos fora de ordem
devem ser inofensivos. Reconciliar pendências com o provedor e definir
tratamento de estorno/chargeback sem retirar item gasto silenciosamente.
Testar no sandbox do provedor antes de habilitar qualquer checkout real.

---

# 77. Notifications

```text
Reward available
Guild application accepted
Marketplace sale
Support answered
Password changed
Punishment
```

Consulta sob demanda.

---

# 78. Support

```text
/account/support
```

Categorias:

```text
Account
Payment
Bug
Player Report
Item
Guild
Other
```

Estados:

```text
Open
Waiting Staff
Waiting Player
Resolved
Closed
```

---

# 79. Self-service

Recursos úteis:

```text
Change password
Recover account
Revoke sessions
Leave guild
Claim rewards
Cancel market listing
Character unstuck
```

---

# 80. Character Unstuck

Regras server-side:

```text
character offline
not in instance
not in event
not in war
cooldown satisfied
```

A API não altera coordenadas diretamente no PostgreSQL.

---

# 81. Guild Panel

```text
/account/guild
```

Membro comum:

```text
Guild overview
Members
Ranks
Online/offline
Statistics
History
Wars
Territories
```

---

# 82. Guild Leader / Officer

Capabilities:

```text
Remove offline member
Promote
Demote
Accept application
Reject application
Edit guild notice
War registration
```

---

# 83. Remove offline member

Fluxo:

```text
Leader
↓
request
↓
wyd-web authorizes
↓
Control API
↓
server confirms leader
↓
server confirms member
↓
server confirms target offline
↓
transaction
```

O navegador nunca decide se o membro está offline. Revalidar liderança,
filiação, alvo e versão no instante da execução. Reservar as contas envolvidas
contra login e salvar guilda/contas de forma consistente; uma consulta
`offline` feita antes da transação não basta.

---

# 84. Guild Applications

Jogador:

```text
[ Apply ]
```

Líder vê:

```text
Character
Class
Level
Evolution
Kingdom
Last online
```

---

# 85. Guild Activity Log

Registrar:

```text
joined
left
removed
promoted
demoted
leader changed
application accepted
war registered
city acquired
city lost
```

---

# 86. Guild War Room

Quando os dados e comandos da modalidade estiverem integrados ao portal:

```text
Next war
Opponent
Participants
Registration
Score
Territories
History
```

Torre/cidades já têm mecânicas e persistência implementadas no servidor,
com gate no client ainda pendente em [Guerras de guilda](guild-wars.md).
O War Room é uma integração nova; RvR/reinos e castelo não herdam esse estado.

---

# 87. `/admin`

Área única para Moderator, GM, Admin e SuperAdmin.

Menu:

```text
Dashboard
Players
Accounts
Guilds
Moderation
World
Events
Bosses
Instances
Wars
Economy
Items
Marketplace
Rewards
Payments
Support
News
Content
Security
Logs
Audit
Server
Staff
```

O menu é filtrado pelas capabilities.

---

# 88. Admin Dashboard

Nada automático.

```text
[ Refresh Status ]
```

Mostrar:

```text
Server status
Uptime
Players
Sessions
Mobs
World tick
World lag
World command queue
PostgreSQL queue
Persistence failures
Memory
Panics
Build version
Content version
Database schema
```

Boa parte dessas métricas já existe em `expvar`.

`/debug/vars` e pprof continuam privados e nunca são expostos pelo portal. O projeto atual deliberadamente restringe ambos a loopback.

---

# 89. Global Admin Search

Uma caixa única:

```text
Search...
```

Detectar:

```text
Account
Character
CharacterUID
Guild
ItemUID
IP
PaymentID
RewardID
RequestID
```

---

# 90. Players Online

```text
/admin/players
```

Tabela:

```text
Name
Account
Class
Level
Map
Guild
Session
Connected
```

Atualização:

```text
[ Refresh ]
```

Estado atual em 13/09/2026: a tabela read-only já existe no `/admin` e usa o
mesmo snapshot autoritativo do `World`, com busca e paginação de até 50 linhas.
Nome, conta, classe, nível e posição runtime estão implementados; guild, sessão,
HP/MP e grupo ficam disponíveis no inspector da própria linha. `Map` nominal e
tempo conectado permanecem fora do contrato enquanto não houver fonte live
confiável definida para esses campos.

---

# 91. Player Inspector

Separar explicitamente:

```text
LIVE STATE
PERSISTENT STATE
```

### Live

Fonte:

```text
World
```

### Persistent

Fonte:

```text
PostgreSQL
```

Mostrar timestamp das duas consultas.

Estado atual em 13/09/2026: a metade `LIVE STATE` está implementada sem endpoint
N+1 adicional. O inspector abre a partir do jogador já copiado no snapshot de
`GET /api/v1/staff/overview` e mostra o `asOf` dessa leitura. A metade
`PERSISTENT STATE` também está implementada por consulta sob demanda em
`GET /api/v1/staff/players/{uid}/persistent?account=...`, usando a conexão
PostgreSQL dedicada read-only e um DTO estreito que não expõe hash de senha,
inventário ou cargo brutos. Cada fonte possui `asOf` independente. O contrato,
os casos 400/404/503, a autorização e o frontend estão cobertos por testes e
build automatizados; a validação manual em browser/runtime permanece pendente.

O overview administrativo também projeta um bloco `diagnostics` versionado e
read-only, coletado no processo do servidor após o snapshot do `World`. Ele
expõe apenas `postgresQueueDepth`, `persistenceFailures` e `panics`, com `asOf`
próprio, sem publicar `/debug/vars` ou pprof. A UI apresenta esse bloco separado
do `WORLD SNAPSHOT` para não sugerir que as duas fontes foram lidas no mesmo
instante. Contrato, frontend e suíte automatizada estão validados; o gate
manual autenticado com servidor/PostgreSQL reais continua pendente.

---

# 92. Live Player State

Mostrar:

```text
HP/MP
Extended score
Map
Coordinates
Guild
Party
Buffs
Cooldown state
Session
Connected time
Inventory
Equipment
Cargo reference
```

Implementado no primeiro inspector live: HP/MP efetivos (com
`RuntimeScore -> Score` como fallback), máximos, coordenadas runtime, guild/rank,
tamanho do grupo, sessão, classe e nível. Extended score completo, mapa nominal,
buffs, cooldowns, connected time, inventário, equipamento e referência de cargo
continuam no backlog e não devem ser inferidos a partir de dados incompletos.

---

# 93. Moderator actions

```text
Send Message
Warning
Kick
Mute
Temporary Ban
Permanent Ban
```

---

# 94. GM/Admin actions

Além das anteriores:

```text
Teleport
Force Save
Create Reward
Revive when allowed
Event control
Boss control
```

---

# 95. Advanced actions

Somente capability específica:

```text
Direct item grant
Direct gold grant
Direct item removal
Direct currency removal
```

Sempre:

```text
reason required
confirmation required
audit required
idempotency required
```

---

# 96. Accounts

```text
/admin/accounts
```

Mostrar:

```text
Account
Created
Last login
Characters
Status
Punishments
Web sessions
Payments
Support cases
```

Nunca mostrar password hash na interface.

---

# 97. Moderation

```text
/admin/moderation
```

Áreas:

```text
Warnings
Mutes
Temporary bans
Permanent bans
Player reports
Cases
```

---

# 98. Punishments

Persistir a punição não garante sua aplicação. Ban deve ser verificado no
login do jogo e no web conforme o escopo, invalidar acessos existentes quando
exigido e sobreviver a restart. Mute precisa ser consultado pelos caminhos de
chat. Expiração, revogação e comando live devem ser coordenados e auditados;
a nova tabela não pode ser apenas um registro que o jogo ignora.

Criar tabela própria:

```text
punishments
```

Campos:

```text
id
type
account_key
character_uid
reason_public
reason_internal
staff_account
created_at
expires_at
revoked_at
revoked_by
```

---

# 99. Moderation Case

Um case pode agregar:

```text
reports
logs
punishments
staff notes
characters
accounts
IPs
```

Isso melhora muito investigação.

---

# 100. Guild Admin

```text
/admin/guilds
```

Mostrar:

```text
Guild
Leader
Kingdom
Members
Online
Territories
Wars
```

Ações:

```text
Inspect
Change leader
Repair invalid membership
Force remove
Rename
```

Somente comandos semânticos.

---

# 101. World Inspector

```text
/admin/world
```

Mostrar:

```text
Map
Players
Active mobs
Bosses
Instances
Events
Ground items count
```

Usar índices/contadores existentes.

Não fazer varreduras globais frequentes.

O servidor já possui grid espacial e ativação por proximidade.

---

# 102. Event Manager

```text
/admin/events
```

Tabs:

```text
Definitions
Schedules
Running
History
```

Ações:

```text
Start
Cancel
Finish
Schedule
Enable
Disable
```

---

# 103. Event Scheduler

Exemplo:

```text
Big Cube
Saturday 20:00

Boss Event
Daily 22:00
```

O scheduler roda no `wyd-server`.

Se:

```text
Cloudflare
wyd-web
browser
```

estiverem indisponíveis, o evento continua funcionando.

---

# 104. Announcement Scheduler

Exemplo:

```text
19:50  Event starts in 10 minutes.
19:55  Event starts in 5 minutes.
19:59  Event starts in 1 minute.
20:00  Start Event
```

Executado pelo servidor.

---

# 105. Boss Manager

```text
/admin/bosses
```

Mostrar:

```text
Boss
Status
Map
HP
Phase
Shield
Adds
Respawn
Players nearby
```

Ações:

```text
Enable
Disable
Spawn if absent
Despawn
```

Estado implementado neste incremento: `GET/POST /api/v1/staff/bosses` lista os
bosses configurados e permite `summon` de um boss morto com a capability
`game.boss.summon`. O estado retornado inclui HP, posição, spawn, respawn e
revisão do lifecycle. `epoch + expectedRevision + operationId` protegem contra
requisições antigas, corrida com o lifecycle e retry de resultado incerto. A
operação é enfileirada no World e reutiliza o spawn normal do boss.

`Enable`, `Disable` e `Despawn`, assim como fase, shield, adds e players nearby,
continuam sendo backlog desta seção e não devem ser interpretados como já
disponíveis no painel atual.

---

# 106. Instance Manager

```text
/admin/instances
```

Mostrar:

```text
InstanceID
Type
State
Players
Started
Expires
Stage
```

Ações administrativas limitadas:

```text
Inspect
Message participants
Force finish
```

---

# 107. War Manager

Quando a modalidade e seu contrato de controle estiverem integrados:

```text
/admin/wars
```

Áreas:

```text
Guild War
Kingdom War
Castle War
```

Mostrar:

```text
Registration
Participants
Defender
Attackers
Score
Objectives
Territory
Winner
History
```

---

# 108. Economy Dashboard

```text
/admin/economy
```

Mostrar:

```text
Gold supply
Gold created
Gold destroyed
Items created
Items destroyed
Trade volume
Craft volume
Marketplace volume
Rewards
Shop purchases
```

Períodos:

```text
24h
7d
30d
Season
```

---

# 109. Economy Alerts

Exemplos:

```text
Gold supply +20% / 24h
Item 413 supply +300%
Marketplace volume anomaly
Unusual reward creation
```

Ajuda a identificar:

```text
exploit
configuração errada
evento excessivo
dupe
```

---

# 110. Item UID Explorer

```text
/admin/items/{uid}
```

Mostrar:

```text
UID
Item
Effects
Owner
Location
Created at
Created by
Current state
```

---

# 111. Item Provenance

Criar progressivamente um ledger de eventos de item:

```text
created
picked up
moved
traded
crafted
refined
market listed
market sold
destroyed
```

Visual:

```text
Kefra
 ↓ drop
PlayerA
 ↓ trade
PlayerB
 ↓ marketplace
PlayerC
```

Como o servidor já preserva UID durante movimento, equipamento, cargo, trade, refino e transformação, essa rastreabilidade encaixa naturalmente no modelo existente.

---

# 112. Marketplace Admin

```text
/admin/marketplace
```

Mostrar:

```text
Listings
Transactions
Cancelled listings
Fees
High-value trades
Suspicious trades
```

---

# 113. Rewards Admin

A operação administrativa padrão deve ser:

```text
Create Reward
```

e não:

```text
Inject Item
```

Exemplo:

```text
Target: Felipe
Item: Laktorerium Powder
Quantity: 10
Reason: Event compensation
```

Vai para Reward Inbox.

---

# 114. Direct Grant

Ainda pode existir para manutenção excepcional.

Mas:

```text
SuperAdmin only
advanced section
reason required
double confirmation
audit
```

---

# 115. Payments Admin

```text
/admin/payments
```

Mostrar:

```text
PaymentID
Account
Provider
Amount
Status
Webhook status
Reward
Delivery status
```

---

# 116. Support Admin

```text
/admin/support
```

Staff pode:

```text
Claim
Reply
Assign
Escalate
Resolve
Close
```

Contexto automático:

```text
Account
Characters
Guild
Punishments
Payments
Relevant ItemUID
```

---

# 117. News CMS

```text
/admin/news
```

Capabilities:

```text
Create
Draft
Preview
Publish
Unpublish
Schedule
```

---

# 118. Guides

Guias longos e estruturais devem continuar no Git:

```text
Markdown / MDX
```

Benefícios:

```text
version history
review
diff
validação editorial em main
```

---

# 119. Content Inspector

```text
/admin/content
```

Read-only inicialmente.

Mostrar:

```text
Items
Skills
NPCs
Monsters
Quests
Crafting
Boss profiles
```

Todos vindos de `data/`.

---

# 120. Futuro Content Editor

Somente depois de termos validação segura:

```text
Edit
↓
Validate with real loader
↓
Show diff
↓
Atomic save
↓
Reload if supported
```

Nunca criar uma segunda tabela duplicando ItemList/SkillData.

---

# 121. Security Dashboard

```text
/admin/security
```

Mostrar:

```text
Login failures
Protocol violations
Flood violations
Invalid movement
Invalid skill
Invalid item action
Invalid economy action
Rate-limit disconnects
```

O servidor já trata cliente como não confiável, com framing, fases de sessão e validações server-side.

---

# 122. Logs

```text
/admin/logs
```

Filtros:

```text
Period
Level
Category
Account
Character
Session
Opcode
RequestID
```

Resultado paginado.

Não implementar `tail -f` inicialmente.

---

# 123. Audit Log

```text
/admin/audit
```

Toda mutação administrativa registra:

```text
RequestID
Actor
Action
Target
Parameters
Reason
Result
Timestamp
Remote IP
```

Para operações críticas:

```text
before
after
```

Omitir ou mascarar dados sensíveis e limitar parâmetros registrados; nunca gravar senha,
cookie, token ou hash. Registrar intenção, rejeição e resultado com vínculo à
operação. Efeito econômico e registro durável de auditoria/outbox compartilham
o commit: falha ao gravar auditoria não pode deixar uma mutação invisível.
Definir retenção, acesso e exportação restrita para logs e dados pessoais.

---

# 124. Request ID

Todo comando administrativo recebe:

```text
RequestID
```

Exemplo:

```text
01K5...
```

Ele percorre:

```text
Browser
↓
wyd-web
↓
Control API
↓
World
↓
PostgreSQL
↓
Audit
```

Facilita investigação completa.

---

# 125. Idempotency

Operações econômicas usam:

```text
Idempotency-Key
```

Obrigatório para:

```text
Reward
Payment delivery
Give item
Give currency
Marketplace purchase
Marketplace listing
```

Clique duplo ou retry HTTP não duplica operação.

Isso é um requisito a testar, não uma propriedade do header. Usar chave única
por ator + tipo de operação + chave idempotente, vinculada a um hash canônico
do payload e alvo. Mesma chave com payload diferente retorna conflito.
Persistir estado, resultado e efeito econômico na mesma transação; a entrega
ao cliente ocorre depois do commit. Reinício e timeout preservam a consulta
do resultado. Expiração do registro não pode permitir repetir uma entrega:
RewardID/PaymentID/listagem também têm unicidade de negócio durável.

---

# 126. Server Management

```text
/admin/server
```

Mostrar:

```text
Process status
Uptime
Build
Content version
DB schema
Client version required
Maintenance state
```

---

# 127. Maintenance Mode

Ações:

```text
[ Enter Maintenance ]
[ Leave Maintenance ]
```

Entrar:

```text
block new logins
broadcast warning
optional countdown
```

---

# 128. Graceful Shutdown

```text
[ Graceful Shutdown ]
```

deve entrar como comando do `World`.

O shutdown atual já é coordenado pelo game loop, salva contas sincronamente e faz `Flush()` da persistência assíncrona.

---

# 129. Start Server

A Control API desaparece quando `wyd-server` está parado.

Portanto start/restart não pode depender dela.

`wyd-web` terá um `Supervisor` extremamente limitado.

Fora do MVP. Implementar como interface de serviço/helper com identidade
separada e unidade fixa; o processo web não recebe shell ou privilégios
genéricos de root. Registrar operação durável, rejeitar starts/restarts
concorrentes e definir o deadline de drain antes de permitir força explícita.

Linux:

```text
systemd
```

Permitido apenas:

```text
status
start
stop
restart
```

Nenhum shell arbitrário.

---

# 130. Restart

Fluxo ideal:

```text
wyd-web
↓
Control API graceful shutdown
↓
server exits
↓
systemd start
↓
health check
```

Se Control API não responder:

```text
staff explicit force restart
```

com capability separada.

---

# 131. Backup

`/admin/server` pode mostrar:

```text
Last PostgreSQL backup
Backup size
Backup result
```

E permitir:

```text
[ Create Backup ]
```

Restaurar banco não deve fazer parte inicialmente do painel web.

Restore é operação offline/manual.

---

# 132. Staff Management

```text
/admin/staff
```

SuperAdmin pode:

```text
Grant role
Revoke role
View capabilities
View sessions
Revoke staff sessions
```

Toda mudança auditada.

---

# 133. Build Information

Mostrar:

```text
Server Git commit
Build date
Go version
Client version
Database schema
Content hash
Web version
```

---

# 134. Content Version

Gerar hash dos arquivos autoritativos relevantes:

```text
itemlist.csv
Itemname.csv
ItemEffect.h
SkillData.csv
quests
boss
NPC data
```

Exemplo:

```text
Server Build   a71bc92
Content        d2f71e4
Schema         v12
Client         7.48.3
```

Isso facilita reproduzir bugs.

---

# 135. Organização de código

Layout proposto, sem criar uma terceira source na raiz. Criar cada diretório
somente quando sua fase precisar dele:

```text
wyd-go/
├── AGENTS.md
├── DOCS/
│   └── WYD-WEB-PLATAFORM.md
├── tmproject/                       # client e runtime existentes
└── wydgo748/                        # raiz do go.mod
    ├── cmd/
    │   ├── server/                  # existente
    │   ├── account-api/             # preservar durante a migração
    │   ├── account-create/          # existente
    │   ├── web-api/                 # novo processo web
    │   └── web-export/              # exportador offline futuro
    ├── internal/
    │   ├── account/                 # regras compartilhadas existentes
    │   ├── accountapi/              # HTTP de cadastro existente
    │   ├── game/                    # World e comandos autoritativos
    │   ├── control/                 # contrato/transporte privado read-only
    │   ├── webadmin/                # auth staff, sessão, API e static site
    │   ├── store/                   # persistência e migrations
    │   ├── model/
    │   ├── data/
    │   ├── wire/
    │   └── net/
    ├── api/
    │   └── openapi.yaml
    ├── web/
    │   └── portal/
    │       ├── src/
    │       ├── public/
    │       ├── functions/
    │       ├── package.json
    │       └── package-lock.json
    └── bin/                        # artefatos Go ignorados
```

Conteúdo editorial Markdown/MDX consumido pelo build é parte do produto e
pode ficar em `web/portal/src/content/`; relatórios e instruções continuam
em `DOCS/`. Antes de introduzir esse conteúdo, ajustar pontualmente o
validador de layout para reconhecer essa exceção de produto, sem liberar
documentação genérica nas sources. Não alterar o validador nesta revisão.

No primeiro lote de frontend, acrescentar ignores delimitados para
`node_modules/`, `dist/` e caches do portal. Fixar versões de ferramentas
e dependências no projeto; nenhum executável, export temporário ou segredo
entra no Git.

---

# 136. API pública

Estrutura:

```text
/api/v1/public/*
/api/v1/auth/*
/api/v1/account/*
/api/v1/guild/*
/api/v1/market/*
/api/v1/staff/*
```

DTOs públicos são contratos próprios, não serialização direta dos modelos do
servidor. IDs persistentes/UIDs serão strings; inteiros que possam exceder a
precisão segura do JavaScript serão strings decimais. Datas usam RFC 3339 em
UTC. Não usar índice de sessão como identidade durável de conta/personagem.
Definir paginação limitada, tamanhos máximos, erros estáveis e compatibilidade
de versões antes de gerar o cliente. Rotas futuras só entram quando sua fase
for implementada; este namespace não substitui o cadastro existente.

---

# 137. Exemplos de endpoints públicos

```text
GET /api/v1/public/status
GET /api/v1/public/news
GET /api/v1/public/events
GET /api/v1/public/rankings/players
GET /api/v1/public/rankings/guilds
GET /api/v1/public/characters/{name}
GET /api/v1/public/guilds/{name}
GET /api/v1/public/economy
```

---

# 138. Auth

```text
POST /api/v1/auth/register
POST /api/v1/auth/login
POST /api/v1/auth/logout
POST /api/v1/auth/recover
GET  /api/v1/auth/me
```

---

# 139. Account

```text
GET  /api/v1/account
POST /api/v1/account/password

GET /api/v1/account/characters
GET /api/v1/account/characters/{uid}

GET /api/v1/account/rewards
POST /api/v1/account/rewards/{id}/claim

GET /api/v1/account/security/sessions
DELETE /api/v1/account/security/sessions/{id}
```

---

# 140. Guild

```text
GET /api/v1/guild
GET /api/v1/guild/members

POST   /api/v1/guild/applications
POST   /api/v1/guild/applications/{id}/accept
DELETE /api/v1/guild/applications/{id}

DELETE /api/v1/guild/members/{uid}
POST   /api/v1/guild/members/{uid}/promote
POST   /api/v1/guild/members/{uid}/demote
```

---

# 141. Staff

```text
GET /api/v1/staff/server/status

GET /api/v1/staff/players
GET /api/v1/staff/players/{uid}

POST /api/v1/staff/players/{uid}/message
POST /api/v1/staff/players/{uid}/kick
POST /api/v1/staff/players/{uid}/teleport
POST /api/v1/staff/players/{uid}/ban

GET  /api/v1/staff/events
POST /api/v1/staff/events/{id}/start
POST /api/v1/staff/events/{id}/finish

GET  /api/v1/staff/bosses
POST /api/v1/staff/bosses

GET  /api/v1/staff/audit
```

---

# 142. Control API interna

Não precisa espelhar REST público.

Pode usar comandos tipados:

```text
QueryServerStatus
QueryOnlinePlayers
QueryPlayerLive
QueryWorld
QueryBosses
QueryInstances

SendMessage
Broadcast
DisconnectPlayer
TeleportPlayer
ForceSavePlayer

StartEvent
FinishEvent

SpawnBoss
DespawnBoss

EnterMaintenance
LeaveMaintenance
GracefulShutdown

CreateGameplayReward
```

---

# 143. Estrutura de Admin Command

Conceitualmente:

```go
type AdminMeta struct {
    RequestID string
    OperationID string
    IdempotencyKey string
    Actor     string
    Reason    string
    IssuedAt  time.Time
}

type AdminResult struct {
    RequestID string
    OperationID string
    Status    string
    Success   bool
    Code      string
    Message   string
}
```

Esboço, não tipos existentes. `RequestID` correlaciona uma tentativa HTTP;
`OperationID` identifica a operação e `IdempotencyKey` vincula retries do mesmo
ator/ação/payload. `Actor` vem da sessão e do canal de serviço autenticados.
Cada comando define alvo persistente, precondições/versão esperada quando
necessária, capacidade exigida e resultado tipado. `Success` isolado não
expressa aceitação, pendência nem resultado desconhecido (seção 157).

---

# 144. Database web

Tabelas candidatas, criadas incrementalmente por fase, além das atuais de
gameplay. O prefixo/nome não concede ao serviço web permissão de escrita;
ownership e grants seguem a seção 145. Notícias/changelog continuam no Git
no MVP; suas tabelas só existem se o CMS for aprovado.

```text
web_account_profiles
web_sessions
web_staff_roles
web_notifications

web_news
web_changelog

guild_applications
guild_activity_log

support_tickets
support_messages

punishments

payments
payment_events

reward_claims
reward_entitlements
reward_deliveries

control_operations
idempotency_records
transactional_outbox

market_listings
market_transactions

ranking_current
ranking_history

economy_snapshots
item_ledger

admin_audit_log
```

---

# 145. Separação de permissões PostgreSQL

Papéis propostos e responsabilidades:

| Papel | Acesso permitido | Restrição |
| --- | --- | --- |
| Migrador de implantação | DDL e migrações versionadas de jogo/web | Credencial ausente dos processos HTTP |
| `wyd_server` | Gameplay, contas existentes, custódia, entregas e resultados de comandos | Única autoridade de mutação desses estados |
| `wyd_web` | Sessões/perfis, editorial, suporte, pagamentos e leitura de views aprovadas | Sem UPDATE de snapshots de conta, personagens, itens ou guilda |
| Cadastro restrito | Criar conta nova pela operação de domínio | Sem substituir conta existente nem conceder acesso genérico ao store |
| Projetor | Ler fonte aprovada e atualizar projeções reconstruíveis | Pool/tempo limitados; sem mutar gameplay |

Definir GRANTs por schema/tabela/coluna/função e testar com os usuários reais
de serviço. Cadastro necessita de uma operação INSERT restrita que preserve
as constraints existentes; a autenticação precisa de um acesso interno mínimo
à credencial, nunca um endpoint que devolva `accounts.payload`. Reutilizar
`internal/account` por interfaces estreitas, não expor o store inteiro.

O construtor atual `NewPostgresStore` aplica schema e inicia persistência.
Antes de usar o papel web, separar migrations de abertura de conexão e criar
adaptadores próprios para leitura, autenticação e cadastro. O construtor
read-only existente não oferece por si só escrita de sessões e cadastro.

Tabelas da seção 144 não são todas de escrita web: claims/entregas, custódia,
operações econômicas e punições efetivas pertencem aos contratos do servidor.
O web registra pedidos/direitos e lê resultados; o servidor valida sua origem.
Cada tabela deve ganhar dono, constraints e transação na fase que a cria.

Migrações devem ser incrementais, compatíveis com a versão anterior durante
a implantação e testadas em cópia/fixture PostgreSQL. Prever backfill,
rollback da aplicação e restauração ensaiada; rollback não presume apagar
dados econômicos. Banco indisponível bloqueia escritas e entrega; não habilita
fallback para edição de arquivos.

---

# 146. Cache

## Estático

```text
Guides
Database
Classes
Rules
Download instructions
```

## Cache público curto

```text
News
Rankings
Guild profiles
Event calendar
Economy summary
```

## No-store

```text
Account
Guild management
Payments
Support
/admin
```

---

# 147. Frequências iniciais

Sugestão:

```text
News             5 min
Rankings         60 s
Guild public     60 s
Event calendar   5 min
Economy public   5 min
```

Não é polling no browser.

É política de cache HTTP.

---

# 148. Paginação

Usar cursor/keyset em:

```text
Logs
Audit
Payments
Support
Marketplace history
Economy ledger
```

Nunca retornar todos os registros.

---

# 149. OpenAPI

Manter:

```text
wydgo748/api/openapi.yaml
```

Gerar cliente TypeScript para o Astro e verificar geração reproduzível no CI.
Uma alteração no DTO Go não quebra o frontend automaticamente: são necessários
testes de conformidade dos handlers/DTOs com o OpenAPI, detecção de drift dos
arquivos gerados e typecheck/testes dos consumidores. Registrar respostas de
erro, autorização, cookies/CSRF, paginação, IDs e operações assíncronas.
Mudanças incompatíveis exigem política explícita de versão e rollout.

---

# 150. Design visual

Direção:

```text
Dark fantasy
WYD identity
Metal/stone details
Clean information hierarchy
Modern cards/tables
Low visual noise
```

Site público pode ser mais visual.

`/admin` deve priorizar densidade e legibilidade.

---

# 151. Mobile

### Público

Mobile-first.

### Account

Totalmente funcional em celular.

### Admin

Responsivo, mas desktop-first.

Operações complexas de economia/admin podem inclusive exigir tela maior.

---

# 152. Renderização e SEO

Astro pré-renderiza no build por padrão; renderização sob demanda requer
adaptação de runtime. Não é coerente prometer HTML estático atualizado sem
rebuild para todo conteúdo criado posteriormente. Referência: [renderização
sob demanda do Astro](https://docs.astro.build/en/guides/on-demand-rendering/).

| Conteúdo | Estratégia inicial | Evolução |
| --- | --- | --- |
| Institucional, guias, notícias e changelog | Git → build → HTML estático | CMS com publicação/rebuild rastreável ou SSR deliberado |
| Itens, skills e demais catálogos liberados | Export versionado → build → HTML estático | Rebuild ao publicar versão de conteúdo |
| Conta, guild management e admin | Shell estático + API autenticada; `noindex` | Sem dados privados no HTML de build |
| Rankings e perfis públicos | Shell estático + API/projeção, com `asOf` | SSR/snapshots publicados se SEO desses dados for requisito |

Rotas públicas por slug usam identidade estável, canonical, sitemap e 404
real. No estágio estático, gerar somente slugs conhecidos no build;
rotas privadas/dinâmicas usam shell de rota fixa com ID em query string ou
roteamento deliberado, sem assumir que um slug novo existe no Pages. Testar
acesso direto e refresh. Antes de habilitar CMS sem rebuild, escolher SSR ou
shell dinâmico e registrar o efeito sobre SEO; a Function proxy não renderiza
automaticamente páginas Astro.

---

# 153. O que NÃO existirá

Não criar:

```text
SQL console
shell web
raw JSON editor
packet sender
memory editor
ExtendedScore editor
arbitrary command endpoint
```

Não existir:

```text
POST /execute
```

Tudo será comando tipado.

---

# 154. Performance

Objetivo:

```text
ninguém usando o site
≈
zero custo no World
```

Página pública:

```text
Cloudflare/static/cache
```

Conta:

```text
wyd-web/PostgreSQL
```

Admin status:

```text
somente ao clicar Refresh
```

Gameplay command:

```text
somente quando jogador ou staff solicitar ação autoritativa
```

“Zero custo” é uma meta de ausência de consultas por page view, não uma
garantia literal: projeções, exportações e resumos agendados têm custo e devem
ser limitados e medidos. Consultas de status devem usar snapshots/cache sempre
que possível, com coalescência e limite de frequência mesmo no Refresh manual.

---

# 155. Concorrência

A Control API read-only atual integra-se ao modelo sem acessar estado mutável
do World fora de sua goroutine:

```text
Game session ───┐
Scheduler ──────┤
Admin command ──┤
                ▼
             World
```

Mas:

```text
ranking
news
support
login web
```

não entram nessa fila.

Essa separação é essencial.

---

# 156. Segurança contra lentidão

O `World` não deve:

```text
serializar JSON
esperar browser
esperar Cloudflare
esperar conexão HTTP externa
```

Fluxo:

```text
World
↓
cria snapshot/result
↓
responde channel
↓
Control API serializa fora do World
```

---

# 157. Timeouts

Definir deadlines por ação e separar espera HTTP, admissão na fila e execução.
Fila cheia deve rejeitar com erro estável/retry orientado antes da execução;
limitar também trabalho por tick para preservar a justiça com comandos do jogo.
Respostas ao handler precisam ser limitadas e não bloquear o World quando o
cliente HTTP desconectar. Queries devolvem cópias/snapshots, não ponteiros vivos.

Estados propostos da operação:

```text
accepted → running → succeeded / rejected / failed
```

`202 Accepted` com promessa de acompanhamento exige registro durável e consulta
por `OperationID`, autorizada para o ator. Uma fila apenas em memória não prova
aceitação durável. Antes de implementar recuperação após restart, não prometer
que um comando enfileirado sobrevive à queda do processo.

Cancelamento antes da execução pode impedir a ação; após início/commit, timeout
do browser não implica rollback. Resultado desconhecido exige consulta e
reconciliação, não reenvio com chave nova. Retries usam a mesma chave e payload;
uma chave reutilizada com payload diferente deve ser rejeitada.

Em operações econômicas, efeito e registro do resultado/idempotência devem
compartilhar a transação durável. Para efeitos externos ou de lifecycle,
definir recuperação/reconciliação específica: um `RequestID` não oferece
garantia de execução exatamente uma vez. Shutdown/restart usam a operação
persistida do supervisor, não uma conexão HTTP mantida aberta indefinidamente.

---

# 158. Validação proporcional

Para o lote administrativo atual, os alvos Go e frontend já existem. O gate de
integração inclui testes, vet, builds, layout/links e `git diff --check`.

Cada lote Go executa testes dos pacotes alterados e consumidores; mudanças
em autorização/persistência usam PostgreSQL real de teste e rejeições,
rollback, retry e concorrência. No fechamento de integração, a partir do root:

```powershell
Push-Location .\wydgo748
try {
    go test -count=1 ./...
    if ($LASTEXITCODE -ne 0) { throw 'go test falhou' }
    go vet ./...
    if ($LASTEXITCODE -ne 0) { throw 'go vet falhou' }
    go build -o bin/wyd-web-check.exe ./cmd/web-api
    if ($LASTEXITCODE -ne 0) { throw 'build web falhou' }
    # Construir cmd/server quando o lote alterar o servidor/Control API.
    go build -o bin/server-check.exe ./cmd/server
    if ($LASTEXITCODE -ne 0) { throw 'build server falhou' }
} finally {
    Pop-Location
}
git diff --check
```

No frontend em `wydgo748/web/portal`: `npm ci`, geração/checagem do cliente
OpenAPI, checagem de tipos, testes do fluxo alterado e `npm run build`. O build
Astro isolado não comprova checagem de tipos ou compatibilidade de DTOs.

Testar o proxy em ambiente equivalente ao Pages e o browser de ponta a ponta:
cookies, CSRF, login/logout, acesso direto às rotas, mobile/teclado e isolamento
entre contas. Dados de produção não entram em fixtures. Testes ignorados por
falta de banco/serviço deixam o gate pendente.

Não recompilar C++ por uma edição web sem efeito no client. Se uma feature
interceptar contrato legado, aplicar as skills correspondentes e validar os
dois lados. Registrar separadamente `STATICALLY VERIFIED`,
`AUTOMATED TESTED` e `CLIENT_TESTED`.

---

# 159. Testes específicos

Adicionar cobertura para:

```text
authentication
session expiry
CSRF
RBAC/capabilities
guild leader authorization
moderator authorization
admin authorization

control API authentication

player snapshots
teleport
kick
ban
broadcast
event start

rollback
idempotency
reward claim

market escrow
market purchase

payment webhook duplication
audit logging

slow web client
control timeout
server unavailable
PostgreSQL unavailable
```

---

# 160. Load testing

Testar separadamente:

### Web

```text
ranking
login
public character search
market search
```

### Control

```text
status queries
player queries
moderate command rate
```

Comparar baseline e carga web com o mesmo cenário de jogadores, banco e
recursos. Métricas já existentes em
[`metrics.go`](../wydgo748/internal/game/metrics.go):

```text
world_tick_duration_micros
world_loop_lag_micros
world_command_queue_depth
world_command_queue_age_micros
world_tick_duration_buckets
```

Antes do gate de implantação, registrar duração do ensaio, concorrência,
volumes de dados e orçamento numérico de latência/erro por fluxo, além de
limites de p95/p99 do jogo. Não considerar “sem alteração relevante” um aceite
mensurável. Os limites dependem do baseline homologado e são uma decisão
pendente, não um benchmark executado nesta revisão. Gauges de último tick não
são percentis: usar deltas dos histogramas e amostragem compatível.

Exigir ausência de crescimento sustentado da fila, esgotamento do pool ou
starvation do World. Incluir login sob abuso, banco lento/indisponível,
timeouts do proxy, fila cheia e restart com resultado pendente. Verificar
também isolamento de CPU/memória/IO no host compartilhado e limites de custo
do proxy. Não publicar expvar/pprof pela API pública.

---

# 161. Roadmap de implementação

Cada fase entrega um fluxo utilizável e mantém as demais rotas ocultas.
Não criar todas as tabelas/módulos na fundação. A passagem de fase depende
dos gates abaixo, sem datas estimadas antes de medir o primeiro lote.

A execução começou pela fatia administrativa da fase 3 porque é a prioridade
operacional atual. O canal privado, snapshots read-only, autenticação staff,
capabilities, auditoria, shell `/admin` e as primeiras mutações autoritativas
de gameplay já estão implementados; validação
manual completa no browser/deployment permanece como gate pendente. As fases
0–2 continuam relevantes para site público, conta do jogador e projeções.

| Fase | Entrega e dependências | Gate de saída |
| --- | --- | --- |
| 0 — contratos e implantação | Layout da seção 135; OpenAPI mínimo; modelo de sessão; acesso restrito a contas; migrations; upstream HTTPS; Pages proxy; isolamento de previews | Homologação alcança o Go com autenticação de origem, GRANTs reais funcionam sem DDL no HTTP, health/readiness e rollback testados |
| 1 — MVP público e conta | Home, regras, download, news/changelog em Git; cadastro compatível; login/logout; overview e lista privada de personagens persistidos; revogação de sessões | Fluxo no browser completo, credenciais compatíveis com jogo, rejeição de acesso entre contas, CSRF/cache/limites testados; timestamps honestos |
| 2 — catálogo e projeções | Exportador começando por itens/skills; rankings/perfis/agenda somente após seus projetores; expansão progressiva para drops/quests/crafting | Export determinístico, IDs/links válidos, campos públicos aprovados, fonte/versionamento/atraso visíveis e reconstrução demonstrada |
| 3 — fundação administrativa e Control API | Canal privado read-only para snapshots; `/admin` com sessão staff, capabilities, auditoria e comandos tipados enfileirados no World. Drop global, quiz e summon de boss são o primeiro lote mutável | Sem ponteiros do World fora do loop; autorização revalidada antes da mutação; negação de serviço/ator indevido, saturação, timeout e desconexão sem bloquear ticks; browser validado |
| 4 — identidade e moderação | Troca/recuperação de senha com contrato da seção 72; kick/message/broadcast; ban/mute com enforcement no jogo | Autosave/login concorrentes e restart preservam credencial/punição; autorização revalidada; resultado/auditoria recuperáveis |
| 5 — guild e demais operações de gameplay | Primeiro guild read-only; depois liderança/filiação offline, teleport, moderação, disparo de guerras e maintenance. Eventos adicionais e controles avançados de boss entram aqui quando exigirem novos contratos | Atomicidade entre contas/guilda, revalidação de cargo e estado, rejeições/lifecycle por comando; fluxo no client quando afetado |
| 6 — rastreabilidade e recompensas | Ledger/proveniência para os novos efeitos, registro de operações, auditoria durável e Reward Inbox/claim | Rollback, clique duplo, concorrência, inventário cheio e crash após commit sem duplicar/perder entrega; PostgreSQL real |
| 7 — pagamentos | Depende da fase 6; provedor em sandbox, webhook, reconciliação, direito à recompensa e estorno | Assinatura, valor/moeda, duplicação, ordem invertida, retry e recuperação cobertos; operação conciliável antes do checkout real |
| 8 — marketplace | Depende da fase 6; migração de custódia, escrow, compra/cancelamento/taxas/entrega; pagamentos não são dependência se usar só moeda do jogo | Conservação de itens/moeda, único comprador, conflitos com login/trade/autosave e reinício cobertos; sem escrita direta pela web |
| 9 — modalidades de guerra | Consultas podem entrar na fase 2; comandos exigem fases 3–5 e contrato da modalidade | Reutilizar torre/cidades existentes e cumprir gate client pendente; RvR/reinos/castelo têm escopo próprio |
| 10 — operação avançada | Supervisor, backup por job, CMS sem rebuild e editor de conteúdo são lotes independentes posteriores | Privilégios mínimos e restore ensaiado; decisão de renderização; loader/diff/reload validado por ferramenta |

Dashboards de economia podem crescer com a fase 6. Dados históricos começam
na ativação da coleta, não no início fictício de uma season. Direct grant,
remoção de moeda/item e força de restart só entram após seus contratos e
auditoria; não são atalhos para implementar rewards.

## Primeiro lote concreto

Concluído na fundação administrativa e no primeiro lote mutável:

1. `cmd/web-api` com health/readiness, sessão staff, senha da conta + PIN
   administrativo temporário configurado no servidor, CSRF, revogação,
   auditoria e conexão PostgreSQL dedicada read-only. TOTP/2FA permanece como
   endurecimento obrigatório antes de expor acesso de staff em produção.
2. Control API privada no `cmd/server`, loopback-only e protegida por token,
   com consultas serializadas pela fila autoritativa do `World`.
3. OpenAPI mínimo e shell Astro `/admin` com visão geral, jogadores online e
   Player Inspector com `LIVE STATE` do snapshot do `World` e `PERSISTENT STATE`
   sob demanda no PostgreSQL, com timestamps independentes e sem N+1 no overview;
   o dashboard também mostra diagnósticos operacionais tipados do processo com
   timestamp independente do snapshot do `World`.
4. Testes automatizados de auth, contrato, timeout/rejeições, static serving,
   headers, Host/método, projeção persistente, diagnósticos operacionais e
   resolução dos assets compilados.
5. `GET/POST /api/v1/staff/events/global-drop`,
   `GET/POST /api/v1/staff/events/quiz` e `GET/POST /api/v1/staff/bosses`, com
   as capabilities `game.event.global-drop`, `game.event.quiz` e
   `game.boss.summon`. Os efeitos são enfileirados no `World`, não escritos
   diretamente pelo HTTP. O quiz usa desafio server-issued com token por
   participante/rodada e prazo de 10 segundos, impedindo que replay de WPE ou
   abertura local da janela gere recompensa por si só.

### Execução local pelo tm.exe

O `cmd/server` agora inicia o painel dentro do próprio processo, sem subprocesso
`go run`, sem redirecionar o console e sem reiniciar outro servidor.
Classificação: `MODERNIZACAO_COMPATIVEL`; o protocolo do client não muda.

Configuração em `wydgo748/data/server.txt` (já habilitada no exemplo local):

```ini
web_admin_enabled=true
web_admin_address=127.0.0.1:8082
web_admin_staff=data/staff.json
web_admin_static=web/portal/dist
```

O padrão do código é desabilitado para configurações antigas. Para desativar
no arquivo local, use `web_admin_enabled=false` e reinicie o servidor.
Os caminhos relativos seguem o diretório de trabalho, assim como os demais
assets do jogo: execute o binário a partir de `wydgo748`.

Preparação após clonar ou alterar o frontend/servidor, a partir do root:

```powershell
.\tools\web-admin\Start-WYDAdmin.ps1 -NoBrowser
Push-Location .\wydgo748
go build -o tm.exe ./cmd/server
Pop-Location
```

Se o `tm.exe` antigo estiver aberto, encerre-o normalmente antes de substituir
o executável. O script nunca encerra processos. Ele instala dependências quando
necessário, compila o frontend e, se o painel já estiver disponível, pode abrir
o navegador (omita `-NoBrowser`). `-SkipBuild` apenas pula o build do frontend.

Uso diário: abra `wydgo748/tm.exe` e acesse
`http://127.0.0.1:8082/admin/`. Não é necessário rodar PowerShell, npm ou uma
segunda API a cada início. O navegador não abre automaticamente pelo servidor;
o endereço aparece no console. Entre com `felipetr`, a senha normal da conta e
o `admin_access_pin` já configurado. O PIN estático não é TOTP/2FA.

O painel integrado aceita apenas IP literal de loopback, mantém as capabilities,
CSRF, sessões e auditoria do handler existente. Snapshots e comandos passam pela
fila do `World`; somente os endpoints explicitamente tipados de drop global,
quiz e summon de boss podem mutar gameplay neste lote. Não precisa de
`WYD_CONTROL_TOKEN` nem da porta 8081. A Control API HTTP separada permanece
read-only, e `cmd/web-api` continua disponível para implantação separada.

Para PostgreSQL, usa um pool separado em modo de leitura, sem migrations nem
worker de escrita. Prefere `WYD_WEB_DATABASE_URL`; se ausente, usa a URL efetiva
do servidor (`database_url` ou a variável indicada por `database_url_env`).
Esse fallback local não transforma a credencial do jogo em uma role SQL de
privilégios mínimos. Implantação separada continua exigindo role dedicada,
HTTPS e endurecimento de autenticação antes de disponibilizar acesso público.

Porta ocupada, assets ausentes ou falha no banco do painel são informados no
console sem encerrar o jogo; corrija a causa e reinicie normalmente para tentar
novamente. No Ctrl+C, o HTTP drena as requisições, fecha o pool web e o servidor
persiste o jogo antes de sair. Fechar à força o processo não garante persistência.
As sessões web são em memória e exigem novo login após reiniciar.

Validação deste lote: testes de configuração, loopback, porta ocupada sem afetar
seu dono, adaptador de consulta/cancelamento e start/stop HTTP; integração opt-in
`TestStartWebAdminPostgres` com banco real em leitura (`WYD_WEB_TEST_DATABASE_URL`),
assets de teste, health/readiness, sessão e rejeição de overview anônimo.
Isso não substitui o teste manual do login e do estado live no browser com o
`tm.exe` atualizado. Drop global, quiz e summon de boss já formam o primeiro
lote mutável; os demais comandos continuam sujeitos aos gates das fases abaixo.

---

# 162. MVP recomendado

MVP = fases 0 e 1 concluídas:

- Público: Home, informações/regras, instruções de instalação, download com
  versão/hash e notícias/changelog publicados por build.
- Conta: cadastro, login/logout, resumo, lista privada de personagens com
  `asOf`, consulta/revogação de sessões.
- Operação: health/readiness, logs estruturados, limites e procedimento de
  implantação/rollback. O painel administrativo mutável já existe como trilha
  operacional paralela, mas não é dependência do MVP público das fases 0 e 1.

Critérios de aceite: usuário cadastra uma conta compatível com o client,
autentica no portal, vê somente seus personagens, encerra/revoga a sessão e
recebe erro recuperável quando um serviço está indisponível. Backend nunca
expõe payload bruto de conta; frontend não promete estado live.

Database ampla, rankings, guild management, progression advisor, suporte
integrado, demais mutações administrativas, troca/recuperação de senha, rewards,
pagamentos, market e comandos de guerras continuam no backlog por fase. A
fundação Control API/admin e o primeiro lote tipado de mutações existem; novas
funções só aparecem ao cumprir o gate correspondente.

---

# 163. Resultado arquitetural final

```text
                     WYD WEB PLATFORM

                           │
                 Cloudflare Pages
                           │
                    Astro Static
                           │
                   Pages API Proxy
                           │
                           ▼
                       wyd-web
                           │
            ┌──────────────┴──────────────┐
            │                             │
            ▼                             ▼
       PostgreSQL                  Control Client
                                          │
                                          ▼
                                  ┌───────────────┐
                                  │  wyd-server   │
                                  │               │
                                  │ Control API   │
                                  │      │        │
                                  │      ▼        │
                                  │    World      │
                                  └───────────────┘
```

Fontes de verdade:

```text
Gameplay online       → World
Gameplay persistente  → PostgreSQL
Conteúdo              → data/
Editorial             → web domain / Git
Visualização          → Astro
```

O portal nunca vira uma segunda implementação do jogo.

Ele funciona como **interface web sobre as fontes autoritativas que já existem**.
