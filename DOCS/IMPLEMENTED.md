# WYD-Go 7.48 — estado implementado

Atualizado em 11/08/2026. HEAD de referência: `7ccfdd1`.

Este é o documento canônico do que existe no servidor. Regras ainda não
implementadas ficam exclusivamente em `DOCS/ROADMAP.md`. O código e os arquivos de
`data/` continuam sendo a autoridade quando houver divergência documental.

## Contratos arquiteturais

- `World` é o único escritor do estado mutável de gameplay.
- O client 7.48 é uma fronteira não confiável. Pacotes representam intenção;
  posição, alvo, item, preço, requisito, cooldown e resultado são recalculados.
- `ExtendedScore` v2 é a fonte de verdade dos atributos. O score legado existe
  apenas como projeção compatível com o protocolo.
- Operações econômicas seguem `validar → mutar snapshot → persistir → publicar`.
- Itens materializados possuem UID de 128 bits somente no servidor.
- `CharacterUID` é identidade durável; `ClientID` é efêmero e reutilizável.
- Consultas espaciais locais usam células, não scans globais player-triggered.
- PostgreSQL é o store autoritativo de produção. JSON é adaptador explícito de
  desenvolvimento e nunca é fallback silencioso.

## Organização

```text
cmd/                 composição dos executáveis
internal/model/      domínio persistente e validação
internal/wire/       ABI do client 7.48
internal/net/        TCP, sessão, framing e filas
internal/data/       loaders dos arquivos autoritativos
internal/store/      PostgreSQL e JSON de desenvolvimento
internal/account/    autenticação e criação de conta
internal/game/       sistemas de gameplay coordenados pelo World
data/                conteúdo e configuração server-side
client748/           client e cadeia reprodutível de patches
```

## Conta, autenticação e admissão

- Cadastro por CLI e API HTTP.
- Senhas PBKDF2-HMAC-SHA256 versionadas; texto puro é rejeitado.
- Login e criação de personagem usam layouts nativos do client.
- Até quatro personagens por conta.
- Nickname ASCII com 4–13 letras, case-insensitive e globalmente único, salvo o
  par Mortal/Arch nativo dentro da mesma conta.
- Uma conta não pode manter duas sessões simultâneas.
- Limites globais, por IPv4 e por IPv6 `/64` existem antes e depois do login.
- O padrão permite quatro clients autenticados por origem pública.
- Handshake, frame parcial e sessão ociosa possuem deadlines separados.
- Login possui limites por origem e conta antes do PBKDF2.
- `data/network_admission.json` aplica `allow`, `deny` ou `limit` por CIDR sem
  consulta externa no caminho de login.
- A Account API só confia em `X-Forwarded-For` vindo de proxy configurado e
  percorre a cadeia da direita para a esquerda.

## Personagem e progressão

- Templates das quatro classes ficam em `data/character_templates.json`.
- Entrada e reentrada normal usam `(2100,2100)`; última posição não é usada como
  spawn de login.
- Nível interno é base zero: `0` aparece como nível 1 e `399` como nível 400.
- EXP Mortal usa a tabela cumulativa nativa, piso/rate configuráveis e bônus de
  party integral.
- Stats, mastery e skill points possuem orçamento autoritativo por nível.
- Arch nasce com os bônus nativos de 168 skill points e 112 mastery points;
  somente Mortal precisa cumprir o requisito de nível ao comprar skill. Arch,
  Celestial e SubCelestial ainda validam classe, pontos, mastery, cadeia e gold.
- Celestial/Sub usam o orçamento W2PP de 1600 pontos iniciais mais a curva
  normal de 3/4 pontos por nível; mastery permanece em 855 por forma.
- Equipamentos são recalculados server-side ao equipar, remover, trocar ou
  descartar.
- HP, MP, STR, INT, DEX, CON, ataque, ataque mágico e defesa usam `uint32` no
  domínio e no pacote extended do client patched.
- Regeneração, morte, revive, recall e retorno à visibilidade são sincronizados
  para dono e observadores.
- Atributos, buffs e troca de equipamento atualizam score/estado sem recriar o
  avatar e sem interromper movimento.

## Mundo, terreno e movimento

- `HeightMap.dat` e `AttributeMap.dat` são autoritativos para altura, bloqueio e
  linha transitável.
- `NPCGener.txt` materializa NPCs e mobs permanentes no boot.
- Spawn inválido é reposicionado dentro de limite controlado; pequenas
  diferenças de altura transitáveis não bloqueiam a IA.
- Mobs patrulham, adormecem sem jogador próximo, acordam por área e possuem
  aquisição, perseguição, ataque, retorno e respawn.
- NPC com `Merchant != 0` nunca participa de combate.
- Players, mobs, summons, ground items e lojas fantasma possuem índices
  espaciais.
- Movimento valida origem, rota, velocidade, terreno e orçamento temporal.
- A posição física autoritativa avança ao longo da rota; o destino futuro não
  concede antecipadamente alcance para ataque, coleta, trade ou NPC.
- O `0x366` contínuo do client 7.48 pode replanejar a caminhada sem reiniciar o
  passo em curso; a cadência autoritativa é exatamente `RunSpeed` (1–6 tiles/s).
- Quando o próximo `0x366` começa alguns tiles adiante, o servidor reconstrói
  uma curva curta pelo terreno em vez de exigir linha reta; a ponte continua
  temporizada e limitada, portanto não concede teleporte nem speedhack.
- Passos de reconciliação já percorridos na tela usam a velocidade máxima 6;
  somente o trecho futuro permanece na velocidade real do personagem.
- O `ActionStop 0x367` preserva o pequeno trajeto `PosXY -> TargetXY` omitido
  da `Route[24]` pelo client; a parada também avança no relógio autoritativo,
  sem deixar o servidor atrás da posição visual nem conceder um salto imediato.
- Como no client nativo, entidades dinâmicas bloqueiam o destino final, mas não
  interrompem um trajeto apenas por ocuparem temporariamente um tile intermediário.
- Observadores recebem a rota validada para interpolar o movimento, evitando
  correções em forma de pequenos teleportes.
- Portais nativos resolvem a entrada pelo bloco 4x4 correspondente à célula
  `Teleport` do `AttributeMap`, em vez de exigir uma coordenada pontual.
- A área de interesse normal é ±32 tiles.

## Combate, skills e affects

- Ataques físicos melee e ranged, dano mágico, defesa, Double Hit, crítico,
  accuracy/evasion, resistências e overkill visual são calculados no servidor.
- A mesma regra de acerto vale para PvE, PvP, skills ofensivas, mobs, bosses e
  summons. Toda entidade parte de 100% de accuracy e 0% de evasion. A DEX do
  defensor gera até 80% de evasion em 4000 pontos e reduz diretamente a
  accuracy do atacante; dez pontos de item/montaria/buff equivalem a 1%. A
  accuracy nunca cai abaixo de 20%. Concentração acrescenta 15% e eleva seu
  piso para 35%.
- `/parry <player>` mostra em uma única linha a accuracy e a evasion do jogador
  contra o alvo online, sempre a partir dos dois scores autoritativos.
- LoS, range, gameplay space, alvo vivo e cooldown são revalidados. Mobs e
  skills de boss repetem a checagem de LoS no instante do dano.
- PvP não concede EXP nem gold do jogador morto.
- As 96 skills das classes e os cinco livros Sephira possuem caminhos de
  execução, custo, mastery, cooldown, efeitos e persistência.
- Buffs/debuffs usam affects públicos e timers privados do client.
- Macro normal de skills/buffs renova buffs dentro da janela configurada sem
  loop de recast.
- Summons BM e summons de contrato seguem o dono, herdam o gameplay space,
  atacam o alvo do dono/agressor e não entram na party.
- Summon de contrato é planejado antes do consumo; falha preserva item e summon
  anterior.
- Morte multi-alvo usa uma transação de recompensa; falha restaura personagens,
  mob e instância sem publicar prêmio parcial.
- Ações físicas possuem intervalo fixo de 400 ms. Attack Speed acima de 100%
  alimenta a progressão W2PP de Double Hit; em 200% todo ataque que acerta é
  Double. Critical é independente, pode coexistir, e os bits nativos fazem o
  client desenhar Double/Critical/MISS sem confiar em resultado C→S.
- Uma intenção física com alvo morto, ausente, fora de range ou sem LoS não
  consome o cooldown do próximo alvo válido. Skills continuam usando o `Delay`
  autoritativo e um piso curto independente contra busy-loop.
- Skills ofensivas usam os raios W2PP por `TargetType` (1, 2, 3 ou cone 3–6),
  limitam o resultado a 13 entidades, ordenam alvos por distância/ID e preservam
  skills nativas de dois alvos, multihit, summons e casos especiais.
- Skill single-target wide usa `0x39D/60` com `DMGX`, contador e dano uint32;
  o formato físico continua separado em `0x39D/52`.
- Os contadores em `/debug/vars` separam pacotes de skill aceitos/rejeitados e
  ataques físicos rejeitados para diagnosticar perda de hit sem inundar o log.

## Inventário, cargo e economia

- Equip possui 16 slots; `Equip[14]` é montaria e `Equip[15]` é capa.
- Carry possui 64 posições estruturais e 63 visíveis; o slot 63 é inacessível.
- Cargo possui 128 posições estruturais e 120 utilizáveis pelo jogador.
- Cada item ocupa uma célula no Carry e no Cargo do client patched.
- Compra, venda, split, delete, swap, equip, drop e pickup revalidam container,
  slot, item e UID autoritativos.
- Itens de loja preservam efeitos base do `itemlist.csv`, incluindo pilhas.
- Drop de inventário cheio pode cair no chão; ground loot é efêmero por decisão
  arquitetural.
- Gold mantém o limite nativo de 32 bits assinado.
- `/limparinv`/`/clearinv` limpa somente o Carry visível e respeita itens cuja
  destruição é bloqueada.
- `/fame`, `/cp`, `/chaos`, `/parry <player>` e os demais diagnósticos consultam
  somente estado server-side.

## NPCs, lojas, trade e crafting

- Lojas comuns, lojas de skill, Cargo Keeper, mestres, NPCs de quest e NPCs
  especiais usam o mesmo limite autoritativo de interação.
- Loja Fantasma usa itens do Cargo e um clone fixo enquanto o dono continua
  jogando.
- Trade entre jogadores é transacional, preserva UIDs, gold e cápsulas
  Celestial e recusa store sem commit multi-account.
- Recompra mantém dez entradas autoritativas com preço recalculado.
- Os sete compositores tipo 8 estão implementados: Tiny, Lindy, Compositor,
  Agatha, Aylin, Ehre e Alquimista Odin.
- Receitas validam os oito itens enviados contra snapshots do servidor, cobram
  gold/fama quando aplicável, usam uma única rolagem e persistem antes da
  resposta `0x3A7`.
- Lindy implementa os destraves internos 354/369; somente o segundo consome uma
  fama.
- Skill Master reseta skills pelos materiais aceitos; Mount Master revive
  montarias mortas.
- Lojas de teste em Armia e junto aos artesãos fornecem itens para o beta; devem
  ser removidas/desativadas antes da abertura pública.
- Durante o beta, `data/NPCGenerTest.txt` é carregado por padrão junto do
  `NPCGener.txt`, mantendo em Armia as lojas e NPCs necessários aos testes. A
  retirada antes da abertura pública será uma decisão manual de operação.

## Itens, volatiles e montarias

- `itemlist.csv`, `ItemName.csv`, `ItemEffect.h`, `SkillData.csv`,
  `volatiles.json`, `repliction.json` e `mounts.json` são autoritativos.
- Todo item real com `EF_VOLATILE` resolve para regra explícita; nenhum cai em
  handler genérico silencioso.
- Estão implementadas as famílias de poções, gold, teleporte, buffs, resets,
  progressão, livros Sephira, contratos, caixas, chaves, tickets, Water, Cube,
  Nightmare, Hell Gate, selos, contadores e consumíveis nativos confirmados.
- Courage de comida adiciona bônus somente ao hit PvE: físico e mágico possuem
  valores separados; não aumenta o score nem funciona em PvP.
- Repliction substitui os dois adicionais permitidos a partir dos pools
  configurados; refinamento/tintura conserva o slot reservado.
- Garnet, Emerald, Diamond e Coral aceitam os alvos/refinos confirmados.
- Adamantita transforma equipamento na variante Lendária correspondente do
  catálogo.
- Ovos de montaria usam incubação por poeira, espera online e equipada, redução
  de estágio e transformação final em cria.
- Montarias possuem HP, food, longevidade, recuperação, morte visual e revive;
  a montaria adulta viva consome 1 de Food por hora completa online e equipada.
- Fadas usam saldo temporal persistido e contam somente enquanto equipadas e
  online. O calendário absoluto de trajes e montarias temporários ainda não foi
  implementado; esses itens não devem anunciar duração antes desse suporte.
- Premium Firecracker valida bitmap, posição e cooldown antes de publicar o
  desenho para observadores.
- `gameplay_log=summary` agrega skills, ataques, mortes, drops e lotes por
  minuto; `verbose` fica reservado a reproduções curtas e `quiet` remove o I/O
  quente. Em lotes AoE, `UpdateEtc`/party/appearance são coalescidos por
  personagem sem reduzir confirmações de morte, dano ou EXP.

## Party, chat, guild e reinos

- Party suporta até 13 jogadores com ordem estável.
- Cada membro elegível recebe EXP integral e o grupo recebe 2% por membro,
  chegando a 126%.
- Bônus de EXP aplicados ao monstro pelo matador também participam do ganho dos
  membros elegíveis.
- Chat local, global, guild, party, whisper e carta online usam canais e cores
  nativas; limites por conta/canal reduzem spam.
- Guild cobre criação, convite, aceite, expulsão, sucessão, chat, recrutamento,
  rank, identidade nos pacotes e exportação de `Guilds.txt`.
- Akelonia/Hekalotia cobrem cidadania, capas, promoção, rei e comandos de
  consulta.
- Guildmark server-side está nos pacotes; o BMP ainda depende do host do client
  e consta como pendência em `DOCS/ROADMAP.md`.

## Quests e instâncias

- `data/quests.json` contém 18 quests NPC-driven com requisitos, consumo,
  recompensa, diálogo, teleporte, refino, contadores, recharge e encadeamento.
- Quest e charstate são confirmados na mesma transação; falha restaura item,
  gold, EXP, contador, conclusão e posição.
- As cinco zonas Mortais possuem reset global de dez minutos.
- O motor de instâncias possui RuntimeID, ownership por CharacterUID,
  visibilidade privada/compartilhada, deadlines, rollback e persistência.
- Water Normal/Mystic/Arcane usa salas e boss independentes, chain anti-skip e
  autoavanço server-side pelo UID recém-concedido quando Silver Angel está ativa.
- Magic Chamber, Cube Normal/Mystic/Arcane, Nightmare e Hell Gate possuem
  controladores próprios sobre o motor comum.
- Uxmal/Pista de Runas possui tickets, slots de party, fases, mobs, runas,
  deadlines e recuperação de estado.
- Big Cube possui o núcleo privado O/X: pergunta, plataformas, eliminação e
  retorno a Armia. A cadeia retail completa não foi inventada e permanece no
  plano por falta da tabela nativa confirmada.
- Gameplay spaces isolam players, mobs, summons, affects, loot e colisão mesmo
  quando runtimes compartilham coordenadas físicas.

## Mortal, Arch, Celestial e SubCelestial

- Mortal → Sefirot → Pedra Eterna → Arch está implementado.
- Arch exige nível interno 369, herda nome/corpo do Mortal, usa armas de qualquer
  classe e armaduras pela classe corporal, sem requisitos comuns de stats/level.
- Travas internas 354/369, quatro juras, Soul e Cythera estão implementadas.
- Celestial/SubCelestial possuem criação, troca de forma, EXP, pontos
  compartilhados, reduções, travas e progressão separada.
- A EXP de combate usa a curva W2PP por receptor: Arch perde EXP
  progressivamente até receber `1/120` no fim da evolução; Celestial e
  SubCelestial começam em `1/40` e recebem divisores progressivos conforme o
  nível. Mortal permanece sem essa redução avançada neste servidor.
- A criação de Arch ou Celestial força o retorno à seleção (`0x116`) e atualiza
  os quatro slots (`0x110`) depois do commit, para o client reconstruir corpo,
  score e skills. A criação Celestial também gera anúncio global em inglês;
  nenhum dos dois efeitos é publicado quando a persistência falha.
- O score de evolução segue o ramo normal do W2PP: Celestial/SubCelestial usam
  `ATK 488`, DEF `954/984/1004` conforme os cristais, atributos naturais da
  classe, bônus flat de HP/MP por classe/cristais, `+399` apenas no ataque e
  crescimento não-Mortal de HP/MP e defesa.
- O login migra uma única vez somente a assinatura Celestial/Sub legada
  conhecida (`ATK` da template e `DEF 4`), preservando os pontos distribuídos;
  score customizado não é reescrito e falha do commit impede a entrada.
- O desbloqueio nativo do nível 40 Celestial usa a ordem de materiais do client
  (`4127, 4127, 5135, 5113, 5129, 5112, 5110`) e consome 200 Fame somente após
  a validação completa da receita.
- Spirit's Seal 3443 encapsula uma ficha com snapshot/UID persistentes.
- `0x2CD` consulta a cápsula e `0x3CC` extrai a ficha; trade transfere item e
  snapshot atomicamente.
- Item 3455 permanece sem uso direto porque o handler nativo confirmado não o
  aceita.

## Bosses

- Todo boss continua sendo um `Mob` comum com runtime paralelo.
- Perfis Lua em `data/boss/*.lua` são validados e executados em sandbox.
- A versão atual possui eventos de spawn, dano, limiar e morte; condições por
  HP/cooldown/once; ações de mensagem, skill, shield, summon e mudança de fase.
- Spawn/registro ocorre antes da publicação; falha não deixa boss órfão.
- Drops especiais de inventário participam do mesmo commit da recompensa da
  morte.
- O formato completo dos arquivos está em `data/boss/README.md`.

## Persistência

- PostgreSQL usa schema v3, migrations no boot e transações `SERIALIZABLE`.
- Conta, nickname, CharacterUID, ItemUID, charstate, guild relacionada e estado
  de instância possuem operações atômicas quando participam da mesma feature.
- Autosave roda a cada três segundos com snapshots imutáveis, dirty tracking de
  itens e coalescência por conta/personagem.
- A fila assíncrona atribui geração a cada snapshot e a barreira de `Flush`
  drena todo overflow anterior; autosave antigo não pode ultrapassar nem
  sobrescrever um commit crítico mais novo.
- Operações críticas usam orçamento total configurável e não renovam timeout em
  cada retry.
- Falha de PostgreSQL nunca abre fallback JSON.
- JSONStore usa escrita temporária, `fsync`, rename e sincronização de diretório
  quando suportada; instance state exige EOF.
- Shutdown SIGINT/SIGTERM salva conta+charstate, força snapshot de instâncias,
  drena filas e retorna falha se a durabilidade final não for confirmada.

## Protocolo e segurança

- Header, checksum, tamanho exato, opcode e fase são validados antes do World.
- Opcode C→S desconhecido é recusado com métrica de cardinalidade fixa.
- Score enviado pelo client é descartado.
- Movimento, ataque, skill, NPC, loja, cargo, trade e item são novamente
  validados contra o estado do World.
- Rate limits cobrem pacotes, bytes, login e chats.
- Linha de visão bloqueia ataques e summons através de paredes.
- Slots invisíveis, equipamento impossível, UID repetido e item forjado são
  recusados.
- O servidor não depende de detectar WPE, Cheat Engine, VM ou MAC enviado pelo
  client para manter autoridade.

## Operação e testes

- `/debug/vars` e `/debug/pprof` só podem escutar em loopback.
- Métricas cobrem tick, lag, fila, comandos, players, mobs e persistência.
- `cmd/loadtest` provisiona até 990 bots em PostgreSQL descartável, clona o Arch
  de referência com novos CharacterUIDs/ItemUIDs, movimenta e combate em Noatum.
- O loadbot separa posição prevista da última correção recebida do servidor.
- Testes cobrem modelo, loaders, wire, rede, store e transições de gameplay,
  incluindo rollback, persistência, runtime isolation e segurança adversarial.
- `.github/workflows/ci.yml` executa testes, vet, build e `git diff --check` em
  cada push no `main` e em pull requests, sem job PostgreSQL nesta fase.
- `internal/game/testdata/packets` iniciou o corpus plaintext 7.48 com uma rota
  real; cada nova captura deve provar se está antes ou depois de qualquer bridge
  de protocolo antes de virar fixture canônica.

Validação padrão:

```powershell
go test -count=1 ./...
go vet ./...
go build -o tmp/tm-check.exe ./cmd/server
git diff --check
```

## Client 7.48

O executável versionado é produzido somente pela cadeia documentada em
`client748/PATCHES.md`. O ponto de entrada é `client748/Apply-WYD748.ps1`.

Hash SHA-256 suportado do `WYD.exe`:

```text
F6F99CC0405654629D9867C84F6587B2064B30D58F67A2151E1ACD36F394E72D
```

Os sete elos atuais são base, ExtendedStats, Bypass, macro normal, Lindy, a
coleção de trajes KR e as montarias KR. Foram importados 135 trajes completos, com classificação
corporal 5 (TK/BM), 10 (FM/HT) ou 15 (dinâmica), 129 renderers, 176 registros
de textura de traje e 856 assets. Eles reutilizam o ícone 36, permanecem no slot 13 do
7.48 e estão distribuídos em cinco `ShopCostum*`, com no máximo 27 por loja.
Um gate por lista exata permite que os IDs modernos ultrapassem a faixa nativa
`4151..4200` sem liberar outros itens. O gate e o selector também reconstroem o
ID `4xxx` a partir dos 12 bits transportados no visual do mundo, mantendo o
traje após `EnterWorld`. Partes vazias são omitidas sem deslocar as partes
seguintes. Os materiais extraídos do registro KR `@510` são preservados.
Outros 96 itens catalogados não são habilitados porque os clients fornecidos
não possuem todas as dependências referenciadas. O temporizador KR não foi
simulado por um efeito incompatível. O
WaterMacro foi removido; Water automática pertence ao servidor. O ajuste de
clique do Warrior's Seal ocorre no `ItemList.bin` pelo orquestrador.

Também foram importadas 45 aparências de montaria disponíveis comprovadas pelo renderer KR.
Elas reutilizam o contrato da Shire (`342`) no `Equip[14]`, sem ovo/cria,
comida, longevidade ou temporizador moderno, e estão divididas entre
`ShopKRMt01` e `ShopKRMt02`, respeitando o limite de 27 itens por loja. O patch
trata tanto a materialização completa quanto o `UpdateEquip 0x36B`; a aparência
da montaria muda imediatamente ao equipar ou desequipar. A tabela visual vem do
`MountDataV.bin` KR e os offsets de assento comprovados para os tipos modernos
cobertos foram portados; os 50 pathnames de textura usados em runtime estão
registrados na tabela legada.

## Fontes de referência

Prioridade para comportamento nativo:

1. implementação viva e testes do WYD-Go;
2. W2PP 7.59 para algoritmos;
3. Secrets 7.54;
4. Micronics e capturas do client 7.48 para confirmação final.

Portar semântica e fórmulas; nunca copiar structs, offsets ou endereços de outra
versão.

## Limitações conhecidas

Tudo que ainda precisa de implementação, pesquisa ou validação in-game está em
`DOCS/ROADMAP.md`. Nenhum item pendente deve ser descrito como entregue aqui.
