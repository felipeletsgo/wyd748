# WYD-Go 7.48 â€” arquitetura e roadmap

Documento de direÃ§Ã£o do emulador em `wyd-go/`. O estado comprovado em cÃ³digo e
testes estÃ¡ em `wyd-go/DOCS/IMPLEMENTATION_STATUS.md`; detalhes byte a byte ficam
em `wyd-go/DOCS/EMULATOR.md`.

## Objetivo arquitetural

O servidor Ã© modular e server-authoritative. Alterar uma fÃ³rmula, uma skill ou
um sistema nÃ£o deve exigir ler milhares de linhas nem confiar em cÃ¡lculo do
client. As fronteiras atuais sÃ£o:

```text
cmd/server          composiÃ§Ã£o e configuraÃ§Ã£o
internal/model      domÃ­nio puro
internal/wire       protocolo 7.48 byte a byte
internal/net        sockets, sessÃ£o, filas
internal/data       catÃ¡logos, NPCGener, terreno, templates
internal/store      persistÃªncia de contas
internal/account    cadastro, autenticaÃ§Ã£o e hash
internal/game       ator World e sistemas de jogo fragmentados
```

Uma Ãºnica goroutine de `World` serializa mutaÃ§Ãµes. Isso fornece ordem natural
para inventÃ¡rio, loot, trade, party e combate e reduz a superfÃ­cie de dupe. I/O
de socket e leitura de login ficam fora do caminho crÃ­tico; mensagens retornam
ao ator por comandos.

JSON continua disponÃ­vel apenas como adaptador explÃ­cito de desenvolvimento.
PostgreSQL Ã© a implementaÃ§Ã£o autoritativa de `store.Store`, sem consultas no
caminho quente de movimento/combate. Lua poderÃ¡ ser incorporado depois para regras que
precisem hot reload (quests, eventos, NPCs e tabelas de balanceamento), mas nÃ£o
deve substituir o estado persistente nem os invariantes Go.

## Estado funcional consolidado

- conta via CLI/API, PBKDF2, PostgreSQL e bloqueio de login duplo;
- criaÃ§Ã£o de atÃ© quatro personagens, nickname ASCII Ãºnico de 4â€“13 letras e
  templates das quatro classes;
- login/enter-world completo, autosave de 3 s e transaÃ§Ãµes PostgreSQL;
- Ã¡rea de interesse Â±32 para players, mobs, NPCs e itens do chÃ£o;
- movimento interpolado, velocidade pÃºblica, colisÃ£o e terreno nativo;
- NPCGener persistente no mundo, patrulha, respawn e populaÃ§Ã£o por gerador;
- stats/equipamentos server-side, distribuiÃ§Ã£o de pontos, EXP e level-up;
- combate playerâ†”mob, mobâ†”player e PvP de skills, morte, recall e reentrada;
- party de atÃ© 13 membros, ordem estÃ¡vel e EXP integral com +2% por membro;
- inventÃ¡rio visÃ­vel de 63 slots, drop/loot, loja, consumÃ­veis e volatiles
  (124 de 124 cÃ³digos `EF_VOLATILE` com contrato explÃ­cito â€” restauro, gold,
  teleporte, buff/comida, EXP, salvar/voltar posiÃ§Ã£o, transformaÃ§Ã£o de rosto,
  refino Ori/Lac/Molar, tintura/repliction, o sistema completo de montaria,
  chaves de portÃ£o, contadores, juras do Arch, Magic Chamber, Nightmare,
  Cube/Hell Gate, acessos temporizados e caixas; Celestial/SubCelestial usa
  Ideal/Fury/Mysterious Stone e somente a extraÃ§Ã£o 206 segue bloqueada sem consumo; detalhes em
  `DOCS/IMPLEMENTATION_STATUS.md` Â§ConsumÃ­veis);
- contadores nomeados por personagem (`kefra_ticket`, `fame`) sobre o charstate,
  com ordem transacional sidecar-antes-da-conta;
- catÃ¡logo server-side de itens e skills;
- compra/aprendizado, atalhos, buffs, debuffs, passivas, summons e execuÃ§Ã£o das
  96 skills bÃ¡sicas com caminhos separados por classe;
- efeitos pÃºblicos por CreateMob/0x336 e Ã­cones privados por 0x3B9.
- Arch, Celestial e SubCelestial, incluindo progressÃ£o por forma, pontos
  cruzados, travas 40/90, Soul, Cythera e troca em cidade.

## Invariantes de protocolo

1. Todo pacote Ã© construÃ­do manualmente em `internal/wire`; structs Go nunca sÃ£o
   despejadas diretamente no socket.
2. `CreateMob` leva Score@100 e Affect[16]@66.
3. `UpdateScore 0x336` leva 16 affects compactos @42..73 e Ã© pÃºblico.
4. `UpdateAffect 0x3B9` leva 16 affects completos, usa tempo em blocos de 8 s e
   Ã© privado ao dono para Ã­cones/timers.
5. InventÃ¡rio mantÃ©m 64 estruturas no wire, porÃ©m o slot 63 Ã© reservado.
6. Respostas de skill usam 0x39D (1), 0x39E (2) ou 0x36C (atÃ© 13 alvos).
7. O client envia intenÃ§Ã£o; preÃ§o, dano, efeito, cooldown, alcance, alvo,
   propriedade e posiÃ§Ã£o sÃ£o revalidados no servidor.

## PrÃ³ximas fatias de features

### 1. Fechar e estabilizar as 96 skills

- montar uma matriz de teste por conta/classe e skill, registrando: conjuraÃ§Ã£o,
  alvo, visual, dano/cura, affect, duraÃ§Ã£o, cooldown, MP e persistÃªncia;
- comparar cada divergÃªncia com W2PP/7.59 e portar apenas a fÃ³rmula;
- completar efeitos visuais especÃ­ficos ainda sem equivalÃªncia confirmada;
- adicionar testes determinÃ­sticos por famÃ­lia de skill e expiraÃ§Ã£o de affect.

CritÃ©rio de pronto: cada Ã­ndice 0..95 possui comportamento funcional confirmado
no client, teste server-side e nenhuma corrupÃ§Ã£o de ExtendedScore/Chaos/pontos.

### 2. Trade seguro entre jogadores

- sessÃ£o de trade com estados invite/open/locked/confirmed/cancelled;
- reservar itens e gold enquanto a janela estiver aberta;
- validar novamente ambos os lados no commit Ãºnico;
- cancelar em distÃ¢ncia, morte, desconexÃ£o ou mudanÃ§a de inventÃ¡rio;
- persistir as duas contas antes de confirmar visualmente.

CritÃ©rio de pronto: testes de corrida/desconexÃ£o nÃ£o duplicam nem perdem itens.

### 3. Quest, crafting e progressÃ£o avanÃ§ada

- **volatiles Fase D concluÃ­dos localmente**: contratos 41-43/46-48, livros
  Sephira 32-36 nos bits 25-29 de `LearnedSkill`, HP temporizado 58, Magical
  Pill 6 e Hunting Scroll 195. Falta somente a bateria visual in-game no 7.48;
- **craft implementado para os sete NPCs tipo 8** (Tiny, Lindy, Compositor,
  Agatha, Aylin, Ehre, Alquimista Odin) â€” ver `DOCS/CRAFTING.md`. Falta a
  matriz de validaÃ§Ã£o visual in-game receita por receita. A cápsula Spirit
  Seal para Celestial/SubCelestial já está implementada; falta somente a
  validação visual do fluxo no client (`DOCS/CELESTIAL_CAPSULE.md`);
- **cadeia Mortal principal implementada localmente**: cinco NPCs-portais
  repetÃ­veis, reset global das Ã¡reas a cada 10 minutos e caixas 4117â€“4121 com
  EXP/gold transacionais. Falta a validaÃ§Ã£o in-game de cada tier;
- **portÃ£o das cinco Ã¡reas ajustado para o client 7.48**: as quests 1/3/4/5/6
  validam somente evoluÃ§Ã£o Mortal e faixa de level. Os itens de entrada
  4038â€“4042 nÃ£o sÃ£o exigidos nem consumidos;
- ampliar o catÃ¡logo alÃ©m da cadeia Mortal; `QUEST_CAPAREAL` e a troca da Helen
  jÃ¡ foram incorporadas ao motor configurÃ¡vel. **Lacuna**: o item 4125
  (`Balance's_Piece`) nÃ£o tem fonte, entÃ£o a quest da Helen Ã© inalcanÃ§Ã¡vel â€” no
  nativo ele nÃ£o estÃ¡ no `MobDropList` e Ã© concedido por um caminho que
  teleporta quem o pega (`DropControl.cpp:90`), ainda nÃ£o portado;
- **cadeia do novato completa**: o passo 1 (`Trainer1`) exige a First Gate Key
  (451) sem consumi-la e entrega 60 Healing Potion. Os passos 2 e 3
  (`Trainer2`/`Trainer3`) deixaram de nÃ£o caber: `QuestRewards.Refine` foi
  criado para a recompensa deles â€” refinar o equipamento **vestido**, arma para
  +7 e todas as peÃ§as para +6, como o nativo grava em `EF_SANC`;
- **sÃ£o 18 quests configuradas**, contra 8 na rodada anterior;
- **correÃ§Ã£o de levantamento**: `KRUNO`, `QUEST_BENCAODEDEUS`,
  `QUEST_EQUILIBRIO`, `CAPAVERDE_TELEPORT` e `MOLARGARGULA` **nÃ£o** sÃ£o
  data-only como eu havia registrado. Os NPCs que as hospedam nÃ£o existem no
  nosso 7.48 â€” sÃ£o adiÃ§Ãµes do W2PP (ex.: `Sacerdote_Kruno` em 2664,1970). Nas
  quatro regiÃµes de destino o nosso mundo sÃ³ tem monstros. PortÃ¡-las exige
  criar NPC novo (JSON + NPCGener), nÃ£o sÃ³ editar `quests.json`;
- o `Survivor` foi portado com o motor de contadores: cobra uma entrada de
  Kefra por visita e, com o saldo **zerado**, troca o selo 4127 por 100 entradas
  (`recharge`). Dos NPCs de quest restantes, `Mestre_de_Hab` e `Uxmal` tÃªm
  sistemas prÃ³prios â€” o `Uxmal` depende do motor de instÃ¢ncia, ainda inexistente;
- **objetos permanentes de mundo implementados**: `data/init_items.csv` porta os
  50 objetos do `InitItem.csv` nativo (portÃµes de castelo, as quatro portas de
  teste, Guild Battle Gate, canhÃµes, Torre do Triunfo). Entram no mapa no boot,
  nÃ£o expiram e nÃ£o podem ser recolhidos. Falta validar a aparÃªncia in-game â€”
  em especial a rotaÃ§Ã£o e se o client colide com eles;
- validar in-game a adesÃ£o/saÃ­da de Akelonia e Hekalotia, a promoÃ§Ã£o
  545/546â†’543/544, as conversÃµes Elite/Hero/Master, Lindy e os comandos
  `/kingdom` e `/king`; Castle/Guild War fica por Ãºltimo;
- **Arch implementado** (`DOCS/ASCENSION.md`): Sefirot, Pedra Eterna, ascensÃ£o
  no rei com nÃ­vel mÃ­nimo 370 e orÃ§amento de pontos proporcional ao nÃ­vel do
  Mortal, recalculado a cada login; **caps 355/370** barrando a EXP e
  destravados na Lindy (o 370 cobra 1 de fama); **quatro juras elementais** a
  100 milhÃµes de EXP cada, em ordem. Falta validaÃ§Ã£o in-game.
  **Celestial/SubCelestial** já possuem a cápsula Spirit Seal e os handlers
  `0x2CD`/`0x3CC`; falta somente a validação visual in-game.
- **encontros de boss implementados** (`DOCS/BOSS.md`): quatro tipos hardcoded
  configurados por `data/boss/*.lua`. Timers, seletores de alvo e arena ficam
  para quando um encontro precisar;
- estrutura bÃ¡sica de guild jÃ¡ implementada; guerra, zonas, torre e alianÃ§as pendentes.

As regras configurÃ¡veis sÃ£o candidatas a Lua com carregamento versionado e troca
atÃ´mica. O runtime Go continua dono de inventÃ¡rio, transaÃ§Ãµes e validaÃ§Ãµes.

### 4. Escala e operaÃ§Ã£o

- grid espacial 16x16 e ativaÃ§Ã£o de mobs por proximidade implementados; manter
  auditoria das buscas lineares restantes fora do caminho quente;
- fila assÃ­ncrona/coalescida de persistÃªncia sem violar a ordem do ator;
- **mÃ©tricas de tick, fila e sessÃµes implementadas** via `expvar`, com pprof
  restrito a loopback e desligamento controlado (`DOCS/OPERATION.md`); falta
  medir latÃªncia de save;
- implementaÃ§Ã£o SQL de `store.Store`, migraÃ§Ã£o e backup;
- separaÃ§Ã£o futura em canais/mapas somente depois de medir o limite de um World.

## Ordem recomendada imediata

1. **Resolver a barra de HP/MP do client.** Ã‰ o Ãºnico bug aberto que atrapalha
   qualquer outro teste in-game: com HP wide alto as duas barras desenham quase
   zeradas. O servidor estÃ¡ coerente (`/hpdebug`), e todos os pontos de leitura
   do patch apontam para o sidecar â€” a suspeita Ã© o sidecar nÃ£o ser alimentado
   em algum caminho. DiagnÃ³stico Ã© OllyDbg com breakpoint na escrita do sidecar.
2. **Validar o Arch in-game** (`DOCS/ASCENSION.md`): o NPC de teste `ShopArch`
   em Armia (2100,2102) vende os insumos, os atalhos e os itens de EXP.
   Lembrar de removÃª-lo antes de abrir para jogadores.
3. Percorrer a matriz de lojas de teste de Armia: `ShopVolTest1`–`ShopVolTest7`
   cobrem todos os itens voláteis, enquanto `ShopFairy`, `ShopMt*`,
   `ShopCostume` e `ShopFiral` cobrem fadas, montarias, temporizadores e trajes.
   As lojas funcionais continuam confirmando as regras nativas; nenhuma lista de
   teste substitui a autoridade de `itemlist.csv`/`volatiles.json`.
4. Concluir a matriz visual in-game das cinco skills Sephira e das sete
   receitas de craft (`DOCS/CRAFTING.md`).
5. Validar os quatro bosses no client (escudo, adds, mensagens de fase).
6. **Motor de instÃ¢ncia** (`Uxmal`, Chaos Cube, Nightmareâ€¦): registro de party
   por sala, ticket de entrada sobre os contadores, temporizador e expulsÃ£o. SÃ£o
   23 das 41 quests do catÃ¡logo do client â€” Ã© um projeto, nÃ£o uma tarefa. O
   motor de contadores, que Ã© a base do ticket, jÃ¡ estÃ¡ de pÃ©.
7. Validar no client a cápsula Spirit Seal e os fluxos Celestial/SubCelestial.
8. Validar no client os cinco tiers Mortais e depois expandir o catÃ¡logo de quests.
9. Guildmark no servidor foi concluÃ­da: `Guild`/`GuildLevel` sÃ£o projetados
   nos pacotes nativos e `Guilds.txt` Ã© exportado. Implementar Guild War/Castle
   War somente depois de validar a textura no client 7.48.

## Fontes de referÃªncia

- `SERVER W2PP/SOURCE GAME`: verdade sobre leitura e apresentaÃ§Ã£o no client;
- `SERVER W2PP/SOURCE SERVER` e `source 7.59`: algoritmos e fÃ³rmulas completas;
- `SOURCE(secrets) para referÃªncia/DescompilaÃ§Ã£o`: comportamento prÃ³ximo do 7.54;
- dumps reais do Micronics: confirmaÃ§Ã£o final dos offsets 7.48.

Regra permanente: portar algoritmos, nunca layouts de struct, offsets de memÃ³ria
ou endereÃ§os de outra versÃ£o.

## Roadmap reconciliado — 02/08/2026

Este bloco substitui a ordem histórica acima quando houver conflito com ela.

### Fase 0 — consolidação

- Instâncias, Big Cube O/X, NPCs de craft, quests da cadeia Mortal, Arch,
  Celestial/SubCelestial, reinos, party, loja fantasma, UID de item e proteção
  de pacotes estão implementados no código atual.
- A validação restante desses sistemas é in-game e não deve ser confundida com
  uma lacuna de implementação.

### Fase 1 — cápsula Celestial (entregue nesta rodada)

- `3443 Spirit's Seal` agora encapsula Celestial/SubCelestial com snapshot
  completo, UID persistente, item codificado e commit atômico.
- `0x2CD` consulta o selo e `0x3CC` materializa a ficha em slot livre. O fluxo
  remove o personagem do mundo somente depois do commit e não duplica item,
  ficha ou charstate.
- `3455 Extraction_of_Magical_Power` continua `no_direct_use`, pois o W2PP não
  o aceita no handler de extração; não foi criada uma semântica inventada.
- Detalhes e testes: `DOCS/CELESTIAL_CAPSULE.md`.

### Próxima ordem

1. Validar com dois clients o movimento remoto por rota e os fluxos in-game de
   Uxmal, Spirit's Seal, Celestial/SubCelestial, gemas e Adamantita.
2. Completar a tabela nativa de perguntas/recompensas do Big Cube; o motor O/X,
   a audiência privada e a expulsão para Armia já estão implementados.
3. Expandir as quests complementares e o catálogo retail sem duplicar motores
   de instância já existentes.
4. Corrigir e validar no binário a UI wide de HP/MP e publicar o host/patch da
   textura de guildmark.
5. Implementar Guild War, Kingdom War e Castle War por último, com estado,
   persistência e auditoria próprios.

Toda nova fatia continua seguindo `validar → mutar cópia autoritativa →
persistir → publicar`; a interface JSON é somente adaptador de desenvolvimento
 e o PostgreSQL permanece a fonte de produção.

## Atualização de execução — 02/08/2026

Uxmal/Pista de Runas, gemas/Adamantita, recompra, protocolos sociais e a
identidade server-side de guild saíram da lista de implementação pendente. A
ordem de trabalho atual é:

1. concluir a validação in-game de Uxmal, movimento remoto, Spirit's Seal,
   Celestial/SubCelestial, gemas/Adamantita e quests já confirmadas na fonte;
2. [entregue em 02/08/2026] adicionar os handlers sociais 0x3D5 (convite de
   guild) e 0x3E8 (lista de recompra), com limites de tamanho e persistência
   autoritativa;
3. [entregue em 02/08/2026] publicar a identidade visual de guild: `Guild`
   empacotado em 12 bits, `GuildLevel` em CreateMob/UpdateScore/EnterWorld,
   sincronização sem recriar movimento e exportação de `Guilds.txt`; a textura
   BMP continua sendo baixada pelo client do host hardcoded;
4. completar o catálogo nativo do Big Cube e a investigação da UI wide HP/MP;
5. deixar Guild War/Kingdom War/Castle War por último.

O runtime de Uxmal permanece configurável em `data/volatiles.json`; o NPC e a
loja de teste ficam em `data/npcs/ShopUxmal.json` e no gerador 5132 e devem ser
removidos ou desativados antes de uma abertura pública.

## Water e automação server-side — entregue em 09/08/2026

O encadeamento Water permanece modelado como instâncias independentes: Room 8
abre o boss somente pelo Boss Scroll, e o boss aceita o Room 1 Scroll durante a
exit grace para iniciar um novo RuntimeID. `ChainNextItem` expressa essa aresta
sem transformar o boss em stage interno. A validação de slot, posição,
EntryArea, tier, sequência, prazo e persistência continua server-side; Hunting
Scrolls não foi alterado.

O auto-avanço agora pertence exclusivamente ao servidor. Ao concluir uma sala,
o próximo pergaminho é materializado com UID server-side e persistido antes de
qualquer transição. Se o líder estiver com Silver Angel (`3914`) ativa em
`Equip[13]` naquele instante, o servidor usa somente esse UID recém-concedido
pela mesma rotina de domínio do clique manual e persiste a próxima sala antes
de publicar teleporte/spawn. Carry cheio, fada fora do slot, fada de outro
membro, equip posterior e reward zero não acionam automação; falha do segundo
commit conserva o pergaminho já durável. O WaterMacro client-side e os comandos
`/macropergaon`/`/macropergaoff` foram removidos; o macro normal de skills/buffs
permanece. Detalhes em `DOCS/WATER.md` e `client748/PATCHES.md`.

## Auditoria transversal de isolamento — entregue em 08/08/2026

Foi concluída a revisão que faltava entre instâncias privadas, zonas
compartilhadas e mundo público. `gameplaySpaceForPlayer` passou a ser a
fronteira única para visibilidade, aquisição de alvo, skills/AoE, IA, party EXP,
affects, summons, colisão e loot temporário. Monstros públicos não são
materializados nem atacam participantes de runtime; mobs de runtime, drops e
efeitos não atravessam a execução mesmo quando as coordenadas físicas se
sobrepõem.

Effects hostis persistidos agora carregam `OwnerCharacterUID`; um `ClientID`
reciclado nunca reassocia debuff antigo. Water, Nightmare compartilhado e Hell
Gate preservam membros por CharacterUID no restart e logout, enquanto salas
curtas continuam processuais. Alocadores de mob e Hell Gate ignoram entidades
de outros runtimes, mas respeitam terreno, NPCs e a própria sala.

Novos testes cobrem a matriz público/runtime, loot, party EXP, UID de affect e
colisão de Hell Gate. A validação desta rodada passou em `go test -count=1 ./...`,
`go vet ./...`, `go test -race ./internal/game`, build do servidor e
`git diff --check`.

### Complemento de isolamento de runtime — entregue em 08/08/2026

- A autoria de affect em mobs foi migrada do array paralelo por ClientID para
  `model.Affect.OwnerCharacterUID` (jogador) ou `OwnerID` de um mob vivo no
  mesmo GameplaySpace (efeito server-owned). A troca/reuso de sessão remove o
  efeito e não concede crédito; efeitos de mob que matam não geram EXP, gold ou
  loot.
- Toda alocação/movimento de jogador, mob, summon, Hell Gate e instância usa
  `positionOccupiedInGameplaySpace`. Terreno, NPCs globais e lojas fantasma
  continuam compartilhados; entidades dinâmicas de outros runtimes não
  bloqueiam uma posição privada.
- Limites de entradas compartilhadas contabilizam identidades únicas vivas e
  pendentes (`MemberCharacterUIDs`/`pendingInstanceMembers`). Reconectar um UID
  pendente converte a reserva para o novo ClientID sem consumir outra vaga.
- `playersByCharacterUID` fornece lookup O(1) no runtime, com fallback de scan
  somente para fixtures/estado legado. Testes de matriz, reuso de ClientID,
  efeitos mob-owned, colisão e reconexão foram adicionados.
- Validação concluída: `go test -count=1 ./...`, `go vet ./...`,
  `go test -race ./internal/game`, build do servidor e `git diff --check`.
