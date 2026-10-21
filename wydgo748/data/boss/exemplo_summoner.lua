-- Boss 4: Ancient Necromancer, summons adds.
--
--
-- The adds are ORDINARY catalog mobs: they use the normal AI, grant EXP and die
-- normally. `max_alive` caps how many live at once; without it a short cooldown
-- would flood the screen in a long fight.

return {
  id   = "necromante_ancestral",
  npc  = "Anubis",
  name = "Ancient Necromancer",
  type = "summoner",

  spawn = {
    x = 2083, y = 347,
    respawn_seconds = 240,
  },

  stats = {
    level      = 300,
    con        = 5000,
    max_hp     = 25000000 * 12,
    attack     = 850,
    defense    = 560,
    exp_reward = 1200000,
    gold       = 40000,
  },


  summons = {
    {
      npc = "Skeleton",
      count = 10,
      max_alive = 10,
      cooldown_seconds = 60,
      message = "The Ancient Necromancer calls the dead!",
    },
  },

  -- Items checked against data/itemlist.csv.
  drops = {
    { item = 4131, chance_percent = 100 },  -- Bundles_Sapphire(10)
    { item = 1775, chance_percent = 15 },   -- 11_Refine_Item
  },
  -- Shared reward: dropped on the GROUND around the body, one unit per cell,
  -- for everyone present to pick up. The `drops` above go only to the player
  -- who landed the final blow.
  area_reward = { item = 420, amount = 50 },


  spawn_message = "The Ancient Necromancer rises!",
  death_message = "The Ancient Necromancer returns to dust.",
}
