# WYD-Go 7.48 — plano de implementação

Atualizado em 11/08/2026. HEAD de referência: `7ccfdd1`.

Este é o único arquivo de planejamento do projeto. O estado já entregue está em
`DOCS/IMPLEMENTED.md`. Planos históricos foram removidos para que uma tarefa
concluída não volte a aparecer como pendente.

## Regras permanentes

- Não inventar mecânica sem evidência no 7.48/7.54.
- Prioridade de pesquisa: implementação atual → W2PP → Secrets → Micronics/client.
- Portar comportamento, nunca layout/offset de outra versão.
- Manter o servidor autoritativo e o `World` como único escritor.
- Toda operação econômica segue `validar → mutar snapshot → persistir → publicar`.
- Conteúdo e coordenadas ficam em `data/`; invariantes permanecem em Go.
- Guild War, Kingdom War e Castle War ficam por último.

## Próximo passo recomendado — ledger econômico

O PostgreSQL preserva integridade, mas ainda não existe uma trilha append-only
capaz de responder quem criou, moveu, vendeu, transformou ou removeu um ItemUID.
Esse é o próximo trabalho recomendado antes de ampliar sistemas econômicos ou
abrir o servidor ao público.

### Escopo

1. Criar migration/schema para `economy_events`.
2. Registrar `event_id`, data, operação, conta, CharacterUID, contraparte,
   ItemUID, item index, delta de gold, origem, `correlation_id` e metadados.
3. Inserir o evento na mesma transação da mutação correspondente.
4. Cobrir trade, Loja Fantasma, NPC buy/sell, craft/refino, rewards raros,
   volatiles econômicos e futuras operações administrativas.
5. Criar uma ferramenta read-only para consulta por ItemUID, conta,
   CharacterUID e correlation ID.
6. Adicionar retenção/índices sem colocar consulta do ledger no hot path.

### Aceite

- commit bem-sucedido grava mutação e evento juntos;
- falha não deixa evento órfão nem mutação sem evento;
- um ItemUID pode ter sua cadeia de custódia reconstruída;
- testes PostgreSQL cobrem rollback, trade multi-account e transformação;
- nenhum dado do ledger é enviado ao client.

## Fase 1 — consolidação e validação in-game

Estas features existem no código, mas precisam de confirmação visual/multiplayer
antes de serem consideradas prontas para produção:

- cinco skills Sephira: Cannon, Thorn Wall, Resurrect, Cancellation e Spectral;
- sete compositores: sucesso, falha, consumo, aparência, relogin e rollback;
- Uxmal completo;
- Spirit's Seal: hover, extração, seleção e reentrada;
- criação/troca Celestial e SubCelestial, Soul, Cythera e travas;
- gems e Adamantita em armas/armaduras elegíveis;
- quatro bosses: shield, adds, mensagens e mudança de fase;
- montarias: incubação real, morte visual, revive e temporizadores;
- adesão/saída de Akelonia/Hekalotia e promoções de capa;
- objetos permanentes: visual, rotação e colisão;
- movimento remoto com dois clients em latência real;
- shutdown gracioso real no Windows e no Ubuntu.

Resultado de cada caso deve registrar: pré-condição, ação, resposta do client,
estado server-side, relogin e comportamento após falha de persistência.

## Fase 2 — correções do client 7.48

### HP/MP wide

O servidor e o pacote extended usam os valores corretos, mas a barra do client
ainda pode desenhar HP/MP altos quase zerados. Próxima investigação:

1. breakpoint na escrita do sidecar wide;
2. confirmar todos os produtores de CurHP/MaxHP/CurMP/MaxMP;
3. identificar a rotina da barra e sua escala;
4. criar novo patch independente, com SHA guard e teste de bytes;
5. atualizar `client748/PATCHES.md` e reproduzir a cadeia completa.

Não alterar o servidor para compensar um erro exclusivamente visual.

### Macro de combate 7.48

O `0x39D/96` C→S observado no macro continua como compatibilidade temporária.
A descompilação estática confirma que macro e clique manual chegam ao mesmo
construtor nativo, que seleciona `0x39D/48`, `0x39E/52` ou `0x36C/96` por
`SkillData.MaxTarget`; portanto não há evidência para trocar o builder S→C de
skill single nem para inventar o conteúdo da cauda de 96 bytes.

Próxima validação:

1. capturar plaintext da mesma skill/alvo por clique manual e macro;
2. comparar integralmente `@00..47` e `@48..95`;
3. localizar no executável o ponto que altera o tamanho efetivamente enviado;
4. fazer o macro reutilizar o caminho canônico ou aplicar adapter mínimo;
5. somente após distribuir o client corrigido remover `0x39D/96` do servidor.

### Guildmark

- publicar os BMP 16x12 em host controlado;
- localizar/confirmar a URL usada pelo client 7.48;
- aplicar patch reprodutível somente se o host nativo não puder ser usado;
- validar cache, atualização e visual para membros/observadores.

### Assets

Remasterização de texturas, modelos e interface continua opcional e separada das
correções funcionais.

## Fase 3 — quests e conteúdo retail

O arquivo atual possui 18 quests configuradas. A expansão deve portar somente
cadeias confirmadas nas fontes e reutilizar os motores já existentes.

### Trabalho

- inventariar NPCs/itens/flags presentes no catálogo 7.48;
- separar quest NPC-driven, quest por item e instância;
- adicionar objetivos/progresso persistente apenas quando a fonte exigir;
- ligar drops, diálogos, requisitos, recompensas e teleporte aos dados;
- validar referências de NPC, item, mapa e quest anterior no boot;
- não recriar Water, Uxmal, Nightmare, Cube ou Hell Gate dentro do motor de quest.

### Aceite

- cada quest possui fonte e versão registradas;
- requisito usa nível interno base zero;
- repetível não suja `QuestsDone`;
- consumo/recompensa/charstate participam da mesma transação;
- inventário cheio segue a regra nativa da recompensa;
- relogin e falha do banco não duplicam progresso ou item.

## Fase 4 — Big Cube retail

O núcleo O/X já existe. A cadeia completa está bloqueada porque as fontes locais
não contêm uma tabela inequívoca de rodadas.

Antes de implementar, obter e validar:

- coordenadas e estados dos portões;
- salas, rodadas, NPCs e quantidades;
- perguntas, respostas e plataformas;
- regras de eliminação, tempo e reset;
- requisitos e recompensas;
- objetos permanentes envolvidos.

Quando a tabela for confirmada, o trabalho deve ser data-driven sobre o
`state_machine` existente. Não criar uma cadeia especulativa em Go.

## Fase 5 — bosses avançados

Implementar apenas quando um encontro real exigir:

- timers declarativos;
- seletores de alvo;
- arena/leash próprio;
- reset completo do encontro;
- cancelamento/substituição de ações pendentes;
- novas condições/ações source-confirmed;
- métricas por encontro.

Manter boss como `Mob` comum com runtime paralelo e sem goroutine própria.

## Fase 6 — sistemas sociais pendentes

- carta offline durável;
- ranking e arenas 1x1/5x5/10x10;
- sub-guild e alianças completas;
- operações de guild com membro offline quando a regra nativa permitir;
- painel administrativo separado do chat comum;
- mute, ban, kick, inspeção de UID e ações com operador/motivo.

O control plane deve escutar apenas em loopback/Unix socket ou atrás de proxy
autenticado, com RBAC e integração ao ledger.

## Fase 7 — operação pública

- importar e revisar feeds CIDR de VPS/VPN/datacenter;
- configurar firewall/SYN flood/file descriptors no Ubuntu;
- readiness real e política de overload shedding;
- métricas de conexão, auth, rejeições, commits, rollback e economia;
- logs estruturados por nível e categoria;
- backup e teste periódico de restauração PostgreSQL;
- carga de 500 e 990 jogadores com combate, movimento, reconnect e economia;
- medir p95/p99 do tick, fila e commit crítico;
- executar `go test -race ./...` no Ubuntu/CI.

Detecção de VM/HWID é futura telemetria client-side e nunca substitui a proteção
server-side.

## Fase 8 — guerras (última fase)

### Guild War

- desafio `0x28E/0x28F` e confirmação `0xE0E`;
- agenda, taxa, participantes, placar, vitória e persistência;
- torres, zonas e regras de desconexão/reentrada;
- auditoria econômica e anti-packet replay.

### Kingdom War

- guerra Akelonia × Hekalotia;
- elegibilidade por cidadania/capa;
- objetivos, placar, premiação e reset;
- sincronização do reino para observadores.

### Castle War

- território, portões, torres e objetos permanentes;
- owner, tributação, agenda, vitória e transição atômica;
- recuperação após restart sem estado dividido.

As três guerras devem compartilhar infraestrutura de agenda, participantes,
placar e resultado, mas conservar regras próprias.

## Fase 9 — escala futura

Somente depois de profiling real:

- scheduler por deadline para registries ainda varridos;
- AOI incremental em áreas densas;
- commits críticos pendentes sem permitir concorrência sobre o mesmo agregado;
- canais configuráveis e, posteriormente, múltiplos processos/canais;
- Lua versionada/hot reload apenas para conteúdo seguro e validado.

Não introduzir ECS, goroutine por mob, microservices ou mutex por subsistema.

## Itens deliberadamente não implementados

- uso direto do item 3455 sem handler nativo confirmado;
- opcodes de versões posteriores ausentes no client 7.48;
- crafting W2PP cujo NPC/item não existe no catálogo 7.48;
- tabela inventada do Big Cube;
- confiança em MAC/HWID enviado pelo client;
- fallback automático PostgreSQL → JSON;
- WaterMacro client-side.

## Definition of Done

Uma feature só sai deste plano quando:

- a regra estiver confirmada na fonte adequada;
- o servidor revalidar toda entrada do client;
- mutação durável ocorrer antes da publicação;
- rollback integral estiver testado;
- logout/relogin/restart preservarem o estado quando aplicável;
- owner e observers receberem os pacotes corretos;
- visibilidade/gameplay space não vazarem;
- testes, vet, build e `git diff --check` passarem;
- a validação in-game necessária estiver registrada;
- `DOCS/IMPLEMENTED.md` for atualizado no mesmo commit.
