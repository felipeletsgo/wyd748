import type { components } from '../lib/api.generated';
type Zone = components['schemas']['ZoneInfo'];
type Snapshot = components['schemas']['MapSnapshot'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

// Map cells to canvas pixels: the terrain image has one pixel per 4x4 cells.
export const toPixel = (cell: number) => Math.floor(cell / 4);
export const toCell = (pixel: number, scale: number) => Math.max(0, Math.min(4095, Math.round(pixel * scale * 4)));

export function zoneRows(zone: Zone): [string, string][] {
  const rows: [string, string][] = [
    ['Cell', `${zone.x}, ${zone.y}`],
    ['Terrain', zone.terrainLoaded ? `height ${zone.height} · attribute 0x${zone.attribute.toString(16).padStart(2, '0')}` : 'not loaded'],
    ['Movement', zone.blocked ? 'Blocked' : zone.walkable ? 'Walkable' : 'Not walkable'],
  ];
  const flags = [zone.teleportActivation && 'client teleport attribute', zone.newbieZone && 'newbie field'].filter(Boolean);
  if (flags.length) rows.push(['Flags', flags.join(' · ')]);
  if (zone.teleports.length) rows.push(['Portal', zone.teleports.map(t => `${t.name || 'unnamed'} → ${t.destX}, ${t.destY}${t.price ? ` (${t.price} gold)` : ''}`).join(' · ')]);
  if (zone.questZones.length) rows.push(['Quest zones', zone.questZones.join(', ')]);
  if (zone.clearAreas.length) rows.push(['Clear areas', zone.clearAreas.join(', ')]);
  if (zone.instances.length) rows.push(['Instance areas', zone.instances.map(i => i.name || i.id).join(', ')]);
  return rows;
}

export function setupWorldMap(api: API, explain: (error: unknown) => string, onPlayer?: (uid: string) => void) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const canvas = el<HTMLCanvasElement>('world-canvas');
  let terrain: HTMLImageElement | undefined;
  let snapshot: Snapshot | undefined;
  let busy = false;
  let generation = 0;

  function draw() {
    const context = canvas.getContext('2d');
    if (!context) return;
    context.clearRect(0, 0, canvas.width, canvas.height);
    if (terrain) context.drawImage(terrain, 0, 0);
    if (!snapshot) return;
    const r = snapshot.radius;
    context.strokeStyle = '#ffffff'; context.lineWidth = 1;
    context.strokeRect(toPixel(snapshot.x - r), toPixel(snapshot.y - r), Math.max(2, toPixel(2 * r)), Math.max(2, toPixel(2 * r)));
    const dot = (x: number, y: number, color: string) => { context.fillStyle = color; context.fillRect(toPixel(x) - 1, toPixel(y) - 1, 3, 3); };
    for (const mob of snapshot.mobs) dot(mob.x, mob.y, mob.boss ? '#ff9a1f' : mob.npc ? '#c9c9c9' : '#e0483a');
    for (const player of snapshot.players) dot(player.x, player.y, '#3aa0ff');
  }

  function renderZone(zone: Zone) {
    const rows = zoneRows(zone).map(([label, value]) => {
      const div = document.createElement('div');
      const dt = document.createElement('dt'); dt.textContent = label;
      const dd = document.createElement('dd'); dd.textContent = value;
      div.append(dt, dd); return div;
    });
    el('world-zone').replaceChildren(...rows);
  }

  function renderLive(value: Snapshot) {
    const items: HTMLElement[] = [];
    for (const player of value.players) {
      const li = document.createElement('li');
      const link = document.createElement('button'); link.type = 'button'; link.className = 'button subtle';
      link.textContent = `Player ${player.name} · ${player.account} · level ${player.level} · ${player.x}, ${player.y}`;
      link.addEventListener('click', () => onPlayer?.(player.uid));
      li.append(link); items.push(li);
    }
    for (const mob of value.mobs) {
      const li = document.createElement('li');
      li.textContent = `${mob.boss ? 'Boss' : mob.npc ? 'NPC' : 'Monster'} ${mob.name} #${mob.id} · ${mob.hp}/${mob.maxHp} HP · ${mob.x}, ${mob.y}`;
      items.push(li);
    }
    if (!items.length) { const li = document.createElement('li'); li.textContent = 'Nothing online in this area.'; items.push(li); }
    el('world-live').replaceChildren(...items);
  }

  async function inspect(x: number, y: number) {
    if (busy) return;
    const current = generation;
    busy = true;
    el('world-status').textContent = 'Reading…';
    const radius = el<HTMLSelectElement>('world-radius').value;
    const cell = new URLSearchParams({ x: String(x), y: String(y) });
    const [zone, live] = await Promise.allSettled([api<Zone>(`/staff/world/zone?${cell}`),
      api<Snapshot>(`/staff/world/map?${new URLSearchParams({ x: String(x), y: String(y), radius })}`)]);
    if (current !== generation) return;
    if (zone.status === 'fulfilled') renderZone(zone.value); else el('world-zone').replaceChildren();
    if (live.status === 'fulfilled') { snapshot = live.value; renderLive(live.value); }
    else { snapshot = undefined; el('world-live').replaceChildren(); }
    el('world-status').textContent = live.status === 'fulfilled'
      ? `${live.value.players.length} player(s), ${live.value.mobs.length} monster(s)${live.value.truncated ? ' · list capped' : ''} · ${new Date(live.value.asOf).toLocaleTimeString('en-US')}`
      : `Live view: ${explain(live.reason)}`;
    draw();
    busy = false;
  }

  el('world-load').addEventListener('click', () => {
    const current = generation;
    const image = new Image();
    image.onload = () => {
      if (current !== generation) return;
      terrain = image; el('world-status').textContent = 'Terrain loaded. Click the map or enter a cell.'; draw();
    };
    image.onerror = () => {
      if (current !== generation) return;
      el('world-status').textContent = 'Terrain unavailable. Check permissions or the panel connection.';
    };
    image.src = '/api/v1/staff/world/terrain';
  });
  canvas.addEventListener('click', event => {
    const box = canvas.getBoundingClientRect();
    const scale = canvas.width / box.width;
    const x = toCell(event.clientX - box.left, scale), y = toCell(event.clientY - box.top, scale);
    el<HTMLInputElement>('world-x').value = String(x); el<HTMLInputElement>('world-y').value = String(y);
    void inspect(x, y);
  });
  el<HTMLFormElement>('world-form').addEventListener('submit', event => {
    event.preventDefault();
    const x = Number(el<HTMLInputElement>('world-x').value), y = Number(el<HTMLInputElement>('world-y').value);
    if (!Number.isInteger(x) || !Number.isInteger(y) || x < 0 || y < 0 || x > 4095 || y > 4095) { el('world-status').textContent = 'Enter a cell from 0 to 4095.'; return; }
    void inspect(x, y);
  });
  return {
    focus(x: number, y: number) {
      el<HTMLInputElement>('world-x').value = String(x); el<HTMLInputElement>('world-y').value = String(y);
      document.getElementById('world')?.scrollIntoView({ block: 'start' });
      void inspect(x, y);
    },
    reset() {
      generation++;
      snapshot = undefined; busy = false; draw();
      el('world-zone').replaceChildren(); el('world-live').replaceChildren();
      el('world-status').textContent = terrain ? 'Terrain loaded. Click the map or enter a cell.' : 'Terrain not loaded.';
    },
  };
}
