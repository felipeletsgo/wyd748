import type { components } from '../lib/api.generated';
type View = components['schemas']['InstancesView'];
type Instance = components['schemas']['InstanceStatus'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

// The earliest upcoming deadline, labeled, or an empty string.
export function nextDeadline(instance: Instance, now: number) {
  const candidates: [string, string | null][] = [['combat ends', instance.combatEnds], ['transition', instance.transitionAt],
    ['quiz ends', instance.quizEnds], ['exit', instance.exitAt], ['closes', instance.hardDeadline], ['schedule ends', instance.scheduleEnd]];
  const upcoming = candidates.filter((c): c is [string, string] => !!c[1] && Date.parse(c[1]) > now)
    .sort((a, b) => Date.parse(a[1]) - Date.parse(b[1]));
  if (!upcoming.length) return '';
  const seconds = Math.round((Date.parse(upcoming[0][1]) - now) / 1000);
  return `${upcoming[0][0]} in ${seconds >= 60 ? `${Math.floor(seconds / 60)} min ${seconds % 60} s` : `${seconds} s`}`;
}

export function setupInstances(api: API, explain: (error: unknown) => string, onLocate?: (x: number, y: number) => void) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  let busy = false;

  function render(value: View) {
    const now = Date.parse(value.asOf);
    const rows = value.instances.map(instance => {
      const row = document.createElement('tr');
      const name = document.createElement('td'); name.textContent = instance.name || instance.configId;
      const id = document.createElement('small'); id.textContent = instance.runtimeId; name.append(id);
      row.append(name);
      const members = instance.members.map(m => `${m.name}${m.leader ? ' (leader)' : ''}`).join(', ');
      for (const value of [`${instance.state || 'running'} · stage ${instance.stage}`,
        `${instance.members.length}${instance.maxPlayers ? `/${instance.maxPlayers}` : ''} ${members ? `· ${members}` : ''}`,
        `${instance.mobs} alive${instance.bossSpawned ? ' · boss spawned' : ''}${instance.rewardGranted ? ' · reward granted' : ''}`,
        nextDeadline(instance, now) || '—']) {
        const cell = document.createElement('td'); cell.textContent = value; row.append(cell);
      }
      const where = document.createElement('td');
      const locate = document.createElement('button'); locate.type = 'button'; locate.className = 'button subtle';
      locate.textContent = `${instance.x}, ${instance.y}`;
      locate.addEventListener('click', () => onLocate?.(instance.x, instance.y));
      where.append(locate); row.append(where);
      return row;
    });
    if (rows.length) el('instances-rows').replaceChildren(...rows);
    else {
      const cell = document.createElement('td'); cell.colSpan = 6; cell.className = 'empty'; cell.textContent = 'No active instances.';
      const row = document.createElement('tr'); row.append(cell); el('instances-rows').replaceChildren(row);
    }
    el('instances-status').textContent = `Snapshot from ${new Date(value.asOf).toLocaleString('en-US')}.`;
  }

  async function refresh() {
    if (busy) return;
    busy = true; el<HTMLButtonElement>('instances-refresh').disabled = true;
    try { render(await api<View>('/staff/world/instances')); }
    catch (error) { el('instances-status').textContent = explain(error); }
    finally { busy = false; el<HTMLButtonElement>('instances-refresh').disabled = false; }
  }

  el('instances-refresh').addEventListener('click', () => void refresh());
  return { refresh, reset() {
    busy = false; el('instances-status').textContent = '';
    const cell = document.createElement('td'); cell.colSpan = 6; cell.className = 'empty'; cell.textContent = 'Not queried.';
    const row = document.createElement('tr'); row.append(cell); el('instances-rows').replaceChildren(row);
  } };
}
