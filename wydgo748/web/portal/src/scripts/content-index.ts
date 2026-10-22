import type { components } from '../lib/api.generated';
type Search = components['schemas']['ContentSearch'];
type Item = components['schemas']['ContentItem'];
type Mob = components['schemas']['ContentMob'];
type Issues = components['schemas']['ContentIssues'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

// The server reports the effective rate the World rolls (level band, special
// slots, cap) before the killer's drop bonus; zero or always means every kill.
export const chance = (oneIn: number, always = false) => always || oneIn <= 0 ? 'every kill' : `1 in ${oneIn.toLocaleString('en-US')}`;
const base = (oneIn: number, baseOneIn?: number) => baseOneIn && baseOneIn !== oneIn ? ` (base 1 in ${baseOneIn.toLocaleString('en-US')})` : '';
const roles: Record<string, string> = { leader: 'leader', follower: 'follower', both: 'leader and follower' };
const reservations: Record<string, string> = { item_instance: 'item instance', colosseum: 'Colosseum' };

export function itemLines(item: Item) {
  return [
    ...item.soldBy.map(s => `Sold by ${s.npc} (slot ${s.slot})`),
    ...item.droppedBy.map(d => `Dropped by ${d.mob} · carry slot ${d.slot} · ${d.always || d.oneIn <= 0 ? 'every kill' : `${chance(d.oneIn)} per kill`}${base(d.oneIn, d.baseOneIn)}`),
    ...item.quests.map(q => `Quest ${q.name} (#${q.id}) · ${q.role}`),
  ];
}

export function mobLines(mob: Mob) {
  return [
    `${mob.kind === 'monster' ? 'Monster' : 'NPC'} · level ${mob.level}${mob.gold ? ` · gold ${mob.gold}` : ''}${mob.merchant ? ' · merchant' : ''}`,
    ...mob.spawns.map(s => `${s.reserved ? `Reserved for ${reservations[s.reserved] ?? s.reserved} (no open-world spawn): ` : 'Spawns via '}NPCGener[${s.generator}] as ${roles[s.role] ?? s.role} at ${s.x}, ${s.y} · up to ${s.maxMobs}${s.enabled ? '' : ' · disabled'}`),
    ...mob.drops.map(d => `Drops ${d.name || d.itemId} (${d.itemId}) · slot ${d.slot} · ${chance(d.oneIn ?? 0, d.always)}${base(d.oneIn ?? 0, d.baseOneIn)}`),
    ...mob.sells.map(d => `Sells ${d.name || d.itemId} (${d.itemId}) · slot ${d.slot}`),
  ];
}

export function setupContentIndex(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;

  function list(id: string, lines: string[]) {
    el(id).replaceChildren(...lines.map(line => { const li = document.createElement('li'); li.textContent = line; return li; }));
  }

  async function open(kind: string, key: string, name: string) {
    el('content-detail-title').textContent = name;
    try {
      if (kind === 'item') {
        const item = await api<Item>(`/staff/content/items/${encodeURIComponent(key)}`);
        const lines = itemLines(item);
        list('content-detail', lines.length ? lines : ['Not sold, dropped or used by any quest in the loaded data.']);
      } else {
        list('content-detail', mobLines(await api<Mob>(`/staff/content/mobs/${encodeURIComponent(key)}`)));
      }
      el('content-status').textContent = '';
    } catch (error) { el('content-status').textContent = explain(error); }
  }

  el<HTMLFormElement>('content-form').addEventListener('submit', async event => {
    event.preventDefault();
    const query = el<HTMLInputElement>('content-query').value.trim();
    if (!query) return;
    try {
      const result = await api<Search>(`/staff/content/search?${new URLSearchParams({ q: query })}`);
      const items = result.hits.map(hit => {
        const li = document.createElement('li');
        const button = document.createElement('button'); button.type = 'button'; button.className = 'button subtle';
        button.textContent = `${hit.kind === 'item' ? `Item ${hit.key}` : 'Template'} · ${hit.name}`;
        button.addEventListener('click', () => void open(hit.kind, hit.key, hit.name));
        li.append(button); return li;
      });
      if (!items.length) { const li = document.createElement('li'); li.textContent = 'No match.'; items.push(li); }
      el('content-hits').replaceChildren(...items);
      el('content-status').textContent = `Index built ${new Date(result.builtAt).toLocaleString('en-US')}.`;
    } catch (error) { el('content-status').textContent = explain(error); }
  });
  el('content-issues').addEventListener('click', async () => {
    try {
      const result = await api<Issues>('/staff/content/issues');
      el('content-detail-title').textContent = `${result.issues.length} unresolved reference(s)`;
      list('content-detail', result.issues.map(i => `${i.severity.toUpperCase()} · ${i.subject} · ${i.detail}`));
    } catch (error) { el('content-status').textContent = explain(error); }
  });
  return { reset() {
    for (const id of ['content-hits', 'content-detail']) el(id).replaceChildren();
    el('content-detail-title').textContent = ''; el('content-status').textContent = '';
  } };
}
