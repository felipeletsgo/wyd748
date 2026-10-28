-- Boss 3: Demon Lord, HP phases (the most complete type).
--
--
-- The type defines what is REQUIRED, not what is allowed: this boss is
-- `phased` and still has `skills` and `summons`, so all three rule sets apply.
--
-- Classic mechanic: at 60% it raises a shield that absorbs EVERYTHING and calls
-- guards. The shield drops only when the last guard dies, so
-- `shield_until_adds_dead` requires the boss to have `summons` (the validator
-- rejects the impossible combination, or the shield would never drop).

return {
  id   = "senhor_demoniaco",
  npc  = "Murmy",
  name = "Demon Lord",
  type = "phased",

  spawn = {
    x = 731, y = 3173,
    respawn_seconds = 360,
  },

  stats = {
    level      = 400,
    con        = 5000,
    max_hp     = 20000000 * 20,
    attack     = 1100,
    defense    = 620,
    exp_reward = 2500000,
    gold       = 60000,
  },


  -- Ice Storm (23), checked against data/catalog/SkillData.csv.
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
    -- At 60%: full shield until the guards die.
    {
      hp_percent = 60,
      type = "summoner",
      shield_percent = 100,
      shield_until_adds_dead = true,
      message = "The Demon Lord summons his guardians!",
    },
    -- At 25%: enrage, no shield.
    {
      hp_percent = 25,
      type = "caster",
      message = "The Demon Lord flies into a rage!",
    },
  },

  -- Items checked against data/catalog/itemlist.csv.
  drops = {
    { item = 4131, chance_percent = 100 },  -- Bundles_Sapphire(10) (guaranteed)
    { item = 1775, chance_percent = 50 },   -- 11_Refine_Item
    { item = 1776, chance_percent = 5 },    -- 12_Refine_Item (very rare)
  },
  -- Shared reward: dropped on the GROUND around the body, one unit per cell,
  -- for everyone present to pick up. The `drops` above go only to the player
  -- who landed the final blow.
  area_reward = { item = 420, amount = 50 },


  spawn_message = "The Demon Lord emerges from the abyss!",
  death_message = "The Demon Lord has been defeated!",
}
