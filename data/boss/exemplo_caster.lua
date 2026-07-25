-- EXEMPLO: boss que usa magia.
--
--
-- O caster lanca as skills configuradas ao levar dano. Cada skill tem cooldown
-- e alcance proprios; `max_hp_percent` transforma uma skill em recurso de
-- desespero, liberado so quando o boss ja apanhou bastante.

return {
  id   = "arquimago_negro",
  npc  = "Arq_Ciclope",
  name = "Black Archmage",
  type = "caster",

  spawn = {
    x = 2206, y = 2145,
    respawn_seconds = 1800,
  },

  stats = {
    level      = 200,
    max_hp     = 20000000 * 12,
    attack     = 900,
    defense    = 500,
    exp_reward = 800000,
    gold       = 35000,
  },


  -- IDs e alcances conferem com data/SkillData.csv. Use magia OFENSIVA:
  -- a 27 (Cura) e a 26 (Flash), por exemplo, nao sao skills de dano.
  skills = {
    -- Tempestade de Gelo (23): dano de gelo, alcance 4.
    {
      id = 23,
      cooldown_seconds = 2,
      range = 10,
      max_hp_percent = 100,      -- sempre disponivel
      message = "The Black Archmage summons an ice storm!",
    },
    -- Choque Divino (28): mais forte, so a partir de 40% de HP.
    {
      id = 28,
      cooldown_seconds = 2,
      range = 10,
      max_hp_percent = 40,
      message = "The Black Archmage unleashes his fury!",
    },
  },

  -- Itens conferidos em data/itemlist.csv.
  drops = {
    { item = 697,  chance_percent = 100, amount = 5 },  -- Sapphire
    { item = 1775, chance_percent = 20 },               -- 11_Refine_Item
  },

  spawn_message = "The Black Archmage rises from the shadows!",
  death_message = "The Black Archmage has been silenced.",
}
