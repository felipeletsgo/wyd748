// Editorial content is reviewed in Git and published with the static portal.
// Do not fill unknown values with estimates or describe planned systems as live.
export const clientRelease = {
  version: 1,
  clientVersion: '7.48',
  patchVersion: 10035,
  publishedAt: '2026-10-18',
  launcherVersion: null,
  minimumLauncherVersion: null,
  fullClient: {
    url: 'https://drive.google.com/file/d/1Kvjl4pOY7FYUsmFsEvbQhivAwhqJRXEj/view?usp=drive_link',
    sizeBytes: null,
    sha256: null,
  },
  patch: {
    url: '/patch/patch-10035.zip',
    sizeBytes: 846770,
    sha256: '8172AFA041A234E8C01BD956DEB36591062E70EEE8A99B0A3FFCC62B24C77E2A',
    executableSha256: '251CF754409F8AE4C0251C9FDD88EDA14C6799E78A377BD0665B6E858FDAFAB0',
  },
  // This is a website receipt, not a replacement for the legacy launcher feed.
  updateFeed: '/patch/update.htm',
} as const;

export const news = [
  {
    slug: 'client-safety-10035', date: '2026-10-18', category: 'News',
    title: 'Patch 10035: client stability safeguards',
    summary: 'Harden packet and text handling, minimap uploads, actor cleanup and skin-mesh loading.',
    paragraphs: [
      'This update adds bounded packet and text handling, safer minimap texture copying and cleanup after aborted UI frames.',
      'Nested network notifications are serialized while packet handlers are active. Actor removal clears nested effect references, and skin-mesh loading validates sizes, bone palettes, positions and indices before publishing resources.',
      'These safeguards do not change skill visuals, rewards or encounter rules. Automated checks passed; the causes of the historical Mana Control, item-drop and heap-corruption incidents remain under investigation.',
      'Close the game and open WYDLauncher.exe to receive patch 10035. Actual gameplay acceptance and broader graphics-resource recovery remain pending.',
    ],
  },
  {
    slug: 'hell-gate-routes-10034', date: '2026-10-17', category: 'News',
    title: 'Patch 10034: Hell Gate routes and defense',
    summary: 'Restore the event route through corridor bosses, room guardians, Liches and the defense of Dionys.',
    paragraphs: [
      'Hell Gate lasts twenty minutes total: two minutes for entry and eighteen for the encounter. Defeat the entry spider, find the corridor passage and clear the room guardians and Liches to reach the defense of Dionys.',
      'The three internal portals now use server-validated destinations inside the encounter. The client displays the generic transport prompt for these routes. Wrong corridor or Lich choices do not permanently block progression.',
      'Entry spiders, corridor bosses and Liches have 400,000 HP. Keep Dionys alive until the deadline for the 10,000,000 completion EXP reward, subject to normal progression limits. Event state and rewards are persisted.',
      'Close the game and open WYDLauncher.exe to receive patch 10034. Automated checks passed; actual route, visual and solo-balance acceptance remain pending. These encounter settings do not claim complete native parity.',
    ],
  },
  {
    slug: 'instance-party-recovery', date: '2026-10-17', category: 'News',
    title: 'Server update: instance parties and reconnect recovery',
    summary: 'Group tickets keep eligible party members together; Nightmare reconnects restore the encounter HUD and available party membership.',
    paragraphs: [
      'Group Nightmare entry requires the party leader and consumes only that leader\'s ticket. Eligible members enter the same encounter together. Individual tickets transport and charge only their user.',
      'Reconnecting to a live Nightmare restores its actual remaining time and protected-NPC count without resetting the encounter or charging another ticket. The original party is restored when it still exists and has a free slot; a returning former leader does not replace the current leader.',
      'Magic Chamber group entries keep eligible members in the same run; individual entries remain separate. Party relocation and room visibility are synchronized together.',
      'Compounder remains passive, and friendly city guards target monsters rather than players. Hell Gate now uses dedicated monster generators; its remaining event-design corrections are still pending.',
      'This server update requires no additional client patch. The latest launcher patch remains 10033. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'combat-controls-and-balances-10033', date: '2026-10-17', category: 'News',
    title: 'Patch 10033: double hits, controls and balance notices',
    summary: 'Paired client and server fixes for double hits, physical autoattack, number-row skills and private balance notices.',
    paragraphs: [
      'Double hits are guaranteed at 201% attack speed and above, not at exactly 200%. The 900 ms interval between attack actions remains unchanged. Both floating hit numbers use the same trajectory, with the second appearing after 120 ms.',
      'A manual physical attack keeps attacking its selected target until it dies or an interruption cancels the action. Number-row keys 1-0 select skills; Shift+1-0 assigns the hovered skill. F1-F10 do not select skills.',
      'The Helper scrollbar down arrow stays inside the panel. PageUp and PageDown scroll the visible chat list. Other previously reported Helper shortcut gaps are not claimed as completed.',
      'Use /balance or /saldo for Donate, Fame, Honor, event passes and Kefra entries. Use /medals or /medalhas separately for Gold, Red and Blue medals. Each command displays one private top notification, not chat; gold carried and CP are excluded. Unconfigured Honor is shown as N/A.',
      'Close the game and open WYDLauncher.exe to receive patch 10033. Automated checks passed; gameplay confirmation remains pending.',
    ],
  },
  {
    slug: 'instance-leveling-balance', date: '2026-10-16', category: 'News',
    title: 'Server update: dedicated leveling monsters',
    summary: 'Nightmare, Water and Magic Chamber use dedicated monsters with low damage, tier-based HP and EXP, and no monster loot.',
    paragraphs: [
      'All three activities now use exclusive monster definitions rather than changing monsters shared with other maps. Physical and magic attacks deal at most one damage before absorption, and defense is one.',
      'Nightmare Normal, Mystic and Arcane monsters have 2,000, 5,000 and 10,000 HP and grant base EXP of 200,000, 400,000 and 600,000 respectively. Each dead protector still reduces the original Nightmare EXP by 10%.',
      'Water and Magic Chamber Normal, Mystic and Arcane monsters have 1,000, 4,000 and 10,000 HP and grant base EXP of 200,000, 300,000 and 500,000 respectively, across every room in each tier.',
      'These monsters drop neither gold nor items, including event and boss loot. Existing room-completion rewards and Water Scroll progression are unchanged. Existing EXP modifiers and party sharing still apply.',
      'This server-only update requires no new client patch; the latest launcher patch remains 10032. Automated server checks passed; gameplay confirmation remains pending.',
    ],
  },
  {
    slug: 'nightmare-lifecycle-10032', date: '2026-10-16', category: 'News',
    title: 'Patch 10032: Nightmare clocks, protectors and Lugefer',
    summary: 'Separate admission from 15-minute combat, restore continuous common-monster respawns and count protected NPCs instead of hostile monsters.',
    paragraphs: [
      'Nightmare admission lasts four minutes. Combat then lasts 15 minutes on a shared clock; entering late or restoring the encounter does not reset its deadline.',
      'The remaining counter now reports living protected NPCs. Each dead protector removes 10% of the original monster EXP before party sharing. Common monsters keep respawning until the combat clock ends, even when every protector has died.',
      'In Arcane Nightmare, Lugefer appears once with five minutes remaining if at least one protector is alive. Defeating this boss does not start another boss respawn; its defeat is saved with player rewards.',
      'Close the game and open WYDLauncher.exe to receive patch 10032. Automated checks passed; in-game confirmation and complete native population and loot parity remain pending.',
    ],
  },
  {
    slug: 'chat-labels-and-buff-icons-10031', date: '2026-10-16', category: 'News',
    title: 'Patch 10031: chat labels, buff icons and guild kingdoms',
    summary: 'Colored tab messages, SND status, distinct buff icons and the pending guild, cape and weapon server corrections.',
    paragraphs: [
      'Use /tab text or /tab2 through /tab9 to set a colored character label. Use /snd text to set private presentation text returned by an empty /PlayerName lookup. Sending a command without text clears its message.',
      'Samaritan and Critical Armor now display their respective skill icons. Coin Armor is unchanged. Client and server are updated together.',
      'Guild recruitment requires matching kingdoms. Ordinary members and sub-leaders cannot change kingdom independently; a leader change updates member capes, with offline characters synchronized when they enter.',
      'Warrior\'s Seal grants eligible Mortals at level 355 or higher the Elite cape for their kingdom, or the white Adventurer cape without a kingdom. Removing the main weapon promotes an eligible remaining off-hand weapon to the main hand.',
      'This update also delivers the previously published skill descriptions, Character menu and C.C. client work through the cumulative launcher feed. The skill text asset keeps the same 102 descriptions with adjusted line wrapping.',
      'Close the game and open WYDLauncher.exe to receive patch 10031. Automated checks passed; in-game confirmation remains pending. This update does not claim to complete the Nightmare or Backpack gameplay corrections.',
    ],
  },
  {
    slug: 'player-report-fixes-10029', date: '2026-10-15', category: 'News',
    title: 'Patch 10029: player-reported fixes, buff durations and Soul Limit',
    summary: 'Aylin and Cythera +10, earring refinement, buff durations, C.C. rebuff, summons, sub-leader recruitment and invisible costumes and monsters.',
    paragraphs: [
      'Aylin follows the original rules: two identical +9 items, a Light Stone and four gems. Ancient items need their own gem (Diamond, Emerald, Coral or Garnet) and weapons must be Ancient. Aylin now explains why a combination is refused.',
      'Celestials receive a second Cythera when they unlock level 90, so two Cytheras +9 can be composed to +10 at Aylin.',
      'Lac Dust refines the equipped earring from +9 to +10 and from +10 to +11 at 16%; a failed attempt breaks the earring. Gems now change the effect of a +10 or +11 earring.',
      'All support buffs last 5 minutes at mastery 0 up to 10 minutes at mastery 320. Soul Limit lasts 5 minutes for a new Celestial up to 15 minutes at Celestial level 199, and its bonus now raises physical and magic attack, defense, attack speed, evasion, HP and MP.',
      'Samaritan is a self buff. Golden Shield costs 100 gold per mastery point. Enchant Ice and Lightning Storm require a bow. C.C. no longer stops when one buff on the bar cannot be cast.',
      'Summons follow their owner through teleports and stay close when idle. Guild leaders can recruit directly as Sub-leader 1, 2 or 3; sub-leader marks have a silver border.',
      'The female Skeleton Set, the KR Carnival Mount and the Nightmare Mystic swarms are visible again, and oversized monsters have a normal size.',
      'Close the game and open WYDLauncher.exe to receive patch 10029. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'inventory-lighting-costumes-and-equipment', date: '2026-10-15', category: 'News',
    title: 'Patch 10027: inventory lighting, costumes and equipment',
    summary: 'A follow-up for dull item icons, invisible ShopCostum06 costumes, monster reentry and weapon poses.',
    paragraphs: [
      'Item icons now use the native fixed inventory lights, white ambient and transparency cutoffs instead of map lighting. This addresses the additional dull-icon defect reported after patch 10025, including unrefined Glihtning Stone. Refinement overlays also use the native grade-to-texture lookup.',
      'Eight imported ShopCostum06 costumes receive explicit renderers and 24 missing mesh files. Changing from two weapons to one refreshes the remaining weapon pose.',
      'A new appearance packet replaces a retired remote actor instead of keeping its pending-deletion state. This fixes one monster reentry defect; it does not establish that every reported Carta disappearance has the same cause.',
      'Close the game and open WYDLauncher.exe to receive patch 10027. Automated checks passed; in-game confirmation remains pending. This is a client-only publication: no game-server update or restart, and the pending Ehre and private-instance HUD server fixes are not included.',
    ],
  },
  {
    slug: 'guild-marks-and-cc-update', date: '2026-10-15', category: 'News',
    title: 'Patch 10026: guild marks, C.C. rebuff and server fixes',
    summary: 'Guild leaders can upload a guild mark, C.C. renews buffs before they expire, and several monsters and test shops are fixed.',
    paragraphs: [
      'Guild marks: the guild leader opens the player panel, section Guild, and sends a ready-to-use 16x12, 24-bit BMP. Other images (PNG, JPEG, GIF or BMP up to 64 KB) are converted automatically. The mark appears next to the name of every guild member in the game, including other players on screen.',
      'C.C. now renews your buffs shortly before they expire, so the character is never left unbuffed. Support buffs last as long as in the original game: longer with higher mastery.',
      'Basilisks, Lizards, Grub and Liggle swarms and other monsters that appeared giant now have their normal size.',
      'Magic Chamber (Carta) rooms and the Nightmare spawn their monsters in the original groups and places instead of piling up around the party. Some Carta and Nightmare monsters that were wrongly spawning in the open world near Noatum were removed.',
      'The training field is for mortals below level 35: reaching level 35 there returns the character to the city, and the field cannot be entered again.',
      'New test NPCs in Armia: the mortal D and E weapon shops and the D and E armor set shops.',
      'Close the game and open WYDLauncher.exe to receive patch 10026. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'shining-item-icons', date: '2026-10-14', category: 'News',
    title: 'Patch 10025: shining item icons',
    summary: 'Item icons keep their shine again, such as the Glittering Stone and the gems.',
    paragraphs: [
      'Inventory, equipment, shop and trade icons are drawn with the original transparency and shine again. Shining items such as the Glittering Stone no longer look flat and opaque.',
      'Close the game and open WYDLauncher.exe to receive patch 10025. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'mounts-interface-and-equipment-update', date: '2026-10-14', category: 'News',
    title: 'Patch 10024: mounts, interface and equipment fixes',
    summary: 'Mount magic and level glow, native trumpet chat, equipment icons and party controls receive corrections.',
    paragraphs: [
      'Living adult mounts now increase spell damage through magic amplification. Mount glow uses the native level textures, including appearances received by other players.',
      'Trumpets use green chat with the player name; server notices remain at the top. Party Leave, Escape menu closing, Mystic Amulet equipment and fairy, costume and mount icon sizing are corrected.',
      'Normal Lac Dust can refine +9 earrings to +10 at 16% without destroying the earring on failure. Other +9 equipment still requires Aylin. Noatum event towers are excluded from ordinary spawns, and the Erion tower position is corrected; this does not implement the complete Noatum war.',
      'Armia test weapon shops offer 49 mortal D/E weapons with +28 magic or +63 physical damage. Close the game and open WYDLauncher.exe to receive patch 10024. Automated checks passed; in-game confirmation remains pending. The item-drop crash remains unresolved.',
    ],
  },
  {
    slug: 'aylin-native-glow-follow-up', date: '2026-10-14', category: 'News',
    title: 'Patch 10023: native Aylin previews and gem glow follow-up',
    summary: 'Native preview combinations and Diamond/Garnet glow mappings receive additional corrections.',
    paragraphs: [
      'The four Aylin previews use the native additional-effect combinations without changing the staged ingredients. Removing ingredients or closing the panel clears the temporary previews.',
      'Diamond +10 is included in the UI glow selection, Garnet uses its native absorption texture, and imported weapon models 2889–2901 use the refined gem variant instead of falling back to Diamond. Models 2906–2909 retain their existing fixed glow.',
      'Close the game and open WYDLauncher.exe to receive patch 10023. Automated checks passed; in-game confirmation remains pending. The separately reported item-drop crash is still under investigation and is not fixed by this patch.',
    ],
  },
  {
    slug: 'aylin-preview-refine-glow', date: '2026-10-14', category: 'News',
    title: 'Patch 10022: Aylin previews, +10 glow and Lac +11',
    summary: 'Aylin shows the four possible +10 results again, and every +10 gem has its own glow.',
    paragraphs: [
      'Aylin shows the four possible results again as soon as a refinement gem is placed: the two +9 items combined with each second additional option, already marked with the gem variant (Diamond drop, Emerald piercing, Coral EXP, Garnet absorption). The previews update when an item is added or removed.',
      'This initial refinement-glow update is followed by patch 10023, which corrects the native gem texture mapping and the affected imported weapon models. In-game confirmation remains pending.',
      'Lac Dust can now refine a +10 item to +11 with a 5% chance, keeping its gem effect. +9 to +10 remains possible only through Aylin, and Lac does not go beyond +11.',
      'Close the game and open WYDLauncher.exe to receive patch 10022. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'dye-trade-chat-fixes', date: '2026-10-14', category: 'News',
    title: 'Patch 10021: dye, trade and chat fixes',
    summary: 'Dye paints only armor set pieces, trade slots keep their background, and @ chat reaches the whole channel.',
    paragraphs: [
      'Dye paints only the five armor set pieces: helmet, chest, pants, gloves and boots. Weapons, shields, capes, mounts and accessories are refused and the dye is kept. The enamel remover still cleans any already dyed item.',
      'Items placed in either side of the trade window are drawn over the normal slot background instead of a black square.',
      'Chat: @ text reaches everyone on the channel, and @@ text reaches every character with the same citizenship. Sending @@ without citizenship now shows a notice. Creating an Arch starts the new character without citizenship, so get it again from Kibita.',
      'New test NPCs next to the other test shops in Armia: ShopArchSet sells the four Arch armor sets, ShopArchWpn the Thron weapons and the Hoplon shield, and ShopAmulet the earrings, Mystic Amulets with +45 damage and +16 magic, Spirit Stones and Ankhs. ShopTest2 now sells Ori and Lac in stacks of 120.',
      'Close the game and open WYDLauncher.exe to receive patch 10021. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'kefra-uxmal-passage-fix', date: '2026-10-14', category: 'News',
    title: 'Patch 10020: Kefra passage activation',
    summary: 'The hall mirror now opens the Uxmal confirmation after Kefra is defeated.',
    paragraphs: [
      'The client now recognizes the hall mirror activation area. After Kefra is defeated, stepping onto it opens the Uxmal confirmation with the existing 700 gold fee. The server still validates passage access.',
      'While Kefra remains defeated, all players receive +100% combat EXP. This bonus survives restart and ends at the committed weekly reset; quest rewards are unchanged.',
      'Kefra temporarily has 1 HP for testing. This does not reset an existing defeat or force another spawn, and an evaded attack still deals no damage.',
      'Close the game and open WYDLauncher.exe to receive patch 10020. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'combat-capes-kefra-update', date: '2026-10-14', category: 'News',
    title: 'Patch 10019: combat, capes and Kefra',
    summary: 'Attack-speed calculation, cape refinement glow, Kefra movement and portal admission fixes.',
    paragraphs: [
      'The server enforces the 900 ms attack interval and calculates double-hit chance from the displayed attack speed. At 110% attack speed, double hits do not occur.',
      'Cape glow follows cape refinement. The open Kefra doorway restores its original floor height, and the hall mirror appears only after Kefra is defeated.',
      'Survivor exchanges one Locked Seal for 50 visits. The immediate teleport uses the first visit, leaving 49. The ground portal requires an existing visit balance and consumes one visit per successful entry.',
      'Close the game and open WYDLauncher.exe to receive patch 10019. Automated checks passed; in-game confirmation remains pending.',
    ],
  },
  {
    slug: 'fps-command-update', date: '2026-10-14', category: 'News',
    title: 'Client update: /fps command',
    summary: 'Patch 10018 adds the /fps command to show your frame rate and ping under the minimap.',
    paragraphs: [
      'Type /fps in the chat to show your frame rate and ping under the minimap coordinates, and type it again to hide them. The display starts hidden every session.',
      'Ping is the connection round-trip time measured by Windows; it shows -- when Windows does not report it.',
      'Close the game and open WYDLauncher.exe to receive patch 10018.',
    ],
  },
  {
    slug: 'transformation-citizenship-fix', date: '2026-10-14', category: 'News',
    title: 'Server fix: transformations and citizenship',
    summary: 'Buying citizenship or changing kingdom while transformed keeps your transformation visible.',
    paragraphs: [
      'A BeastMaster who bought citizenship from Kibita, or changed kingdom, while transformed lost the transformed appearance although the buff remained. The server now restores the transformation right after the update.',
      'This fix is on the server only; no client update is needed beyond patch 10017.',
    ],
  },
  {
    slug: 'transformations-and-armia-update', date: '2026-10-14', category: 'News',
    title: 'Client update: transformations, minimap and Armia',
    summary: 'Patch 10017 restores BeastMaster transformation animations, adds the minimap header and fixes the Armia birth point.',
    paragraphs: [
      'BeastMaster Wolf and Bear forms now walk, run and attack with their own animations, and every transformation glows brighter as your Nature mastery and level grow.',
      'The minimap header shows your server and channel again, with the coordinates in the original position.',
      'New characters and Archs are born in Armia, around 2091,2100. Characters that an old template had left bound to Nippleheim were returned to Armia; use the city teleporter if you prefer another hometown.',
      'Kingdom guards no longer attack players who wear their own kingdom cape.',
      'Close the game and open WYDLauncher.exe to receive patch 10017.',
    ],
  },
  {
    slug: 'costumes-and-refinement-update', date: '2026-10-14', category: 'News',
    title: 'Client update: costumes and refinement rules',
    summary: 'Patch 10016 adds eight costumes, fixes untextured costumes and blocks refining costumes and fairies.',
    paragraphs: [
      'Costumes and fairies are no longer refinable. Using Ori, Lac or Odin refinement on them is refused and the refinement item is kept.',
      'Eight more costumes are available, including the Female Wizard, Son of the Dragon, Santa and Rudolph sets. They are sold by the new ShopCostum06, next to the other costume merchants in Armia.',
      'Eleven imported costumes that appeared without texture now display correctly.',
      'Close the game and open WYDLauncher.exe to receive patch 10016.',
    ],
  },
  {
    slug: 'kr-mounts-update', date: '2026-10-14', category: 'News',
    title: 'Client update: 40 new KR mounts',
    summary: 'Patch 10015 adds 40 KR mounts, a new mount merchant and correct rider placement.',
    paragraphs: [
      'Forty more mounts from the Korean client are now available, including griffins, unicorns, dragons, tigers, Fenrirs, Smilodons, Kongkongi variants, the Pet Duck and the Immortal Courser.',
      'They are sold by the KR mount merchants in Armia; the new ShopKRMt04 stands next to the others.',
      'Riders now sit at the original height on every mount. Several existing KR mounts, such as the Iceland Wolf, the premium Dragons and Spiders, BearBear and the Jackals, now show their own models instead of a borrowed one.',
      'Close the game and open WYDLauncher.exe to receive patch 10015. It updates both the game and the item list.',
    ],
  },
  {
    slug: 'critical-and-chat-update', date: '2026-10-14', category: 'News',
    title: 'Client update: critical chance and chat',
    summary: 'Patches 10013 and 10014 make critical chance match item tooltips and fix the enlarged chat scroll bar.',
    paragraphs: [
      'Critical chance now uses the same scale everywhere. An item that shows 9.6% critical adds exactly 9.6% to your character, the Character window shows your real chance, and the maximum is 100%. Skill and buff critical bonuses keep the values they already showed.',
      'Enlarging the chat with + now stretches the whole scroll bar and moves its down arrow to the new bottom.',
      'If the game closes unexpectedly, the client now records a complete crash report next to WYD.exe. Send it with your bug report.',
      'Close the game and open WYDLauncher.exe to receive patches 10013 and 10014. The new critical scale requires patch 10013; older clients show the wrong value.',
    ],
  },
  {
    slug: 'capes-and-arch-update', date: '2026-10-14', category: 'News',
    title: 'Kingdom capes, guild names and Arch creation',
    summary: 'Patches 10011 and 10012 restore cape symbols, the kingdom icon and guild names; new Arch characters start without equipment.',
    paragraphs: [
      'Kingdom capes show their symbol for you and for other players, and the Character window shows your kingdom icon and guild name again.',
      'Every kingdom cape owner receives a persistent symbol that stays through promotion, re-equipping and relogging. Citizenship and its gameplay rules are unchanged.',
      'New Arch characters start without equipment or ordinary inventory items, as in the original game. Skill book tooltips show the mastery your class actually needs.',
      'Close the game and open WYDLauncher.exe to receive patches 10011 and 10012.',
    ],
  },
  {
    slug: 'world-and-interface-fixes', date: '2026-10-13', category: 'News',
    title: 'World and interface fixes',
    summary: 'Patches 10005 to 10010 fix icons, the minimap, map borders, map objects and buff visuals.',
    paragraphs: [
      'Costumes and KR mounts have their proper icons, KR mounts can be equipped by every class, and LavaGolem and LavaGrub are visible again.',
      'The minimap uses the original orientation, so doors and entrances match what you see. Chat and list scroll bars use the original 7.48 artwork.',
      'Invisible walls were removed from the city-war arena corridor and from the borders of the newer map regions. Gates, towers and obelisks appear after a teleport, and the Noatum castle gate starts open.',
      'Nightmare instances open correctly, and every timed area, including the Game Room, shows its remaining time. Persistent buff effects use the original sizes and timing.',
      'Close the game and open WYDLauncher.exe to receive every pending patch. Let the launcher finish before pressing Start.',
    ],
  },
  {
    slug: 'account-protection', date: '2026-10-12', category: 'Announcement',
    title: 'Stronger protection for your account',
    summary: 'Human verification now protects registration and website sign-in.',
    paragraphs: [
      'Registration and player or staff sign-in require human verification. Request limits help protect the service against repeated automated attempts.',
      'If verification expires, complete the new challenge and try again. Keep your game password and staff PIN private; never include them in a bug report.',
      'A website session is separate from your game connection. Signing out of the website does not disconnect your character.',
    ],
  },
  {
    slug: 'weather-update', date: '2026-10-12', category: 'News',
    title: 'Client update: weather effects',
    summary: 'Patch 10004 separates rain and snow activation and clears particles when weather ends.',
    paragraphs: [
      'The latest client update corrects the weather effect lifecycle, including transitions between regions and the snow sound layers.',
      'Close the game and open WYDLauncher.exe to receive patch 10004. Let the launcher finish before pressing Start; do not change the version in config.txt manually.',
      'Automated checks and publication have been verified. If you still see overlapping effects, record the region and what changed immediately before the problem.',
    ],
  },
] as const;

export const changelog = [
  { date: '2026-10-18', category: 'Client', title: 'Patch 10035 — client stability safeguards', entries: [
    'Validate packet/text boundaries and minimap uploads; clean UI draw lists after aborted frames.',
    'Serialize nested network notifications and clear nested effect-owner references during final actor removal.',
    'Bound skin-mesh payloads, bone palettes, positions and indices; roll back failed GPU buffer creation and uploads.',
    'Automated checks passed. Historical crash root causes and scenario-specific gameplay acceptance remain pending.',
  ] },
  { date: '2026-10-17', category: 'Client and server', title: 'Patch 10034 — Hell Gate routes and defense', entries: [
    'Restore corridor, room, Lich and Dionys-defense progression with private encounter portals and persisted lifecycle state.',
    'Use a twenty-minute total duration, two-minute entry, 400,000 HP bosses and a 10,000,000 completion EXP reward.',
    'Add the generic client transport prompt for internal event routes. Automated checks passed; gameplay acceptance remains pending.',
  ] },
  { date: '2026-10-17', category: 'Server', title: 'Instance party and reconnect recovery', entries: [
    'Synchronize group Nightmare and Magic Chamber relocation while keeping individual tickets individual.',
    'Restore the current Nightmare timer, protected-NPC count and available original party membership on reconnect.',
    'Keep Compounder passive and city guards hostile only to monsters; add dedicated Hell Gate generators without claiming full event parity.',
    'Launcher patch remains 10033. Automated checks passed; gameplay confirmation remains pending.',
  ] },
  { date: '2026-10-17', category: 'Client and server', title: 'Patch 10033 — combat, controls and balances', entries: [
    'Guarantee double hits above 200% attack speed; align the two floating damage trajectories and preserve the 900 ms action interval.',
    'Continue manual physical attacks, select skills with 1-0 and assign with Shift+1-0, and correct Helper scrollbar placement and chat scrolling.',
    'Add private single-notice /balance and /saldo reports, with medals separated into /medals and /medalhas.',
    'Automated checks passed. In-game confirmation remains pending.',
  ] },
  { date: '2026-10-16', category: 'Server', title: 'Dedicated instance leveling monsters', entries: [
    'Exclusive Nightmare, Water and Magic Chamber monsters with tier-based HP and base EXP, one defense and incoming damage capped at one.',
    'No monster gold, native drops, event drops or special boss loot; existing progression and completion rewards remain unchanged.',
    'Client patch remains 10032. Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-16', category: 'Client and server', title: 'Patch 10032 — Nightmare lifecycle', entries: [
    'Separate four-minute admission from the shared 15-minute combat period and preserve the absolute deadline across restoration.',
    'Show protected NPC survivors, apply a 10% original EXP penalty per lost protector and continuously replace common monsters until expiry.',
    'Spawn Arcane Lugefer once with five minutes remaining while a protector survives; persist its defeat with player rewards.',
    'Automated checks passed. In-game confirmation remains pending; this update does not certify complete native population or loot parity.',
  ] },
  { date: '2026-10-16', category: 'Client and server', title: 'Patch 10031 — chat labels and buff icons', entries: [
    'Restore /tab and /snd dispatch and add nine coordinated tab colors without respawning the character.',
    'Give Samaritan and Critical Armor their respective skill icons while preserving canonical server effects.',
    'Deploy guild kingdom recruitment/change rules, eligible Warrior\'s Seal Elite capes and remaining off-hand weapon promotion.',
    'Include the adjusted skill-description wrapping; previous client updates remain available through the cumulative launcher feed.',
    'Automated checks passed. In-game confirmation remains pending; Nightmare and Backpack gameplay are not completed by this batch.',
  ] },
  { date: '2026-10-15', category: 'Client and server', title: 'Patch 10029 — player-reported fixes', entries: [
    'Aylin uses the W2PP grade/gem rules and requires Ancient weapons; the level-90 Celestial unlock grants the second Cythera.',
    'Lac refines the slot-8 earring +9→+10 and +10→+11 at 16% and breaks it on failure; gems apply to +10/+11 earrings.',
    'Support buffs last 5–10 minutes by mastery; Soul Limit 5–15 minutes by Celestial level and feeds derived stats.',
    'Samaritan is self-only; Golden Shield costs 100 gold per mastery; Enchant Ice and Lightning Storm need a bow; C.C. skips buffs that do not cast.',
    'Summons follow teleports and stay near idle owners; InviteType 1–3 recruits sub-leaders; sub-leader guild marks keep the silver border.',
    'Female Skeleton Set meshes restored, KR Carnival Mount visible, 64 monster templates use Con 1.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-15', category: 'Client', title: 'Patch 10027 — inventory lighting, costumes and equipment', entries: [
    'Restore native fixed inventory lighting, white ambient, transparency cutoffs and the refinement texture lookup.',
    'Add renderers and 24 missing meshes for eight ShopCostum06 costumes; refresh the remaining weapon pose after removing a second weapon.',
    'Replace retired remote actors when a new appearance arrives, fixing a concrete visibility-reentry lifecycle defect.',
    'Client-only publication without a game-server update or restart. Ehre and private-instance HUD server corrections remain pending. Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-15', category: 'Client and server', title: 'Patch 10026 — guild marks, C.C. rebuff and server fixes', entries: [
    'Guild leaders upload the guild mark on the player panel; the client shows it next to own and visible characters.',
    'C.C. renews buffs before they expire; support buffs use the native mastery-based duration.',
    'Monsters with a native negative Constitution (Basilisk, Lizard, swarms) no longer appear giant.',
    'Magic Chamber and Nightmare waves spawn from dedicated generator files at their native positions; stray open-world spawns were removed.',
    'The training field returns and blocks characters from level 35; mortal D/E weapon and armor set test shops are placed in Armia.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client', title: 'Patch 10025 — shining item icons', entries: [
    '3D item icons keep the native alpha blending and alpha test, so shining items such as the Glittering Stone are no longer drawn opaque.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10024 — mounts, interface and equipment fixes', entries: [
    'Mount magic amplification affects spells, and mount level glow uses native textures for owner and observer appearances.',
    'Native green player trumpet chat is separate from server top notices. Party Leave, Escape menus, Mystic Amulet equipment and shared equipment icon fitting are corrected.',
    'Normal Lac Dust can refine +9 earrings to +10 at 16%; Noatum ordinary tower spawning is blocked and the Erion tower anchor is corrected.',
    'Armia mortal D/E test shops offer 49 weapons with +28 magic or +63 physical damage.',
    'Automated checks passed; in-game confirmation remains pending. The item-drop crash is not fixed by this patch.',
  ] },
  { date: '2026-10-14', category: 'Client', title: 'Patch 10023 — native Aylin previews and gem glow follow-up', entries: [
    'Aylin previews preserve native additional-effect combinations and clear temporary results when ingredients are removed.',
    'Diamond +10 UI glow, native Garnet texture and refined variants on imported weapons receive additional corrections.',
    'The reported item-drop crash is not fixed by this patch. Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client', title: 'Patch 10022 — Aylin previews, +10 glow and Lac +11', entries: [
    'Aylin shows the four native +10 result previews once a refinement gem is staged.',
    '+10 Diamond, Emerald, Coral and Garnet glows use the native textures; Garnet no longer borrows another variant.',
    'Lac Dust refines +10 to +11 at 5% and keeps the gem variant; +9 to +10 stays exclusive to Aylin.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10021 — dye, trade and chat fixes', entries: [
    'Dye paints only armor set pieces (helmet, chest, pants, gloves, boots); other items are refused without consuming the dye.',
    'Occupied trade slots keep the normal slot background instead of a black square.',
    '@ chat reaches the whole channel; @@ chat reaches the same citizenship and warns a sender without one.',
    'New Armia test NPCs: ShopArchSet (Arch armor sets), ShopArchWpn (Thron weapons and Hoplon shield) and ShopAmulet (earrings, Mystic Amulets +45 damage/+16 magic, Spirit Stones, Ankhs); ShopTest2 sells Ori and Lac in stacks of 120.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10020 — Kefra passage activation', entries: [
    'The hall mirror opens the Uxmal confirmation after Kefra is defeated; the existing 700 gold fee and server-side admission remain unchanged.',
    'All players receive +100% combat EXP while the durable Kefra defeat flag is set. Quest rewards are unchanged.',
    'Kefra temporarily has 1 HP for testing, without resetting the weekly state or forcing a spawn.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10019 — combat, capes and Kefra', entries: [
    'Attack actions respect the 900 ms interval; 110% displayed attack speed has no double-hit chance.',
    'Cape refinement controls its glow, including appearance updates and unequipping.',
    'The open Kefra doorway retains its original floor height, and the hall mirror follows the durable boss-defeated state.',
    'Survivor grants 50 visits per Locked Seal, including the immediate first entry. The ground portal blocks zero balances and saves the visit and gold debit before teleporting.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client', title: 'Patch 10018 — /fps command', entries: [
    'Type /fps in the chat to show or hide your frame rate and ping under the minimap coordinates.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Server', title: 'Transformation kept after citizenship', entries: [
    'Buying citizenship from Kibita or changing kingdom while transformed no longer removes the transformation appearance while its buff is still active.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10017 — transformations, minimap and Armia', entries: [
    'BeastMaster Wolf and Bear forms walk, run and attack with their animations instead of sliding.',
    'Transformations glow brighter as Nature mastery and level grow.',
    'The minimap header shows the server and channel, and the coordinates use the original layout.',
    'Characters are born in Armia around 2091,2100; new Archs no longer appear in Nippleheim, and characters left bound to Nippleheim by the old template now return to Armia.',
    'Kingdom guards no longer attack players wearing their own kingdom cape.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10016 — costumes and refinement rules', entries: [
    'Costumes and fairies can no longer be refined with Ori, Lac or Odin refinement.',
    'Eight more costumes: Female Wizard, Son of the Dragon, Red and White Santa, Purple and Blue Rudolph and the second Red Santa and Purple Rudolph sets, sold by the new ShopCostum06 in Armia.',
    'Eleven imported costumes that appeared without texture now load their correct textures.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10015 — new KR mounts and rider placement', entries: [
    '40 more KR mounts: Pet Duck, Kongkongi variants, Heidrun, Elk, tigers, griffins, unicorns, horses, Smilodons, Fenrirs, dragons, White Tamed Bear, Jackals and the Immortal Courser.',
    'A new mount merchant, ShopKRMt04, stands next to the other KR mount merchants in Armia.',
    'Riders sit at the original height on every mount instead of sinking into imported KR mounts.',
    'KR Iceland Wolf, BearBear, Jackals, Dragons, Spiders, Loki, Heidrun, 20th Horse, Fimbulvetr Messenger, Deer and Barbarian Horse/Boar now show their own models.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client', title: 'Patch 10014 — enlarged chat scroll bar', entries: [
    'Enlarging the chat with + now stretches the scroll bar track and moves its down arrow to the new bottom.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10013 — critical chance scale', entries: [
    'Critical now uses the item tooltip scale: an item showing 9.6% adds exactly 9.6% to the character.',
    'The Character panel shows the real server chance, up to 100%; skill and buff bonuses keep their displayed values.',
    'Crash reports now record a complete dump and module offsets.',
    'Automated checks passed; in-game confirmation remains pending.',
  ] },
  { date: '2026-10-13', category: 'Client and server', title: 'Patch 10012 — cape and character presentation', entries: [
    'Register native cape emblem textures and restore the kingdom icon for emblem variants.',
    'Display guild names from the launcher guild directory and use class-specific skill mastery requirements.',
    'Kingdom cape owners receive a persistent random symbol independently of their citizenship.',
    'Automated checks passed; launcher extraction and in-game confirmation remain pending.',
  ] },
  { date: '2026-10-14', category: 'Client and server', title: 'Patch 10011 — citizenship cape emblems', entries: [
    'Citizenship cape emblems update in character selection and the world after a confirmed purchase.',
    'New Arch characters start without equipment or ordinary inventory items; reserved slots are preserved.',
    'Automated checks passed; launcher extraction and in-game confirmation remain pending.',
  ] },
  { date: '2026-10-14', category: 'Client', title: 'Patch 10010 — map objects after teleport', entries: [
    'Gates, towers and obelisks received just before entering a region are placed on the terrain once it loads.',
    'The Noatum castle gate starts open; only keyed dungeon gates start locked.',
    'In-game confirmation remains pending.',
  ] },
  { date: '2026-10-14', category: 'Client', title: 'Patch 10009 — new map borders', entries: [
    'Map regions imported from the newer terrain no longer have invisible walls along their borders.',
    'In-game confirmation remains pending.',
  ] },
  { date: '2026-10-13', category: 'Client', title: 'Patch 10008 — city-war arena corridor', entries: [
    'The central corridor of the city-war arena is walkable again, as in the original 7.48 map.',
    'This patch only updates the terrain attribute map; the game executable is unchanged.',
  ] },
  { date: '2026-10-13', category: 'Client', title: 'Patch 10007 — minimap and scroll bars', entries: [
    'The minimap uses the native 7.48 orientation, so doors and entrances match the 3D view.',
    'Chat and list scroll bars use the native 7.48 arrows, thumb and track.',
    'In-game confirmation remains pending.',
  ] },
  { date: '2026-10-13', category: 'Client', title: 'Patch 10006 — costume and mount icons', entries: [
    'Costumes use a costume icon instead of a sword, and KR mounts use the horse icon.',
    'KR mounts can be equipped by every class; mount tooltips no longer show a speed increase.',
    'LavaGolem and LavaGrub use the Aquagolem and Purple Worm models. In-game confirmation remains pending.',
  ] },
  { date: '2026-10-13', category: 'Client', title: 'Patch 10005 — buff visuals', entries: [
    'Restore the native Skill Amp ground aura scale and height.',
    'Correct persistent buff scales, particle timing, weapon emissions and Slow Slash tint against recorded native 7.48 parameters.',
    'Build and automated checks passed; in-game visual comparison remains pending.',
  ] },
  { date: '2026-10-13', category: 'Website', title: 'Database and account tools', entries: [
    'Search item and skill tables, inspect base values and follow skill-book links.',
    'Filter the level ranking and manage active website sessions from your account.',
    'Getting-started guides, news, roadmap and download verification details.',
  ] },
  { date: '2026-10-12', category: 'Security', title: 'Website authentication', entries: [
    'Human verification for account creation, player login and staff login.',
    'Persistent request limits and stricter account-creation authorization.',
  ] },
  { date: '2026-10-12', category: 'Client', title: 'Patch 10004 — weather', entries: [
    'Rain and snow activation is mutually exclusive; clear weather disables particles.',
    'Regional transitions retain the last server weather; snow layers no longer compete for sound.',
  ] },
  { date: '2026-10-12', category: 'Client', title: 'Patch 10003 — quest timers and timed items', entries: [
    'Quest-zone countdown display and timed-item presentation updates.',
    'Required costume and sound dependencies included in the update package.',
  ] },
] as const;

export const roadmap = [
  { state: 'Available', title: 'Accounts and player overview', detail: 'Register, sign in, view your saved characters and manage active website sessions.' },
  { state: 'Available', title: 'Downloads and level ranking', detail: 'Client download, launcher updates and a ranking from saved character snapshots.' },
  { state: 'Available', title: 'Guides and release information', detail: 'Getting started, troubleshooting, news, changelog and a versioned download receipt.' },
  { state: 'Testing', title: 'Gameplay and client parity', detail: 'Compatibility and visual fixes remain under testing. A build or automated test is not proof of every in-game flow.' },
  { state: 'Available', title: 'Item, skill and quest database', detail: 'Search server tables, inspect requirements and rewards, and follow related items.' },
  { state: 'Planned', title: 'Expanded game database', detail: 'Drops, crafting and full NPC relationships need validated public projections.' },
  { state: 'Planned', title: 'Events and world information', detail: 'Public schedules, boss information and guild projections need reliable server-owned data.' },
  { state: 'Planned', title: 'Password recovery and account security', detail: 'Verified recovery channels and a password-change flow that cannot be overwritten by game saves.' },
  { state: 'Planned', title: 'Guild tools and support', detail: 'Account-owned support requests and guild operations with explicit permissions.' },
  { state: 'Planned', title: 'Rewards, donations and marketplace', detail: 'Durable delivery, reconciliation and item custody are required before transactions can be enabled.' },
] as const;

export const playPages = [
  {
    slug: 'getting-started', title: 'Getting started', summary: 'From a new account to your first character.',
    sections: [
      { title: '1. Create your account', paragraphs: ['Register on this website and complete human verification. Your account name contains 4–12 letters or digits; new passwords contain 8–10 printable ASCII characters without spaces. The website and game use the same credentials.'] },
      { title: '2. Install the client', paragraphs: ['Download the full client and extract the entire archive to a folder you can write to. Keep the assets and executables together. An incremental patch is not a complete installation.'] },
      { title: '3. Update and launch', paragraphs: ['Open WYDLauncher.exe, wait for updates and press Start. Use Settings for resolution and window mode. Do not launch an old project.exe from another installation.'] },
      { title: '4. Enter the world', paragraphs: ['Sign in, create a character and enter the game. Your account panel and ranking show saved snapshots, so they may lag behind your latest actions.'] },
    ],
  },
  {
    slug: 'server', title: 'Server information', summary: 'What this test environment currently provides.',
    sections: [
      { title: 'WYD 7.48', paragraphs: ['The project pairs a 7.48-compatible Windows client with an authoritative Go game server. Character state and economy changes are validated by the server.'] },
      { title: 'Accounts and progression', paragraphs: ['Each game account has up to four character slots. The character panel displays the saved class, level and evolution. Mortal, Arch, Celestial and Subcelestial are represented in the current ranking.'] },
      { title: 'Testing status', paragraphs: ['Client compatibility and gameplay are still being validated. Availability of a feature does not mean every interaction has completed in-game testing. Follow the changelog for concrete changes.'] },
      { title: 'Rates and schedules', paragraphs: ['This page does not publish unverified rates, level caps or event schedules. Check in-game information until those values have an authoritative public feed.'] },
    ],
  },
  {
    slug: 'classes', title: 'Classes', summary: 'Four character classes are available at creation.',
    sections: [
      { title: 'TransKnight', paragraphs: ['A combat-focused class. Inspect the in-game skill descriptions and equipment requirements before choosing a build.'] },
      { title: 'Foema', paragraphs: ['A magic-focused class with Holy Magic, Black Magic and Special Magic skill groups. Exact costs and effects are shown by the game.'] },
      { title: 'BeastMaster', paragraphs: ['A class built around its distinct combat and transformation skills. Use the in-game skill tree as the source for your current build.'] },
      { title: 'Huntress', paragraphs: ['Shown as Hunter in some client screens. Its skill groups include Weapon Mastery, Survival, Trade and Capture.'] },
      { title: 'Build guidance', paragraphs: ['The Database lists base skill costs from the current server tables. Skill-specific rules and character bonuses may change actual costs and effects; use the in-game descriptions for your build.'] },
    ],
  },
  {
    slug: 'rules', title: 'Account and testing guidelines', summary: 'Keep your account safe and make bug reports useful.',
    sections: [
      { title: 'Protect your credentials', paragraphs: ['Do not share passwords, verification tokens or staff PINs. Use a password you do not use on another service. A screenshot of an error should never contain your credentials.'] },
      { title: 'Use the official download', paragraphs: ['Use the download linked from this website and update through the included launcher. Do not mix files from unrelated clients or bypass updates by editing the version counter.'] },
      { title: 'Report problems responsibly', paragraphs: ['Record the time, character, region and steps that caused a problem. Report duplication or account-access vulnerabilities privately to the server operator, without exploiting them or publishing other players’ data.'] },
      { title: 'Testing is still in progress', paragraphs: ['The roadmap distinguishes available features from planned work. These guidelines do not announce new penalties, a wipe policy or guarantees about future progression.'] },
    ],
  },
  {
    slug: 'faq', title: 'Frequently asked questions', summary: 'Installation, updates, sign-in and saved characters.',
    sections: [
      { title: 'Which executable should I open?', paragraphs: ['Open WYDLauncher.exe. It updates the client and starts WYD.exe. An older project.exe may not include current fixes.'] },
      { title: 'Human verification is unavailable', paragraphs: ['Reload the page and allow the Cloudflare verification challenge for this website. If a privacy extension blocks it, allow the challenge on this site only. Do not disable your browser security globally. If it still fails, try again later.'] },
      { title: 'I reached the registration limit', paragraphs: ['The short registration quota is five attempts per hour per network; other protective limits also apply. People sharing an internet connection may share that quota. Wait before trying again; changing account names does not reset it.'] },
      { title: 'The client reports missing files or cannot initialize', paragraphs: ['Re-extract the complete official package into a new writable folder, then run the launcher. Preserve your old folder until the new installation works. Do not download individual DLLs from unknown websites or turn off antivirus protection.'] },
      { title: 'My level or character is not updated on the website', paragraphs: ['The account panel and ranking read saved data, not a live view of your character. The panel shows the last persisted timestamp. The ranking also has a short cache and is ordered by evolution, level and experience.'] },
      { title: 'Does website logout disconnect the game?', paragraphs: ['No. Website sessions and game connections are separate. Logging out of all website sessions also does not log you out of the staff panel.'] },
      { title: 'How do I change or recover a password?', paragraphs: ['Self-service password change and recovery are not available yet. Do not send your current password or use a third-party recovery page.'] },
    ],
  },
] as const;
