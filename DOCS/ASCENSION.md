# Ascensão Mortal → Arch — WYD-Go 7.48

Atualizado em 29/07/2026.

Portado de `SERVER W2PP/SOURCE SERVER/Code/TMSrv/_MSG_Quest.cpp` (cases
`COMP_SEPHI`, `BLACKORACLE` e `KING`), `DBSrv/CFileDB.cpp`
(`_MSG_DBCreateArchCharacter`) e `Basedef.cpp` (orçamento de pontos).

Todos os itens da cadeia **existem no catálogo 7.48** — conferidos um a um em
`data/itemlist.csv`.

## As três etapas

Cada uma num NPC diferente, todos já spawnados no mundo.

### 1. Sefirot — no MESTRE DE SKILL da classe

| Requisito | Valor |
|---|---|
| 8 pedras Sephira | 1744 Kocuna, 1745 Veean, 1746 Keced, 1747 Gebra, 1748 Tipalet, 1749 Nechark, 1750 Yesord, 1751 Marcute (uma de cada) |
| Gold | 30.000.000 |
| Entrega | Sefirot da classe do NPC: 1760 TK, 1761 FM, 1762 BM, 1763 HT |

O compositor **é o mestre de skill da classe**, não um NPC dedicado —
confirmado in-game (o client abre a janela de confirmação no `Foema_Ancian`) e
na fonte goldvivi ("Skills Teacher (Armia town) - creates Sephiroth"). Os NPCs
`Sephirot_*` do catálogo **não** são os compositores.

| NPC | Skills que vende | Classe | Entrega |
|---|---|---|---|
| `Knight_Leader` | 0–23 | TK | 1760 |
| `Foema_Ancian` | 24–47 | FM | 1761 |
| `Mestre_Archi` | 48–71 | BM | 1762 |
| `ForeLearner` | 72–95 | HT | 1763 |

A classe vem da **faixa de skills** que cada mestre vende (índice 5000+skill),
critério objetivo; o nosso JSON não preserva o discriminador nativo (`Exp`
994–997).

**Ordem no clique:** o mestre de skill também é loja, então a composição é
testada **antes** da abertura da loja — senão o clique sempre abriria a janela
de skills e a composição nunca aconteceria. Dois gates evitam sequestrar o
clique de quem só quer comprar skill: `ClickOk=1` (o client só envia depois do
"Sim" na janela de confirmação) e posse das oito pedras.

### 2. Pedra Eterna — NPC `Black_Oracle`

| Requisito | Valor |
|---|---|
| Espíritos | 1740 Unicon's_Spirit e 1741 Phoenix's_Spirit em slots **adjacentes**, nessa ordem |
| Safiras | 10 (697 avulsas ou 4131, o pacote de dez) |
| Entrega | 1742 Eternal_Stone — a "Pedra da Imortalidade" |

A **adjacência é exigência do nativo**, não detalhe de implementação: é assim
que o jogador monta o par no inventário. Ordem invertida não serve.

Reusa `sapphireCount`/`consumeSapphires` de `kingdom.go`, que já tratam o pacote
como dez unidades.

### 3. Arch — no rei

| Requisito | Valor |
|---|---|
| Evolução | Mortal (`Evolution` vazio) |
| Nível interno | ≥ 370 |
| Slot 10 | 1742 Eternal_Stone |
| Slot 11 | Sefirot 1760–1763 — **define as habilidades/classe lógica do Arch** |
| Conta | precisa de um slot livre |

Consome os dois itens. O Arch nasce como personagem **novo** no primeiro slot
livre; **o Mortal permanece** existindo e jogável, exatamente como no nativo.

No clique do rei a ascensão vem **antes** da lógica de reino: com Pedra e
Sefirot equipados o clique é pedido de Arch, não de capa. Sem eles, segue para
o fluxo de capa normalmente.

**Anúncio global.** Ao concluir, todos os jogadores recebem
`The character NICK has become a God!` pelo canal `[SERVER]` (cor 7, o mesmo do
`/spk`). "God" é como o client global e as fontes chamam o Arch. O aviso sai
**depois** da persistência: uma ascensão que falhou não vira notícia.

## Nível mínimo: 370

As fontes divergem e cada servidor calibra o próprio portão:

| Fonte | Nível |
|---|---|
| W2PP (`_MSG_Quest.cpp`) | 380 |
| goldvivi / Supreme Destiny | "370-400 it can be reborn" |
| tutorialwyd | 355 |
| dragondicas, wydbrloucos | 300 |

**Regra do nosso servidor: 370** (decisão do felipe). Está registrado em
comentário no código para ninguém "corrigir" de volta para 380 achando que é bug.

## Rosto do Arch

`Equip[0] = rostoDoMortal + 5 + classeDoSefirot` (`CFileDB.cpp:1993`).

O Sephirot define `Char.Class` e, portanto, a árvore de habilidades. Ele não
troca o corpo herdado do Mortal. A dezena do rosto resultante continua
identificando a classe corporal (`rosto / 10`), exatamente como
`BASE_CanEquip` usa `extra->MortalFace / 10` no W2PP.

Verificado contra o catálogo real — cada classe ocupa um bloco de dez índices e
todos os quatro resultados existem e são da classe certa:

| Classe | Rosto Mortal | Rosto Arch |
|---|---|---|
| TK | 1 | 6 (TransKnight) |
| FM | 11 | 17 (Foema) |
| BM | 21 | 28 (Beast_Master) |
| HT | 31 | 39 (Hunter) |

Exemplo cruzado: Mortal TK (rosto 1) + Sephirot HT (classe 3) gera rosto Arch 9.
O personagem possui habilidades HT, mas `9 / 10 = 0`, portanto continua
vestindo equipamentos TK.

## Equipamentos do Arch

- Não exige level, STR, INT, DEX ou CON para equipar.
- Armas nos slots 6/7 podem ser de qualquer classe.
- Armaduras continuam restritas à classe do corpo Mortal, não à classe do
  Sephirot.
- A posição permitida pelo item continua obrigatória; a isenção não permite
  colocar um item em slot incompatível.

Essa separação reproduz `BASE_CanEquip` do W2PP: evoluções superiores zeram os
cinco requisitos, ignoram a classe somente para armas e validam os demais
equipamentos pelo bloco do rosto Mortal.

## Pontos de status do Arch

Ramo `ARCH` de `BASE_GetCurrentScore` (`Basedef.cpp:1043-1056`):

```text
leveluse  = nível * 6
leveluse += (nívelDoMortal - 299) * 5
leveluse += 28
se nível >= 354: leveluse += (nível - 354) * 6
```

O termo do Mortal é o que faz um Arch nascido de um Mortal 400 ser mais forte
que um de 370 — **150 pontos a mais**, na prática.

**O bônus é dinâmico.** `ProcessDBMessage.cpp:1002` recalcula o valor a cada
login a partir do nível *atual* do Mortal de origem: continuar upando o
Mortal **depois** da ascensão segue fortalecendo o Arch. Um dos tutoriais afirma
o contrário ("o mortal para de receber pontos") — a fonte diz o oposto.

Implementação: `Char.ArchMortalUID` guarda a origem e `Char.ArchMortalLevel` o
nível em cache (o cálculo de pontos recebe apenas o `Char`, sem acesso à conta).
`refreshArchMortalLevel` atualiza o cache no login. Mortal apagado mantém o
último valor conhecido, em vez de zerar pontos já distribuídos.

O Mortal continua na fórmula antiga — `statusPointBudget` escolhe pela evolução,
com teste garantindo que nenhum personagem existente muda.

## Decisão: o Arch herda o nome do Mortal

**Fiel ao nativo** (decisão do felipe), com uma consequência conhecida.

Nosso servidor exige nome globalmente único e vários sistemas resolvem jogador
por nome — sussurro, convite de grupo, guild. A criação do Arch é **isenta** da
checagem de unicidade: o nome já pertence à conta.

O nome é somente visual/protocolo. Mortal e Arch recebem `Char.UID` distintos;
charstate, fama e contadores usam esse UID. O Arch guarda `ArchMortalUID`, não o
slot, então excluir ou reutilizar um slot não troca sua origem.

**Pendência real:** as buscas por nome não têm critério de desempate. Só um dos
dois pode estar online por vez, o que cobre a maior parte dos casos, mas a
ambiguidade existe. Não há mais carta offline no servidor (toda mensagem exige
o jogador online), o que reduz a superfície.

## Progressão do Arch

O Arch nasce do template Mortal da classe: HP/MP e tabela de EXP são as mesmas.
Isso é fiel — no nativo `ClassMaster == ARCH || MORTAL` compartilham a fórmula
(`Basedef.cpp:1488`, `1501`, `4997`).

Skills e itens exclusivos de Arch são fase seguinte.

## Travas de nível 355 e 370

Duas flags persistidas no personagem — `Char.ArchLevel355` e `Char.ArchLevel370`
— barram a progressão do Arch em dois pontos fixos. Enquanto a trava estiver de
pé, **nenhuma EXP é creditada**: não é um teto que ignora o excedente, o ganho é
barrado por inteiro (`archExperienceLocked`, chamado por
`canReceiveMortalExperience`). Sem EXP não há level-up, então as duas metades do
nativo (`GetFunc.cpp:1565` no ganho e `CMob.cpp:2079` no level-up) ficam cobertas
por uma checagem só.

**Os níveis são internos:** 354 e 369, que o client exibe como 355 e 370 — a
mesma convenção base-zero das quests.

**O destravar é no craft da Lindy**, não numa quest (`_MSG_CombineItemLindy.cpp:54-115`):

| | |
|---|---|
| quem | Arch parado **exatamente** em 354 ou 369, com a trava ainda de pé |
| receita V754 | `3448 ×10`, `3448 ×10`, **`4127`**, mais quatro `413` avulsos |
| custo | **10 de fama** em cada destrave (contador do personagem) |
| entrega | a capa Elite do reino (3191 Hekalotia / 3192 Akelonia / 3193 neutra), efeito 54 valor 16 |

Fora dessas duas paradas a receita é rejeitada sem consumir ingredientes; o
handler não transforma o craft em uma troca genérica de capa.

O item **4127 é o mesmo** que o `Survivor` troca por 100 entradas de Kefra.

**Ordem transacional.** A fama vive no charstate identificado pelo UID do Arch.
No PostgreSQL, fama, receita e destrave são confirmados juntos por
`SavePlayerState`; uma falha desfaz tudo. `arch_locks_test.go`, `fame_test.go` e
os testes de integração PostgreSQL cobrem os dois lados.

## As quatro juras elementais

Volatile 187, portado de `_MSG_UseItem.cpp:7184`. Os cristais 4106–4109 (Elime,
Sylphid, Salion, Nohas) caem em **Kefra** — a mesma área cuja entrada o
`Survivor` cobra em `kefra_ticket`.

| | |
|---|---|
| quem | Arch, nível interno ≥ 355 |
| ordem | obrigatória: o cristal N só vale com os N-1 anteriores feitos |
| custo | **100 milhões de EXP** por jura |
| progresso | `Char.ArchCrystals` (0..4) |

O nível é **recalculado a partir da EXP reduzida**, então o personagem *desce*
de nível. Isso é do nativo, não efeito colateral — `grantExp` só sobe, por isso
existe `levelForExp`.

O piso de nível é conferido **antes** de descontar, e não depois: como cada jura
rebaixa o nível, verificar no fim deixaria a segunda em diante impossível — o
próprio desconto derrubaria o personagem abaixo do piso. Há também guarda de
underflow explícita, porque `Exp` é `uint32` e um estado inconsistente faria
`Exp -= custo` dar a volta para ~4 bilhões, presenteando em vez de cobrar.

Persist-before-confirm: 100 milhões de EXP é caro demais para sair sem estar em
disco. Uma falha devolve EXP, nível, cristal e item.

Cada cristal acrescenta **100 pontos** ao orçamento futuro do Celestial, mas
**não é requisito para criá-lo**.

**Divergência necessária:** o `if` isolado do nativo testa
`ClassMaster == CELESTIAL`, embora o mesmo fluxo exija nível interno 355, que um
Celestial 7.54 nunca alcança (cap interno 199). O servidor aceita o uso pelo
**Arch**, coerente com `QuestInfo.Arch.Cristal`, com a faixa de nível exigida e
com o consumo desse contador pela fórmula posterior de pontos.

## Celestial e SubCelestial

A implementação segue prioritariamente o **Secrets 7.54**, porque ele
corresponde à geração do nosso client. O W2PP 7.59 foi usado para confirmar o
fluxo e localizar lacunas, mas seus limites ampliados e suas regras customizadas
de criação não foram copiados.

### Criação do Celestial

1. No rei, um Arch equipa `1742 Eternal_Stone` no slot 10 e um Sephirot
   `1760–1763` no slot 11, carrega as quatro pedras secretas `5334–5337` e
   recebe a `5338 Ideal_Stone`. Todos os materiais são consumidos.
2. Ao usar a Pedra Ideal, o Arch precisa estar no nível interno `355` ou maior
   (356 exibido), ter o Mortal respectivo no nível interno `399` (400 exibido)
   e não estar transformado.
3. A transformação ocorre **no mesmo personagem/UID**: EXP e nível voltam a
   zero, STR/INT/DEX/CON a 5, masteries a zero, o orçamento de mastery a 855 e
   `LearnedSkill` recebe o bit 30 da `Soul of Limits`.

Os quatro cristais Arch **não são portão de criação** no 7.54. Eles concedem
100 pontos cada ao orçamento do Celestial. O nível em que o Arch ascende define
outro bônus e a Cythera inicial:

| Arch interno (exibido) | bônus | Cythera |
|---|---:|---|
| 355–369 (356–370) | 100 | 3500 Silver |
| 370–379 (371–380) | 300 | 3500 Silver |
| 380–397 (381–398) | 600 | 3501 Gold |
| 398 (399) | 900 | 3501 Gold |
| 399 (400) | 1.200 | 3502 Mystic |

A capa passa à variante Master do mesmo reino (`3197/3198/3199`). A identidade,
inventário, gold, equipamentos, reino/capa, guild, fama, quests, cristais e
configuração da Soul permanecem compartilhados.

### Criação e troca da SubCelestial

O Celestial ativo no nível interno `120` (121 exibido) equipa um novo Sephirot
no slot 11 e usa outra Pedra Ideal. A Sub nasce no nível zero, com stats 5,
masteries zero, orçamento 855 e Soul aprendida. O Sephirot define sua classe de
skills; o corpo permanece o mesmo e só o rosto lógico é ajustado.

O servidor entrega dez `4148 Mysterious_Stone`. Cada uso dentro de uma cidade
alterna Celestial/SubCelestial e consome uma unidade. Não há novo personagem,
novo UID ou cópia de inventário.

São **específicos de cada forma**: evolução ativa, classe, rosto, EXP/nível,
atributos distribuídos, masteries, skills aprendidas, barra e affects. São
**compartilhados**: todo o restante do personagem citado acima.

A criação da Sub exige **100 de fama**, além do nível 121, Sephirot e Pedra
Ideal. A regra segue o WYD KR/guias retail escolhidos para esta progressão. As
duas formas continuam no mesmo slot e a troca em cidade usa Pedra Misteriosa.

### Pontos, EXP e travas

O orçamento de status da forma ativa é:

```text
1001
+ 100 × cristais Arch concluídos
+ bônus da faixa em que o Arch virou Celestial
+ 10 × nível interno da forma ativa
+ 6 × nível interno da outra forma (quando existir)
+ 290 quando a forma ativa passa do nível interno 189
```

Os atributos gastos continuam separados por forma. Portanto, o nível da forma
inativa aumenta o orçamento da ativa, mas não copia STR/INT/DEX/CON já
distribuídos.

- skill points: `1500 + 4 × nível ativo`, mais o bônus único da Magical Pill;
- mastery: 855 pontos por forma; cada ramo limita em 200, ou 255 quando sua
  oitava skill estiver aprendida;
- EXP acumulada: `20.000.000 × nível interno`;
- teto 7.54: nível interno 199, exibido como 200;
- somente o Celestial principal trava nos níveis internos 39 e 89 (40/90
  exibidos); a Sub não usa essas duas travas;
- EXP de combate é dividida cumulativamente por dois nos níveis internos
  149, 159, 169, 179 e 189. Recompensas fixas de item/quest não sofrem essa
  redução.

O destrave 40 permanece no Alquimista Odin. O destrave 90 usa `3020 Fury_Stone`
e 500 de fama. No Secrets 7.54 esse destrave é determinístico; a chance de 50%
pertence à composição da Cythera Arcana no nível 200: Mystic Cythera +11,
quatro pedras secretas, Fury Stone e 500 de fama.

### Cythera e Soul

Ao atingir os níveis internos 120, 150, 180 e 198, a Cythera recebe,
respectivamente, adicionais AC/HP `30/40`, `35/50`, `45/60` e `50/70`. O valor
bruto do refino é preservado e normalizado para `EF_SANC`; tintura e outros
efeitos antigos são substituídos, como em `PutAddOnItem` da 7.54. Cythera sem
refino não recebe `+3` artificial. Cada marco compartilhado só é aplicado uma
vez.

A skill especial 102 (`Soul of Limits`) usa o bit 30 de `LearnedSkill` e o
affect 29. `SoulInfo` é compartilhado entre as formas; o affect ativo e sua
duração pertencem à forma. Os dez perfis nativos (120% puro ou combinações
80%/40% e 40%/80%) são calculados sobre os atributos crus server-side.

Troca de forma e criação sincronizam score, pontos, skills, affects, barra,
equipamentos, aparência e HP/MP sem recriar o jogador por `CreateMob`.

## Arquivos

```text
internal/game/ascension.go       as três etapas + isArch/refreshArchMortalLevel
internal/game/stats.go           archStatusPointBudget e statusPointBudget
internal/game/progression.go     archLockLevel355/370 e archExperienceLocked
internal/game/crafting_simple.go lindyLevelUnlock (o destrave na Lindy)
internal/game/arch_crystals.go   as quatro juras elementais
internal/game/celestial.go       criação, troca, Fury/Arcana e Cythera
internal/game/counters.go        a fama e o kefra_ticket
internal/model/model.go          ArchMortalUID / ArchMortalLevel /
                                 ArchLevel355 / ArchLevel370 / ArchCrystals /
                                 AlternateCelestial e flags Celestial
```

## Testes

`ascension_test.go` cobre requisitos, consumo, rollback por falha de disco, o
Mortal permanecendo, o nome herdado e o rosto nas quatro classes.
`arch_points_test.go` cobre a fórmula contra valores calculados à mão, o bônus
dinâmico e o Mortal apagado. `arch_locks_test.go` cobre a EXP barrada nas duas
travas e liberada depois do destrave; `arch_crystals_test.go` cobre a ordem, o
piso de nível, o desconto de EXP e o rollback por falha de disco.

## Como testar in-game

O NPC de teste **`ShopArch`** fica em Armia, em **(2100, 2102)** — dois tiles ao
norte do ponto de nascimento, para não ocupar o tile onde os jogadores aparecem.
Vende os insumos da cadeia, os atalhos e itens de EXP. (Chamava-se `LojaArch`
antes da tradução dos nomes de NPC.)

Caminho mais barato (~150.000 de gold):

1. **~27 Fairy_Dust** (5.000 cada, 100M de EXP cada) → nível 371. O nível 371
   exige 2,66 bilhões de EXP e um personagem novo começa com 0 de gold.
   `Eye_Wax` é grátis mas dá 500k, o que exigiria ~5.300 usos.
2. **Sefirot** (7.000) e **Eternal Stone** (5.000) pelos atalhos.
3. Equipar Pedra no slot 10 e Sefirot no 11 → clicar no rei.

Para testar a **cadeia completa** de composição: 4M nas 8 pedras + 30M exigidos
pelo craft do Sefirot + ~10M nas safiras.

> `ShopArch` é NPC de teste, como as três lojas de volatile ao lado dele
> (2103-2105, 2102). Durante o beta eles ficam no ar de propósito — todos os
> itens precisam estar disponíveis para teste. Remova os JSONs e os blocos
> correspondentes do `data/NPCGener.txt` antes de abrir para jogadores.

## Pendências

- **Nada validado in-game.**
- Desempate nas buscas por nome duplicado.
- Validação visual in-game da criação, troca de forma, Soul, quatro marcos da
  Cythera e ambos os destraves.
- O volatile 206 agora possui o fluxo nativo de Spirit's Seal: 3443 encapsula
  a ficha Celestial/SubCelestial e os handlers 0x2CD/0x3CC consultam/extraem o
  snapshot com UID e persistência atômica. O item 3455 permanece no-direct-use,
  pois o W2PP não o aceita no PutoutSeal. A validação visual está em
  `DOCS/CELESTIAL_CAPSULE.md`.
- Skills e itens exclusivos de Arch.
