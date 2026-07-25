# data/boss — encontros de boss

Cada arquivo `.lua` deste diretório define **um** boss. O servidor lê todos os
`.lua` na ordem alfabética durante o boot.

Os quatro `exemplo_*.lua` estão **ativos** — um boss de cada tipo. Para desligar
um, apague o arquivo ou troque a extensão (só `.lua` é carregado).

Um erro em qualquer arquivo **derruba o boot** de propósito: é melhor falhar na
hora, com a mensagem apontando o campo, do que subir com um chefe que nunca
nasce ou que nasce fraco por um campo digitado errado.

## Como funciona

- **O comportamento é hardcoded em Go** (`internal/game/boss_types.go`). O Lua
  só escolhe o tipo pelo nome e fornece os números. Nenhum arquivo daqui
  consegue inventar regra nova nem alterar estado do servidor.
- **Os assets vêm do NPC** indicado em `npc` (rosto, equipamento e atributos
  base saem de `data/npcs/<nome>.json`). O bloco `stats` sobrescreve o que
  quiser; o que for omitido mantém o valor do NPC.
- **Prefira um NPC que o `NPCGener.txt` não spawna.** Reaproveitar a aparência
  de um mob comum ou de quest (Hidra_Imortal, Aparicao, Servo_Elfo…) confunde o
  jogador: o chefe fica com a cara de um monstro que ele mata o dia inteiro. O
  catálogo tem 491 NPCs e boa parte não é spawnada — são assets livres.
  Bons candidatos: `Demon_Lord__` (nível 400), `Dark_Shadow___` (400),
  `Astaroth`, `Arq_Ciclope` (200), `Golem_de_Fogo` (218), `E-Dracolich`.
  Para listar os livres:

  ```bash
  comm -23 <(ls data/npcs/*.json | sed 's|.*/||; s|\.json$||' | sort) \
           <(tr -s ' \t' '\n' < data/NPCGener.txt | grep -oE '^[A-Za-z_.][A-Za-z0-9_.]*$' | sort -u)
  ```

  Adds (`summons`) são diferentes: ali **é esperado** usar mob comum, porque
  eles são mobs comuns mesmo.
- **Bosses não passam pelo NPCGener.** Posição e renascimento são deste arquivo.
  Nenhum mob comum do mundo vira boss.
- **O HP não se recupera.** O dano permanece onde os jogadores o deixaram: um
  chefe é feito para ter HP altíssimo e cair ao longo de horas, possivelmente em
  várias sessões. Nada restaura vida — só a morte, que faz nascer uma instância
  nova depois do `respawn_seconds`.
- **A perseguição é a padrão do jogo.** O boss usa os mesmos alcances de
  aquisição, leash e ataque de qualquer mob — não há o que configurar. O leash
  de 16 é proposital: mais que isso e o chefe se afastaria demais do ponto de
  origem.
- O `carry` do NPC base é ignorado: os drops do boss são os de `drops`.

## Sandbox

O interpretador sobe **sem** `os`, `io`, `package`, `debug`, `dofile`,
`loadfile` e `load`, e com limite de 5 segundos por arquivo. As bibliotecas
seguras (`string`, `table`, `math`) continuam disponíveis — dá para escrever
`max_hp = 500 * 1000` ou montar uma lista com um laço.

## Tipos

| Tipo | Comportamento | Exige |
|---|---|---|
| `chaser` | Persegue e ataca corpo a corpo. | — |
| `caster` | Lança as skills configuradas ao levar dano. | `skills` |
| `summoner` | Invoca grupos de adds durante a luta. | `summons` |
| `phased` | Troca de comportamento em limiares de HP. | `phases` |

O tipo define o que é **obrigatório**, não o que é permitido: um `phased` pode
ter `skills` e `summons` também, e recebe as regras dos três.

## Campos

```lua
return {
  id   = "identificador_unico",   -- obrigatório, único entre os arquivos
  npc  = "Nome_Do_NPC",           -- obrigatório, precisa existir em data/npcs
  name = "Nome exibido",          -- opcional; vazio usa o nome do NPC
  type = "phased",                -- chaser | caster | summoner | phased

  spawn = {
    x = 2100, y = 2100,           -- obrigatório (nenhum pode ser 0)
    respawn_seconds = 1800,       -- 0 ou ausente = não renasce
  },

  stats = {                       -- tudo opcional; omitido mantém o do NPC
    level = 300, max_hp = 500000,
    attack = 900, defense = 450,
    attack_run = 0x64,
    exp_reward = 2000000, gold = 50000,
  },


  skills = {                      -- usado por caster
    { id = 23, cooldown_seconds = 8, range = 4,   -- id/range: SkillData.csv
      max_hp_percent = 100,       -- só libera com HP <= isso; 100 = sempre
      message = "texto" },
  },

  summons = {                     -- usado por summoner
    { npc = "Esqueleto", count = 4,
      max_alive = 8,              -- teto simultâneo; 0 = sem teto
      cooldown_seconds = 30, message = "texto" },
  },

  phases = {                      -- usado por phased
    { hp_percent = 60,            -- limiar, atravessado de cima para baixo
      type = "summoner",          -- vazio mantém o tipo do topo
      shield_percent = 100,       -- 0 = sem escudo; 100 = imune
      shield_until_adds_dead = true,
      message = "texto" },
  },

  drops = {                       -- recompensas próprias, além de exp/gold
    { item = 697, chance_percent = 100, amount = 3 },   -- item: itemlist.csv
  },

  spawn_message = "texto",        -- anunciado ao mundo no nascimento
  death_message = "texto",
}
```

Campo desconhecido é **erro**, não aviso: `atack = 500` seria silenciosamente
ignorado e o boss nasceria fraco sem ninguém entender o porquê.

## Confira os IDs

O servidor valida a *estrutura*, mas não sabe se um item ou skill faz sentido:

- **Skills** (`data/SkillData.csv`): use magia **ofensiva**. A 27 é `Cura` e a
  26 é `Flash` — configurá-las faria o chefe "atacar" curando. Boas opções:
  23 `Tempestade_de_Gelo` (alcance 4), 28 `Choque_Divino` (5),
  7 `Destino` (4), 22 `Exterminar` (3).
- **Itens** (`data/itemlist.csv`): um índice inexistente vira um drop que o
  cliente não sabe desenhar. Conferir com:

  ```bash
  awk -F, -v i=697 '$1==i {print $1" = "$2}' data/itemlist.csv
  ```
