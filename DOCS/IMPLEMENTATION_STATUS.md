# Estado de implementação — WYD-Go 7.48

Atualizado em 18/07/2026. Este documento registra as features implementadas no
emulador Go e os detalhes de protocolo validados contra W2PP, Secrets 7.54 e o
cliente 7.48.

## Personagem e progressão

- `Score` v2 é a única fonte autoritativa dos atributos. `Char.Extended`
  guarda a base persistida e `ExtendedRuntime` é recomposto com equipamentos,
  passivas e affects. O `STRUCT_SCORE` de 28 bytes é apenas uma projeção de wire.
- Os atributos naturais por classe são os de `BaseSIDCHM` do W2PP:
  TK `(8,4,7,6)`, FM `(5,8,5,5)`, BM `(6,6,9,5)` e HT `(8,9,13,6)`.
- HP/MP Mortal seguem `BASE_GetHpMp`: base da classe, apenas CON/INT acima do
  atributo natural e progressão por nível. HP/nível é `[3,1,1,2]` e MP/nível
  `[1,3,2,1]` para TK/FM/BM/HT.
- Pontos de atributo começam em `level × 5`, com os thresholds adicionais de
  254, 299 e 354 da W2PP. O servidor normaliza qualquer saldo que ultrapasse o
  orçamento sem reduzir um atributo abaixo da base natural.
- Pontos de mastery: `level × 2 − mastery já aplicada`.
- Pontos de skill Mortal: `level × 3`, mais um por nível acima de 199, menos
  `SkillData.SkillPoint` de cada skill aprendida. O custo vem do CSV do servidor.
- Mastery respeita o limite por nível e os tetos 200/255/320 conforme a oitava
  skill Mortal e as skills secundárias aprendidas.
- EXP Mortal usa os 401 marcos cumulativos de `g_pNextLevel` do W2PP. O nível
  interno é base zero: `0` aparece como nível 1 e `399` como nível 400. O marco
  400 (`4.100.000.000`) é o teto de EXP, não um nível interno adicional.
- Cada NPC define `expReward` no JSON. Antes do ganho, o servidor aplica
  `max(expReward, exp_minimum)`, depois `exp_rate/100` e por fim o bônus de
  party. Os padrões em `data/server.txt` são piso 10.000, rate 100% e 2% por
  membro; portanto, um jogador sozinho recebe no mínimo 10.200. `exp_rate=0`
  desativa o ganho.
- A conta é salva ao ganhar EXP por uma morte de mob, além do salvamento no
  logout.
- Contas online recebem autosave completo a cada 3 segundos. A posição atual é
  sincronizada antes da gravação e o JSON é substituído atomicamente somente
  depois que o arquivo temporário foi escrito e sincronizado.
- `RegenHP` e `RegenMP` são derivados integralmente dos efeitos server-side dos
  itens, incluindo o item-base da classe. A cada três segundos, personagens
  vivos recuperam `Max × Regen / 120 + 10`, limitados ao máximo de HP/MP.

## Combate e morte de mobs

- Ataque físico usa `0x39D`; o servidor valida o alvo realmente enviado em @44,
  alcance, posição observada, ClientTick e intervalo mínimo de 600 ms. Nenhum
  dano ou alvo alternativo é aceito do cliente.
- A fórmula do golpe segue `BASE_GetDamage`: defesa reduz metade, variação
  aleatória e suavização de dano baixo.
- Parry usa EF_PARRY limitado a 100 e DEX em pesos 1/2, 1/4 e 1/8, com resultado
  entre 1 e 650 por mil. Crítico usa cursor server-side de 1024 entradas.
- O pacote de hit não marca ataque duplo em golpes simples. Esse bit fazia o
  cliente dividir o número flutuante de dano por dois.
- Morte segue a sequência nativa: `0x338 CNFMobKill` (EXP e início da morte)
  seguido de `0x165 RemoveMob` (remoção do cadáver após a animação).
- Monstros detectam jogadores vivos em oito tiles, perseguem sem atravessar
  coordenadas ocupadas e atacam quando ficam adjacentes.
- Atacar um monstro o coloca imediatamente em batalha, reproduzindo `SetBattle`.
- A perseguição possui leash de 30 tiles em relação à origem do gerador.
- O dano mob→player usa a mesma defesa, acerto/evasão e `BASE_GetDamage` do
  combate player→mob. O hit visual continua sendo o `0x39D` 7.48.
- Em HP zero, `0x338 CNFMobKill` chama `TMHuman::Die()` no cliente sem remover o
  jogador. Depois, o cliente pode solicitar `0x289 Restart`; após os quatro
  segundos nativos, o servidor restaura até 200 HP e executa recall para Armia.

## NPCGener, IA e colisão

- `data/NPCGener.txt` é carregado e gera grupos de NPCs/monstros.
- RouteType 2/3 percorre os segmentos de ida e volta; RouteType 4 reinicia a
  rota.
- O movimento usa `AttackRun & 0x0F`, com limite server-side 4. Cada Action
  carrega um trecho de até `2 × Speed` tiles (8 em speed 4), evitando reiniciar
  a animação a cada coordenada.
- HeightMap é interpretado como `int8`, igual ao `char` do TMSrv. Spawns usam
  faixa vertical estrita `<50`; cada passo usa o `MH=8` de `BASE_GetRoute`.
  Ocupação, `CantGo` e altura 127 continuam bloqueando a célula.
- O log do NPCGener possui `quiet|summary|verbose`; `summary` é o padrão e
  substitui milhares de linhas de spawn por um resumo agregado por minuto.
- O pacote de movimento 7.48 usa `Speed@16` e `Effect@20`; a ordem foi
  verificada contra a captura 7.48. Inverter esses campos faz o cliente deslizar
  ou teleportar. A rota fica vazia para o client reconstruir e interpolar o
  trecho completo.

## Catálogo server-side de itens e skills

- `itemlist.csv`, `Itemname.csv` e `SkillData.csv`, convertidos do client 7.48,
  são carregados integralmente durante o boot.
- A base Micronics foi convertida em 476 templates sob `data/npcs`.
  A conversão preserva efeitos de equipamento/drop/loja, mastery e campos
  auxiliares do `STRUCT_MOB`; quatro índices corrompidos acima de 6.500 foram
  removidos e recompensas Gold/EXP recebem saneamento seletivo.
- O layout de Equip dos NPCs segue a ABI real: acessórios 8-13, montaria 14 e
  capa 15. `CreateMob` recebe também o `AnctCode` calculado dos equipamentos.
- O loader da source em `WYD748Assets.cpp` normaliza `EF_GRID` no catálogo
  runtime, enquanto as tabelas de footprint/máscara da source permanecem 1x1.
  O `ItemList.bin` ativo não é reescrito e nenhum patch binário participa do
  contrato usado por inventário, cargo, trade e lojas.
- Preço, nome, slot permitido, classe, requisitos e efeitos dos itens pertencem
  ao servidor. Valores enviados pelo cliente não participam dos cálculos.
- Equipar valida `EF_POS`, `EF_CLASS`, level, STR, INT, DEX e CON antes do swap.
- O score soma efeitos estáticos do `itemlist` e os três efeitos do
  `STRUCT_ITEM`, incluindo refinação, conjunto de armadura e regra de duas armas.
- Refinação +11..+15 usa 220%, 250%, 280%, 320% e 370%; metadados como volatile,
  incubação, requisitos e previous bonus nunca são refinados.
- Dual wield usa 50% da arma menor quando o Unique coincide e 30% quando difere;
  passivas TK/HT acrescentam respectivamente 15%/10%, sem promover a mão a 100%.
- Ataque, defesa, HP/MP, atributos, especiais, accuracy/evasion, crítico,
  resistências, regeneração, alcance e velocidades passam a ser derivados no
  servidor.
- Todo personagem usa `Score` v2 `uint32` como base persistida e
  `ExtendedRuntime` como score efetivo. O loader rejeita qualquer campo antigo,
  versão diferente de 2 ou inventário que não possua os 64 slots estruturais.
- Todo NPC/monstro também persiste seus atributos exclusivamente em
  `NPCDef.Extended`; ataque, defesa, HP/MP, atributos, masteries e resistências
  planos foram removidos dos modelos e dos 476 JSONs.
- O `Score` de ABI foi retirado dos cálculos de atributos, equipamento, combate,
  skills, progressão, regeneração e pontos. Ele existe somente como projeção
  proporcional até 30000 exigida pelo protocolo/client 7.48.
- HP/MP acima de 32767 usam o estado real wide no servidor e no sidecar `.xstat`.
  `0x336`, `0x181` e os pacotes de ataque mantem os dois canais separados, sem
  wrap de mana e sem travar as barras em 100%.
- `SkillData.csv` já fornece custos, mana, delay, range, alvo e efeitos ao
  catálogo.

## Aprendizado e execução de skills

- Quatro mestres ficam em Armia: TK `(2115,2088)`, Foema `(2117,2088)`, BM
  `(2119,2088)` e Huntress `(2121,2088)`.
- Cada mestre usa `Merchant=19` e abre `0x17C ShopType=3` com os 24 itens
  virtuais da classe (`5000..5095`). A lista ocupa 27 slots: `8`, `17` e `26`
  ficam vazios como separadores dos três ramos exigidos pelo grid do client.
  Eles não entram no inventário.
- O aprendizado usa a compra `0x379` no mestre `Merchant=19`; `0x277` permanece
  exclusivo para distribuição de atributos (`BonusType=0`) e mastery
  (`BonusType=1`). A compra valida no servidor: mestre aberto,
  classe, skill ofertada, pontos, level, mastery, pré-requisitos de ramo e gold
  da oitava skill. `LearnedSkill` é persistido e devolvido pelo `0x337`.
- A execução valida skill aprendida, classe, cooldown, MP e alcance. Mana,
  mastery, save-mana e dano usam `SkillData.csv`; o cálculo mágico porta
  `BASE_GetSkillDamage` e o fluxo de `_MSG_Attack` da source 7.59.
- Foema e BM usam `INT/3 + INT/30`; TK Confiança/Espada Mágica usam
  `INT/4 + INT/40 + arma`; TK Trans usa `3*STR + 3*arma`. Depois são aplicados
  Magic Amp e o fator `5/4` apenas nas famílias previstas pela 7.59.
- Personagens e NPCs possuem resistências server-side a fogo, gelo, sagrado e
  trovão. Itens somam `EF_RESIST1..4` e `EF_RESISTALL`; a resistência efetiva é
  limitada a 100. O dano elemental segue `(150 - resistência)%`, usando metade
  da resistência quando o alvo é mob, conforme a 7.59.
- Skills agressivas já causam dano/morte/EXP server-side e usam `0x39D` para o
  alvo único, `0x39E` para duas entidades e `0x36C` para área. `MaxTarget` limita
  os alvos e cada mob recebe defesa e resistência próprias. Skills ativas de
  suporte já executam visual, MP e cooldown.
- Mortes produzidas por uma mesma skill multi-alvo acumulam EXP e level-ups em
  memória e fazem uma única gravação atômica da conta ao final do lote.

### Buffs TK / sistema de Affect

- O personagem possui 16 slots temporários de `Affect`. O `0x336` 7.48 leva
  os 16 affects compactos (`Time,Index` em @42..73); o `0x3B9` de 140
  bytes leva os 16 `STRUCT_AFFECT` completos (`Type,Value,Level,Time`) e alimenta
  ícone, descrição e contador. O servidor conserva os valores autoritativos.
- Buffs iguais renovam o slot sem acumular bônus, mas um efeito mais fraco ou
  mais curto não substitui o existente. Ao expirar, o score é recalculado desde
  a base e sincronizado, impedindo bônus permanentes.
- O client possui dois canais: `CreateMob@66`/`0x336@42` alimentam
  `m_usAffect` e executam `TMHuman::CheckAffect`, portanto controlam a aura
  visível para todos; `0x3B9` alimenta `m_stAffect` somente no próprio jogador,
  controlando ícones e contadores. Ambos usam uma unidade de oito segundos.
- Aplicação e expiração do canal visual são enviadas para todos os observadores.
  Affects ativos também entram no `CreateMob` quando uma entidade cruza a borda
  da visão. Mobs recebem `MobScore 0x336`, permitindo visualizar slow, freeze,
  veneno e a remoção desses efeitos ao expirar.
- O modo automático não recasta continuamente: enquanto restarem 10 segundos
  ou mais o pedido é ignorado antes de cobrar MP/animar. Abaixo de 10 segundos,
  a renovação só é aceita se a própria skill continuar equipada na barra.
- Fórmulas 7.59 implementadas para TK: `Samaritano` (Affect 24, defesa),
  `Possuído` (14, CON/HP), `Aura da Vida` (17, cura a cada 8 s), `Assalto`
  (13, dano com penalidade de defesa) e `Armadura Crítica` (31, defesa).
- O carregador normaliza os parâmetros desses cinco nomes para a 7.59 sem
  alterar o `SkillData.csv` cp1252 nem as posições reposicionadas do client 7.48.

- A Aura da Vida, por usar `TickType`, reenvia o affect 17 quando o client 7.48
  tenta relancá-la antes da janela de renovação. A tentativa não cobra MP, não
  inicia cooldown e não reproduz a skill.

### Executor completo das quatro classes

- Os indices `0..95` possuem cobertura server-side verificada por teste usando o
  `SkillData.csv` real. O executor separa dano, suporte, controle, efeito
  periodico, mobilidade, passiva, transformacao e invocacao.
- TK e Foema incluem controles sem dano artificial, curas individual/grupo,
  desintoxicacao, Flash, teleporte, ressurreicao, Cancelamento e os efeitos
  elementais/AoE.
- BM possui as oito invocacoes da W2PP (`BaseSummon` + `pSummonBonus`). Summons
  escalam com INT, CON e maestria, aparecem apenas na area de interesse, seguem
  o dono e atacam monstros. As cinco transformacoes usam `pTransBonus`, exibem
  o corpo correto e restauram o visual ao expirar.
- Huntress possui invisibilidade com remocao de aggro, imunidade, evasao,
  meditacao, buffs, passivas, dano/AoE e efeitos periodicos.
- Skills podem atingir jogadores: grupo e alcance sao validados; defesa,
  resistencias, buffs/debuffs, HP/MP e morte continuam autoritativos no servidor.
- Buffs de todas as classes usam a mesma regra de renovacao: somente abaixo de
  10 segundos e com a skill ainda equipada. Debuffs de mobs alteram defesa,
  ataque, velocidade e resistencias no calculo real.
- `Extracao` e `Alquimia` abrem seus fluxos de composicao; a cobranca inicial de
  Alquimia (`10 x level`) ja e server-side. As receitas/packets de composicao
  pertencem ao subsistema de crafting, separado do executor de combate.

## Pacotes relevantes corrigidos

| Pacote | Implementação final 7.48 |
| --- | --- |
| `0x114 EnterWorld` | 788 bytes. A cauda do `STRUCT_MOB` inclui LearnedSkill, pontos, SkillBar, MagicIncrement, regen e quatro resistências em @748..771; zerar essa região fazia o client inicializar skills/campos incorretamente. |
| `0x336 UpdateScore` | 236 bytes no client source-built: prefixo nativo de 92 bytes com Score@12 e Affect[16]@42..73; Score `uint32` @92..228 e assinatura canonical Score@232. É público e aciona os efeitos visuais de players e mobs. |
| `0x3B9 UpdateAffect` | 140 bytes: 16 estruturas `{Type,Value,Level,Time}` em unidades de 8 s. É enviado somente ao dono para ícones, descrição e timer. |
| `0x337 UpdateEtc` | 36 bytes: Hold/Chaos@12, EXP@16, LearnedSkill@20, status@24, mastery@26, skill@28, Magic@30, gold@32. É o layout p754 confirmado pelo dump real. |
| `0x338 CNFMobKill` | 24 bytes: FakeExp@12, morto/assassino@16, EXP@20. Atualiza EXP quando o morto é mob e chama `Die()` quando o morto é o jogador. |
| `0x39D AttackOne` | Target/Damage no layout pós-`PacketProtocolV754`, em @44/@46. |
| `0x366 Action` | Movimento de mobs com Speed@16 e Effect@20. |
| `0x165 RemoveMob` | RemoveType 0 ao sair da visão, 1 após `CNFMobKill` para morte normal e 3 para excluir imediatamente a representação morta antes de recriar um jogador revivido. |
| `0x289 Restart` | Pedido do cliente após morrer; restaura HP e retorna o personagem para Armia. |
| `0x270/0x272` | GetItem 28B e DropItem 32B; campos i32 de tipo/posição são validados antes da conversão. |
| `0x171/0x175` | Confirmações nativas de pegar/jogar item, combinadas com `0x182` autoritativo. |
| `0x373 UseItem` | 36 bytes; valida origem/destino DWORD e usa `itemlist.csv` + `volatiles.json` para poções, barras de gold e teleportes server-side. |
| `0x37F/0x3AB Party` | Convite de 48 bytes e aceite de 32 bytes; IDs, nome do líder, validade e capacidade são validados no mundo. |
| `0x37D/0x37E Party UI` | Adiciona/atualiza membros no painel e remove um membro ou limpa o grupo completo. |
| `0x333/0x334 Chat/Whisper` | Chat local, party (`=texto`), whisper e carta nativa (`!texto` abre o painel H). `/limparinv` limpa o Carry completo e `/spk texto` consome um Shout (3330) antes do anúncio global. |
| `0x215/0x116 CharacterLogout` | Remove somente a entidade do mundo, persiste a conta e retorna a mesma sessão autenticada para a seleção de personagem. |
| `0x211/0x112 DeleteCharacter` | Valida slot, nome e senha novamente; só então persiste o slot vazio e redesenha a seleção. |
| `0x27B / 0x28B` | Dois opcodes no mesmo handler, com layouts diferentes: `0x27B REQShopList` 16B (`TargetID@12`) e `0x28B` 20B (`npcID@12`, `ClickOk@16`). O `0x28B` é o `_MSG_Quest` nativo (`139\|C2G`): clique genérico e quest são o mesmo pacote. Toda interação valida existência, visibilidade e Chebyshev ≤ 8. |
| `0x369 REQMobByID` | Recupera mob, player ou Loja Fantasma dentro da área de interesse quando o client recebeu uma Action antes de materializar a entidade. |
| `0x36A/0x2BC` | Reconhecidos como pacotes informativos do client 7.48; tamanho anômalo recebe log limitado a uma vez por minuto. |
| `0x101 MessagePanel` | Aviso flutuante no topo da tela usado para confirmação e erro de comandos. |

O `p613_SendEtc` do Secrets não é o layout final deste client. O servidor usa o
`p754_SendEtc` de 36 bytes: misturar Status/Mastery em @20 também cria uma máscara
de LearnedSkill falsa, e escrever NextExp em @28 gera SkillPts negativos.

## Funcionalidades já disponíveis

- Configuração centralizada em `data/server.txt`, incluindo listener, caminhos
  de dados e balanceamento global de EXP (`exp_minimum`, `exp_rate` e
  `party_exp_bonus`); flags de linha de comando continuam disponíveis como
  sobrescritas para as opções operacionais. O IP público permanece no
  `serverlist` do client.
- Login, char-list de quatro slots, criação (`0x20F`/`0x110`), seleção e entrada
  no mundo. Os quatro layouts Mortal e a coordenada inicial são configurados em
  `data/character_templates.json`.
- Retorno para seleção sem reconectar (`0x215 -> 0x116`) e exclusão protegida
  por senha (`0x211 -> 0x112`). A conta continua reservada durante a troca de
  personagem e só é liberada no disconnect real.
- Sessão exclusiva por conta: após autenticação correta, uma segunda conexão
  recebe o erro nativo `0x11C AlreadyPlaying`; a reserva é removida no disconnect.
- Atributos, mastery e pontos de skill calculados pelo nível.
- Movimento do jogador, patrulha, aggro, perseguição e ataque de mobs.
- Recall após a morte com colisão: se o ponto de Armia estiver ocupado, escolhe
  um tile livre ao redor e recalcula a visibilidade da região antiga e da nova.
- Reentrada pós-morte corrigida no client 7.48: observadores descartam a entidade
  morta com `RemoveType=3`; ao cruzar novamente a borda, recebem
  `CreateMob → SetHpMp → ActionStop` antes do movimento. Isso impede o jogador
  revivido de permanecer morto ou congelado na margem da área visível.
- NPCGener, colisão de mobs e respawn de grupos.
- Portais Micronics server-side via `data/teleports.ini` e `0x290`, com cobrança,
  terreno, colisão, persistência e atualização regional.
- Merchant funcional preservado do `STRUCT_MOB@17`: lojas nativas Merchant 1,
  mestres Merchant 19 e grade de skills compactada em 27 slots.
- Combate melee contra mobs, dano visual, morte, EXP e level-up Mortal.
- Loja: abrir, comprar, vender, mover/equipar itens e persistir gold/inventário.
- Cargo compartilhado pela conta: `Cargo[128]` no char-list `0x10E`, três
  páginas visíveis (`0..119`), NPC Merchant 2 em Armia, movimentação
  inventário/Cargo via `0x376/0x182` e depósito/saque de gold
  (`0x388/0x387/0x339`) com persistência imediata.
- Loja Fantasma em Armia: `0x397/0x398/0x39A/0x39B`, clone estacionário com ID
  virtual `25000+OwnerID`, título via `0x363`, visibilidade regional e personagem
  real livre para jogar. Somente os slots anunciados do Cargo ficam bloqueados;
  o pagamento das vendas é creditado no gold do Cargo.
  A abertura usa `TargetX/Y@24` do movimento 7.48 e libera/rematerializa o avatar
  real antes de publicar o clone, preservando posição, título e movimento.
  Item/preço/posição/distância, espaço e gold são revalidados server-side; a
  compra das duas contas usa persistência transacional antes de confirmar.
- Trade entre jogadores (`0x383/0x384/0x386`) com convite, 15 itens, gold,
  confirmação dupla, invalidação de checks ao mudar a oferta, `EF_NOTRADE`,
  limites de inventário/gold e persistência recuperável das duas contas.
- Drop por slot do carry do monstro, gold por level e fallback para o chão com
  jogar, pegar, visibilidade regional e expiração.
- Uso server-side de poções HP/MP e barras de gold, incluindo pilhas,
  teto de 2 bilhões e persistência imediata para moedas de alto valor.
- Capacidade efetiva do inventário limitada aos 63 slots visíveis (`0..62`),
  mantendo `Carry[64]` apenas como contrato estrutural do protocolo.
- Catálogo autoritativo 7.48 de itens, nomes e skills.
- Mestres de classe, aprendizado e execução de skills com dano mágico 7.59 e
  resistências elementais.
- Auras e debuffs sincronizados nos dois canais do client: o dono recebe
  ícone/timer e todos dentro da área de interesse recebem o efeito visual,
  inclusive ao entrar depois na região ou quando o efeito expira.
- Party system 7.48 com convite (`0x37F`), aceite (`0x3AB`), painel de membros
  (`0x37D`), saída/expulsão (`0x37E`), promoção de líder e limpeza ao desconectar.
  Cada membro vivo e conectado no mesmo setor `128×128` recebe a EXP integral
  com bônus de 2% por membro elegível (102% a 126%); HP, HP máximo e level são
  sincronizados no painel.
- Chat local, party (`=`), guild (`-`), global (`--`), whisper, cartas pelo
  painel H (`!`) e anúncio de servidor `/spk`; o servidor reconstrói os pacotes
  de saída, nunca repassa bytes cifrados do cliente. O anúncio consome primeiro
  o item Shout 3330 e persiste antes do broadcast. Os canais viajam todos no
  `0x334` e são separados pelo prefixo do texto, com `--` testado antes de `-`.
- Comandos de chat possuem um dispatcher único, chamado tanto pelo `0x333`
  quanto pelo `0x334` (o client usa um ou outro conforme o foco da caixa) e que
  aceita comando sem argumento, como `/aceitar` e `/sair`.

### Sistema de guild (estrutura)

- Registro canônico em `data/guilds.json`: id em 1..4095, nome único ignorando
  caixa (limite 12 como o `GUILDNAME_LENGTH`), exatamente um líder por guild e
  cada personagem em no máximo uma guild. Validação também no boot.
- `Char.GuildID/GuildRank` são cópias desnormalizadas para wire e consulta
  rápida, **reparadas no login** contra o registro: guild dissolvida enquanto o
  jogador estava offline limpa o estado, e rank divergente é corrigido.
- Comandos `/criar`, `/convidar`, `/aceitar`, `/sair` e `/expulsar`. Somente o
  líder convida e expulsa; a saída do líder promove o membro mais antigo e a
  saída do último dissolve a guild — tudo na mesma transação.
- Guild e contas afetadas gravam juntas (`SaveGameState`): o journal carrega um
  tipo de destino explícito por entrada e valida o caminho resolvido, de modo
  que uma queda não deixa membro sem guild nem guild com membro inexistente.
- Neste primeiro marco convite, aceite e expulsão exigem o alvo **conectado**:
  não existe índice personagem→conta para alterar o JSON de um membro offline.
- `GuildID` é publicado no wire em `CreateMob@98` e `0x336@74`, como o WORD
  `(canal << 12) | id` que o client decompõe.

### Sistema de quest

- `data/quests.json` é uma allowlist com id estável e correspondência explícita
  com o NPC (aceita o nome com `_` ou com espaço, como o NPCGener). O boot falha
  se a quest apontar para NPC inexistente **ou** para um tipo que já tem handler
  próprio (loja, mestre, cargo) — seria uma quest que nunca dispararia.
- O roteamento do clique é por allowlist em três níveis: tipo conhecido →
  handler próprio; NPC configurado em `quests.json` → quest; qualquer outro →
  resposta neutra. Nunca "o resto vira quest".
- A confirmação é client-side (tipo 11 + rosto 58, e tipos 10/13/14/15), então o
  servidor recebe um único pacote por interação e valida tudo em cima dele:
  requisitos de level/gold/itens/quest anterior, espaço no inventário, alcance,
  visibilidade e flag de concluída.
- Consumo respeita pilhas de `EF_AMOUNT`; a recompensa usa o mesmo teto de gold
  das barras. Persistência antes da confirmação ao client, com rollback total
  (inventário, gold, EXP, quests concluídas e posição) se a gravação falhar.
- `QuestsDone` é lista de IDs, não máscara de bits: máscara limitaria o projeto
  a 64 quests e amarraria o id ao bit.
- Atenção ao escrever quest: `minLevel`/`maxLevel` usam o nível **interno**,
  base zero — o servidor guarda 0 para o que o client exibe como nível 1.
- Persistência JSON de personagem/conta.
- Autenticação real da senha do `0x20D` usando exclusivamente hash PBKDF2
  versionado. A API HTTP cria contas vazias atomicamente (`cmd/account-api`) e
  o criador administrativo local usa o mesmo contrato
  (`account-create.exe`, `cmd/account-create`). O campo `pass` é inválido.
- Área de interesse server-side de ±32 coordenadas (janela 65×65): `CreateMob`,
  movimento, combate e remoção de NPCs são enviados somente aos jogadores que
  enxergam a entidade; cruzar a borda cria/remove a entidade incrementalmente.
  A materialização de players é simétrica e restaura explicitamente HP/MP,
  pose viva, posição e velocidade observada.

## Limitações atuais

- As receitas server-side dos packets de composição de `Extração` e `Alquimia`
  ainda pertencem ao próximo marco de crafting. Os sete NPCs de composição
  (tipo 8: Agatha, Alquimista_Odin, Aylin, Compositor, Ehre, Lindy, Tiny) já
  estão identificados na base e correspondem 1:1 aos `_MSG_CombineItem*`.
- O fechamento explícito de loja (`0x196`) não possui handler dedicado; abrir
  outra interação ou sair do alcance limpa o contexto pelo fluxo atual. Não há
  evidência de que o client 7.48 envie esse opcode: ele aparece no client 759,
  mas **não** na captura real do 7.48.
- Guild cobre só a estrutura: não há guerra, zonas, torre, taxa, sub-guild nem
  aliança. Convite/aceite/expulsão exigem o alvo conectado.
- O efeito **visual** do `GuildID` no client não foi confirmado in-game. O chat
  de guild funciona por roteamento server-side e não prova que o client leu o
  `@98`/`@74`. A verificação é comparar nome/alvo entre membros da mesma guild
  e de guilds diferentes; o mark depende de download de imagem, que é
  customização do W2PP e provavelmente não existe no 7.48 stock.
- Arch/Celestial usam `g_pNextLevel_2` e regras de evolução/quest próprias; não
  fazem parte da progressão Mortal atual.

## Verificação local

```powershell
cd wyd-go
go test ./...
go vet ./...
go build -o tm.exe ./cmd/server
go build -o account-api.exe ./cmd/account-api
go build -o account-create.exe ./cmd/account-create
```
# Score v2

Os atributos de jogadores, NPCs e monstros foram consolidados em uma única
estrutura `uint32`. O Score WORD do client é somente uma projeção gerada no
wire; o contrato estrito da source está em `DOCS/SCORE.md`.
