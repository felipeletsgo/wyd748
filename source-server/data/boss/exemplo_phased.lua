-- EXEMPLO: boss com fases por HP -- o mais completo.
--
--
-- O tipo define o que e OBRIGATORIO, nao o que e permitido: este boss e
-- `phased` e ainda assim tem `skills` e `summons`, recebendo as regras dos tres.
--
-- Mecanica classica: aos 60% ele ergue um escudo que absorve TUDO e chama
-- guardas. O escudo so cai quando o ultimo guarda morre -- por isso
-- `shield_until_adds_dead` exige que o boss tenha `summons` (o validador recusa
-- a combinacao impossivel, senao o escudo nunca cairia).

return {
  id   = "senhor_demoniaco",
  npc  = "Demon_Lord__",
  name = "Demon Lord",
  type = "phased",

  spawn = {
    x = 2281, y = 2153,
    respawn_seconds = 3600,       -- 1 hora
  },

  stats = {
    level      = 400,
    max_hp     = 20000000 * 20,
    attack     = 1100,
    defense    = 620,
    exp_reward = 2500000,
    gold       = 60000,
  },


  -- Tempestade de Gelo (23), conferida em data/SkillData.csv.
  skills = {
    {
      id = 23,
      cooldown_seconds = 10,
      range = 4,
      max_hp_percent = 100,
      message = "The Demon Lord casts a storm!",
    },
  },

  summons = {
    {
      npc = "Skeleton",
      count = 4,
      max_alive = 6,
      cooldown_seconds = 45,
    },
  },

  phases = {
    -- Aos 60%: escudo total ate os guardas morrerem.
    {
      hp_percent = 60,
      type = "summoner",
      shield_percent = 100,
      shield_until_adds_dead = true,
      message = "The Demon Lord summons his guardians!",
    },
    -- Aos 25%: enrage, sem escudo.
    {
      hp_percent = 25,
      type = "caster",
      message = "The Demon Lord flies into a rage!",
    },
  },

  -- Itens conferidos em data/itemlist.csv.
  drops = {
    { item = 4131, chance_percent = 100 },  -- Bundles_Sapphire(10) (garantido)
    { item = 1775, chance_percent = 50 },   -- 11_Refine_Item
    { item = 1776, chance_percent = 5 },    -- 12_Refine_Item (muito raro)
  },
  -- Premiacao coletiva: cai no CHAO ao redor do corpo, uma unidade por celula,
  -- para todos os presentes recolherem. Os `drops` acima vao so para quem deu
  -- o golpe final.
  area_reward = { item = 420, amount = 50 },


  spawn_message = "The Demon Lord emerges from the abyss!",
  death_message = "The Demon Lord has been defeated!",
}
