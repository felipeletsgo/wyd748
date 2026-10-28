-- Boss 1: Black Archmage, a spell caster.
--
--
-- The caster casts its configured skills when it takes damage. Each skill has
-- its own cooldown and range; `max_hp_percent` turns a skill into a desperate
-- move, unlocked only after the boss has taken enough damage.

return {
  id   = "arquimago_negro",
  npc  = "DeathWorm",
  name = "Black Archmage",
  type = "caster",

  spawn = {
    x = 3910, y = 3418,
    respawn_seconds = 180,
  },

  stats = {
    level      = 200,
    con        = 5000,
    max_hp     = 20000000 * 12,
    attack     = 900,
    defense    = 500,
    exp_reward = 800000,
    gold       = 35000,
  },


  -- IDs and ranges match data/catalog/SkillData.csv. Use OFFENSIVE spells:
  -- 27 (Heal) and 26 (Flash), for example, are not damage skills.
  skills = {
    -- Ice Storm (23): ice damage, range 4.
    {
      id = 23,
      cooldown_seconds = 2,
      range = 10,
      max_hp_percent = 100,      -- always available
      message = "The Black Archmage summons an ice storm!",
    },
    -- Divine Shock (28): stronger, only from 40% HP down.
    {
      id = 28,
      cooldown_seconds = 2,
      range = 10,
      max_hp_percent = 40,
      message = "The Black Archmage unleashes his fury!",
    },
  },

  -- Items checked against data/catalog/itemlist.csv.
  drops = {
    { item = 697,  chance_percent = 100, amount = 5 },  -- Sapphire
    { item = 1775, chance_percent = 20 },               -- 11_Refine_Item
  },
  -- Shared reward: dropped on the GROUND around the body, one unit per cell,
  -- for everyone present to pick up. The `drops` above go only to the player
  -- who landed the final blow.
  area_reward = { item = 420, amount = 50 },


  spawn_message = "The Black Archmage rises from the shadows!",
  death_message = "The Black Archmage has been silenced.",
}
