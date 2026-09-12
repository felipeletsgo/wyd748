-- EXEMPLO: boss perseguidor puro.
--
--
-- O chaser e o tipo mais simples: persegue e bate. Nao precisa de skills,
-- summons nem phases -- so calibra a perseguicao e os atributos.

return {
  id   = "guardiao_das_ruinas",
  npc  = "Fire_Golem",         -- assets (rosto/equip) vem de data/npcs/Fire_Golem.json
  name = "Guardian of the Ruins",
  type = "chaser",

  spawn = {
    x = 2275, y = 2082,
    respawn_seconds = 900,        -- 15 minutos apos a morte
  },

  -- O NPC base tem 18000 de HP; um boss precisa aguentar mais que isso.
  stats = {
    level      = 218,
    max_hp     = 18000000 * 12,      -- da para calcular: e Lua, nao JSON
    attack     = 700,
    defense    = 380,
    exp_reward = 250000,
    gold       = 20000,
  },


  -- Itens conferidos em data/itemlist.csv.
  drops = {
    { item = 697,  chance_percent = 100, amount = 3 },  -- Sapphire (garantido)
    { item = 1775, chance_percent = 25 },               -- 11_Refine_Item (raro)
  },
  -- Premiacao coletiva: cai no CHAO ao redor do corpo, uma unidade por celula,
  -- para todos os presentes recolherem. Os `drops` acima vao so para quem deu
  -- o golpe final.
  area_reward = { item = 420, amount = 50 },


  spawn_message = "The Guardian of the Ruins awakens!",
  death_message = "The Guardian of the Ruins has been defeated.",
}
