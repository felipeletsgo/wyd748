// Editorial content is reviewed in Git and published with the static portal.
// Do not fill unknown values with estimates or describe planned systems as live.
export const clientRelease = {
  version: 1,
  clientVersion: '7.48',
  patchVersion: 10041,
  publishedAt: '2026-10-21',
  serverUpdate: { id: 'combat-events-persistence-10041', publishedAt: '2026-10-21' },
  launcherVersion: null,
  minimumLauncherVersion: null,
  fullClient: {
    url: 'https://drive.google.com/file/d/1Kvjl4pOY7FYUsmFsEvbQhivAwhqJRXEj/view?usp=drive_link',
    sizeBytes: null,
    sha256: null,
  },
  patch: {
    url: '/patch/patch-10041.zip',
    sizeBytes: 900489,
    sha256: '4CF0535F04C58362478F4927D50717FD6D1140B4262439CBB357DC8790BB0628',
    executableSha256: '64C9BDDEEC0EE6E6197B63A0CA915B10BE1AEA8F1C6B03A1F9D3EC47428C3F82',
  },
  // This is a website receipt, not a replacement for the legacy launcher feed.
  updateFeed: '/patch/update.htm',
} as const;

// News posts are Markdown files in src/content/news (one per post).
export { news } from './news';

export const changelog = [
  { date: '2026-10-21', category: 'Client and server', title: 'Patch 10041 — combat resources and event safety', entries: [
    'Bound damage and healing calculations, synchronize active large resource pools, and ship the wide item-attribute companion table.',
    'Commit ghost-shop honor atomically; wait for Colosseum entry payment and retry rejected arena exits after event finalization.',
    'Prevent boss shield overflow and duplicate registration; align OX result regions with their status counts.',
    'Publish the audited staff controls and runtime data with the matching game-server release.',
    'Automated checks passed; actual gameplay and unresolved native 7.48 formula validation remain pending.',
  ] },
  { date: '2026-10-20', category: 'Client', title: 'Patch 10040 — camera height correction', entries: [
    'Keep player camera height and mounted minimum distance independent of CON.',
    'Preserve normal mount and seated camera offsets; no game-server restart is required.',
    'Automated checks passed; in-game acceptance with and without the CON amulet remains pending.',
  ] },
  { date: '2026-10-20', category: 'Client and server', title: 'Patch 10039 — player size correction', entries: [
    'Keep player size independent of CON while preserving monster scaling.',
    'Set the four world boss CON values to 5,000 in the active game-server release.',
    'Automated checks passed; actual gameplay acceptance remains pending.',
  ] },
  { date: '2026-10-20', category: 'Client and server', title: 'Patch 10038 — world bosses and 32-bit limits', entries: [
    'Move the four world bosses to their own areas and add four free Armia gate keepers to their entrances.',
    'Show hit numbers above 1,000,000 and full 32-bit heals; keep local HP and item attribute bonuses in 32 bits.',
    'Remove the non-native Magic Chamber floor squares introduced in patch 10037.',
    'Automated checks passed; actual gameplay acceptance remains pending.',
  ] },
  { date: '2026-10-20', category: 'Client and server', title: 'Patch 10037 — skills, items and exchange stones', entries: [
    'Huntress Backpack opens Cargo anywhere; Coin Armor charges its own gold cost; Enchant Frost rejects claws; Lightning Storm strikes as Thunderbolt.',
    'Exchange Stones swap the names of two account characters on login; Adamantite accepts only D sets; ghost shops earn 1 honor every 10 minutes.',
    'Item buffs survive Celestial changes; Glory Ankh +9, Tiny chances, Master cape refinement, eighth skill cost and fourth capture mastery corrected.',
    'Noatun gate local close removed; Magic Chamber entrance runes added; Repliction text and grade-E appearance fixed; Uxmal moved to the Rune Track entrance.',
    'Automated checks passed; actual gameplay acceptance remains pending.',
  ] },
  { date: '2026-10-19', category: 'Client and server', title: 'Patch 10036 — keyboard and inventory shortcuts', entries: [
    'Restore alternate keyboard accelerators, consumable controls and camera-directed Auto Run while preserving text-input priority.',
    'Synchronize public guild affiliation with server acknowledgement; restore validated equipment and warehouse item-click shortcuts.',
    'Ship three English Help assets alongside the client executable; preserve player settings and the launcher executable.',
    'Automated build gates passed; actual gameplay acceptance remains pending.',
  ] },
  { date: '2026-10-19', category: 'Server', title: 'Instance expiry persistence', entries: [
    'Defer instance teardown while a pending kill still reserves a participant; retire room entities only after a confirmed group exit.',
    'Preserve room state on certified exit rejection and retain fail-closed handling for uncertain persistence outcomes.',
    'Keep existing rewards, instance timers and client protocol unchanged. Automated and isolated PostgreSQL checks passed; gameplay acceptance remains pending.',
  ] },
  { date: '2026-10-18', category: 'Server', title: 'Hell Gate and Rune Track', entries: [
    'Implement Rune Track rooms +0 through +6 with dedicated monsters, room objectives, portals and Freyja gold donations.',
    'Draw each eligible player\'s rune independently from the full historical room pool, without party-position restrictions.',
    'Dispatch imported caster skill bars; correct Hell Gate infantry, formation and Dionys sale handling.',
    'Client patch remains 10035. Automated checks passed; actual gameplay acceptance remains pending.',
  ] },
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
