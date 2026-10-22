import type { components } from '../lib/api.generated';
type Storage = components['schemas']['CharacterStorage'];
type Item = components['schemas']['StaffStoredItem'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

const equipmentSlots = ['Face', 'Helmet', 'Armor', 'Pants', 'Gloves', 'Boots', 'Weapon', 'Shield',
  'Accessory 1', 'Accessory 2', 'Accessory 3', 'Accessory 4', 'Accessory 5', 'Accessory 6', 'Mount', 'Cape'];

// One line per item: slot, catalog name (or ID) and raw effect pairs.
export function itemLine(item: Item, slotName?: string) {
  const pairs = [0, 2, 4].filter(i => item.effects[i] !== 0).map(i => `${item.effects[i]}:${item.effects[i + 1]}`);
  return `${slotName ?? `#${item.slot + 1}`} · ${item.name || `item ${item.itemId}`} (${item.itemId})${pairs.length ? ` · effects ${pairs.join(' ')}` : ''}`;
}

// Saved items load on request only: opening the inspector stays one read.
export function setupPlayerStorage(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  let target: { uid: string; account: string } | undefined;
  let generation = 0;

  function list(id: string, lines: string[], empty: string) {
    const items = (lines.length ? lines : [empty]).map(line => { const li = document.createElement('li'); li.textContent = line; return li; });
    el(id).replaceChildren(...items);
  }

  function clear(state: string) {
    el('storage-state').textContent = state;
    for (const id of ['storage-equipment', 'storage-inventory', 'storage-cargo']) el(id).replaceChildren();
  }

  async function load() {
    if (!target) return;
    const current = ++generation;
    const { uid, account } = target;
    el<HTMLButtonElement>('storage-load').disabled = true;
    clear('Querying PostgreSQL…');
    try {
      const value = await api<Storage>(`/staff/players/${encodeURIComponent(uid)}/storage?${new URLSearchParams({ account })}`);
      if (current !== generation) return;
      if (value.version !== 1 || value.uid !== uid) throw new Error('Invalid storage snapshot');
      el('storage-state').textContent = `Saved ${new Date(value.asOf).toLocaleString('en-US')} · gold ${value.gold.toLocaleString('en-US')} · Cargo gold ${value.cargoGold.toLocaleString('en-US')}`;
      list('storage-equipment', value.equipment.map(item => itemLine(item, equipmentSlots[item.slot])), 'No equipment.');
      list('storage-inventory', value.inventory.map(item => itemLine(item)), 'Inventory empty.');
      list('storage-cargo', value.cargo.map(item => itemLine(item)), 'Cargo empty.');
    } catch (error) {
      if (current !== generation) return;
      clear(explain(error));
    } finally { if (current === generation) el<HTMLButtonElement>('storage-load').disabled = false; }
  }

  el('storage-load').addEventListener('click', () => void load());
  return {
    show(uid: string, account: string) {
      if (target?.uid === uid) return;
      generation++; target = { uid, account: account.toLowerCase() };
      el<HTMLButtonElement>('storage-load').disabled = false;
      clear('Not loaded. Saved items are read only when requested.');
    },
    reset() {
      generation++; target = undefined;
      el<HTMLButtonElement>('storage-load').disabled = true;
      clear('Not loaded.');
    },
  };
}
