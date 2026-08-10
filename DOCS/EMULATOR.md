# WYD-Go — Documentação Completa do Emulador (7.48)

> **Objetivo deste documento:** conter TODA a informação necessária pra reconstruir o
> servidor WYD 7.48 do zero, mesmo que todo o resto se perca. É a referência-mãe:
> protocolo byte-a-byte, criptografia, arquitetura, fórmulas, formatos de dados e os
> "gotchas" ganhos a duras penas (cada um custou horas de debug). Complementa o
> `../CLAUDE.md` (guia rápido) e o `../../plan.md` (arquitetura).

Índice:
1. [Visão geral e decisões](#1-visão-geral-e-decisões)
2. [Build, run e rede](#2-build-run-e-rede)
3. [Arquitetura (modelo de ator)](#3-arquitetura-modelo-de-ator)
4. [Camada wire: header, criptografia, framing](#4-camada-wire-header-criptografia-framing)
5. [Referência completa de pacotes](#5-referência-completa-de-pacotes)
6. [A sequência de login/enter-world](#6-a-sequência-de-loginenter-world)
7. [Os GOTCHAS críticos (lista de ouro)](#7-os-gotchas-críticos-lista-de-ouro)
8. [Modelo de dados e formatos JSON](#8-modelo-de-dados-e-formatos-json)
9. [Sistemas de jogo (combate, loja, persistência)](#9-sistemas-de-jogo)
10. [Estrutura de código](#10-estrutura-de-código)
11. [Como adicionar um handler](#11-como-adicionar-um-handler-de-pacote)
12. [Onde vive a verdade (fontes)](#12-onde-vive-a-verdade-fontes-de-referência)
13. [Estado atual e roadmap](#13-estado-atual-e-roadmap)

---

## 1. Visão geral e decisões

**O que é:** um TMSrv (game server) do WYD 7.48 escrito **do zero em Go**, que fala o
protocolo do **client 7.48 real**. Substitui a tentativa em C++ de estreitar os structs
do engine 759 pro wire 7.48 — que **quebrava o engine 759** (os campos `Merchant`/
`AttackRun` do `STRUCT_SCORE` são bytes CHEIOS que o engine empacota; separar em
nibbles `:4` perdia metade → "todos mortos"; WORD narrowing estourava stats 759).

**Por que Go:** o wire 7.48 é declarado/serializado EXPLICITAMENTE (byte a byte via
`encoding/binary`), então "estreitar um struct" não existe — modelamos os tipos 7.48
desde o início. Uma goroutine por conexão, sem o modelo single-thread do TMSrv nativo.

**Princípio:** fatias verticais finas sobre uma fundação correta. Estrutura acima de
features (pedido explícito do felipe).

---

## 2. Build, run e rede

```bash
cd wyd-go
go build -o tmsrv.exe ./cmd/server   # Go 1.26, módulo "wydgo"
go vet ./...                          # sempre antes de dar por pronto
go test ./...                         # regressões de wire, game, dados e contas
./tmsrv.exe                            # lê data/server.txt
```

- **Configuração operacional:** `data/server.txt` define endereço de escuta e
  caminhos de todos os dados autoritativos. Outro arquivo pode ser selecionado
  com `-config data/server.vps.txt`.
- **Balanceamento global:** `exp_minimum=10000` define o piso da recompensa
  base, `exp_rate=100` é a porcentagem global e `party_exp_bonus=2` é o bônus
  percentual por membro elegível. A ordem é piso → rate → party. Rate zero
  desativa EXP. Em mortes de mobs, os bônus de EXP ligados aos itens do
  matador (Coral, fada ativa e Baú de Experiência) são aplicados à parcela de
  todos os membros elegíveis; as quedas de evolução continuam individuais.
- **Log do NPCGener:** `npcgener_log=summary` agrega grupos, mobs e
  reposicionamentos numa linha inicial e depois, havendo atividade, no máximo
  uma linha por minuto. `quiet` silencia a telemetria do gerador e `verbose`
  restaura uma linha por grupo/reposicionamento. Erros de parse, templates
  ausentes e falhas de inicialização nunca são silenciados.
- **Precedência:** uma flag de linha de comando sobrescreve o valor lido do TXT.
  As flags disponíveis são `-addr`, `-npcs`, `-gener`, `-accounts`, `-items`,
  `-itemnames`, `-skills`, `-droprates`, `-volatiles`, `-quests`,
  `-quest_zones`, `-characters`, `-heightmap` e `-attributemap`. Os dois mapas usam por padrão os arquivos do
  `Server Star Micronics/TMSRV/run`, relativos à raiz de `wyd-go`.
- **IP público:** não deve ser colocado em `listen_address`. No VPS, o servidor
  continua escutando em `0.0.0.0:8281`; o IP público ou domínio é configurado no
  `serverlist` do client. Isso também funciona quando o provedor usa NAT.
- No Ubuntu, copie `HeightMap.dat` e `AttributeMap.dat` para `data/maps/`, altere
  as duas linhas correspondentes no TXT e execute o binário a partir da raiz de
  `wyd-go`, para que os caminhos relativos sejam resolvidos corretamente.
- Há testes automatizados, mas o teste final continua sendo conectar o client 7.48 real.
  O método de investigação: logar um pacote cru (opcode + bytes) no
  `default` do dispatch, fazer a ação no client, ver o opcode, e ler o handler
  correspondente no SOURCE do client. **Probe** (plantar valor reconhecível, perguntar
  o que aparece) resolveu affect, 0x337, e a cor do nome.
- **Rede:** o client aponta pro IP **Radmin VPN `26.95.112.152`** (o LAN IP do felipe
  oscila 192.168.1.9↔.5 em restart de modem). O servidor binda 0.0.0.0.
- **Rebuild não pega se o `tmsrv.exe` antigo ainda roda** → `taskkill //F //IM tmsrv.exe`
  antes de reiniciar.
- O boot informa as etapas de catálogo, terreno, NPCs, geradores, templates,
  volatiles e o endereço de escuta. Falha em dado autoritativo obrigatório encerra
  o processo em vez de iniciar um mundo parcialmente configurado.

---

## 3. Arquitetura (modelo de ator)

Uma **goroutine por World** é dona EXCLUSIVA de todo o estado do jogo. Zero mutex no
caminho quente.

```
[Session read gr] --decode--> world.commands (chan, buf 1024) --\
                                                                  > [GAME LOOP] --Send()--> session.out --> [write gr] --cripto--> socket
[ticker 500ms] -------------------------------------------------/     (único dono do estado)
```

- `net.Session.Serve`: handshake InitCode → loop `wire.ReadPacket` → empurra
  `command{s,pkt}` no `world.commands` (bloqueia só se o buffer encher = backpressure
  daquele client). `pkt==nil` = desconexão.
- `World.Run`: `select` entre `commands` e `ticker`. Processa **linearmente** →
  movimento/itens/trade/combate serializados por construção (base do anti-dupe).
- Mobs e jogadores possuem índices espaciais em células 16x16. Visibilidade,
  colisão e aquisição de alvo consultam apenas células vizinhas; movimento,
  entrada e saída atualizam os índices por evento.
- Monstros sem jogador vivo num raio 16 ficam adormecidos. Uma vez acordados,
  aquisição roda a cada 1 s; perseguição e patrulha iniciam trechos a cada 2 s.
  O executor de ataque roda a cada 500 ms para cumprir o cooldown de 1,5 s sem
  reduzir o DPS. Apenas os affects permanecem em 6 shards (3 s por mob).
- `session.Send` é **não-bloqueante** (`out chan`); goroutine de escrita drena. O loop
  nunca trava num socket lento (se `out` enche, desconecta).
- **`Send` cifra o buffer in-place.** Pra broadcast, o builder é chamado 1x POR player
  (cada um precisa do seu `[]byte`) — ver `World.broadcast`.

**Regra de dependência:** `wire` não conhece `game`; `game` não faz I/O de socket (usa
`net.Session`); `store` é interface; `model` é puro (sem deps de projeto).

---

## 4. Camada wire: header, criptografia, framing

### Header `_MSG` (12 bytes, little-endian)

| off | campo | tipo | descrição |
|----|-------|------|-----------|
| 0 | Size | u16 | tamanho do pacote INTEIRO (o client frameia por isso) |
| 2 | KeyWord | u8 | índice na tabela de chave (aleatório por pacote) |
| 3 | CheckSum | u8 | checksum (validado no recv) |
| 4 | Type | u16 | opcode |
| 6 | ID | u16 | id do cliente/conn OU um Scene* fixo |
| 8 | Tick | u32 | tick do servidor |

### InitCode (handshake)

Ao conectar, o client 7.48 manda **4 bytes = `0x1F11F311`** ANTES de qualquer pacote.
O servidor lê, valida `== 0x1F11F311`, e segue. Sem isso, nada decodifica.

### Criptografia (pKeyWord)

Simétrica, byte-a-byte, a partir do **byte 4** (Type em diante). Tabela **`pKeyWord[512]`**
(512 bytes fixos — ver `internal/wire/crypt.go`; portada EXATA do `CPSock.cpp` do 759).

**Decrypt** (do byte 4 até `Size`):
```
key   = pKeyWord[KeyWord * 2]
pos   = key
sum1 = sum2 = 0
para i de 4 até size-1:
    sum2 += msg[i]
    trans = pKeyWord[(pos % 256) * 2 + 1]
    op por (i & 3):  0: msg[i] -= trans<<1
                     1: msg[i] += trans>>3
                     2: msg[i] -= trans<<2
                     3: msg[i] += trans>>5
    sum1 += msg[i]
    pos++
checksum VÁLIDO se (sum2 - sum1) == CheckSum
```
**Encrypt** é o inverso: `sum1` acumula ANTES da op, ops invertidas (`+=trans<<1`,
`-=trans>>3`, `+=trans<<2`, `-=trans>>5`), `sum2` acumula DEPOIS, e escreve
`CheckSum = sum2 - sum1`. (O jogo irmão Aika usa a MESMA lógica em blocos de 4 bytes.)

### Framing

`ReadPacket`: lê 2 bytes (Size), aloca `Size`, lê o resto, decifra in-place.
`FinishPacket`: escreve `Size = len(buf)`, cifra com um `iKey` (contador atômico).

### Esquema opcode | FLAG (como validar opcodes novos)

`Type = base | FLAG`. `FLAG_GAME2CLIENT = 0x100`, `FLAG_CLIENT2GAME = 0x200`.
Ex.: `SetHpMp = 129|0x100 = 0x181`, `ShopList = 124|0x100 = 0x17C`. Esse casamento é
como validamos opcodes contra o source do client 759. (`0x300 = G2C|C2G`.)

### Scene IDs (campo ID@6 de pacotes "de sistema")

`SceneField = 0x7530` (CreateMob, WarInfo, ShopList), `SceneEnter = 0x7531`
(EnterWorld), `SceneCharList = 0x7532` (CharList). O client casa por esses valores fixos.

---

## 5. Referência completa de pacotes

Structs base usadas abaixo:

**STRUCT_SCORE (28 bytes):** `Level u16@0`, `Defense u16@2`, `Attack u16@4`,
`Merchant u8@6`, `AttackRun u8@7`, `MaxHP u16@8`, `MaxMP u16@10`, `CurHP u16@12`,
`CurMP u16@14`, `Str u16@16`, `Int u16@18`, `Dex u16@20`, `Con u16@22`,
`Mastery[4]@24`. **Merchant = nibble baixo (tipo) | nibble alto (direção); bytes
CHEIOS, nunca nibbles separados.** AttackRun = Att*16+Run.

**STRUCT_ITEM (8 bytes):** `sIndex u16@0` + `Eff[6]@2` (3 pares efeito/valor).

**Place types** (SwapItem/SendItem): `0=equip, 1=inventário, 2=cargo/baú`.

### Client → Servidor

| Op | Nome | Tam | Campos-chave |
|----|------|-----|--------------|
| 0x20D | ConnectAccount | 116 | conta@12 (cstr 16B), senha@28 (12B), **cliver@40 (u32)** |
| 0x20F | CreateCharacter | 36 | slot@12, nome@16 e classe@32; validacao autoritativa no servidor |
| 0x213 | CharacterLogin | 36 | entrar no mundo (slot selecionado) |
| 0x366 | Action (walk) | 52 | PosX/Y@12, Speed@16, Effect@20, TargetX/Y@24, Route[24]@28 |
| 0x376 | SwapItem | 20 | SourType@12, SourPos@13, DestType@14, DestPos@15 |
| 0x28B | UseNPC | 20 | npcid@12, ClickOk@16 (click NPC genérico) |
| 0x27B | ReqShopList | — | TargetID@12 (click em mercador tipo 3) |
| 0x379 | BuyItem | 24 | **TargetID@12 (VEM 0!)**, sellSlot@14, MyCarryPos@16 |
| 0x37A | SellItem | — | **TargetID@12 (VEM 0!)**, MyType@14, MyPos@16 |
| 0x39D | AttackOne | 48 | ataque/skill de um alvo; alvo em pares `{ID,Damage}` a partir de @44 |
| 0x36C | AttackMulti | 96 | skill multi-alvo, capacidade wire de 13 alvos |
| 0x39E | AttackTwo | 52 | skill de dois alvos |
| 0x270 | GetItem | 28 | DestType i32@12, DestPos i32@16, ItemID u16@20, GridX/Y@22/@24 |
| 0x272 | DropItem | 32 | SourType i32@12, SourPos i32@16, Rotate i32@20, GridX/Y@24/@26, ItemID@28 |
| 0x378 | SetShortSkill | 32 | barra de 20 atalhos @12; validada contra skills aprendidas |
| 0x277 | ApplyBonus | 20 | tipo@12: 0=stats, 1=mastery; skill não é aprendida por este pacote |
| 0x37F/0x3AB/0x37E | Party | variável | convite, aceite e remoção de membro |
| 0x383/0x384 | Trade/CloseTrade | 156/12 | convite, oferta, confirmação e cancelamento de troca |
| 0x397 | AutoTrade | 196 | título@12, 12 itens@36, slots@132, preços@144, Tax@192 e TargetID@194 |
| 0x398 | ReqBuy | 36 | anúncio@12, dono@16, preço@20, Tax@24 e item@28 |
| 0x39A | ReqTradeList | 16 | ID virtual da Loja Fantasma@12 |
| 0x333/0x334 | Chat/Whisper | variável | chat local/comando e mensagem privada |
| 0x289 / 0x291 / 0x2CB | Restart/ChangeCity/MoveStop | 12/16/36 | renascer, cidade e interrupção antes do ataque |
| 0x28E / 0x28F | Guild Challenge | 16+ | reconhecidos; disputa de cidades aguarda o sistema de guild/guerra |
| 0x290 | ReqTeleport | 16 | seleciona a rota pela posição atual e usa `data/teleports.ini` |
| 0x399 / 0x166 | PKMode/PKInfo | 16/16 | ativa/desativa e publica o estado de PK |
| 0x367 | ActionStop | 52 | encerra ação/movimento enviado pelo client |
| 0x373 | UseItem | 36 | origem/destino, XY e ItemID; efeito é consultado no servidor |
| 0x3C9 | Premium Firework | 52 | slot do Carry, XY e bitmap 10x10; item e posição são revalidados no servidor |
| 0x3A0 / 0x3AE | Ping/SysQuit | 12/16 | keepalive e saída confirmada |

### Servidor → Client

**0x10E CharList (1800B, ID=0x7532):** `SelChar@12`; dentro do SelChar: HomeTownX@0
(=@12), HomeTownY@8 (=@20), Name@16 (=@28), Score@80 (=@92), Equip[16]@192 (=@204).
`AccountName@1784`.

**0x114 EnterWorld (788B, ID=0x7531):** PosX@12, PosY@14; **mob base @16:**
Name@16 (16B — **MobName[12]=@28 = byte de chaos, 75 = nome neutro**), Class@36,
Coin@40, Exp@44, SPX@48, SPY@50, BaseScore@52, CurrentScore@80, Equip[16]@108,
Inv[64]@236; cauda do MOB: LearnedSkill@748, statusPts@752, masterPts@754,
skillPts@756, Critical/SaveMana@758, SkillBar[4]@760, MagicIncrement@765,
RegenHP/MP@766 e Resist1..4@768; **cauda final DECIFRADA (2026-07-14): Slot@772 (u16)
e `ClientID`@774 (u16) — é DAQUI que o client aprende o ID do próprio personagem**
(`TMSelectCharScene: m_dwCharID = pCharLogin->ClientID`; o "trailing mágico
`02 00 01 00`" da captura era Slot=2 + ClientID=1). Weather@776. Hardcodar ClientID=1
foi a causa-raiz do bug do relog: com player id≠1 o self dessincroniza (CP lixo
gigante, HP/MP travados). Escrever sempre o id REAL. **Size = 788, NÃO 1244** (o
client frameia por Size; 1244 desincroniza → char duplicado/incontrolável).

**0x364 CreateMob (176B, ID=0x7530):** PosX@12, PosY@14, ClientId@16, Name@18 (12B —
**MobName[12]=@30 = chaos, 75 se self spawn=2**), Chaos@30/CurKill@31/TotKill@32,
**ItemEff/mesh[16]@34** (16 WORDs = aparência), **Affect[16]@66**, Guild@98,
**Status(Score 28B)@100** (curHP@112 = vida REAL do mob), **Spawn@128** (0=NPC, 2=player).

**0x363 CreateMobTrade (200B, ID=0x7530):** clone visual da Loja Fantasma.
O prefixo coincide com `CreateMob` até @171; `StoreName[27]` começa em @172.
Esse deslocamento vem diretamente de `PacketProtocolV754/GetCreateMobTrade.cpp`,
que converte o nome da loja para @172. Usar @176 deixa o letreiro truncado ou vazio.

**0x181 SetHpMp (20B nativo / 36B player extended, ID=mobID):** Hp@12, Mp@14,
ReqHp@16, ReqMp@18. (Só baixa a barra.)

**0x336 UpdateScore (92B nativo / 236B player XSC2, ID=mobID):** Score@12,
Critical@40, SaveMana@41,
**Affect[16] compacto@42..73**, GuildMemberType@74, GuildIndex@75,
RegenHP/MP@76/@77, Resist[4]@78..81, CurHP@82, CurMP@84 e Magic@86.
É um pacote público: em players e mobs, atualiza `m_usAffect` e dispara os
efeitos visuais que os demais clientes precisam enxergar.

### Contrato HP/MP extended do client 7.48 patched

Personagens acima do limite nativo usam tres representacoes com papeis
deliberadamente distintos:

1. `Extended` é a base persistida e `ExtendedRuntime` é o estado efetivo
   autoritativo `uint32` do servidor. Equipamento, refinação, affects e passivas
   são sempre recompostos sobre essa base.
2. `Score` e uma projecao proporcional signed-safe, limitada a 30000, usada
   apenas pelo wire, barras, animacoes e regras internas legadas do executavel.
   Nenhum cálculo server-side lê essa projeção.
3. O sidecar `.xstat` do client guarda os valores reais recebidos nas caudas:
   `0x336` tem 236 bytes e `0x181` tem 36 bytes.

Todo personagem, template e NPC usa exclusivamente `ExtendedScore` versão 2.
Os loaders rejeitam campos desconhecidos, versões diferentes e estruturas
antigas; não existe conversão silenciosa durante o boot.

O prefixo estreito de `0x336`/`0x181` deve sempre receber `Score`, preservando a
proporcao atual/maximo. Saturar ambos em 32767 trava a barra em 100%. Os pacotes
de ataque `0x39D/0x39E/0x36C` tambem possuem `CurrentMp` em `WORD`; eles recebem
somente `Score.CurMP`. Converter a mana real diretamente para `uint16` causa
wrap e pode zerar a mana local depois de uma unica skill multi-alvo.

Os wrappers regenerados por `Patch-WYD748-ExtendedStats.ps1` copiam o prefixo
proporcional para `TMHuman+44C..452`, enquanto HUD e painel numerico leem apenas
o sidecar `uint32`. Nunca use a projecao para calculo, gasto, regeneracao ou
persistencia server-side.

**0x3B9 UpdateAffect (140B, ID=mobID):** `STRUCT_AFFECT[16]`@12; cada entrada
possui Type, Value, Level e Time. É enviado somente ao próprio jogador e
alimenta `m_stAffect`: ícones, descrição e contadores. O campo Time usa unidades
de **8 segundos**, arredondadas para cima no wire.

**0x337 UpdateEtc (48B, ID=mobID):** Hold DWORD@12 (zerado quando o hold nativo não é usado), exp@16, LearnedSkill@20,
statusPts@24, masterPts@26, skillPts@28, Magic@30, gold@32. É o `p754_SendEtc`
confirmado pelo dump real do Micronics. `NextExp` não pertence a este pacote.

**0x185 UpdateCarry (528B, ID=mobID):** Item[64]@12, Coin@524. O array deve
continuar com 64 itens para preservar o wire, mas a UI 7.48 só expõe os índices
0..62; o índice 63 é reservado/invisível e nunca recebe loot/compra. Sem este
pacote o client não deixa mover/equipar item.

**0x36B SelfEquip (60B):** 16 WORDs `ItemEff` @12 (mesh/refino visual) e 16
bytes `AnctCode` @44 (cor/tintura). O client chama `SetPacketEquipItem` com o
primeiro bloco e `SetColorItem` com o segundo; omitir os últimos 16 bytes mantém
o equipamento, mas faz a tintura não aparecer.

**0x366 ActionStop (52B):** PosX@12, PosY@14, F1@16=2, F2@20=1, TargetXY@24=PosXY.
Coloca o mob parado em pé. **É o que faz o player "vivo"** pós-CreateMob.

**0x182 SendItem (24B):** DestType@12 (u16), DestPos@14 (u16), Item@16. Confirma UM slot
(o client só aplica swap/compra ao receber isto).

**Itens no chão:** `0x26E CreateItem` (32B, ID=0x7530) usa GridX/Y@12/@14,
ItemID@16, Item@18, Rotate/State/Height/Create@26..29 e Owner@30. `0x16F
RemoveItem` (16B) leva ItemID u32@12. Ao jogar, `0x175 CNFDropItem` (28B, ID=0)
confirma SourType@12, SourPos@16, Rotate@20 e GridX/Y@24/@26. Ao pegar, `0x171
CNFGetItem` (28B, ID=0x7530) confirma DestType@12 e DestPos@16; Item@20 fica
zerado no fluxo nativo e o conteúdo real do slot vem no `0x182`. Os layouts foram
conferidos em `Basedef.h`, `TMFieldScene::OnPacketCNFDropItem` e
`TMFieldScene::OnPacketCNFGetItem`.

**0x17C ShopList (532B, ID=0x7530):** **ShopType@12 = 1** (1=loja gold, 3=skill, 4=donate;
outro = ignorado!), List[64]@16 (STRUCT_ITEM), Tax@528. Abre a janela de loja. Em
`ShopType=3`, o grid usa 27 posições e exige vazios em 8, 17 e 26 para separar
os três ramos de oito skills.

**0x165 RemoveMob (16B, ID=mobID):** RemoveType@12 (u32). **1 = MORRE** (client faz Hp=0
+ Die(), anima e some); 0 = sai normalmente da área de interesse; **3 = exclusão
imediata**, necessária para descartar a instância `ECMOTION_DEAD` antes de
rematerializar um jogador revivido com o mesmo ID.

**0x3A8 WarInfo (24B, ID=0x7530):** payload zerado. Parte da sequência de login.

**Demais respostas implementadas:** `0x110` confirma criação de personagem,
`0x11A` recusa criação, `0x11C` informa conta já conectada, `0x101` exibe aviso
flutuante, `0x338` confirma morte/EXP, `0x378` confirma a barra de skills,
`0x383` replica a oferta de trade, `0x384` fecha a troca e `0x386` confirma o
primeiro check; `0x397/0x398/0x39A/0x39B` publicam, compram, consultam e atualizam
a Loja Fantasma; `0x3AE` confirma saída e `0x37D/0x37E/0x37F` sincronizam membro,
remoção e convite
de party. Os ataques S→C reutilizam 0x39D/0x39E/0x36C nos tamanhos 48/52/96.

Na `CharList`, cada seleção também leva Gold em `sel+712` e Exp em `sel+728`.

---

## 6. A sequência de login/enter-world

**Ordem EXATA** (sem ela os campos do client ficam não-inicializados = lixo negativo):

1. Client conecta → manda **InitCode** (0x1F11F311).
2. Client → **0x20D** (conta/senha/cliver). Servidor carrega a conta → manda **0x10E**
   CharList.
3. Client → **0x213** (entrar no mundo, slot 0). Servidor dispara a sequência:
   1. **0x114** EnterWorld (STRUCT_MOB completo).
   2. **0x364** self-CreateMob **spawn=2** (materializa o player; alimenta chaos/kills
      e a cor do nome via MobName[12]).
   3. **0x3A8** WarInfo.
   4. **0x336** UpdateScore, incluindo os 16 affects visuais públicos.
   5. **0x3B9** UpdateAffect do próprio jogador (ícones e timers).
   6. **0x185** UpdateCarry (inventário interativo).
   7. **0x337** UpdateEtc (exp/skills aprendidas/pontos/gold).
   8. **0x36B** SelfEquip (visual do equip).
   9. **0x378** SetShortSkill (20 atalhos persistidos).
   10. **0x181** SetHpMp.
   11. **0x366** ActionStop (deixa o player em pé/vivo).
   12. Sincronização regional: **0x364** apenas para entidades no raio de visão
       de ±32 tiles; `CreateMob.Affect[16]@66` já leva os efeitos ativos.

---

## 7. Os GOTCHAS críticos (lista de ouro)

Cada um custou horas. **Leia antes de mexer no wire.**

1. **CreateMob Status@100, NÃO @76.** O client 7.48 lê o Score do mob em @100 (layout
   Secrets p754, com Affect[16]@66). Prova: escrever Mastery `FF 07` @100 apareceu como
   "Level 2048" in-game. **Os mobs do Micronics 7.54 ficam "vivos por acidente"**: o
   nativo escreve o score em @76 e deixa lixo 0xCC em @100+ → curHP@112 = 0xCCCC ≠ 0 →
   barra cheia. Zeros limpos em @112 = HP 0 = todo mob renderiza MORTO. **Confie no
   SOURCE do client, não na captura do Micronics, pra semântica do CreateMob.**

2. **O 0x336 possui exatamente 16 affects compactos.** Cada WORD em @42..73 usa
   o byte baixo como tempo (unidades de 8 s) e o byte alto como tipo visual.
   Depois deles vêm guild @74, regen @76, resistências @78, HP/MP @82 e magic
   @86. Escrever esses campos nos offsets antigos cria efeitos fantasmas. O
   `0x336` é público; o `0x3B9` completo é privado e serve aos ícones/timers.

3. **Cor do nome = MobName[12] (byte de chaos/PK).** O byte transmitido é a projeção
   nativa `CP+75`: CP assinado `-75..+75` vira `0..150`, e `75` representa o estado
   neutro. O servidor mantém o CP assinado; a conversão só ocorre na borda do wire
   (enter-world, self-CreateMob e atualizações de visibilidade). Constante
   `NormalNameChaos = 75`; o CP não é colocado no DWORD `Hold` do `UpdateEtc`.

   **Não confundir com C.point/EXP Hold.** O Chaos Point nativo é o contador de
   PK, assinado e limitado a `-75..+75`; `/cp` consulta esse valor e mortes PK
   podem reduzi-lo. O sistema C.point documentado em servidores mais novos é um
   mecanismo separado de experiência retida: ele não deve ser gravado em `Char.CP`
   nem no `Hold@12` do `0x337`. Enquanto esse mecanismo separado não for ativado,
   `Hold@12` permanece zero para que CP nunca contamine a XP recebida.

4. **Merchant: nibble baixo = tipo, nibble alto = direção.** O client reconhece a UI do
   NPC por `m_stScore.Reserved & 0xF` (= o byte Merchant). Mapa (do client W2PP,
   customizado/sobreposto): **0=comum, 1=composição, 2=cargo/baú, 3=LOJA de gold**,
   4/8-15=quest, 6-8=aposta, 7=airmove, 15=agressivo. Confiáveis: **0, 2, 3**.

5. **Loja só abre com ShopType=1.** `OnPacketShopList`: 1=normal, 3=skill, 4=donate;
   qualquer outro = descartado em silêncio. O preço VISUAL vem do itemlist do
   client porque ShopList não o transmite; cobrança e saldo usam exclusivamente
   o itemlist carregado pelo servidor.

6. **Compra/venda é SERVER-AUTHORITATIVE (obrigatório).** O client manda **TargetID=0**
   no buy/sell 0x379/0x37A (compra arrastando pro grid do inventário, lê o merchant id
   errado). Rastreie `Player.ShopNPC` (setado ao abrir a loja) e use ISSO + o `sellSlot`.
   Nunca confie no TargetID do pacote.

7. **Morte de mob = 0x165 RemoveMob (RemoveType=1), NÃO 0x36A** (esse é Motion/Firework).
   0x181 SetHpMp só baixa a barra; o 0x165 dispara Hp=0 + Die() (anima + some).

8. **Melee 7.48 = 0x39D**. Repete ~1x/s ao atacar. **A cidade é zona
   segura**: o client bloqueia o ataque na origem (nem manda 0x39D). Mob de teste tem
   que estar em área PVE.

9. **Movimento 7.48 = 0x366 contínuo**. `0x36C` fica reservado às skills
   multi-alvo de 96 bytes. No 0x366, `PosX/Y@12` é a origem do segmento,
   `Speed@16`, `Effect@20`, `TargetX/Y@24` é o destino e `Route[24]@28`
   carrega os passos. O servidor valida terreno, altura, destino e budget,
   guarda o destino autoritativo e retransmite a rota validada aos observadores.
   Repetições do mesmo destino são suprimidas para não reiniciar a animação.
   `Effect=1` fica reservado a spawn, teleporte e correção dura. Uma chegada
   normal não gera pacote adicional; uma interrupção no meio da rota produz no
   máximo um ajuste `Effect=0`, e a posição parada precisa pertencer à rota
   previamente validada.

10. **Itens autoritativos:** o client só aplica um move ao receber o **0x182** (SendItem)
    do servidor. Sem resposta = "não é possível mover".

11. **Layout de char novo é dado, não código.** Aparência, atributos, localização,
    equipamento e inventário iniciais vêm de `data/character_templates.json`.
    Não injete meshes ou aplique máscaras de refino no codec: isso esconderia erro
    de configuração e faria o visual divergir do item autoritativo.

12. **Cliver (versão do client):** o TMSRV valida `min <= Cliver <= min+3`; `min` fica em
    memória e outra DLL o reescreve. No stack C++ o fix lia o `min` vivo e rescrevia o
    Cliver do pacote pra `min+1`. No Go from-scratch nós SOMOS o servidor, então
    aceitamos o Cliver que vier (o client 7.48 manda 748). (Ver [[project_cliver_fix_status]].)

---

## 8. Modelo de dados e formatos JSON

**`model.Item`** = `{Index u16, Eff [6]byte}`. MarshalJSON omite `eff` zerado →
`{"index":N}`.

**`model.WireScore`** = os 28 bytes do STRUCT_SCORE (ver §5), usado somente como
projeção descartável de ABI no pacote `wire`.

**`model.Char`** persiste apenas `Extended` (`extendedScore` v2). O
`ExtendedRuntime` efetivo é derivado em memória e não é serializado. Também
contém `LearnedSkill`, `ShortSkill[20]`, `Affects[16]`, progressão, localização,
`Equip[16]` e `Inv[64]`.
**JSON:** o equip é OBJETO com 16 slots
NOMEADOS (mesmos nomes e slots da aparência dos NPCs: `rosto, elmo, armadura, calca, luva, bota,
arma, escudo, acessorio1-6, montaria, capa`); o JSON preserva as 64 entradas
(save/load simétrico), mas somente 63 são jogáveis. Internamente são arrays
fixos; a forma nomeada é só (de)serialização. O loader exige o layout atual e
exatamente 64 entradas de inventário.

**`model.Account`** = `{Name, PasswordHash, Chars []Char, Cargo, CargoGold}`.
Somente `passwordHash` é válido; senha em texto não pertence ao schema.
Slots vazios em `Chars` são gravados como `null`, preservando os quatro índices.

**`model.NPCDef`** = `Name, Tipo ("monstro"|"npc"), Equip (16 STRUCT_ITEM nomeados),
ClassInfo, ExpReward, ExtendedScore v2, Carry, Gold, Direction, LearnedSkill,
SkillBar e Vende`. A posição e a rota pertencem ao `NPCGener.txt`, não ao
template. → `data/npcs/<nome>.json`.
`Equip.Slots()` calcula os 16 `ItemEff` visuais, incluindo refino/cor/montaria;
`Equip.AncientCodes()` gera os bytes de cor; `MakeExtendedScore()` clona o estado
wide com o HP atual; `CompatibilityScore()` é chamado somente no wire;
`IsMonster()` → `Tipo=="monstro"`; `Mesh()` → os 16 slots como slice pro ItemEff@34.

**Slots de equip/aparência (ordem 0-15):** 0=rosto/corpo, 1=elmo, 2=armadura, 3=calça,
4=luva, 5=bota, 6=arma (mão dir.), 7=escudo (mão esq.), 8-13=acessórios,
14=montaria e 15=capa. Esse é o único layout aceito pelo loader.

**Base Micronics:** `cmd/npcconvert` lê o `STRUCT_MOB` 7.54 de 756 bytes e gera
`data/npcs/*.json`. `Status@64`, `Equip@92`, `Inventory@220` e
`Resist@752` são os offsets autoritativos. Os três pares de efeito são preservados
em Equip, Carry e Vende; os buracos dos 64 slots continuam posicionais. Índices
fora das 6.500 entradas do ItemList são removidos. Gold acima de 100.000 e EXP
acima de 10.000.000 são tratados como memória residual; recompensas dentro desses
tetos são mantidas. O conjunto atual possui 476 templates e todos os nomes usados
pelo `NPCGener-micronics.txt`.

**`data/itemlist.csv`** (convertido do client 7.48, mas autoritativo no servidor): colunas por
vírgula, 0-based: `0=Index 1=Name 2=Mesh.Texture 3=Req 4=unique` **`5=Price`**
`6=nPos 7=Extra 8=Grade 9+=efeitos`. Linhas `#` = comentário. `data.LoadCatalog`
carrega a definição inteira e cruza nomes de `Itemname.csv`. `ItemEffect.h`
define a relação numérica dos efeitos `EF_*` gravados nos itens; o boot rejeita
nomes `EF_*` do ItemList que não existam nele. IDs proprietários escritos
numericamente no próprio ItemList são preservados sem o servidor inventar um
alias. `SkillData.csv` fornece o catálogo de skills.

O servidor armazena cada `STRUCT_ITEM` em exatamente uma entrada de inventario
ou cargo. No client 7.48, `Patch-WYD748-ExtendedStats.ps1` tambem normaliza as
oito dimensoes e mascaras de `EF_GRID` para 1x1. Assim elmos, armaduras, calcas,
armas e os demais itens ocupam uma unica celula tanto no inventario quanto no
bau; `EF_GRID` continua presente no catalogo apenas como metadado legado.

### Terreno e NPCGener

O servidor lê os binários nativos sem conversão. `HeightMap.dat` possui
4096×4096 = 16.777.216 bytes; altura 127 bloqueia a célula. `AttributeMap.dat`
possui 1024×1024 = 1.048.576 bytes, cada célula representa um bloco 4×4 e o bit
`0x02` é `CantGo`. `TerrainMap.Walkable` combina os dois. Em testes, mapa não
carregado aceita movimento; produção encerra no boot se os arquivos forem
inválidos.

As alturas são `char` assinado no TMSrv: o byte 250 representa `-6`, não 250.
Reposicionamento de spawn aceita diferença absoluta `<50`; uma rota aceita
`<8`, exatamente o `MH` usado pelo servidor e pelo client. Movimento de mob é
enviado em trechos de até `2 × Speed`, limitado a speed 4: um pacote cobre até
8 tiles, com `Speed@16`, `Effect=0@20` e rota vazia. O client reconstrói a rota
e preserva a animação contínua; não se envia `ActionStop` entre trechos.

`NPCGener.txt` é separado em seções `#`; `#*` desativa a entrada. O parser lê
template, líder/seguidor, `MinGroup`, `MaxGroup`, `MaxNumMob`, intervalo, Start,
três segmentos e Dest. `_` no nome vira espaço e seguidor vazio reutiliza o
líder. O gerador mantém a população no World; entrar ou relogar apenas sincroniza
a região e nunca recria os mobs.

`teleports.ini` usa o formato Micronics
`SourceX,SourceY,DestX,DestY,Price,Name`. O `0x290` não carrega destino: o
servidor encontra o portal no quadrado 4×4 da posição atual, cobra o gold,
escolhe uma célula livre/andável no destino, persiste e sincroniza as duas
regiões de visibilidade.

O conversor preserva separadamente `STRUCT_MOB.Merchant@17` (função nativa) e
o nibble alto de `Status.Merchant` (direção visual). Merchant 1 abre loja de
gold, 19 abre loja de skill e códigos especiais, como Kibita 43, permanecem
disponíveis para seus handlers. Mestres compactam as páginas de inventário
0/27/54 para 24 skills utilizáveis; as posições 8, 17 e 26 do pacote ficam
zeradas como os três separadores invisíveis exigidos pelo client 7.48.

### Catálogo, volatiles e templates

`LoadCatalog` cruza `itemlist.csv` e `Itemname.csv`, preservando requisitos,
posição, Unique, preço, efeitos estáticos e nomes, e carrega de `ItemEffect.h`
o dicionário usado pelos três efeitos de cada instância. `SkillData.csv` fornece custo,
alvo, mana, delay, range, instance, tick, affect e flags; `AffectTime` do arquivo
é convertido por `/4`, pois o runtime e o wire trabalham em blocos de oito
segundos. O arquivo autoritativo é o da W2PP/7.59; não existe mais uma tabela
hardcoded que sobrescreva os buffs TK depois do carregamento.

`volatiles.json` aceita, entre outras, `restore`, `buff`, `teleport`,
`magical_pill`, `hunting_teleport`, `summon_contract`, `quest_reward` e
`learn_special_skill`. Os efeitos `EF_VOLATILE` são descobertos no catálogo; regras por item
podem sobrescrever regras por código. `character_templates.json` é a fonte do
layout inicial das quatro classes.

---

## 9. Sistemas de jogo

### Equipamentos, score derivado e progressão

`recalcPlayer` sempre começa no `Char.Extended`, soma itemlist + os três efeitos do
item persistido, aplica refino e só depois passivas/affects. O mapa de efeitos
vem de `ItemEffect.h`, sem cópia hardcoded. Sanc 230..253 representa refino +10..+15. Até +10
usa `(sanc+10)/10`; +11..+15 usa 220%, 250%, 280%, 320% e 370%. Metadados de
posição, requisitos, volatile, incubação e previous bonus não são refinados.
Duas armas aproveitam 30% da menor, 50% quando têm o mesmo `Unique`; passivas
TK/HT acrescentam 15%/10%. Elmo, armadura, calça, luva e bota do mesmo conjunto
dão 105% de AC.

O pipeline calcula ataque, defesa, HP/MP, ataque mágico, precisão, evasão,
crítico, alcance, resistências, save mana, regen e AttackRun. `canEquip` valida
posição, classe, nível e STR/INT/DEX/CON contra o catálogo server-side. Remover
um equipamento recalcula e reduz imediatamente seus bônus.

Pontos são derivados, nunca confiados ao JSON. Stats começam em 5 por nível e
aplicam os thresholds 254/299/354 de `BASE_GetBonusScorePoint`; mastery começa em
2 por nível e respeita os tetos 200/255/320; skill points começam em 3 por nível
e ganham mais um por nível acima de 199. Stats gastos são a diferença para
`BaseSIDCHM`; o clique aplica 100 pontos quando há pelo menos 300 disponíveis.
Skills aprendidas descontam seu custo server-side. HP/MP usa base de classe,
CON/INT acima da base e os incrementos por nível de `BASE_GetHpMp`.

Atualizações puramente numéricas usam somente `UpdateScore`, `UpdateEtc` e,
quando necessário, `SetHpMp`. Mutações visuais de uma entidade já materializada
usam `0x36B UpdateEquip`, enviado ao dono e aos observadores sem coordenadas.
`CreateMob` fica reservado à criação real (NPC, monstro, pet, summon, objeto ou
clone), entrada/reentrada na visão, ressincronização explícita por `REQMobByID`
e renascimento após remover o cadáver. Reutilizá-lo durante uma caminhada
encaixa o modelo no destino e parece um teleporte. O `syncCriaPet` periódico
reconcilia a identidade existente e não recria nem republica a cria.

### Login assíncrono

PBKDF2 (600.000 iterações) não roda dentro do ator. `onLogin` ocupa uma das quatro
vagas de `authSlots`, autentica em goroutine e devolve `loginResult` pelo canal de
comandos. Se as vagas estiverem ocupadas, responde “Servidor de login ocupado”.
Toda mutação de sessão/conta continua ocorrendo somente no World.

### Combate (dano portado do W2PP `Basedef.cpp`)

Regra: dano FÍSICO cresce com FORÇA e DESTREZA; MÁGICO com INTELIGÊNCIA. (`internal/game/combat.go`.)

- **Ataque físico efetivo:** base 7.54 `Attack + Str/3 + Mastery[0] + Level`,
  acrescida da feature `Dex/4` do emulador. O `Score.Attack` enviado já contém
  esse valor derivado e não deve receber os atributos uma segunda vez no hit.
- **Ataque mágico:** `BASE_GetSkillDamage` foi portado da source 7.59. Foema e
  BM usam `INT/3 + INT/30 + levelPart + mastery + InstanceValue`. TK Confiança
  e Espada Mágica usam `INT/4 + INT/40 + arma + levelPart + mastery +
  InstanceValue`; TK Trans é físico e usa `3*STR + 3*arma`. `EF_MAGIC` e
  `EF_MAGICADD` formam o Magic Amp; as famílias mágicas recebem esse percentual
  e o fator final `5/4`. Não existe um atalho genérico que substitua a fórmula
  da skill pelo número exibido como MATK.
- **Resistências mágicas:** a ordem funcional é fogo, gelo, sagrado e trovão
  (`EF_RESIST1..4`). Para `InstanceType 1..5`, `_MSG_Attack` 7.59 aplica
  `(150 - resistência)%`; alvos mob usam metade da resistência configurada.
- **Regeneração:** `EF_REGENHP` e `EF_REGENMP`, inclusive os valores do rosto
  base de cada classe, alimentam o pulso `Max × Regen / 120 + 10` do `RegenMob`.
- **Dano do golpe** (BASE_GetDamage, Basedef.cpp:1553):
  `tdam = dam - ac/2`; `c = combat/2 (max 7)`; `rnd = rand%(12-c) + c + 99` (99..110%);
  `tdam = rnd*tdam/100`; suavização low-end (`<-50→0`; `-50..0→(t+50)/7`; `0..50→5t/4+7`);
  mín 1. `dam`=ataque atacante, `ac`=defesa do alvo, `combat`=skill (0 por ora).

**Fluxo player→mob (0x39D):** decodifica o alvo real em @44, valida ClientTick,
cadência, posição e alcance → `playerHitsMob` → `mob.HP -= dmg` → HP>0: broadcast 0x181;
HP=0: `0x338 CNFMobKill`, EXP/level-up e `0x165 RemoveType=1`. O NPCGener repõe
o grupo no seu `MinuteGenerate`, respeitando `MaxNumMob`.

Na morte, a EXP passa por `max(expReward, exp_minimum)`, `exp_rate/100` e então
pelo bônus de party. O valor realmente aplicado é limitado ao marco final
`g_pNextLevel[400]`. Level é base zero no protocolo: interno `0` = nível 1 no
client e interno `399` = nível 400; o servidor nunca cria level interno 400.

Skills usam a família de ataque do client 7.48 conforme `SkillData.MaxTarget`:
`0x39D` para um alvo, `0x39E` para dois e `0x36C` com até 13 alvos. Assim skills
em área, como Heaven's Dust, não são reduzidas artificialmente a um hit simples.
Quando vários alvos morrem no mesmo ataque, todos os resultados são aplicados
antes de um único commit da conta, evitando uma escrita de disco por mob.

**Fluxo mob→player:** `StandingByProcessor`/`BattleProcessor` portados para
`mob_combat.go`: aggro em 8 tiles, perseguição com colisão, leash de 30 tiles,
ataque adjacente via `0x39D` e dano autoritativo. No golpe letal, `0x181` atualiza
a barra e `0x338` chama `TMHuman::Die()`; HP zero bloqueia movimento/ataque. O
pedido `0x289` restaura até 200 HP e faz recall para Armia.

### Morte, recall e reentrada na visibilidade

O recall procura um tile livre ao redor da coordenada de Armia, evitando dois
jogadores no mesmo ponto. Ao reviver, cada observador que ainda conserva o ID
morto recebe `0x165 RemoveMob` com `RemoveType=3`; o servidor limpa os dois mapas
de visibilidade e recalcula as janelas antiga e nova.

Quando qualquer jogador cruza novamente a borda de ±32 coordenadas, o client
7.48 precisa receber a sequência `CreateMob → SetHpMp → ActionStop` antes do
próximo pacote de movimento. `CreateMob` isolado pode reaproveitar o estado de
morte e deixar o personagem congelado na primeira coordenada visível. Essa
sequência fica centralizada em `playerEnterViewPackets`, é aplicada nos dois
sentidos do par de jogadores e possui teste de regressão.

### Execucao das 96 skills basicas

`skills.go` conserva validacao, custo, cooldown e dano; `affects.go` concentra
buffs/debuffs/ticks; `skill_passives.go`, `skill_summons.go` e `skill_pvp.go`
isolam passivas, invocacoes e PvP. Esta fragmentacao evita colocar toda a regra
das quatro classes em um unico handler.

A W2PP e a referencia funcional. Foram portados `SetAffect`, `SetTick`,
`BASE_GetCurrentScore`, `GenerateSummon`, `pSummonBonus` e `pTransBonus`, sempre
adaptados aos modelos Go e ao wire 7.48. O teste de cobertura carrega o catalogo
real e exige um caminho server-side para cada indice `0..95`.

`baseSkillDamage` porta `BASE_GetSkillDamage`: escolhe ramo pela skill local,
separa explicitamente TK Trans dos ramos Confiança/Espada Mágica, combina
mastery, nível, INT/STR e dano das armas conforme a classe, aplica Magic Amp nas
famílias mágicas e o multiplicador da oitava skill do ramo. Skills
7/15/23 custam 2.000.000 gold, exigem as anteriores e são mutuamente exclusivas.
`skillManaCost` escala com mastery e SaveMana; `skillFinalDamage` possui sua
própria variância/suavização, distinta do hit físico. Resistência usa
`(150-resist)%`, com metade da resistência em mobs, e o dano PvE Mortal recebe
70%. Casos especiais de cura, revive, Explosion Bash, teleporte, invisibilidade
e múltiplos hits ficam explícitos por índice, não disfarçados na fórmula genérica.

Evocações BM ficam fora da party, seguem o dono e atacam seu alvo/agressor. A
quantidade cresce linearmente com mastery até: Condor 7, Javali 6, Lobo 6, Urso
5, Tigre 5, Gorila 4, Dragão Negro 3 e Succubus 2. Trocar o tipo invocado remove
as criaturas antigas; stats são derivados de INT+CON e mastery.

#### Pipeline de affects e efeitos visuais

`model.Affect` é a fonte server-side de duração, valor e nível. A aplicação e a
expiração acontecem no game loop; o client nunca decide se um buff está ativo.
Há dois canais complementares e eles não podem ser substituídos um pelo outro:

| Canal | Destinatário | Função no client |
|---|---|---|
| `CreateMob Affect[16]@66` | quem passa a enxergar a entidade | estado visual inicial |
| `0x336 Affect[16]@42` | todos que enxergam player/mob | `m_usAffect` e efeito público |
| `0x3B9 STRUCT_AFFECT[16]@12` | somente o dono do personagem | `m_stAffect`, ícone e timer |

O WORD compacto é `(tipoVisual << 8) | tempo`, com tempo em blocos de 8 s e
arredondamento para cima. Ao aplicar ou expirar um efeito de mob, `MobScore`
publica o 0x336. Em player, `publishPlayerAffects` publica 0x336 aos observadores
e envia 0x3B9 somente ao dono. O hook do macro agenda buffs equipados para
150 s, renovando-os na janela final de 30 s; o recast manual continua livre.
e se a skill continuar equipada na barra.
Um affect do mesmo tipo só substitui o anterior quando não reduz nem a potência
nem o tempo restante.

Os tipos implementados cobrem slow/speed, redução de resistências/DEX/defesa,
arma mágica, weaken, escudo mágico, assault, transformações e summons de BM,
regen/meditation, poison e ticks de área, proteção elemental, evasão,
invisibilidade, precisão, critical armor, soul drain e spirit exchange. As
fórmulas ficam centralizadas em `affects.go`; particularidades de classe ficam
nos arquivos `skill_*.go`.

| Tipo | Efeito server-side principal |
|---:|---|
| 1 / 2 | reduz / aumenta RunSpeed, limitado a 1..15 |
| 3 / 5 / 7 | reduz resistências / DEX percentual / AttackSpeed+INT |
| 9 / 10 / 11 / 12 | arma mágica / weaken / escudo / redução percentual de defesa |
| 13 / 14 / 15 | Assalto (ATK%, DEF−10%) / Possuído (CON+HP) / mastery total |
| 16 | transformação BM com interpolação por mastery e bônus das passivas |
| 17 / 20 | cura periódica / veneno, pulso de 8 s |
| 18 / 21 | SaveMana / Meditação (ATK por DEF) |
| 22 / 23 | ticks de área, até 6 alvos em raio 4 a cada 8 s |
| 24 / 25 / 26 | Samaritano / proteção elemental / evasão |
| 28 / 30 / 31 | invisibilidade / precisão +2000 / Critical Armor |
| 37 / 38 | Drenar Alma / converter metade do MP máximo em HP máximo |

Imunidade usa tipo 19 na validação de aplicação; o client 7.48 exige o remapeamento
visual 31→24 para Critical Armor. Ao atacar, invisibilidade é removida e publicada.

#### Layout da resposta de ataque/skill

`SkillHits` escolhe o pacote pelo `MaxTarget`: 0x39D/48B (1), 0x39E/52B (2) ou
0x36C/96B (até 13). Campos: atacante@12, quantidade@14, XY do atacante@16,
XY alvo@20, skill@24, MP@26, motion/mastery@28, EXP@32 e pares
`{TargetID,Damage}` a partir de @44. A cor física/mágica é decidida pela
codificação da skill.

**O WORD de dano NÃO é o dano autoritativo.** O client subtrai esse WORD do
`CurHP` do prefixo nativo, que está **escalado** por `ceil(MaxHP/30000)` — então
o dano precisa da **mesma escala**, senão a barra cai `scale` vezes rápido
demais e salta de volta quando chega o `0x181`. A conversão é única, em
`wire.wireDamage`; o dano real viaja na cauda `uint32 @48`, que o client
patcheado usa no número flutuante. Detalhe em `DOCS/EXTENDED_SCORE.md`.

### Loja

Click em mercador tipo-3 → client manda **0x27B** (seta o merchant id sozinho) → servidor
lembra `Player.ShopNPC` e manda **0x17C** (ShopType=1, itens de `NPCDef.Vende`). Comprar
(0x379): lê `Vende[sellSlot]`, cobra `w.items[index].Price`, `addToInv`, manda
0x182 + 0x337. Vender (0x37A) usa a mesma definição server-side e credita 25%.

### Cargo / baú

O Cargo pertence à conta, não ao personagem: `Account.Cargo[128]` e
`Account.CargoGold` são compartilhados pelos quatro slots. O char-list `0x10E`
envia os 128 itens em `@756` e o saldo em `@1780`; a UI 7.48 expõe três páginas
de 40 slots (`0..119`) e mantém `120..127` apenas como estrutura do protocolo.

O NPC `Cargo` usa Merchant low nibble 2, fazendo o próprio client abrir o baú.
Movimentos usam `0x376` com place 2 e são confirmados por `0x182`; o servidor
valida o NPC informado no `WarpID`, bloqueia os oito slots invisíveis e persiste
imediatamente. Depósito e saque usam `0x388/0x387`, com reafirmação do saldo
autoritativo por `0x339` e limite de 2 bilhões.

### Loja Fantasma

A Loja Fantasma porta o desenho encontrado na WebCheats/W2PP sem prender o
personagem: ao publicar `0x397`, o servidor cria um clone estacionário com
`ID = 25000 + OwnerID`. A loja recebe um tile livre próximo ao ponto de abertura
e permanece nessa coordenada enquanto o dono pode andar, combater, trocar e
viajar normalmente.

No movimento `0x366` do client 7.48, `PosX/Y@12` é a origem do segmento e
`TargetX/Y@24` é o destino atual. A posição server-side da abertura usa o
destino, como `pMob.TargetX/Y` na W2PP. Depois de aceitar `0x397`, o servidor
envia `0x384 CloseTrade`, reafirma aparência por `0x36B UpdateEquip` e HP/MP por
`0x181`, todos sem coordenadas, **antes** de publicar o clone `0x363`. Essa ordem
remove o modo comercial local do dono sem reiniciar seu movimento nem apagar o
título do clone.

A abertura só é aceita dentro dos limites nativos de Armia
`X=2052..2171, Y=2052..2163`. O clone faz parte da área de interesse ±32:
entra com `0x363 CreateMobTrade`, sai com `0x165` e não transmite dados para
clientes fora da região. Ao clicar no clone, o client envia `0x39A` com o ID
virtual; o servidor resolve o dono e devolve o `0x397` com os 12 anúncios.

Os itens não são copiados nem removidos antecipadamente. Cada anúncio guarda
`CarryPos + STRUCT_ITEM + preço`, onde `CarryPos` aponta obrigatoriamente para
`Account.Cargo[0..119]`. Apenas esses slots do Cargo ficam bloqueados; o
inventário, equipamentos e trade do personagem continuam totalmente livres.
Na compra `0x398`, distância ao clone, item, posição, preço, Tax, saldo, espaço
e limite de 2 bilhões são revalidados contra o estado server-side. O item sai
do Cargo do vendedor e entra no inventário do comprador; o pagamento vai para
`Account.CargoGold`, como na W2PP. As duas contas são persistidas por
`SaveAccounts` antes de qualquer confirmação ao client. `0x39B ItemSold`
remove o anúncio vendido da janela; estoque vazio fecha o clone.

Uma nova publicação `0x397` enquanto a loja já existe funciona como desligar,
seguindo a W2PP. `0x384` apenas fecha a janela local, portanto um comprador que
também possua loja não encerra acidentalmente o próprio clone. A loja é removida
quando o dono sai do personagem ou desconecta; persistência offline poderá ser
adicionada como um subsistema separado.

### Trade entre jogadores

O client usa **0x383 MSG_Trade (156B)** tanto para convidar quanto para atualizar
e confirmar: `Item[15]@12`, `CarryPos[15]@132`, `TradeMoney@148`,
`MyCheck@152` e `OpponentID@154`. `0x384` cancela/fecha e `0x386` confirma
visualmente o primeiro check.

O servidor não confia nos itens serializados pelo client. Cada `CarryPos` deve
ser único e apontar para `0..62`; o item completo nessa posição precisa coincidir
com o snapshot enviado e com o inventário autoritativo. Gold negativo, acima do
saldo ou capaz de ultrapassar 2 bilhões é rejeitado. `EF_NOTRADE` é consultado
no catálogo server-side.

Colocar um item na janela não o remove do inventário. Cada mudança de item ou
gold invalida os dois checks. No segundo check, ambos os inventários e saldos
são recalculados em cópias, revalidados e só então aplicados juntos pelo ator do
`World`. Em produção, `PostgresStore.SaveAccounts` confirma as duas contas e
seus índices de UID na mesma transação serializável. Os clients recebem `0x185
UpdateCarry` somente depois do commit. O adaptador JSON mantém seu journal
recuperável apenas para desenvolvimento.

Convites expiram em 30 segundos e trades abertos em cinco minutos. Movimento,
combate, morte, desconexão e qualquer mutação de item cancelam a sessão.

### Drop de itens e gold

- Cada monstro possui `carry[0..63]`; a posição do item seleciona a taxa em
  `data/droprate.json`, portada de `g_pDropRate[64]`.
- As bandas de level, o limite de rate 32000, os slots 8–10 com taxa 4 e o slot
  11 garantido seguem o loop de `TMSrv/MobKilled.cpp`.
- O loot vai diretamente ao primeiro slot livre do inventário. Se estiver
  cheio, o item completo, incluindo seus efeitos, é criado no chão.
- O inventário mantém 64 entradas no protocolo, porém somente `0..62` são
  utilizáveis. O slot estrutural 63 é invisível; dados antigos nesse índice são
  migrados no login para o primeiro slot visível livre.
- Gold usa a chance e a fórmula por level do bloco `Drop Gold`, limitado a 2000
  por ocorrência e ao teto de 2 bilhões do personagem.
- Itens no chão têm visibilidade por região, alcance de coleta de quatro tiles e
  expiram após dois minutos. Jogar e pegar são autoritativos e confirmados por
  `0x175/0x171` mais a atualização de slot `0x182`.

### Itens consumíveis

O client envia **0x373 UseItem (36B)**: SourType i32@12, SourPos i32@16,
DestType i32@20, DestPos i32@24, GridX/Y u16@28/30 e ItemID u16@32. O servidor
ignora qualquer descrição de efeito vinda do client e consulta o item real do
inventário e o `itemlist.csv` server-side. `data/volatiles.json` define a ação
de cada código; os itens são descobertos automaticamente pelo `EF_VOLATILE`,
sem duplicar manualmente a lista do CSV. `items` permite substituir a regra de
um item específico.

O `Premium_Firecracker` usa o caminho nativo separado **0x3C9 (52B)**. O client
envia os 100 pixels escolhidos na grade 10x10 em `Parm@34`; o servidor exige o
item configurado com `customPattern`, valida o slot e a posição autoritativa,
persiste o consumo e só então envia **0x3CA (36B)** aos observadores, mantendo o
mesmo bitmap em `@20`. Fogos comuns continuam no `0x373` com Motion 100.

- `EF_VOLATILE=1`: poções somam `EF_HP/HPADD/HPADD2` e
  `EF_MP/MPADD/MPADD2`, limitadas pelo HP/MP máximo, com cooldown de 100 ms.
- `EF_VOLATILE=2`: moedas coletadas do chão creditam
  `EF_HWORDCOIN*256 + EF_LWORDCOIN` diretamente no gold.
- `EF_VOLATILE=11`: Recall usa o destino X/Y configurado server-side.
- `EF_VOLATILE=185`: barras/moedas de gold usam o `Price` server-side; Silver
  Wyden (`4011`) vale 1.000.000.000, respeitando o teto de 2 bilhões.
- `EF_VOLATILE=191`: as caixas 4117–4121 validam evolução Mortal e intervalo
  `[minLevel,maxLevelExclusive)`, aplicam `exp_rate` + double EXP e gold do
  tier, persistem e somente então consomem. Não recebem piso nem bônus de party.
- Os 124 códigos atuais estão classificados explicitamente. Itens que pertencem
  a NPC/comando usam `no_direct_use`; o Spirit Seal/Extraction 206 usa
  `celestial_capsule`, enquanto 3455 permanece `no_direct_use`. Nenhum item
  conhecido depende hoje do fallback `generic`.
- `EF_AMOUNT=61` é decrementado para itens empilhados; a última unidade zera o
  slot. Toda alteração é confirmada por `0x182`.
- Barras são salvas antes da confirmação ao client, impedindo duplicação por
  desconexão entre o consumo e o autosave.

As cinco áreas Mortais ficam em `data/quest_zones.json`. O reset é global por
deadline de 10 minutos, como o `SecCounter%1200` nativo. `recallPlayer` remove
primeiro um eventual cadáver da região antiga, restaura o personagem, resolve
uma posição livre em Armia e sincroniza visibilidade, HP/MP e party.

### Reinos Akelonia e Hekalotia

O reino é derivado exclusivamente da capa em `Equip[15]`, como em
`BASE_GetCapeInfo`; não existe campo de reino duplicado no personagem. Hekalotia
usa o valor nativo 7 e Akelonia usa 8. `King_Harabard` concede Hekalotia por 4
Safiras e `King_Glantuar` concede Akelonia por 15, a partir do nível interno 219
(nível 220 exibido). O `Kingdom_Brocker` converte a capa para o tier neutro por
16 Safiras e não funciona aos domingos. Safira 697 respeita `EF_AMOUNT`; o
pacote 4131 vale dez unidades.

O `Kingdom_Emblem` 4081 equipado no slot 13 substitui o pagamento em Safiras
na adesão/promoção Mortal e é consumido na mesma transação, como no case KING.

As cinco famílias nativas são explícitas e preservam o tier no broker:
Basic 545/546 ↔ 548, Crusader/Shiner 543/544 ↔ 549, Elite
3191/3192 ↔ 3193, Hero 3194/3195 ↔ 3196 e Master 3197/3198 ↔ 3199. Um
Mortal recebe primeiro a Basic; a partir do nível interno 255 (256 exibido),
voltar ao mesmo rei promove 545/546 para 543/544. Capa Elite/Hero neutra é
convertida sem perder efeitos e Celestial recebe o tier Master, independente do
level reiniciado. A Lindy também usa o reino da capa anterior para entregar
3191, 3192 ou 3193.

Toda mudança segue validar, alterar, recalcular, persistir e só então
sincronizar inventário, capa, score e aparência. `/kingdom` e `/reino`
levam à cidade do reino; `/king` e `/rei` levam ao rei, com cooldown
compartilhado de cinco segundos. O client 7.48 não possui chat de reino `@@`;
esse canal aparece somente no TMProject mais recente da W2PP e não faz parte do
protocolo adotado pelo emulador.

### Persistência

`store.SaveAccount` chamado em `onDisconnect` garante que char, itens e gold
sobrevivam ao relog. PostgreSQL é autoritativo em produção: a ficha fica em
JSONB estrito e nomes/itens também ficam em tabelas com constraints. Cada item
materializado possui UID global server-side. O autosave de 3 segundos prepara
snapshot imutável no ator e executa a escrita na fila de persistência. Operações
econômicas continuam síncronas e confirmam somente após commit. O adaptador
`JSONStore` grava `data/accounts/<nome>.json` apenas quando selecionado
explicitamente com `database_driver=json`.

### Party, rede e limites operacionais

Party usa convite 0x37F, aceite 0x3AB, painel 0x37D e remoção 0x37E. O líder
ocupa sempre o índice zero; o snapshot é reenviado em ordem canônica porque o
handler do client remove e reinsere membros atualizados. O limite é 13, convite
expira em 30 s e membros devem estar no mesmo setor 128×128. Cada membro elegível
ganha a EXP integral multiplicada por `100% + 2% × membros` (102% sozinho no
grupo até 126%). HP acima de 32000 é compactado para o painel.

Cada `Session` possui fila de saída de 256 pacotes. Tick começa em 60000 para
evitar valores próximos de zero esperados pelo client. Checksum legado é
ignorado deliberadamente; framing, tamanho e criptografia continuam validados.
`Store` expõe Load/Save/CharacterNameExists. `PostgresStore` acrescenta criação,
busca case-insensitive, charstate, guild e transações multi-conta. Trades usam
uma única transação PostgreSQL para as duas contas e seus UIDs.

### Constantes tunáveis principais

| Constante | Valor | Responsabilidade |
|---|---:|---|
| world tick | 500 ms | executor de ataque, summons e manutenção; percepção ativa 1 s e movimento 2 s |
| autosave | 3 s | janela máxima normal de rollback |
| visão | ±32 tiles | área de interesse |
| inventário wire/jogável | 64 / 63 | slot 63 reservado |
| party | 13 | líder + 12 membros |
| convite de party | 30 s | validade |
| convite / sessão de trade | 30 s / 5 min | validade e limpeza |
| setor de party | 128×128 | elegibilidade |
| piso / rate de EXP | 10.000 / 100% | configuráveis em `server.txt` |
| bônus de party | 2% por membro | configurável; padrão 102%..126% |
| affect wire/tick | 8 s | contador e efeitos periódicos |
| renovação automática | abaixo de 30 s | buff equipado na barra |
| alvos multi wire | 13 | capacidade do 0x36C |
| auth PBKDF2 | 4 vagas / 600.000 | proteção do ator e da CPU |
| fila de sessão | 256 | backpressure de saída |
| expiração de item no chão | 2 min | limpeza do mundo |
| coleta | 4 tiles | validação server-side |
| gold máximo | 2.000.000.000 | clamp econômico |
| stats/mastery/skill | progressão Mortal | thresholds e skills aprendidas |

As constantes completas ficam próximas ao sistema que as utiliza; não crie um
arquivo global de números sem semântica.

### Comandos do jogador

- `/limparinv`: o client converte comandos com `/` em `0x334 MessageWhisper`
  (`MobName="limparinv"`). O servidor apaga todos os itens do inventário do próprio personagem,
  inclusive o slot estrutural invisível 63. A conta é
  salva antes do envio do `0x185 UpdateCarry`; se o save falhar, a alteração é
  revertida e o jogador recebe uma mensagem de erro.
  Confirmação e erro usam `0x101 MessagePanel` (`ID=0`), exibido como aviso
  flutuante no topo da tela, sem inserir texto no chat.
- `/fame`: consulta a fama autoritativa do personagem no contador `fame` do
  charstate e exibe `Fame: N` no mesmo `0x101 MessagePanel`. É somente leitura;
  não altera CP, EXP, pontos de skill nem o inventário.

Trocas envolvendo equipamento recalculam imediatamente o score completo a
partir de `Extended + Equip + Affects` e enviam `SelfEquip`, `UpdateScore`,
`UpdateEtc` e `SetHpMp`. Remover uma peça reduz defesa, ataque, atributos,
HP/MP, resistências e demais bônus sem precisar relogar.

---

## 10. Estrutura de código

```
cmd/server/main.go     composição: todos os loaders + Store + World + Listener
cmd/account-api/       API HTTP de cadastro
cmd/account-create/    criador local interativo
internal/
  account/             validação compartilhada de conta/senha e PBKDF2
  accountapi/          HTTP, limites, cabeçalhos e rate limit
  model/model.go       tipos puros: Item, ExtendedScore, WireScore, Char, Account, Equip, NPCDef
  wire/
    crypt.go           pKeyWord[512] + Encrypt/Decrypt
    packet.go          Header (12B), ReadPacket (framing+decrypt), FinishPacket
    opcodes.go         Op* + InitCode + Scene*
    codec.go           BUILDERS por-pacote (model → []byte): CharList, EnterWorld, CreateMob,
                       SetHpMp, UpdateScore, UpdateEtc, UpdateCarry, SelfEquip, ActionStop,
                       SendItem, WarInfo, RemoveMob, ShopList; helpers PutScore/PutItem/Build
  net/
    session.go         Session{ID,conn,out,done}; Send()=tick+FinishPacket+out<-; Serve(handler)
    listener.go        Listen(addr, onConn)
  store/               interface Store + PostgreSQL autoritativo + adaptador JSON
  data/
    data.go            LoadNPCs (pasta data/npcs/*.json; carry limitado a 64 slots)
    catalog.go         LoadCatalog (itemlist + Itemname + ItemEffect + SkillData)
    droprate.go        LoadDropRates (64 taxas por posição do carry do monstro)
    character_templates.go layouts iniciais das quatro classes
    npcgener.go        parser do NPCGener.txt padrão
    terrain.go         altura/tiles caminháveis usados por spawn e colisão
    volatiles.go       regras server-side de itens clicáveis
  game/
    world.go           World (ator): commands, players, store, npcs, mobs, items, skills; Run/tick/handle/broadcast
    handlers.go        login, criação/entrada, inventário/loja e roteamento de ações
    visibility.go      janela 65x65, materialização, remoção e movimento observado
    party.go           convite, painel estável, liderança e EXP compartilhada
    commands.go        comandos do jogador e avisos fora do chat
    consumables.go     execução autoritativa de volatiles
    drops.go           roll de item por slot, gold por level e fallback para o chão
    combat.go          fórmulas de dano (portadas do W2PP)
    mob_combat.go      FSM, aggro, perseguição e ataque de mobs/summons
    movement.go        movimento, colisão e publicação interpolada
    equipment.go       requisitos e efeitos autoritativos de equipamentos
    progression.go     tabela de EXP, ganho, level-up e party bonus
    regen.go           pulsos de HP/MP e sincronização pública
    skills.go          aprendizado, MP/cooldown e execução de skills
    affects.go         buffs, debuffs, duração, renovação e ticks
    skill_passives.go  passivas das quatro classes
    skill_pvp.go       alvos jogadores, cura, controle e morte PvP
    skill_summons.go   invocações e transformações BM
data/
  npcs/<nome>.json     1 NPC por arquivo
  accounts/<nome>.json estado local/runtime de 1 conta (não versionar contas reais)
  character_templates.json layout de nascimento por classe
  volatiles.json       ações por código volatile e overrides por item
  itemlist.csv         definições, requisitos, preços e efeitos server-side
  Itemname.csv         nomes autoritativos dos itens
  ItemEffect.h         IDs autoritativos dos efeitos EF_* persistidos
  SkillData.csv        custos, alvo, mana, delay, alcance e efeitos das skills
  repliction.json      pares nativos de adicionais por parte da armadura
```

---

## 11. Como adicionar um handler de pacote

O dispatch atual em `game/world.go` possui estas rotas explícitas:

| Opcode(s) | Handler |
|---|---|
| 0x20D / 0x20F / 0x213 | login / criar personagem / entrar no mundo |
| 0x376 / 0x373 | mover item / usar item volatile |
| 0x28B, 0x27B / 0x379 / 0x37A | usar NPC/abrir loja / comprar / vender |
| 0x277 | distribuir bônus de stats ou mastery |
| 0x37F / 0x3AB / 0x37E | convidar / aceitar / remover do grupo |
| 0x272 / 0x270 | jogar / coletar item do chão |
| 0x333 / 0x334 | chat/comando / whisper |
| 0x378 | persistir barra de 20 skills |
| 0x291 / 0x2CB / 0x366 / 0x367 | cidade / stop / movimento / action-stop |
| 0x289 / 0x3A0 / 0x3AE | renascer / keepalive / sair |
| 0x39D / 0x36C / 0x39E | ataque de 1, vários ou 2 alvos |

Pacote desconhecido é apenas registrado; não altera o estado até que seu layout
seja comprovado no client ou nas fontes de referência.

1. **Descubra o opcode e o layout.** Rode, aja no client, veja `sem handler: Type=0x...`
   no log. Confirme o layout no SOURCE do client
   (`SERVER W2PP/SOURCE GAME/Projects/TMProject/Basedef.h` struct `MSG_*` +
   `TMFieldScene.cpp`/`TMHuman.cpp` `OnPacket*`) ou numa captura do Micronics.
2. **Opcode** em `wire/opcodes.go`.
3. **Builder** (se S→C) em `wire/codec.go`, recebendo tipos de `model`.
4. **Handler** `on<Nome>(s, pkt)` em `game/handlers.go`.
5. **Registre** no `switch` de `world.go::handle`.
6. Build + vet + felipe testa. **Prefira log + probe a adivinhar.**

---

## 12. Onde vive a verdade (fontes de referência)

- **`SERVER W2PP/SOURCE GAME/Projects/TMProject/`** — **SOURCE do CLIENT** (759, mesma
  lógica do 7.48). A **autoridade do lado-client**: o que o client manda, quais campos/
  pré-condições exige, como reage. `Basedef.h` (structs `MSG_*` + opcodes), `TMFieldScene.cpp`
  / `TMHuman.cpp` (`OnPacket*`, click handler ~14200-14730, cor do nome ~6020). Resolveu
  ShopType, merchant nibble, affect, cor do nome, death packet.
- **`SERVER W2PP/SOURCE SERVER/Code/`** — server 759 full-source. Fórmulas/lógica:
  `Basedef.cpp` (BASE_GetDamage:1553, BASE_GetCurrentScore, dano físico:4997, mágico:7695),
  `TMSrv/SendFunc.cpp` (SendShopList, SendRemoveMob), `TMSrv/GetFunc.cpp` (GetCreateMob →
  MobName[12]=chaos:1826), parser do itemlist (`BASE_ReadItemListFile`).
- **`SOURCE(secrets) para referência/`** — plugin 7.54 + `Descompilação/`. `BuyItem.cpp:23`
  (Npc->Inventory[sellSlot]), `UseNpc.cpp` (tipos de merchant).
- **`Server Star Micronics/TMSRV/run/send_dump.txt`** — captura dos bytes 7.48 REAIS que o
  servidor Micronics manda (ground truth de layout). `itemlist.csv`, `NPCGener.txt`.
- **Memórias** (auto-memory): `project_wyd_go_rewrite`, `project_wyd_go_shop`,
  `project_wyd_go_combat`, `project_w2pp_748_protocol`.
- Regra de ouro ao usar as fontes: **porte ALGORITMOS/FÓRMULAS, nunca offsets/structs**
  (versões diferentes). O client 759-source dá a LÓGICA; os offsets 7.48 vêm da captura
  ou de probe in-game.

---

## 13. Estado atual e roadmap

**Funcionando:** cadastro/auth e bloqueio de login duplo; criação/seleção de até
quatro personagens; stats, equipamentos, pontos, EXP e level-up Mortal;
NPCGener persistente com patrulha/colisão/terreno/respawn; área de interesse de
players, mobs e itens; combate player↔mob, mob↔player e skills PvP; morte, recall
e reentrada; party estável com EXP; lojas normal e de skill; inventário, drop,
loot, volatiles e trade transacional anti-dupe; execução, buffs, debuffs,
passivas e summons das 96 skills de classe, mais as cinco skills Sephira;
efeitos públicos e ícones/timers privados; persistência PostgreSQL transacional.

O layout inicial das quatro classes fica em `data/character_templates.json`.
Esse arquivo define coordenada de nascimento, atributos base, HP/MP, gold,
aparência, equipamentos e inventário inicial sem exigir recompilação. A criação
usa `0x20F`, confirma em `0x110` e recusa em `0x11A`; nomes aceitam somente
letras ASCII (`A-Z`/`a-z`), possuem de 4 a 13 caracteres e são únicos entre
todas as contas sem diferenciar maiúsculas de minúsculas.

**Próximo:** validar visualmente as cinco skills Sephira, as sete receitas de
craft, Uxmal, Big Cube, Celestial/SubCelestial, gemas e Adamantita. O motor de
instâncias e o fluxo server-side de identidade da guild já existem; expansão do
catálogo de quests, host da textura de guildmark, Guild/Kingdom/Castle War, Lua
e multi-canal pertencem aos marcos seguintes. O grid espacial de mobs/jogadores
e a ativação de IA por proximidade já estão implementados.

O primeiro vertical slice de crafting é o Tiny (`0x3C0`). O servidor compara os
oito `STRUCT_ITEM` do packet com os slots autoritativos, revalida catálogo,
grade, posição e refino, cobra 100 milhões e persiste antes de responder com
`0x3A7`. Falha de save restaura inventário e gold.

---

## 14. Cadastro e autenticação de contas

O game valida `AccountName[16]` e `Password[12]` do pacote `0x20D` na camada
`internal/account`. Senhas usam hash versionado PBKDF2-HMAC-SHA256 com 600.000
iterações, salt aleatório de 16 bytes e chave de 32 bytes. O JSON aceita somente
`passwordHash`; arquivos com senha em texto são rejeitados.

A API é um processo separado e cria apenas contas vazias:

```powershell
go build -o account-api.exe ./cmd/account-api
.\account-api.exe
```

Para administração local, não é necessário iniciar a API. O criador interativo
usa as mesmas validações e o mesmo hash da API, mas grava a conta diretamente
em `data/accounts`:

```powershell
go build -o account-create.exe ./cmd/account-create
.\account-create.exe
```

Execute o programa a partir da raiz de `wyd-go`. A senha não aparece no console
e nunca é aceita como argumento de linha de comando.

Por padrão ela escuta somente em `127.0.0.1:8080`. Para publicar na internet,
coloque Caddy, nginx ou outro proxy HTTPS na frente; não exponha HTTP puro.

Exemplo de cadastro:

```powershell
$body = @{
  username = "NovaConta"
  password = "Senha123!"
  passwordConfirmation = "Senha123!"
} | ConvertTo-Json

Invoke-RestMethod -Method Post `
  -Uri http://127.0.0.1:8080/v1/accounts `
  -ContentType application/json `
  -Body $body
```

Regras atuais: conta com 4–12 letras/números, senha com 4–12 caracteres ASCII
visíveis sem espaço, comparação de conta sem diferenciar maiúsculas e limite
de 10 cadastros por IP/minuto. O endpoint de saúde é `GET /health`.
