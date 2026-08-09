# Estado de implementação — WYD-Go 7.48

Atualizado em 02/08/2026. Este documento registra as features implementadas no
emulador Go e os detalhes de protocolo validados contra W2PP, Secrets 7.54 e o
cliente 7.48.

Sistemas com documento proprio: `DOCS/SECURITY.md` (fronteira não confiável,
validações anti-WPE/Cheat Engine e testes adversariais), `DOCS/BOSS.md`
(encontros de boss),
`DOCS/ASCENSION.md` (cadeia Mortal->Arch, travas de nivel e cristais),
`DOCS/OPERATION.md` (metricas, profiling e desligamento), `DOCS/CRAFTING.md`
 (os sete NPCs tipo 8 e a fama), `client748/PATCHES.md` (a cadeia de patches do
 executavel do client) e `DOCS/LOADTEST.md` (carga isolada de 990 bots em
 Noatum).

## Personagem e progressão

- `ExtendedScore` v2 é a única fonte autoritativa dos atributos. `Char.Extended`
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
  sincronizada antes do snapshot imutável entrar na fila PostgreSQL. Operações
  críticas drenam snapshots anteriores e esperam o commit antes de confirmar.
- `RegenHP` e `RegenMP` são derivados integralmente dos efeitos server-side dos
  itens, incluindo o item-base da classe. A cada três segundos, personagens
  vivos recuperam `Max × Regen / 120 + 10`, limitados ao máximo de HP/MP.
- Distribuir stats/mastery, aprender skill, subir de nível, ganhar EXP, receber
  recompensa de quest ou aplicar buff comum não recria mais o avatar com
  `CreateMob`. Esses fluxos atualizam somente score/ETC/HP-MP e não interrompem
  nem encaixam a animação de movimento.
- Equipar/remover itens, trocar capa ou montaria, refinar, tingir e transformar
  rosto usam `0x36B UpdateEquip` incremental de 60 bytes para dono e
  observadores. `ItemEff[16]@12` leva mesh/refino e `AnctCode[16]@44` leva a
  tintura. O pacote não possui coordenadas e preserva o movimento durante PvP.
- Auditoria de materialização concluída: `CreateMob` permanece somente em spawn
  real, entrada/reentrada na área de interesse, recuperação `REQMobByID` e no
  renascimento que primeiro elimina o cadáver com `RemoveType=3`. Pets, summons
  e objetos Sephira usam o pacote apenas quando a instância nasce ou entra na
  visão; a reconciliação periódica da cria não a recria.

## Combate e morte de mobs

- Ataque físico usa `0x39D`; o servidor valida o alvo realmente enviado em @44,
  alcance, linha transitável, posição observada, ClientTick e intervalo derivado
  do AttackSpeed server-side. Nenhum dano ou alvo alternativo é aceito do client.
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
- A IA usa ativação espacial: monstros só entram nos tickers de combate, rota e
  affects quando existe jogador vivo no raio 16. Acordar/dormir é atualizado
  por movimento, entrada, saída e por uma verificação de segurança após morte.
- Aquisição normal conserva o raio nativo 4, retenção 7 e leash de origem 16.
  Mobs ativos percebem alvo a cada 1 s; perseguição e patrulha iniciam trechos
  a cada 2 s. Ataques continuam limitados pelo cooldown autoritativo de 1,5 s.
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
- Movimento de **jogadores** preserva `Route[24]@28` depois da validação
  server-side. Publicar somente origem/destino fazia cada observador recalcular
  uma rota diferente e gerava pequenas correções visuais. O Action é enviado
  aos observadores existentes antes dos deltas de entrada/saída da visibilidade;
  um client que acabou de entrar no raio recebe apenas a materialização no
  destino, nunca uma rota retroativa. A regra de rota vazia acima continua
  exclusiva do movimento gerado para **mobs**.
- Paradas rotineiras não usam mais o `ActionStop` com `Effect=1`, pois esse é o
  mesmo caminho de correção instantânea/teleporte do client. Chegar ao destino
  encerra a rota sem pacote extra; parar antes do destino emite uma única
  reorientação `Effect=0`. O ponto intermediário só é aceito quando pertence à
  `Route[24]` que já passou pela validação autoritativa.

## Catálogo server-side de itens e skills

- As skills Sephira 97–101 usam os bits 25–29 de `LearnedSkill`. Canhão Guardião
  exige o item de chão 746 sob o caster (ID 15001–15100), alvo a pelo menos quatro
  tiles e dano `15 × nível + 500`. Muro de Espinhos cria a NPC imóvel `Vinha` por
  aproximadamente 21 segundos. Ressurreição é autocast do personagem morto, com
  chance `(nível interno + 1)/5` e retorno em 50% de HP/MP. Concentração aplica o
  affect 34. Força Espectral é passiva: +1 alcance e bit visual na arma.

- `itemlist.csv`, `Itemname.csv` e `ItemEffect.h` vêm do client 7.48.
  `SkillData.csv` vem da W2PP/7.59 para usar custos, durações e parâmetros mais
  atuais; os quatro são carregados durante o boot e não recebem overrides
  hardcoded posteriores. `Itemname` altera somente nomes, enquanto ItemList
  mantém os demais campos.
- A base Micronics foi convertida em 476 templates sob `data/npcs`.
  A conversão preserva efeitos de equipamento/drop/loja, mastery e campos
  auxiliares do `STRUCT_MOB`; quatro índices corrompidos acima de 6.500 foram
  removidos e recompensas Gold/EXP recebem saneamento seletivo.
- O layout de Equip dos NPCs segue a ABI real: acessórios 8-13, montaria 14 e
  capa 15. `CreateMob` recebe também o `AnctCode` calculado dos equipamentos.
- Personagens e templates agora usam exatamente o mesmo layout nomeado: seis
  acessórios em 8-13, montaria em 14 e capa em 15. Os JSONs antigos com
  `manto`/montaria no slot 15 não fazem parte do schema autoritativo.
- O patch do client normaliza todas as oito formas de `EF_GRID` para 1x1,
  incluindo largura/altura visual e mascara de colisao usada por inventario,
  cargo, trade e lojas.
- Preço, nome, slot permitido, classe, requisitos e efeitos dos itens pertencem
  ao servidor. Valores enviados pelo cliente não participam dos cálculos.
- Equipar valida `EF_POS`, `EF_CLASS`, level, STR, INT, DEX e CON antes do swap.
  Arch ignora level/stats e classe de armas, mas mantém `EF_POS` e usa a classe
  corporal do Mortal para armaduras.
- O score soma efeitos estáticos do `itemlist` e os três efeitos do
  `STRUCT_ITEM`, incluindo refinação, conjunto de armadura e regra de duas armas.
- Refinação +11..+15 usa 220%, 250%, 280%, 320% e 370%; metadados como volatile,
  incubação, requisitos e previous bonus nunca são refinados.
- Dual wield usa 50% da arma menor quando o Unique coincide e 30% quando difere;
  passivas TK/HT acrescentam respectivamente 15%/10%, sem promover a mão a 100%.
- Ataque, defesa, HP/MP, atributos, especiais, accuracy/evasion, crítico,
  resistências, regeneração, alcance e velocidades passam a ser derivados no
  servidor.
- Todo personagem usa `ExtendedScore` v2 `uint32` como base persistida e
  `ExtendedRuntime` como score efetivo. O loader rejeita qualquer campo antigo,
  versão diferente de 2 ou inventário que não possua os 64 slots estruturais.
- Todo NPC/monstro também persiste seus atributos exclusivamente em
  `NPCDef.Extended`; ataque, defesa, HP/MP, atributos, masteries e resistências
  planos foram removidos dos modelos e dos 476 JSONs.
- O `WireScore` de ABI foi retirado dos cálculos de atributos, equipamento, combate,
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
- O recast manual nunca é bloqueado. O hook exclusivo do macro agenda buffs a
  cada 150 segundos, renovando os efeitos nominais de 180 segundos dentro da
  janela final de 30 segundos. Ataques continuam usando o cooldown calculado
  pelo client; o servidor apenas valida e executa cada intenção recebida.
- Fórmulas 7.59 implementadas para TK: `Samaritano` (Affect 24, defesa),
  `Possuído` (14, CON/HP), `Aura da Vida` (17, cura a cada 8 s), `Assalto`
  (13, dano com penalidade de defesa) e `Armadura Crítica` (31, defesa).
- Esses parâmetros agora vêm diretamente do `SkillData.csv` da W2PP. O loader
  apenas converte `AffectTime` para os blocos de oito segundos do protocolo,
  sem manter uma segunda tabela hardcoded.

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
- Buffs equipados são reagendados pelo macro para a janela final de 30 segundos;
  recasts manuais continuam livres. Debuffs alteram defesa,
  ataque, velocidade e resistencias no calculo real.
- `Extracao` e `Alquimia` abrem seus fluxos de composicao; a cobranca inicial de
  Alquimia (`10 x level`) ja e server-side. As receitas/packets de composicao
  pertencem ao subsistema de crafting, separado do executor de combate.

## Pacotes relevantes corrigidos

| Pacote | Implementação final 7.48 |
| --- | --- |
| `0x114 EnterWorld` | 788 bytes. A cauda do `STRUCT_MOB` inclui LearnedSkill, pontos, SkillBar, MagicIncrement, regen e quatro resistências em @748..771; zerar essa região fazia o client inicializar skills/campos incorretamente. |
| `0x336 UpdateScore` | 236 bytes no client patched: prefixo nativo de 92 bytes com Score@12 e Affect[16]@42..73; ExtendedScore `uint32` @92..228 e assinatura XSC2@232. É público e aciona os efeitos visuais de players e mobs. |
| `0x3B9 UpdateAffect` | 140 bytes: 16 estruturas `{Type,Value,Level,Time}` em unidades de 8 s. É enviado somente ao dono para ícones, descrição e timer. |
| `0x337 UpdateEtc` | 48 bytes: Hold DWORD@12 (zerado quando não há EXP retida), EXP@16, LearnedSkill@20, status@24, mastery@26, skill@28, Magic@30, gold@32 e pontos wide@36..47. CP não pertence a este pacote: segue como byte `CP+75` em CreateMob. |
| Chaos Point | Modelo autoritativo assinado `-75..+75`, neutro em `0`; o byte nativo de aparência é `CP+75`. O `/cp` consulta este contador. Ele é separado do C.point/EXP Hold, que permanece zero até esse sistema ser implementado. |
| Fame | Contador por personagem em `SpecialCoins["fame"]`; `/fame` consulta o saldo e exibe o valor no `MessagePanel`, sem mutação. O `Warrior's_Seal` (4146/volatile 199) é a fonte de crédito. |
| `0x338 CNFMobKill` | 24 bytes: FakeExp@12, morto/assassino@16, EXP@20. Atualiza EXP quando o morto é mob e chama `Die()` quando o morto é o jogador. |
| `0x39D AttackOne` | Target/Damage no layout pós-`PacketProtocolV754`, em @44/@46. |
| `0x366 Action` | Movimento de mobs com Speed@16 e Effect@20. |
| `0x165 RemoveMob` | RemoveType 0 ao sair da visão, 1 após `CNFMobKill` para morte normal e 3 para excluir imediatamente a representação morta antes de recriar um jogador revivido. |
| `0x289 Restart` | Pedido do cliente após morrer; restaura HP e retorna o personagem para Armia. |
| `0x270/0x272` | GetItem 28B e DropItem 32B; campos i32 de tipo/posição são validados antes da conversão. |
| `0x2E4/0x2E5` | DeleteItem 20B e SplitItem 24B operam somente no Carry visível. Conferem o índice real, persistem antes de confirmar e o split materializa um UID independente. |
| `0x171/0x175` | Confirmações nativas de pegar/jogar item, combinadas com `0x182` autoritativo. |
| `0x373 UseItem` | 36 bytes; valida origem/destino DWORD e usa `itemlist.csv` + `volatiles.json` para poções, barras de gold e teleportes server-side. |
| `0x374 UpdateItem` | Pedido/atualização de objeto permanente em 20B. Abrir portão revalida ID, alcance, visibilidade, estado e a chave pelo `EF_KEYID`; chave é salva antes de publicar a porta aberta. |
| `0x37F/0x3AB Party` | Convite de 48 bytes e aceite de 32 bytes; IDs, nome do líder, validade e capacidade são validados no mundo. |
| `0x37D/0x37E Party UI` | Adiciona/atualiza membros no painel e remove um membro ou limpa o grupo completo. |
| `0x333/0x334 Chat/Whisper` | Chat local, party (`=texto`), whisper e carta nativa (`!texto` abre o painel H). `/limparinv` limpa o Carry completo, `/fame` consulta a fama e `/spk texto` consome um Shout (3330) antes do anúncio global. |
| `0x334 day` | Pedido periódico de calendário recebe o `0x101 "!#11  2"` oculto nativo; não polui o painel nem procura um jogador chamado `day`. |
| `0x336 UpdateScore C→S` | Reconhecido e descartado, como W2PP/Secrets. Nenhum score vindo do client altera o estado autoritativo. |
| `0x368 Illusion` | Action2 de 52B passa pela validação da Illusion e retorna como `0x368`, preservando animação e câmera específicas. |
| `0x215/0x116 CharacterLogout` | Remove somente a entidade do mundo, persiste a conta e retorna a mesma sessão autenticada para a seleção de personagem. |
| `0x211/0x112 DeleteCharacter` | Valida slot, nome e senha novamente; só então persiste o slot vazio e redesenha a seleção. |
| `0x27B / 0x28B` | Dois opcodes no mesmo handler, com layouts diferentes: `0x27B REQShopList` 16B (`TargetID@12`) e `0x28B` 20B (`npcID@12`, `ClickOk@16`). O `0x28B` é o `_MSG_Quest` nativo (`139\|C2G`): clique genérico e quest são o mesmo pacote. Toda interação valida existência, visibilidade e Chebyshev ≤ 8. |
| `0x369 REQMobByID` | Recupera mob, player ou Loja Fantasma dentro da área de interesse quando o client recebeu uma Action antes de materializar a entidade. |
| `0x39F ReqRanking` | Consulta o jogador próximo pelo ID. O modo de inspeção usa `MessagePanel`, pois a janela `0x324` posterior não existe no client 7.48; os modos de duelo ainda dependem do ranking/arena. |
| `0x3A0 Ping` | Keepalive de 12B. Não consulta data/hora e não recebe resposta, exatamente como no W2PP/Secrets. |
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
  real livre para jogar. O clone veste o **rosto do Carbunkle** (resolvido pelo
  catálogo, não por índice cravado) e tem os outros **quinze slots zerados** —
  isso evita o problema em vez de remediá-lo: mesh de monstro no rosto conflita
  com armadura/calça/luva/bota humanas, e sem as peças o clone é só o Carbunkle.
  De quebra, o dono deixa de expor o próprio equipamento enquanto vende. Somente os slots anunciados do Cargo ficam bloqueados;
  o pagamento das vendas é creditado no gold do Cargo.
  A abertura usa `TargetX/Y@24` do movimento 7.48 e libera o avatar real com
  `CloseTrade + UpdateEquip + SetHpMp`, sem `CreateMob`/`ActionStop`, antes de
  publicar o clone; posição, título e movimento são preservados.
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
- A cadeia Mortal principal está configurada nos NPCs `Gravedigger`, `Gardener`,
  `Scout`, `Guard__` e `Envoy`. São portais repetíveis, com os limites e destinos
  confirmados na W2PP. Para compatibilidade com o client 7.48, as cinco entradas
  dependem somente da faixa de level e de ser Mortal: os itens 4038–4042 não
  são exigidos nem consumidos.
- As complementares confirmadas incluem `Drill_Master`/`QUEST_CAPAREAL`
  (Mortal interno 199-253 para 1740,1725) e `Priest_Ameria`/`AMELIA` (interno
  119-123, troca 4125 por 4126). Ambas são repetíveis como o fluxo nativo.
- **São 18 quests configuradas.** Além das anteriores entraram a cadeia do
  novato completa (`Trainer1`, `Trainer2`, `Trainer3`, `Trainer_Leader`),
  `Blessing_Priest`, `Knighthood_Guard`, `Balance_Guard`, `Gargoyle_Watcher`,
  `Priest_Kruno` e o `Survivor`.
- Três campos novos no motor de quest:
  - **`rewards.refine`** entrega refinação no equipamento **vestido** — é a
    recompensa dos guardas do Training Camp, que o nativo grava como `EF_SANC`
    direto no `Equip`. `slot: 6` refina só a arma; `slot: -1` refina todas as
    peças. Era a lacuna que impedia `Trainer2`/`Trainer3` de existirem;
  - **`dialogue`** são falas ditas em voz alta ao interagir, limitadas a
    `MaxQuestLine` (95 bytes — o `0x333` carrega `String[96]` e o builder copia
    95, então o que passar disso é cortado no meio da frase);
  - **`recharge`** porta o comportamento do `Survivor`: com o contador **zerado**,
    o NPC procura um item no inventário, consome uma unidade e credita saldo. É
    campo próprio porque o item não é consumível — nem o nosso `itemlist` nem o
    do W2PP dão `EF_VOLATILE` ao selo 4127, então "usar o item" não é caminho
    válido: quem o gasta é o NPC. A oferta consulta `rechargeCovers`, senão quem
    chega com saldo zero e o selo na mochila seria recusado antes de o NPC ter
    chance de trocar.

  O motor por trás de `counters`/`consumeCounters`/`recharge` está em
  §Contadores por personagem.
- `data/quest_zones.json` guarda os cinco retângulos nativos. Uma varredura
  global por deadline recolhe os jogadores a cada 10 minutos; mortos têm o
  cadáver removido dos observadores antigos antes de serem revividos e
  recriados em Armia.
- As caixas 4117–4121 usam `EF_VOLATILE=191`/`quest_reward`: máximo de nível
  exclusivo, somente Mortal, EXP afetada por `exp_rate` e double EXP, sem piso
  ou party, gold limitado a 2 bilhões e rollback integral em falha de save.
- O drop é exclusivo dos cinco líderes no `carry[56]`, cuja taxa 1 é garantida;
  os followers genéricos não carregam as caixas.

### Reinos Akelonia/Hekalotia

- Reino não é persistido em campo paralelo: é derivado da capa no slot 15.
  Hekalotia=7 e Akelonia=8, conforme `Basedef.h` e `GetCurScore_CapeInfo`.
- `King_Harabard` cobra 4 Safiras e `King_Glantuar` 15; nível mínimo interno
  219. O broker cobra 16 para converter a capa ao equivalente neutro e bloqueia
  a saída no domingo.
- A matriz completa de capas está modelada por tier: Basic 545/546/548,
  Crusader-Shiner 543/544/549, Elite 3191/3192/3193, Hero 3194/3195/3196 e
  Master 3197/3198/3199. No nível interno 255, retornar ao mesmo rei promove a
  capa Basic para Crusader/Shiner; Lindy e Celestial escolhem Elite/Master pelo
  reino atual.
- As transações aceitam Safira 697 empilhada e pacote 4131 como dez unidades,
  persistem antes da confirmação e restauram o personagem inteiro em falha.
- O `Kingdom_Emblem` 4081 equipado no slot 13 é uma forma alternativa de
  pagamento Mortal e também só é consumido depois de toda validação.
- A guild guarda `GuildInfo.Clan` como `Kingdom`; o líder a move junto com sua
  adesão/saída na mesma transação, e convite/aceite recusam reino divergente.
- `/kingdom`/`/reino` e `/king`/`/rei` usam os destinos nativos W2PP e
  cooldown de cinco segundos.
- Não há chat de reino `@@`: ele pertence ao client mais recente da W2PP, não
  ao executável 7.48. Permanecem os canais 7.48 já mapeados pelo projeto.
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

## Consumíveis (EF_VOLATILE)

O `data/itemlist.csv` traz **124 códigos `EF_VOLATILE` distintos em 355 itens**. Cada
código escolhe o `case` no `onUseItem` (`internal/game/consumables.go`); o `Index`
do item escolhe os parâmetros (`data/volatiles.json`, via `rules[code]` +
override `items[itemID]`). Estado atual: **124 códigos com contrato explícito
e 43 ações** (regras por código mais os comportamentos específicos por
`Index`). Não há mais código conhecido caindo no `generic`. O código `206`
usa `celestial_capsule` para o fluxo de encapsulamento; `3455` permanece
`no_direct_use` porque não é aceito diretamente pelo handler nativo.

Famílias com comportamento server-side (**Fase A/B/C concluídas**):

| Família | Códigos | Ação |
|---|---|---|
| Restauração HP/MP | 1 | `restore` |
| Gold (item→gold, gold no chão) | 185, 2 | `gold`, `ground_gold` |
| Teleporte | 11 | `teleport` |
| Buff / comida / drink | 10, 52-57, 60-67, 198 | `buff` |
| Baú/poeira de EXP | 7, 8 | `grant_next_level`, `quest_reward` (Eye Wax customizado) |
| Salvar/voltar posição (Gema) | 12, 13 | `save_position`, `warp_saved` |
| Transformação de rosto | 70-77, 89 | `face_transform`, `face_restore` |
| Refino Ori/Lac + Molar (set) | 4, 5, 194 | `refine`, `refine_set` |
| Tintura / repliction | 186, 190 | `tint`, `untint`, `repliction` |
| Montaria (ração, amago, essência, vida, crescimento, invuln, ovo) | 15, 16, 90-94, 196 | `mount` |
| Adamantita (e família nativa de minérios) | 9 | `ore_upgrade` (troca pelo `Extra`, chance configurável) |
| Gemas Diamond/Emerald/Coral/Garnet | 180-183 | `equipment_gem` (+10..+15 ou arma Ancient abaixo de +10) |
| Magical Pill | 6 | `magical_pill` (+9 pontos uma vez) |
| Livros Sephira | 32-36 | `learn_special_skill` (bits 25-29) |
| Contratos de soldado/guarda | 41-43, 46-48 | `summon_contract` |
| HP temporizado | 58 | `buff` (affect 35, +10% MaxHP) |
| Hunting Scroll | 195 | `hunting_teleport` (10 destinos por zona) |
| Caixas das quests Mortais | 191 | `quest_reward` (EXP + gold por tier) |
| Chaves de portão | 3 | `gate_key` (abre a porta pelo `EF_KEYID`) |
| Selo de fama | 199 | `grant_counter` (+10 de fama por selo) |
| Juras elementais do Arch | 187 | `arch_crystal` (as quatro juras de Kefra) |
| Fogos de artifício | 19 | `firework` (Motion 100, variante 0–5) |
| Água das Fadas | 179 | `refine_equipped` (Naiads/Grewpain até +6) |
| Passes de território | 188 | `territory_pass` (Mortal/Arch/Celestial) |
| Compostos de Chance/Equilíbrio | 192, 193 | `mastery_reset` (one-shot por personagem) |
| Carta de Remissão | 203 | `chaos_remission` (CP assinado para +75; byte visual nativo 150) |
| Celestial/SubCelestial | 211, 213, 214 | `celestial_ideal`, `celestial_fury`, `celestial_switch` |
| Escritura do Pesadelo | 212 | `nightmare_ticket` (+13 entradas, cooldown de 12h) |
| Cadeias Water + Nessus | 21-28, 30, 131-138, 140, 161-168, 170 | `instance_ticket` (24 salas + 3 bosses, party, recompensa sequencial, 120s, expulsão) |
| Cartas Magic Chamber | 20 | `instance_ticket` (N/M/A, 4 salas de 100 mobs + boss, transição de 10s) |
| Nightmare N/M/A | 173-175 | `instance_ticket` (versões party e personal, 240s) |
| Cube/Hell Gate | 54, 171, 172, 176 | `instance_ticket` (Cube O/X e Hell Gate com prazo absoluto, party/personal e retorno autoritativo) |
| Big Cube | 51 | `instance_ticket` O/X server-side; pergunta privada e retorno a Armia |
| Hidden Ruins / Service | 177, 197 | `timed_access` persistente por personagem |
| Passe de evento | 178 | `grant_counter_once` + identificador 4104, sem consumo duplicado |
| Envelopes / honra / donate / medalhas | 18, 184, 189 | `gold`, `grant_counter` |
| Caixas e revival de montaria | 0 por `Index` | `loot_box`, `mount_revive`; itens de NPC/comando usam `no_direct_use` |

### Water

- As três cadeias seguem as fórmulas nativas: `777→...→785`,
  `3173→...→3181` e `3182→...→3190`.
- Ao matar o último mob de uma sala regular, somente o líder recebe o
  pergaminho seguinte e permanece na sala durante a exit grace. A sala 8
  entrega a carta de Nessus, que abre uma instância boss independente; depois
  do boss, um Room 1 Scroll já existente pode iniciar um novo ciclo.
- O item premiado é materializado com UID e persistido antes da confirmação.
  Falha do banco mantém a recompensa pendente; inventário cheio derruba o
  pergaminho no chão, como `PutItem`.
- A ocupação é validada pelo ID da instância e pelos jogadores fisicamente
  dentro da área. Normal aceita Mortal; Mystic aceita Mortal/Arch; Arcane não
  restringe evolução, acompanhando as verificações de `_MSG_UseItem.cpp`.
- As salas de Nessus aceitam composição mista de templates. A configuração
  atual usa as tabelas 7.54 de Secrets (um chefe + dez auxiliares).

### Water — cadeia server-side com Silver Angel (2026-08-09)

- Cada sala e boss continua como `VolatileInstance` independente, com
  `RuntimeID` proprio, exit grace de dez segundos e persistencia
  atomica de conta + estado da instancia.
- Rooms que concedem o proximo pergaminho materializam primeiro um UID
  server-side, persistem o grant e so entao publicam o item ao lider.
- Se o lider estiver com Silver Angel (3914) ativa em `Equip[13]` no
  momento do reward, o servidor usa exclusivamente o UID recem-criado
  pela mesma rotina de dominio do clique manual e persiste a sala
  seguinte antes de teleporte/spawn.
- Fada no inventario, fada de outro membro, equip posterior ao reward,
  Carry cheio com drop no chao e reward zero nao disparam automacao.
  Falha do segundo commit restaura a transicao e conserva o scroll ja
  duravel no Carry.
- O WaterMacro client-side e os comandos `/macropergaon` e
  `/macropergaoff` foram removidos. O client suportado termina no patch
  Lindy; `Patch-WYD748-Macro.ps1` continua sendo apenas o macro normal
  de skills/buffs.
- A cobertura especifica fica em `internal/game/water_auto_test.go`,
  incluindo UID duplicado do mesmo indice e rollback das duas fronteiras
  de persistencia.
- O `Warrior's_Seal` (4146) usa `EF_VOLATILE=199` no servidor. Como o client
  7.48 nao traz a excecao por item do W2PP, `client748/Apply-WYD748.ps1`
  tambem executa `Patch-WYD748-ClientItemUse.ps1`: somente o marcador local
  de clique do `ItemList.bin` vira `1`, enquanto o servidor resolve novamente
  4146 -> 199. O carimbo final do arquivo e preservado e a alteracao possui
  backup/guarda de formato.

### Water — isolamento de gameplay e consistência (2026-08-08)

- `requiresChain` das três entradas Room 1 distingue a entrada externa (livre)
  da plataforma do boss (somente após a conclusão correspondente). Os testes
  carregam o `volatiles.json` real para Normal, Mystic e Arcane.
- O `RuntimeID` privado agora é indexado por personagem em O(1) e é a mesma
  autoridade usada por visibilidade, PvP direto/AoE, suporte, party, trade,
  guild invite, ranking, ataque por ID e summons. Todo runtime de evento —
  inclusive `private_shared_entry`, `shared_timed_zone` e `state_machine` —
  forma um espaço de interação próprio: somente participantes com o mesmo
  `RuntimeID` interagem; um jogador público ou de outra execução no mesmo tile
  não ganha acesso por proximidade física.
- Summons de contrato, BM, cria/pet e Thorn Wall herdam o espaço do dono;
  aquisição de alvo e efeitos periódicos rejeitam/removem referências que
  atravessem a fronteira privada.
- Teleporte, pull e summon separam o dono do runtime da entidade excluída da
  colisão. O caster pode ser o destino da própria movimentação, mas nunca é
  ignorado quando um alvo ou uma criatura está sendo materializado ao redor
  dele.
- Affects hostis de jogador carregam `OwnerCharacterUID`; `OwnerID` fica apenas
  como fallback efêmero para sessões ativas. Ao relogar, efeitos persistidos
  sem identidade estável são descartados, e qualquer affect cujo dono
  desconecte ou atravesse runtimes é removido. Isso impede dano ou debuff sem
  uma origem viva e autorizada e evita reutilização de ClientID.
- Uma transição durante exit grace remove a associação antiga antes do
  snapshot de conta/instância. Em falha de spawn ou persistência, o item,
  posições, membership e índice são restaurados sem runtime parcial.
- Reentrada/reconexão aloca posição contra o runtime correspondente, ignorando
  jogadores de outras salas ou do mundo público, mas mantendo membros do
  próprio runtime, terreno, mobs da sala e objetos públicos como bloqueadores.

### Cube, Big Cube e Hell Gate

- O Cube possui as 25 salas do `NPCGener` nativo. As primeiras 24 abrem a
  pergunta O/X por 14 segundos; o servidor avalia a posição de cada membro,
  concede a EXP crescente ao acerto e retira somente quem errou. Normal,
  Mystic e Arcane compartilham a mesma ocupação física, como exige o mapa.
- Cada participante consome o próprio convite para entrar numa execução ainda
  na primeira sala, até o limite nativo de seis jogadores. Um convite nunca
  transporta a party inteira; depois que a sala avança, novos ingressos são
  recusados. A plataforma O/X usa exatamente `[X-3..X] × [Y-3..Y]` (4×4).
- O primeiro conteúdo é materializado sem publicação antes do commit do
  ingresso. Falha de spawn ou persistência restaura item e posição e remove os
  mobs ainda invisíveis; não existe ticket consumido com sala vazia.
- O ticket do Big Cube (1737/volatile 51) usa uma máquina O/X sem mobs: O é
  Sim, X é Não, a pergunta é privada aos inscritos e o retorno autoritativo é
  Armia `(2100,2100)`. A configuração inicial possui uma pergunta de teste;
  novas rodadas e recompensas devem entrar em `data/volatiles.json` quando a
  tabela nativa completa for confirmada.
- Hell Gate possui versões pessoal e party na mesma zona compartilhada. A
  entrada, os quatro setores com Lich e o corredor final são ondas server-side
  sob o prazo absoluto nativo de quatro minutos; uma combinação errada de Lich
  registra o erro, mas nao invalida a abertura quando o par selecionado e
  concluido.
- Coordenadas, perguntas, respostas, populações e prazos ficam em
  `data/volatiles.json`. Spawns referenciam NPCs reais e são validados pelos
  testes; coordenadas O/X e de spawn também passam pela colisão do terreno.
- A selecao de Lichs e uma mascara autoritativa do servidor; matar um Lich fora
  do par escolhido registra o diagnostico, mas nao invalida o par correto.
- Hell Gate usa a janela de entrada nativa de 240 segundos. `ScheduleEnd`
  encerra novas entradas e o mesmo prazo encerra a execucao; `ActiveDuration`
  permanece opcional somente para futuros eventos cuja fonte confirme uma
  segunda janela.
- O antigo `dungeon_teleport` e seus temporizadores paralelos foram removidos.
  Consumo, entrada, progressão, EXP e retorno usam somente o motor transacional
  de `instance_ticket`.

### Auditoria de integridade das instancias (2026-08-01)

- A adesao compartilhada e individual: cada participante consome o proprio
  convite e so pode entrar enquanto a primeira sala ainda esta parada. A lista
  de membros e alterada antes do commit e restaurada integralmente em caso de
  falha de persistencia.
- A cadeia Water preserva a janela nativa de dez segundos depois da ultima
  morte: o lider pode usar o pergaminho seguinte antes do recall, e o cleanup
  da sala anterior nao teleporta novamente quem ja entrou na proxima.
- `recall`/restart remove o personagem definitivamente da instancia antes de
  revive-lo ou reutilizar o ID de entidade. Logout/desconexao remove o ID de
  mundo, mas preserva o CharacterUID de qualquer evento retomavel (Water,
  Nightmare compartilhado e Hell Gate); o ultimo membro offline nao cria uma
  vaga fantasma e pode ser reanexado antes do proximo EnterWorld.
- O prazo da sala, o prazo absoluto e a janela horaria tem precedencia sobre
  quiz/transicao. Uma transicao nao abre a onda seguinte depois do timeout.
- Spawns sao validados contra terreno, ocupacao e IDs disponiveis. Falha
  parcial depois da publicacao envia remocao aos observadores, evitando mobs
  fantasmas ou recompensas concedidas com uma sala vazia. Geradores permanentes
  tambem ficam reservados quando um ponto ou corredor da rota cruza a sala.
- Transicoes de salas persistem posicao e `CurrentStage` no mesmo commit; a
  resposta do Cube grava EXP, membros e proxima transicao de forma atomica.
  Mobs de instancia nao carregam `GenerIndex` de NPCGener e, portanto, suas
  mortes nunca reduzem a capacidade ou o contador de respawn de geradores
  permanentes.
- A IA de mobs com `InstanceID` e privada: somente membros vivos e dentro da
  area atual podem ser alvos. Monstros publicos so adquirem jogadores do
  espaco publico. Cada passo respeita o limite da sala e uma referencia antiga
  fora do limite so pode aproximar o mob do centro, nunca perseguir um jogador
  externo.
- A visibilidade tambem e privada no protocolo: `CreateMob`, movimento, dano,
  morte e remocao de mobs de instancia sao publicados somente aos membros
  autorizados da sala atual. Ao trocar de sala, `CurrentStage` e atualizado
  antes do primeiro `CreateMob`, evitando uma onda invisivel ou vazamento para
  um observador externo.
- Jogadores de Water seguem a mesma fronteira: o `RuntimeID` e comparado antes
  de publicar `CreateMob`, movimento, vitais, affects ou morte. Membros da
  mesma sala continuam visiveis; jogadores do mundo publico ou de outra sala
  privada nao vazam mesmo quando ocupam os mesmos tiles fisicos.
- Todo loot temporário criado em runtime (Water, Cube, Nightmare, Hell Gate,
  Uxmal, recompensas e drops de jogador) carrega a mesma marca de gameplay.
  Coleta e visibilidade são filtradas server-side; o item permanece privado
  até o encerramento do runtime, quando o cleanup faz a liberação pública
  prevista pelo evento. Objetos permanentes continuam públicos e não coletáveis.
- Nightmare N/M/A agora declara `nightmareTier` nos dados. Normal e Mystic
  recusam Celestial a partir do nivel interno 180; Arcane recusa a partir de
  239 e cobra exatamente uma entrada NT por Celestial elegivel. Celestiais
  inelegiveis da party sao ignorados, enquanto o lider/solitario e recusado.
  O saldo NT e restaurado quando o spawn ou a persistencia do ingresso falha.
- As entradas de party preservam o limite nativo de tres execucoes por tier e
  por janela de horario (`partyRunLimit`); tickets pessoais nao consomem esse
  contador.
- Janelas horarias repetidas sao calculadas circularmente dentro da hora; uma
  janela que inicia em 59:00 continua valida apos a virada para 00:xx, sem
  abrir uma brecha de recusa ou de contador duplicado.
- `Cube`, `Nightmare` e `Hell Gate` usam as areas/janelas configuradas em
  `data/volatiles.json`. Water agora expande conjuntos reutilizaveis de areas
  de entrada a partir da posicao autoritativa do personagem, incluindo os oito
  setores regulares, o setor do boss e o portal legado. As coordenadas e
  duracoes atuais de Water e Magic Chamber permanecem as tabelas 7.54
  escolhidas pelo projeto; Nightmare aplica agora o limite nativo de tres
  execucoes compartilhadas por janela, sem limitar tickets pessoais.

### Courage e itens Love

- Secret/Wonder Drug of Courage usam o affect 30. Ele não enfeita o score:
  acrescenta `+1.000` ao hit físico ou `+2.000` ao hit mágico apenas contra
  mobs, sem bônus no PvP. As durações são específicas por item.
- Love Chocolate e Love Candy usam os índices de skill nativos `43/44/45/41`
  apenas para consultar o `SkillData.csv`; os affects efetivamente aplicados são
  `11/9/15/2`, com valores autoritativos `15/90/7/1`. O pacote, o consumo e o
  `character_states` são confirmados na mesma transação; qualquer falha restaura
  todos os affects e o item.
- O Baú de Experiência continua acumulando duas horas por uso, limitado a
  24 horas (`10800` unidades de oito segundos).

Na auditoria dos itens portados de `_MSG_UseItem.cpp`, a Poeira de Fada passou a
recusar Arch/Celestial como no nativo, e buffs consumíveis agora persistem item
e affect no mesmo save antes de responder ao client. Eye Wax permanece com a
regra customizada solicitada pelo projeto.

### Repliction

- Os itens `4016–4020` são Repliction normal A–E e aceitam refino até +6;
  `4021–4025` são Premium A–E e aceitam até +9.
- A classe A–E precisa coincidir com `EF_ITEMLEVEL` do equipamento. Somente
  elmo, peito, calça, luva e bota podem receber o efeito; Cytheras e tipos de
  mob incompatíveis são recusados server-side.
- O uso não soma nem subtrai o adicional existente. Ele escolhe um par completo
  da tabela nativa da parte da armadura e substitui os dois adicionais de uma
  vez. O terceiro par fica reservado a `EF_SANC` ou à tintura `116–125`, que é
  preservada com seu valor.
- `data/repliction.json` é a transcrição autoritativa de `SetItemBonus2` e
  `g_pBonusValue2..5` da W2PP. As tabelas não ficam duplicadas no código Go.

O affect 30 das comidas (`Courage`, incluindo Roast Chicken) não altera
`ExtendedScore` nem os números da janela do personagem. Enquanto ativo, ele
acrescenta **+1.000 a cada hit físico PvE** e **+2.000 a cada hit mágico PvE**,
com clamp wide normal. O bônus não transforma erro em acerto e não é aplicado
contra jogadores.

**Fase D implementada e validada localmente** (testes, vet, build e boot). A
validação visual dos cinco livros e dos summons no client 7.48 permanece como
checklist in-game.

A chave e a porta se reconhecem pelo **`EF_KEYID`**, não pelo índice: a
`First_Gate_Key` (451) tem `EF_VOLATILE 3` + `EF_KEYID 2` e a porta no chão
(458) tem `EF_GROUND 1` + o **mesmo** `EF_KEYID 2`. São 38 itens com `EF_KEYID`
no catálogo 7.48. A leitura usa `staticEffect`, sem refinação: `EF_KEYID` não
está em `nonRefinableEffects`, então uma chave refinada teria o id multiplicado.

Fora de escopo desta rodada: a implementação nativa do item `3455` como clique
direto (ele continua `no_direct_use`). O Spirit Seal (`206`) já tem
encapsulamento e extração por `0x2CD`/`0x3CC`. A Pedra Ideal (`211`), a Pedra da Fúria (`213`) e a Pedra Misteriosa
(`214`) já usam o sistema Celestial/SubCelestial. `WYD_TOTO` (`207`), Contract
Seal (`208`), Shout, Return of Ability
e ingredientes de NPC usam `no_direct_use`: são válidos, mas devem ser
entregues ao NPC/comando responsável em vez de consumidos por clique.

**Cobertura de teste do beta.** As lojas de volatile em Armia colocam os 124
códigos ao alcance. `ShopUse20A`/`ShopUse20B` `(2106-2107,2102)` expõem as 47
variantes desta rodada; Fury Stone e Mysterious Stone também ficam disponíveis
na loja de teste `ShopPhaseD`. `TestTodoVolatileTemLoja` trava a cobertura por código e
`TestLojasDeVolatileCabemNaJanela` garante o máximo de **27 slots** do client.

### Contadores por personagem

Saldo nomeado por personagem — o `extra.KefraTicket` e o `extra.Fame` do nativo.
Não existe campo novo: `Player.SpecialCoins` (`map[string]uint32`, persistido em
`character_states` pelo `Char.UID`) já é exatamente isso, já é zerado por
`resetCharacterRuntime` e já é **substituído, nunca mesclado**, por
`applyCharState`. Um campo paralelo traria duas armadilhas: `CharStateVersion` é
validado por igualdade exata (subir a versão invalidaria todo sidecar existente)
e o store **apaga** o arquivo quando `Affects` e `SpecialCoins` estão ambos
vazios, então um campo fora dessa condição sumiria no primeiro autosave.

| Contador | Ganha | Gasta |
|---|---|---|
| `kefra_ticket` | item 4127 pelo `Survivor` (+100) ou volatile `grant_counter` | 1 por entrada em Kefra |
| `fame` | `Warrior's_Seal` 4146, volatile 199 (+10) | Lindy V754 (destraves 355/370): 10; Odin (Celestial 40): 200; Fury/Arcana: 500 |

Vocabulário em `data/quests.json`: `requires.counters` (exige sem gastar),
`consumeCounters` (gasta) e `rewards.counters` (credita). O teto por contador é
`model.MaxQuestCounter` (1.000.000); `grantCounters` satura no teto em vez de dar
a volta, e `spendCounters` falha **sem debitar nada** se algum saldo não cobrir.

**Ordem transacional obrigatória.** No PostgreSQL, crafts que gastam contador
junto com itens usam `SavePlayerState`: conta e charstate entram na mesma
transação `SERIALIZABLE`, e qualquer falha desfaz ambos. O adaptador JSON de
desenvolvimento conserva o fallback sidecar-antes-da-conta. `fame_test.go`,
`counters_test.go` e os testes PostgreSQL cobrem rollback e isolamento entre
personagens homônimos.

## Idioma do servidor

O client 7.48 é **global (inglês)**, então todo texto que chega ao jogador está
em inglês. A terminologia segue `client748/UI/UIString.txt`: Party, Guild,
Whisper, Trade, Quest, Gold, Kingdom, Cargo, Mount, Refine, Shop, Buy — e
**Auto Trade** para a Loja Fantasma, que é como o client a chama.

Traduzidos: as mensagens de `internal/game` (chamadas diretas e as funções que
*retornam* texto, como `guildAuthMessage` e os requisitos de quest), as falas de
NPC do `quests.json`, os nomes/falas dos bosses em `data/boss/*.lua` e — na
rodada de 25/07 — os **nomes dos NPCs** em `data/npcs`.

### Nomes de NPC

O nome do NPC é o texto mais visível do jogo: fica flutuando sobre a cabeça
dele. Os 502 templates foram varridos e os nomes em português traduzidos.
Dois testes travam o resultado:

- `TestNomesDeNPCEstaoEmIngles` recusa marcas de português no nome;
- `TestNomesDeNPCCabemNoWire` conta os nomes acima de **12 bytes**, que é quanto
  o `CreateMob` copia (`copy(b[18:18+12], name)`) — nome maior chega **cortado**
  ao client, sem erro nem aviso. São 62 nomes longos herdados; o teste falha
  tanto se o número **subir** (nome novo cortado) quanto se **descer** sem
  atualizar a constante (para o ganho ficar travado).

**Regra de colisão:** quando a tradução colide com um NPC existente, compare a
estrutura; se forem diferentes, acrescente `_` ao final. Daí saem `Guard__` e
`Orc_Guard__`, e não nomes inventados.

Renomear um NPC quebra tudo que o referencia **pelo nome**. As quests já tinham
guarda; bosses e zonas não tinham, e um `npc = "Arq_Ciclope"` sobreviveu à
tradução e só apareceu como falha de boot. `npc_refs_test.go` fechou o buraco:
NPC base de boss, adds invocáveis e zonas de quest agora falham em segundos, não
no arranque.

**Permanecem em português de propósito:** logs e comentários (são do operador) e
os campos `description` do `volatiles.json`, que nunca são enviados ao client —
são documentação do arquivo.

Os **comandos digitados** aceitam os dois idiomas: `/criar` e `/create`,
`/convidar` e `/invite`, `/aceitar` e `/accept`, `/sair` e `/leave`,
`/expulsar` e `/expel`, `/limparinv` e `/clearinv`, além de `/reino`+`/kingdom`,
`/rei`+`/king` e `/criarsub`+`/subcreate`+`/createsub`. A tradução em massa
chegou a renomear `case "convidar"` para `case "invite"`, trocando
silenciosamente o que o jogador precisa digitar;
`TestChatCommandsAcceptBothLanguages` trava os dois nomes de cada comando.

## Encontros de boss

Subsistema aditivo: todo boss é um `Mob` comum com um `BossRuntime` paralelo.
Quatro tipos hardcoded em Go (`chaser`, `caster`, `summoner`, `phased`)
configurados por `data/boss/*.lua`, com sandbox e validação de boot. Nenhum mob
do NPCGener vira boss — cada encontro tem posição e respawn próprios.
Detalhes, decisões e pendências em `DOCS/BOSS.md`.

Dependências externas diretas: **gopher-lua** (bosses em sandbox) e **pgx**
(PostgreSQL), ambas MIT.

## Persistência PostgreSQL e UIDs

- PostgreSQL é o store autoritativo quando `database_driver=postgres`.
- Conta, índices de nickname, instâncias de item e alteração de guild relacionada
  são confirmados juntos em transação `SERIALIZABLE`, com retry limitado para
  serialização/deadlock.
- Cada item materializado recebe UID aleatório de 128 bits. O UID é exclusivo do
  servidor e não altera os oito bytes do `STRUCT_ITEM` enviados ao client 7.48.
- A chave primária global do UID bloqueia cópias entre contas; a restrição de
  localização bloqueia duas instâncias no mesmo slot.
- Loot, compra, quest, boss e resultado novo de craft emitem UID. Movimento,
  equip, cargo, drop/coleta, trade, refino, tintura e transformação preservam o
  UID da instância.
- Cada personagem também possui UID server-side. Mortal e Arch podem manter o
  mesmo nome visual sem compartilhar buffs, fama ou contadores; o Arch referencia
  o Mortal pelo UID, e excluir a ficha apaga seu charstate por cascata.
- Autosaves usam snapshots imutáveis numa fila dedicada e não bloqueante.
  Saturar a fila coalesce pelo nome da conta/UID do personagem; itens inalterados
  não são apagados nem reinseridos.
- O schema é aplicado no boot e está em `internal/store/postgres_schema.sql`.
  Instalação, teste real e backup estão em `DOCS/POSTGRESQL.md`.
- JSON permanece como adaptador explicitamente selecionado para desenvolvimento;
  falha do PostgreSQL nunca ativa fallback silencioso.

## Operação e diagnóstico

Relógio e RNG injetáveis (testes determinísticos sem `time.Sleep`), métricas
`expvar` em `/debug/vars`, pprof em `/debug/pprof` — ambos **restritos a
loopback**, com o boot derrubado se o endereço for público — e desligamento
controlado por SIGTERM/SIGINT que persiste antes de sair. Detalhes em
`DOCS/OPERATION.md`.

## Invariantes: estado que atravessa uma fronteira

Classe de bug que nenhuma ferramenta encontra (`vet`/`deadcode` não veem):
**estado que atravessa uma fronteira sem ser saneado**. O personagem que entrava
morto era um caso; a auditoria de 24/07/2026 achou outros quatro. São três
fronteiras, e cada uma tem um invariante a preservar:

**1. Ponteiro para dentro de uma fatia → mutação da fatia.** `p.Char` é
`&p.Account.Chars[slot]`. Um `append` em `Account.Chars` pode REALOCAR o array
e deixar o ponteiro órfão: as escritas vão para memória que ninguém persiste,
enquanto o save grava o array novo. Na ascensão isso deixava a Pedra e o Sefirot
intactos no disco — **ascensão repetível**. Use `growCharacterSlots`, que cresce
a fatia e re-aponta `p.Char`; trocar a fatia no rollback também exige re-apontar.

**2. Tela de seleção → mundo.** O `Player` é REUSADO no vaivém (a conta continua
autenticada), então todo campo de escopo-personagem não zerado vaza para o
próximo. Vazavam: moedas especiais (**duplicação** — o autosave as gravava no
charstate do personagem seguinte), cooldowns de skill, convite de guild (dentro
dos 30 s do TTL o personagem ERRADO podia aceitar) e mais uma dúzia de
temporizadores. `resetCharacterRuntime` zera tudo que é do personagem,
preservando apenas `Session` e `Account`.

`TestCharacterRuntimeIsFullyReset` percorre os campos do `Player` por reflexão e
exige que todos, fora dessa allowlist, fiquem zerados — **um campo novo
adicionado sem reset quebra o teste**.

**3. Conta em disco → personagem em jogo.** Ver a seção seguinte (HP zerado).
`applyCharState` SUBSTITUI o estado lido, nunca acumula.

## Entrada no mundo com personagem morto

Quem morre e sai tem `CurHP=0` persistido. Entrar assim **trava o jogador**: o
client desenha a pose de morte e não responde a nada — nem ao `/restart`.

`onEnterWorld` devolve **1 de HP** quando o personagem entra com HP zerado, e
limpa o `DeadAt`. O revive acontece **depois** do recalc (para `MaxHP` já estar
correto) e **antes** de qualquer pacote, senão `EnterWorld`/`CreateMob`/
`UpdateScore`/`SetHpMp` levariam HP 0 e o client desenharia a morte mesmo com o
servidor já corrigido.

Regressão já observada in-game; `enter_world_test.go` cobre os três casos
(entra morto, entra vivo, e o HP correto no momento em que os pacotes saem).

## Objetos permanentes de mundo

`data/init_items.csv` porta os **50 objetos** do `InitItem.csv` nativo: portões
de castelo, as quatro portas de teste, Guild Battle Gate, canhões e a Torre do
Triunfo. Entram no mapa no boot, **não expiram** e não podem ser recolhidos —
o nativo os mantém isentos do decay começando `ProcessDecayItem` em
`g_dwInitItem+1`, justamente para nunca varrer essa faixa.

São a outra metade do `gate_key`: a porta no chão é um objeto permanente, e a
chave a encontra pelo `EF_KEYID` dentro do alcance de recolhimento.

Falta validar a aparência in-game — em especial a rotação e se o client colide
com eles.

## HP acima de 32767: as três escalas

Três números decidem uma barra de vida, e confundi-los produziu os dois bugs
desta rodada:

| número | onde vive | quem lê |
|---|---|---|
| **base** | `Char.Extended`, persistido, **sem** equipamento e **sem** affect | o disco, e o operador que edita à mão |
| **efetivo** | `Char.ExtendedRuntime`, recomposto a cada recalc | a cauda wide do `0x336`/`0x181`, e o HUD patcheado |
| **projetado** | WORD de 16 bits do `STRUCT_SCORE` nativo | o handler legado do client |

**Invariante: `CurHP` nunca excede `MaxHP` — em cada uma das três escalas.**

O flagrante foi um `base=1202183/1000000` no disco: o `CurHP` base 20% acima do
`MaxHP` base. A causa estava em dois lugares que limitavam pelo teto **efetivo**
e gravavam o resultado no **base**, que não tem o buff:

- `equipment.go` copiava o `CurHP` do runtime para o base ao recalcular;
- `setPlayerCurHP`/`setPlayerCurMP` limitavam pelo `playerMaxHP` efetivo.

Os dois agora aplicam `minU32(valor, base.MaxHP)`. Limitar o base **não** custa
HP em jogo: `recalcExtendedPlayer` captura `oldHP`/`wasFullHP` a partir do
**runtime**, então o valor vivo sobrevive ao clamp. `hp_base_invariant_test.go`
e `hp_equip_base_test.go` cobrem as duas metades — a invariante e a não-perda.

O MP foi o que denunciou a causa: **nenhum** affect aumenta `MaxMP`, então um
`curMP > maxMP` só podia vir do teto efetivo do equipamento.

**Onde editar a base:** `Char.Extended` no JSON da conta. Todo cálculo é
`base + equipamento + affects`; o runtime é derivado e nunca vai ao disco.

O dano viaja na escala do alvo — detalhe em `DOCS/EXTENDED_SCORE.md`.

`/hpdebug` imprime as três escalas lado a lado com as proporções. É **comando de
beta**: some quando a barra estiver resolvida.

## Cadeia de patches do client

O `WYD.exe` em uso **não** é o executável original: é o resultado de seis
scripts aplicados em ordem, cada elo verificado por SHA-256. A cadeia inteira
reproduz o binário em uso bit a bit, e a linha-base
(`WYD.pre-extended-stats.exe`) está de volta na pasta. Detalhe, ordem de
reaplicação e os quatro bypasses ficam em `client748/PATCHES.md`. O ponto
único de entrada é
`client748/Apply-WYD748.ps1`, que retoma a partir do SHA atual e valida cada
elo. O primeiro patch também registra o byte de título da linha-base em
`0x1C5069`. O quinto elo é
`client748/Patch-WYD748-Lindy.ps1`: ele exige a entrada pré-Lindy
`4E916C1FD94D60D5EF7F8914B621BAB3787E7BF5460FB251C59F71BCC4D9BA2F`, troca
somente `81 FA AA 0F 00 00` por `81 FA 78 0D 00 00` em `0x13FB7` e imprime a
SHA final. A cópia verificada produziu
`9762B1AC6EFB4AB3C800877DE1DA048DD43EA407FCEEA945C755DF6986607F18` antes do
elo Water Macro; a saída final versionada é
`F76D9D8CEDFFBD3E046F10C5282CF0139E6D94BFC7DF30BCCA549324B0D1107E`.

## Limitações atuais

- O framework de composição (`0x3C0` e demais opcodes por NPC, packet nativo
  de 84 bytes, oito snapshots validados contra o inventário, throttle de
  800 ms, receita/chance server-side, persistência com rollback e resposta
  comum `0x3A7`) está **implementado para os sete NPCs tipo 8** (Tiny, Lindy,
  Compositor, Agatha, Aylin, Ehre, Alquimista Odin). Detalhe completo por
  NPC/receita em `DOCS/CRAFTING.md`. Pendências reais: (a) a matriz de
  validação visual in-game receita por receita (sucesso/falha/consumo/visual/
  relogin/rollback), já listada em `CRAFTING.md`; (b) **Extração** continua
  fora de escopo, embora Celestial/SubCelestial já estejam implementados.
  **Nota de nomenclatura:** "Alquimia" é uma **skill
  da classe HT**, distinta das receitas do Alquimista Odin — não confundir as
  duas.
- O fechamento explícito de loja (`0x196`) não possui handler dedicado; abrir
  outra interação ou sair do alcance limpa o contexto pelo fluxo atual. Não há
  evidência de que o client 7.48 envie esse opcode: ele aparece no client 759,
  mas **não** na captura real do 7.48.
- Guild já cobre criação, convite, aceite, expulsão, sucessão, chat, recrutamento
  nativo `0x3D5`, identidade/rank nos pacotes 7.48 e exportação de `Guilds.txt`.
  Ainda não há guerra, zonas, torre, taxa, sub-guild nem aliança; operações que
  envolvem outro membro exigem o alvo conectado.
- A identidade server-side da guildmark está concluída, mas o desenho ainda
  depende do BMP 16x12 que o próprio client baixa de um host hardcoded. Faltam
  publicar esse asset (ou aplicar o patch de URL) e validar o resultado in-game.
- **Arch e Celestial/SubCelestial implementados** — cadeia completa (Sefirot,
  Pedra Eterna, ascensão no rei e Pedra Ideal), nível mínimo 370 para Arch,
  criação Celestial 356+/Mortal respectivo 400, criação Sub no 121, progressão
  separada por forma, pontos compartilhados pela fórmula nativa, travas 40/90,
  redução cumulativa de EXP, Soul, Cythera e troca por Pedra Misteriosa.
  A parte Arch mantém
  rei), nível mínimo 370, orçamento de pontos próprio proporcional ao nível
  do Mortal de origem, **travas de nível 355/370** destravadas na Lindy e as
  **quatro juras elementais** de Kefra. A equipagem também segue o nativo:
  sem requisitos de level/stats, armas de qualquer classe e armaduras pela
  classe corporal do Mortal (`rosto/10`), não pelo Sephirot. Detalhes em
  `DOCS/ASCENSION.md`. **A nova cadeia Celestial ainda não foi validada
  in-game.**
- **A barra de HP/MP do client ainda desenha errado** com HP wide alto: as duas
  aparecem quase zeradas. Os números do servidor estão coerentes entre si
  (`/hpdebug` mostra efetivo e projetado na mesma proporção) e todos os pontos
  de leitura do patch apontam para o sidecar, então a suspeita é o sidecar não
  estar sendo alimentado em algum caminho. Diagnóstico seguinte é OllyDbg com
  breakpoint na escrita do sidecar — trabalho de binário, não de servidor.
- O Arch herda o **nome** do Mortal (fiel ao nativo), então o servidor tem
  homônimos. Dois invariantes tornam isso seguro, e ambos são load-bearing:
  1. homônimos **só** nascem da ascensão, logo estão sempre na **mesma conta**
     (a criação normal, `0x20F`, exige nome globalmente único);
  2. uma conta **não entra duas vezes** ao mesmo tempo (`claimAccountSession`,
     coberto por `TestAccountSessionIsExclusiveCaseInsensitive`).

  Daí decorre no máximo **um homônimo online**, e as buscas por nome (sussurro,
  convite de grupo, guild) **não precisam de desempate** — a limitação que
  constava aqui não existia. Se algum dos dois invariantes cair, essas buscas
  passam a ser ambíguas **em silêncio**; o raciocínio está registrado em
  `playerByCharacterName`.

  O que era um furo real: `onDeleteCharacter` liberava o nome de `charNames` sem
  checar o gêmeo, então apagar o Arch devolvia o nome do Mortal ao pool global e
  outra conta podia criá-lo. Corrigido com `accountUsesName`; a bateria fica em
  `internal/game/character_name_test.go`.
- Não existe **carta offline**: toda mensagem exige o destinatário online e
  responde com aviso quando ele não está.
- **Boss v1** cobre 4 eventos, 6 condições e 5 ações; timers, seletores de
  alvo, arena e reset de encontro seguem fora (`DOCS/BOSS.md`).
- Desligamento controlado **não validado em processo real** no Windows
  (`Stop-Process` é kill forçado, não sinal).

## Cobertura automatizada

Medição consolidada de 02/08/2026 com `go test ./... -cover`:

| Pacote | Cobertura |
|---|---:|
| `internal/wire` | 88,2% |
| `internal/game` | 78,0% |
| `internal/data` | 70,1% |
| `internal/account` | 71,6% |
| `internal/accountapi` | 69,6% |
| `internal/store` | 62,8% |
| `internal/model` | 53,3% |
| `internal/net` | 67,5% |
| `internal/loadtest` | 33,5% |
| **Total do módulo** | **72,4%** |

O núcleo `game` partiu de 44,3% e permanece próximo de 78%. Toda função finita do pacote
é executada por teste; somente `World.Run` permanece em 0%, pois é o loop
infinito de produção. A bateria cobre fluxos integrados de party, chat, guild,
reinos, lojas, Cargo, trade entre jogadores, loja fantasma, inventário,
drop/coleta, movimento, desconexão, skills comuns e Sephira, buffs/debuffs,
volatiles, crafting, montarias, combate PvE/PvP, morte/revive, visibilidade,
charstate, IA ativa dos mobs e o ciclo completo de spawn, morte, drop e respawn
de bosses.
Cada código `EF_VOLATILE` real possui um subteste de resolução contra
`itemlist.csv`, e os 50 opcodes de gameplay reconhecidos atravessam framing,
fase de sessão e dispatcher em casos isolados. Os tamanhos de todos os pacotes
7.48 confirmados também são verificados por uma tabela exaustiva.
Ela também encontrou e passou a proteger o rollback da aliança de guild:
o snapshot agora é tirado antes de alterar `Guild.Ally`, portanto uma falha de
gravação não deixa o estado em memória diferente do arquivo persistido.

Os pacotes `cmd/*` continuam em 0% porque são pontos de composição/processo
(`main`); a lógica que eles chamam está nos pacotes internos acima. Cobertura
não substitui o teste in-game do client 7.48, especialmente para animações,
efeitos visuais e patches do executável.

## Verificação local

```powershell
cd wyd-go
go test ./...
go vet ./...
go build -o tm.exe ./cmd/server
go build -o account-api.exe ./cmd/account-api
go build -o account-create.exe ./cmd/account-create
```
# ExtendedScore v2

## Auditoria de instancias (2026-08-01)

As instancias agora compartilham uma fronteira unica de seguranca: alvo,
`REQMobByID`, `CreateMob`, movimento, dano, HP, morte e remocao verificam a
associacao do jogador a sala. O ataque fisico deixou de usar `broadcast` para
mobs e passou a publicar somente aos observadores autorizados.

O estado agregado de Nightmare e Hell Gate sobrevive ao reinicio em
`instance_state.json` ou na tabela PostgreSQL `instance_state`. A abertura e o
ingresso de zonas compartilhadas persistem ticket, posicao, membros, contador e
janela no mesmo commit das contas. Hell Gate reconstroi entry, puzzle de Lichs e
final; HP individual e IDs de mobs nao sao persistidos por serem entidades
volateis.

Cube usa `noCombatTimeout=true`: os 180 segundos antigos nao encerram a sala.
Nightmare conserva parties de ate 13 jogadores, limite de tres entradas de
party por tier/janela e capacidade fisica de 39; tickets pessoais nao contam.
Big Cube possui o núcleo O/X habilitado; a cadeia completa de rodadas e
recompensas permanece configurável até a tabela nativa 7.48 ser confirmada.

Os atributos de jogadores, NPCs e monstros foram consolidados em uma única
estrutura `uint32`. O WireScore WORD do client é somente uma projeção gerada no
wire; o contrato estrito e o patch do client estão em `DOCS/EXTENDED_SCORE.md`.

## Cápsula Celestial e protocolos 0x2CD/0x3CC

O volatile 206 deixou de ser um bloqueio genérico. O item 3443 (`Spirit's Seal`)
é aceito somente para a forma Celestial/SubCelestial ativa e cria um snapshot
persistente em `Account.CelestialCapsules`. O item codifica o ID no par de
efeitos `59/high/59/low` e é colocado no Cargo com UID próprio. A conta, o
Cargo e o charstate do personagem retirado são confirmados em uma única
transação PostgreSQL; o fallback JSON mantém a mesma ordem de rollback.

`0x2CD` valida o ID do selo e responde `0xDC3` com o `MSG_CAPSULEINFO` compacto
de 52 bytes confirmado diretamente no `WYD.exe` 7.48. `0x3CC` valida origem Carry/Cargo, item UID, nome ASCII,
terreno lógico e slot livre; somente depois do commit remove o selo e cria uma
nova ficha com CharacterUID novo. A ficha preserva rosto/capa e progressão,
mas limpa carry, gold, guild e os equipamentos restantes conforme
`DBOutCapsule.cpp`. O personagem é removido do World após a confirmação, sem
cadáver ou clone residual.

O item 3455 (`Extraction_of_Magical_Power`) continua `no_direct_use`: embora
compartilhe EF_VOLATILE 206 no itemlist, o `_MSG_PutoutSeal.cpp` nativo só aceita
o índice 3443. A auditoria e os testes estão em
`DOCS/CELESTIAL_CAPSULE.md` e `internal/game/celestial_capsule_test.go`.

## Uxmal / Pista de Runas

O NPC `Uxmal` agora inicia a Pista de Runas pelo fluxo nativo, sem transformar
o clique em loja ou quest genérica. A configuração autoritativa fica em
`data/volatiles.json` (`instances.uxmal`): as janelas de entrada são 16--19,
36--39 e 56--59, a entrada exige o líder e os membros na caixa nativa, e a
primeira sala possui duas vagas enquanto as demais possuem três.

Cada execução recebe uma chave de runtime `uxmal:<sala>:<vaga>`, move a party
em uma transação, cria somente os mobs da sala e restringe a visibilidade aos
membros. Ao limpar a sala, cada membro recebe uma runa da tabela nativa e o
líder recebe a próxima `Clue_Of_Runes` com `EF_SANC` da sala seguinte. Item,
posição, mobs e estado da instância seguem o contrato validar -> persistir ->
publicar; falha de persistência restaura o ticket, a party e os mobs. Quando o
inventário está cheio, os itens de chão são materializados sem publicar antes
do commit; qualquer falha de criação ou persistência remove os objetos
temporários e deixa a recompensa pendente para nova tentativa.

`ShopUxmal` e o gerador 5132 são somente acessórios de teste para fornecer o
ticket 5134 e as runas no client. Os testes de `internal/game/uxmal_test.go`
verificam janela, caixa de entrada, capacidade, spawn, cadeia de recompensas,
rollback de inventário e rollback de recompensa no chão.

## Robustez de alocação e autosave JSON

O allocator de mobs nunca publica o ID zero: spawns de NPCGener, bosses,
evocações, pets e objetos Sephira abortam ou interrompem a operação quando a
faixa está esgotada. O allocator de itens de chão também percorre a faixa no
máximo uma vez, respeita os 100 IDs reservados para canhões e falha de forma
explícita em vez de entrar em loop após o wrap de `uint16`.

O adaptador JSON mantém a fila de autosave não bloqueante: snapshots da mesma
conta ou charstate são coalescidos quando o canal está cheio, enquanto as
barreiras de saves síncronos permanecem ordenadas e nunca são descartadas.

Na medição final de 02/08/2026, `go test ./... -cover` registrou 78,0% em
`internal/game`, 62,8% em `internal/store` e 88,2% em `internal/wire` (72,4%
no módulo completo, incluindo os executáveis sem lógica unitária própria).

## Guildmark e identidade visual

O client 7.48 não recebe o desenho da guildmark em um pacote do TMSrv. Ele
extrai o índice de guild dos 12 bits baixos de `Guild` e o rank do
`GuildLevel` em `CreateMob`, `UpdateScore` e `EnterWorld`. O servidor agora
preenche os três campos, limita o índice à ABI nativa, reenvia a identidade
somente quando guild/rank mudam e reescreve `Guilds.txt` após o commit.

A textura é buscada pelo próprio client como `b010000<guild-id>.bmp` no host
hardcoded do executável. Portanto, uma guild nova precisa ter o BMP 16x12
24-bit publicado nesse host ou receber um patch de URL no client; não foi
inventado um pacote de textura incompatível.

## Protocolos sociais 0x3D5 e 0x3E8

O convite nativo de guilda (`0x3D5`) agora valida o ClientID do recrutador,
alvo online, reino, rank, tipo (0--3), domingo, limite de 64.000 membros,
gold e intervalo de dois segundos. A inclusão do membro, o gold, a guild e as
duas contas entram no mesmo `SaveGameState`; falha restaura os personagens e o
registro. Depois do commit, o GuildID é rematerializado somente uma vez para
atualizar a identidade visual, sem usar CreateMob durante movimento.

A recompra (`0x3E8`) usa o layout real do client 7.48 de 176 bytes (dez
entradas de `Order + STRUCT_ITEM + Price`). A lixeira é efêmera por personagem:
venda de item preserva o UID, mantém no máximo dez entradas e desloca a mais
antiga quando cheia. O custo da recompra é recalculado pelo `itemlist.csv` e
pela quantidade `EF_AMOUNT`, nunca pelo preço enviado pelo client. Item, gold e
lixeira são persistidos juntos; o fluxo aceita somente uma loja aberta e
restaura tudo quando o save falha. O pacote completo e o pedido curto de header
são aceitos de forma explícita pela fronteira de segurança.

## Auditoria geral de robustez — 02/08/2026

A fronteira de rede agora exige o tamanho exato confirmado para todo opcode
despachado pelo World, exceto as duas formas documentadas de `0x3E8`. Isso
inclui chat, whisper, criação/exclusão de personagem, todos os compositores e o
`0x336` ignorado; caudas anexadas são recusadas antes de qualquer parser. A
`Session.Send` também rejeita builders nulos, truncados ou maiores que o limite
do protocolo sem entrar em panic. Os três fuzzers de pacote passaram por mais
de dois milhões de execuções curtas sem crash.

O processamento round-robin passou a drenar primeiro o backlog mais antigo,
evitando starvation sob flood contínuo. Depois do snapshot de shutdown nenhum
comando já enfileirado pode alterar o mundo. Se um handler entrar em panic, o
servidor fecha as sessões e marca os snapshots em RAM como não persistíveis;
assim um estado parcial não chega ao banco no disconnect ou autosave.

Mortes simples, skills multi-alvo e dano periódico consolidam as contas numa
única transação. O dono do loot participa sempre, inclusive no level cap ou
quando somente outros membros recebem EXP. Se a confirmação do combate falhar,
todas as contas envolvidas são isoladas e reconectam no último commit válido,
em vez de um autosave posterior confirmar gold, item ou EXP pela metade.

A recuperação de instâncias prioriza o `ConfigID` exato antes de aliases de
grupo compartilhado e grava snapshots em ordem determinística. O boot de Uxmal
valida NPC, catálogo de ticket/runas, áreas, terreno e sanção da próxima pista.
Allocators de mob/drop são limitados, drops nas bordas não sofrem wrap de
`uint16`, e uma célula sorteada bloqueada conserva a origem caminhável.

Foram removidos wrappers e fórmulas sem chamador real de combate, IA, skills,
instâncias, load test e wire. `staticcheck`, `go vet`, `govulncheck`, build,
testes normais, testes embaralhados e `go test -race ./internal/game` estão
verdes.

## Morte PvP sem recompensa econômica

Matar outro jogador não concede EXP nem gold. O `0x338 CNFMobKill` contém um
total absoluto de EXP, e o client chama `SetMyHumanExp` quando o killer é o
próprio personagem ou alguém da party. Por isso a morte de jogador é publicada
individualmente: cada destinatário recebe sua própria EXP atual, nunca a EXP da
vítima ou do killer. O mesmo serviço atende ataque físico, skills, monstros e
summons que matam jogadores; Chaos Point, carta e animação de morte permanecem.

### Complemento de isolamento de runtime — entregue em 08/08/2026

Novos testes cobrem a matriz público/runtime, loot, party EXP, UID de affect,
colisão de Hell Gate, efeitos mob-owned sem crédito e reconexão de membros
pendentes. A validação desta rodada passou em `go test -count=1 ./...`,
`go vet ./...`, `go test -race ./internal/game`, build do servidor e
`git diff --check`.

O `Mob` não possui mais um array paralelo de donos por ClientID. Affects
criados por jogadores usam `OwnerCharacterUID`; efeitos do próprio mob/boss
usam somente o ID vivo do mob no mesmo GameplaySpace e nunca atribuem EXP,
gold ou loot a um jogador. A colisão agora é resolvida por uma função única:
terreno, NPCs estáticos e lojas fantasma são globais; jogadores, monstros e
summons dinâmicos bloqueiam apenas o espaço correspondente. Capacidade de
instância é calculada por identidades únicas, incluindo UIDs pendentes de
reconexão, e o índice de jogadores por CharacterUID é O(1) no runtime real.
