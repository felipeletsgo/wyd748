-- Boss 2: Guardian of the Ruins, a pure chaser.
--
--
-- The chaser is the simplest type: it chases and hits. It needs no skills,
-- summons or phases; only the chase and the attributes are tuned.

return {
  id   = "guardiao_das_ruinas",
  npc  = "LegendBerial",         -- base NPC supplies the face and equipment
  name = "Guardian of the Ruins",
  type = "chaser",

  spawn = {
    x = 3806, y = 4019,
    respawn_seconds = 90,
  },

  -- The base NPC has 18000 HP; a boss must withstand more than that.
  stats = {
    level      = 218,
    con        = 5000,
    max_hp     = 18000000 * 12,      -- arithmetic is allowed: this is Lua, not JSON
    attack     = 700,
    defense    = 380,
    exp_reward = 250000,
    gold       = 20000,
  },


  -- Items checked against data/catalog/itemlist.csv.
  drops = {
    { item = 697,  chance_percent = 100, amount = 3 },  -- Sapphire (guaranteed)
    { item = 1775, chance_percent = 25 },               -- 11_Refine_Item (rare)
  },
  -- Shared reward: dropped on the GROUND around the body, one unit per cell,
  -- for everyone present to pick up. The `drops` above go only to the player
  -- who landed the final blow.
  area_reward = { item = 420, amount = 50 },


  spawn_message = "The Guardian of the Ruins awakens!",
  death_message = "The Guardian of the Ruins has been defeated.",
}
