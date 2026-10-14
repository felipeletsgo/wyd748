export type PublicGuild = { id: number; name: string };
const maximumBytes = 131072;
const pageSize = 20;

// Reuse the existing public client export, not private guild/member records.
// This deployment has one group and channel (0, 0); IDs occupy 12 bits.
export function parseGuildDirectory(source: string): PublicGuild[] {
  if (source.length > maximumBytes) throw new Error('Invalid guild directory');
  const ids = new Set<number>();
  const names = new Set<string>();
  const rows: PublicGuild[] = [];
  for (const line of source.split(/\r?\n/)) {
    if (!line.trim()) continue;
    const match = /^0[ \t]+0[ \t]+([1-9][0-9]{0,3})[ \t]+([A-Za-z0-9](?:[A-Za-z0-9 ]{0,10}[A-Za-z0-9])?)$/.exec(line);
    if (!match) throw new Error('Invalid guild directory');
    const id = Number(match[1]); const name = match[2]; const key = name.toLowerCase();
    if (id > 4095 || ids.has(id) || names.has(key)) throw new Error('Invalid guild directory');
    ids.add(id); names.add(key); rows.push({ id, name });
  }
  return rows.sort((a, b) => a.name.toLowerCase().localeCompare(b.name.toLowerCase(), 'en'));
}

export function mountGuilds(root: HTMLElement) {
  const body = root.querySelector<HTMLElement>('#guilds-list')!;
  const search = root.querySelector<HTMLInputElement>('#guilds-search')!;
  const status = root.querySelector<HTMLElement>('#guilds-status')!;
  const position = root.querySelector<HTMLElement>('#guilds-page')!;
  const previous = root.querySelector<HTMLButtonElement>('#guilds-previous')!;
  const next = root.querySelector<HTMLButtonElement>('#guilds-next')!;
  const refresh = root.querySelector<HTMLButtonElement>('#guilds-refresh')!;
  let rows: PublicGuild[] = []; let page = 1; let loaded = false; let loading = false;
  function render() {
    const term = search.value.slice(0, 12).trim().toLowerCase();
    const matches = rows.filter(row => row.name.toLowerCase().includes(term));
    const pages = Math.max(1, Math.ceil(matches.length / pageSize));
    page = Math.max(1, Math.min(page, pages));
    const fragment = document.createDocumentFragment();
    for (const guild of matches.slice((page - 1) * pageSize, page * pageSize)) {
      const row = document.createElement('tr'); const cell = document.createElement('td');
      const link = document.createElement('a');
      link.textContent = guild.name; link.href = `/guilds/${encodeURIComponent(guild.name)}/`;
      cell.append(link); row.append(cell); fragment.append(row);
    }
    if (!fragment.childNodes.length) {
      const row = document.createElement('tr'); const cell = document.createElement('td'); cell.className = 'empty-row';
      cell.textContent = loaded ? rows.length ? 'No matching guilds.' : 'No guilds yet.' : 'Guild directory unavailable.';
      row.append(cell); fragment.append(row);
    }
    body.replaceChildren(fragment);
    position.textContent = loaded ? `${page} / ${pages} · ${matches.length} guilds` : '';
    previous.disabled = loading || !loaded || page <= 1;
    next.disabled = loading || !loaded || page >= pages;
  }
  async function load() {
    if (loading) return;
    loading = true; refresh.disabled = true; search.disabled = true;
    previous.disabled = true; next.disabled = true; root.setAttribute('aria-busy', 'true');
    status.textContent = 'Loading…'; status.dataset.error = 'false';
    try {
      const response = await fetch('/patch/guilds.txt', { credentials: 'omit', cache: 'no-store', signal: AbortSignal.timeout(8000) });
      if (!response.ok || Number(response.headers.get('Content-Length') ?? 0) > maximumBytes) throw new Error('Unavailable');
      rows = parseGuildDirectory(await response.text()); loaded = true;
      status.textContent = '';
    } catch {
      rows = []; loaded = false;
      status.textContent = 'Guild directory unavailable. Try Refresh.'; status.dataset.error = 'true';
    } finally {
      loading = false; refresh.disabled = false; search.disabled = !loaded;
      root.setAttribute('aria-busy', 'false'); render();
    }
  }
  search.addEventListener('input', () => { page = 1; render(); });
  previous.addEventListener('click', () => { if (!loading && loaded) { page--; render(); } });
  next.addEventListener('click', () => { if (!loading && loaded) { page++; render(); } });
  refresh.addEventListener('click', () => { void load(); });
  void load();
}
