# Crafting server-side

Status: implementado localmente; aguardando a matriz de testes no client 7.48.

## NPCs e protocolo

| NPC | Opcode | Receitas cobertas |
| --- | ---: | --- |
| Tiny | `0x3C0` | transferencia de adicional para arma Arch |
| Lindy | `0x2C3` | criacao de capa |
| Compositor | `0x3A6` | conversao anct por joia e 4 materiais set D/E +7..+9 |
| Agatha | `0x3BA` | transferencia Mortal para Arch |
| Aylin | `0x3B5` | composicao +10 colorida |
| Ehre | `0x2D3` | almas e refinacao abencoada |
| Alquimista Odin | `0x2D2` | runas, pedras secretas e refinacao avancada |

Os handlers recebem o `MSG_CombineItem` nativo de 84 bytes. Os oito itens e
posicoes enviados pelo client sao somente referencias: cada snapshot precisa
ser identico ao slot atual do inventario autoritativo. O servidor valida receita,
catalogo, grade, tipo, posicao, refino, gold e progressao. A alteracao e salva
antes da resposta comum `0x3A7`; falha de persistencia restaura o estado.

As taxas do Compositor ficam em `data/server.txt` (`compositor_base` e
`compositor_refine_7/8/9`); os demais custos e chances de craft continuam
constantes no codigo.

## Item bloqueado em toda composicao

`Atila's_Crown` (indice 747) em QUALQUER um dos oito slots recusa a composicao.
A trava fica em `beginCombine`, de uma vez para todos os artesaos. Embora
`GetMatchCombineAgatha` nao repita a varredura, o proprio handler da Agatha
recusa o item em `_MSG_CombineItemAgatha.cpp:54-55`.

## Compositor: a chance de sucesso

A composicao exige **exatamente quatro materiais**, nas quatro posicoes nativas
`2..5` da janela. Nao se compoe com tres, dois, um ou nenhum, nem com itens
injetados nos slots ocultos `6..7`. Cada material precisa ser:

- equipavel (`EF_POS != 0`);
- do **set D** (`EF_ITEMLEVEL` 4) ou **set E** (`EF_ITEMLEVEL` 5);
- refinado **+7, +8 ou +9**.

Qualquer material fora disso recusa a composicao inteira. O **`Grade` do item nao
entra na conta** -- so o refino entra.

```
chance = compositor_base + Σ compositor_refine_<refino> dos quatro materiais
```

Com os valores de `data/server.txt`:

| materiais | conta | chance |
| --- | --- | ---: |
| 4 × +7 | 10 + 3+3+3+3 | 22% |
| 4 × +8 | 10 + 5+5+5+5 | 30% |
| 4 × +9 | 10 + 8+8+8+8 | **42%** (teto) |

O sorteio e o nativo (`_MSG_CombineItem.cpp:74-78`): `rand()%115`, e se sair >= 100
subtrai 15; sucesso se o valor for `<= chance`. A dobra faz a faixa 85..99 ter
peso duplo -- e assim de proposito.

O log traz a quebra por material, para diagnosticar sem reconstruir a conta:

```
CRAFT Compositor sucesso=true chance=42 (base 10 + 741(setE+9:8) ... )
```

### Por que divergimos do que a W2PP executa

A W2PP tem **duas rotinas concorrentes**, e a que roda nao e a que o autor
pretendia:

| onde | o que | estado |
| --- | --- | --- |
| `GetFunc.cpp:51-207` `GetMatchCombine` | tabela por `(nivel, grade, refino)`, valores 4..12, somados sobre ate 6 materiais | **e o que compila e roda** |
| `Basedef.cpp:158` `g_pAnctChance[3] = {4,8,12}` | bonus por material, indexado por refino | **nenhuma funcao le esse array** |
| `CReadFiles.cpp:584-599` | le `COMPOSITOR ITEM_+7/+8/+9` de `Common/Settings/CompRate.txt` | grava no array morto |
| `CompRate.txt:14-16` | `Compositor Item_+7 2 / Item_+8 4 / Item_+9 10` | **nunca casa**: o parser so faz `_strupr(str1)`, entao `Item_+7` != `"ITEM_+7"` |

Ou seja: a chance configuravel morreu duas vezes na W2PP, e a tabela velha ficou
rodando por inercia. Nossa implementacao adota a **forma** que o autor projetou
(base + bonus por refino, configuravel) e a faz funcionar de verdade. Os
**valores** (10 / 3 / 5 / 8) e a regra dos quatro materiais sao definicao do
servidor, nao daquele array.

Isso fica registrado aqui para nao ser lido depois como erro de porte.

## Lojas de teste

Cada loja de materiais fica ao lado do respectivo artesao em sua coordenada
nativa. Isso preserva os seletores regionais da janela 7.48: Tiny
`(3646,3115)`, Lindy `(1706,1727)`, Compositor/Aylin/Agatha na regiao
`(2525..2538,1734)`, Ehre `(2454,2000)` e Odin `(3237,1691)`.
Odin possui `TestOdin` e `TestOdinExtra`, pois uma unica lista ultrapassaria os
24 slots visiveis da loja no client 7.48.

As lojas entregam armas e joias ja refinadas, pilhas com quantidade correta e
catalisadores adequados. Elas existem somente para a validacao in-game.

## Restricoes temporarias conhecidas

- Lindy aceita somente Arch exatamente nas travas internas 354/369. No destrave
  354 substitui a capa por 3191 (Hekalotia), 3192 (Akelonia) ou 3193 (neutro);
  no destrave 369 consome uma fame e preserva a capa já equipada.
- Ehre e parte das receitas de Odin exigem personagem Arch/Celestial, nivel,
  fame ou EXP. A loja fornece os itens, mas nao ignora esses requisitos.

## Fama

A fama e um contador POR PERSONAGEM, e nao um campo da conta: vive em
`CharState.SpecialCoins["fame"]`, gravado por `Char.UID`. Dois
personagens da mesma conta tem saldos independentes.

| | onde | quanto |
|---|---|---|
| ganha | `Warrior's_Seal` (item 4146, volatile 199) | +10 por selo |
| gasta | Lindy, destrave do nivel 370 do Arch | 1 |
| gasta | Odin, nivel 40 do Celestial | 200 |

O valor de +10 vem de `_MSG_UseItem.cpp:7147` (`extra.Fame += 10`).

**Ordem transacional obrigatoria.** No PostgreSQL, os dois crafts que gastam
fama usam `SavePlayerState`: receita, destrave e charstate entram no mesmo
commit `SERIALIZABLE`. Falha de item, constraint ou banco restaura tudo; nao
existe janela para destrave gratuito. O JSON de desenvolvimento preserva o
fallback sidecar-antes-da-conta.
- A validacao final deve registrar, receita por receita, sucesso, falha,
  consumo, resultado visual, relogin e rollback.
