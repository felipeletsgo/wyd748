// Editorial content is reviewed in Git and published with the static portal.
// Do not fill unknown values with estimates or describe planned systems as live.
export const clientRelease = {
  version: 2,
  // Release line 7.48: the launcher's [VERSION] 48001 is shown as 7.48.001.
  clientVersion: '7.48.001',
  feedVersion: 48001,
  publishedAt: '2026-10-23' as string | null,
  launcherVersion: null,
  minimumLauncherVersion: null,
  fullClient: {
    url: 'https://drive.google.com/file/d/1ouCjq-A8JOnHRXpO0m8fEhRm2NuGgmFk/view?usp=drive_link' as string | null,
    // Size and checksum of the uploaded archive are not recorded yet.
    sizeBytes: null as number | null,
    sha256: null as string | null,
  },
  // The update history restarts with this full client; no patch is published yet.
  patch: null,
  // This is a website receipt, not a replacement for the legacy launcher feed.
  updateFeed: '/patch/update.htm',
} as const;

// News posts are Markdown files in src/content/news (one per post).
export { news } from './news';

export const changelog = [
  { date: '2026-10-23', category: 'Client and server', title: 'Client 7.48.001 — new full client', entries: [
    'New full client with game text in English or Brazilian Portuguese, selected in the launcher.',
    'The launcher shows version 7.48.001 and the update history restarts from this client.',
    'Required update: the server accepts only client 7.48.001; older clients must download the new full client.',
    'Automated checks passed; real-client confirmation remains pending.',
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
