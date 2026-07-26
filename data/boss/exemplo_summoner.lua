-- EXEMPLO: boss que invoca adds.
--
--
-- Os adds sao mobs COMUNS do catalogo: usam a IA normal, dao exp e morrem
-- normalmente. `max_alive` e o teto simultaneo -- sem ele, um cooldown curto
-- encheria a tela numa luta longa.

return {
  id   = "necromante_ancestral",
  npc  = "Astaroth",
  name = "Ancient Necromancer",
  type = "summoner",

  spawn = {
    x = 2336, y = 2125,
    respawn_seconds = 2400,
  },

  stats = {
    level      = 300,
    max_hp     = 25000000 * 12,
    attack     = 850,
    defense    = 560,
    exp_reward = 1200000,
    gold       = 40000,
  },


  summons = {
    {
      npc = "Skeleton",
      count = 4,
      max_alive = 8,
      cooldown_seconds = 30,
      message = "The Ancient Necromancer calls the dead!",
    },
  },

  -- Itens conferidos em data/itemlist.csv.
  drops = {
    { item = 4131, chance_percent = 100 },  -- Bundles_Sapphire(10)
    { item = 1775, chance_percent = 15 },   -- 11_Refine_Item
  },
  -- Premiacao coletiva: cai no CHAO ao redor do corpo, uma unidade por celula,
  -- para todos os presentes recolherem. Os `drops` acima vao so para quem deu
  -- o golpe final.
  area_reward = { item = 420, amount = 50 },


  spawn_message = "The Ancient Necromancer rises!",
  death_message = "The Ancient Necromancer returns to dust.",
}
