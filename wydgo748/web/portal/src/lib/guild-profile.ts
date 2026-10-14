import type { components } from './api.generated';
type Profile = components['schemas']['PublicGuildProfile'];
const guildName = /^[A-Za-z0-9](?:[A-Za-z0-9 ]{0,10}[A-Za-z0-9])?$/;
const roles: Record<number, string> = { 0: 'Member', 6: 'Officer', 7: 'Officer', 8: 'Officer', 9: 'Leader' };
const kingdoms: Record<number, string> = { 0: 'Neutral', 7: 'Hekalotia', 8: 'Akelonia' };
const maximumBytes = 8388608;

export function parseGuildProfile(value: unknown, expected: string): Profile {
  const data = value as { version?: unknown; guild?: Profile; source?: unknown; asOf?: unknown };
  const g = data?.guild;
  if (data?.version !== 1 || data.source !== 'persisted snapshot' || typeof data.asOf !== 'string' || !Number.isFinite(Date.parse(data.asOf)) || !g ||
    typeof g.name !== 'string' || !guildName.test(g.name) || g.name.toLowerCase() !== expected.toLowerCase() || !Number.isInteger(g.kingdom) || !Object.hasOwn(kingdoms, g.kingdom) ||
    !Number.isInteger(g.fame) || g.fame < 0 || g.fame > 4294967295 || !Array.isArray(g.members) || !g.members.length) throw new Error('Invalid guild profile');
  for (const name of [g.ally, g.warTarget]) if (name !== null && (typeof name !== 'string' || !guildName.test(name) || name.toLowerCase() === g.name.toLowerCase())) throw new Error('Invalid guild profile');
  const names = new Set<string>(); const officers = new Set<number>(); let leaders = 0;
  for (const member of g.members) {
    if (!member || typeof member.name !== 'string' || !member.name || new TextEncoder().encode(member.name).length > 16 || /[\x00\r\n]/.test(member.name) || !Number.isInteger(member.rank) || !Object.hasOwn(roles, member.rank) || names.has(member.name.toLowerCase())) throw new Error('Invalid guild profile');
    names.add(member.name.toLowerCase());
    if (member.rank === 9) leaders++;
    else if (member.rank !== 0) { if (officers.has(member.rank)) throw new Error('Invalid guild profile'); officers.add(member.rank); }
  }
  if (leaders !== 1) throw new Error('Invalid guild profile');
  return g;
}

export function mountGuildProfile(root: HTMLElement) {
  const get = <T extends HTMLElement>(id: string) => root.querySelector<T>(`#guild-profile-${id}`)!;
  const details = get('details'), status = get('status'), title = get('name'), body = get('members'), relations = get('relations'), position = get('page');
  const search = get<HTMLInputElement>('search'), refresh = get<HTMLButtonElement>('refresh'), previous = get<HTMLButtonElement>('previous'), next = get<HTMLButtonElement>('next');
  let selected: Profile | null = null, page = 1, loading = false, name = '';
  try { const match = /^\/guilds\/([^/]+)\/?$/.exec(window.location.pathname); if (match) name = decodeURIComponent(match[1]); } catch { /* Invalid path is handled without a request. */ }
  function clear() { selected = null; details.hidden = true; title.textContent = 'Guild'; body.replaceChildren(); relations.textContent = ''; relations.hidden = true; position.textContent = ''; search.disabled = true; previous.disabled = true; next.disabled = true; get('kingdom').textContent = ''; get('fame').textContent = ''; }
  function render() {
    if (!selected) return;
    const matches = selected.members.filter(m => m.name.toLowerCase().includes(search.value.slice(0, 16).trim().toLowerCase()));
    const pages = Math.max(1, Math.ceil(matches.length / 20)); page = Math.max(1, Math.min(page, pages));
    const fragment = document.createDocumentFragment();
    for (const member of matches.slice((page - 1) * 20, page * 20)) {
      const row = document.createElement('tr');
      for (const value of [member.name, roles[member.rank]]) { const cell = document.createElement('td'); cell.textContent = value; row.append(cell); }
      fragment.append(row);
    }
    if (!matches.length) { const row = document.createElement('tr'), cell = document.createElement('td'); cell.colSpan = 2; cell.textContent = 'No matching members.'; row.append(cell); fragment.append(row); }
    body.replaceChildren(fragment); position.textContent = `${page} / ${pages} · ${matches.length} members`;
    previous.disabled = page <= 1; next.disabled = page >= pages;
  }
  async function load() {
    if (loading) return;
    clear();
    if (!guildName.test(name)) { status.textContent = 'Guild not found.'; refresh.disabled = true; return; }
    loading = true; refresh.disabled = true; root.setAttribute('aria-busy', 'true'); status.textContent = 'Loading…'; status.dataset.error = 'false';
    try {
      const response = await fetch(`/api/player/v1/guilds/${encodeURIComponent(name)}`, { credentials: 'omit', cache: 'no-store', signal: AbortSignal.timeout(8000) });
      if (response.status === 404) { status.textContent = 'Guild not found.'; return; }
      if (!response.ok || Number(response.headers.get('Content-Length') ?? 0) > maximumBytes) throw new Error('Unavailable');
      const text = await response.text(); if (new TextEncoder().encode(text).length > maximumBytes) throw new Error('Unavailable');
      selected = parseGuildProfile(JSON.parse(text), name); page = 1;
      title.textContent = selected.name; get('kingdom').textContent = kingdoms[selected.kingdom]; get('fame').textContent = selected.fame.toLocaleString('en-US');
      relations.textContent = [selected.ally ? `Ally: ${selected.ally}` : '', selected.warTarget ? `War target: ${selected.warTarget}` : ''].filter(Boolean).join(' · '); relations.hidden = !relations.textContent;
      details.hidden = false; search.disabled = false; status.textContent = ''; render();
    } catch { clear(); status.textContent = 'Guild unavailable. Try Refresh.'; status.dataset.error = 'true'; }
    finally { loading = false; refresh.disabled = false; root.setAttribute('aria-busy', 'false'); }
  }
  search.addEventListener('input', () => { page = 1; render(); });
  previous.addEventListener('click', () => { if (!loading) { page--; render(); } }); next.addEventListener('click', () => { if (!loading) { page++; render(); } });
  refresh.addEventListener('click', () => { void load(); }); void load();
}
