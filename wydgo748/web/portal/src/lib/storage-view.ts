import type { components } from './api.generated';
import { displayTime } from './account-sessions';

type Response = components['schemas']['PlayerStorageResponse'];

// Fetch only when expanded. A selection, collapse or sign-out invalidates
// in-flight reads; saved state never becomes a browser-side economy editor.
export class StorageView {
  private generation = 0;
  private slot: number | null | undefined;
  constructor(private disclosure: HTMLDetailsElement, private items: HTMLElement,
    private summary: HTMLElement, private timestamp: HTMLElement, private status: HTMLElement,
    private refresh: HTMLButtonElement, private api: <T>(path: string) => Promise<T>,
    private identity: () => string | undefined) {
    disclosure.addEventListener('toggle', () => {
      if (disclosure.open) void this.load(); else this.reset();
    });
    refresh.addEventListener('click', () => { void this.load(); });
  }
  private reset() {
    this.generation++; this.refresh.disabled = true;
    for (const node of [this.items, this.summary, this.timestamp, this.status]) node.replaceChildren();
  }
  clear() { this.slot = undefined; this.disclosure.open = false; this.reset(); }
  select(slot: number | null) {
    this.clear();
    if (slot === null || Number.isInteger(slot) && slot >= 0 && slot <= 3) this.slot = slot;
  }
  async load() {
    const owner = this.identity(), slot = this.slot;
    if (!owner || slot === undefined || !this.disclosure.open) { this.reset(); return; }
    this.reset(); const generation = this.generation;
    this.status.textContent = 'Loading…'; this.status.dataset.error = 'false';
    try {
      const response = await this.api<Response>(slot === null ? '/cargo' : `/characters/${slot}/inventory`);
      if (generation !== this.generation || owner !== this.identity()) return;
      const storage = response.storage, capacity = slot === null ? 120 : 63;
      if (response.version !== 1 || response.source !== 'persisted snapshot' || storage.capacity !== capacity ||
        !Number.isFinite(Date.parse(storage.asOf)) || !Array.isArray(storage.items) || storage.items.length > capacity ||
        (slot === null ? storage.characterSlot !== undefined || !Number.isInteger(storage.gold) || storage.gold! < 0 || storage.gold! > 4294967295 : storage.characterSlot !== slot || storage.gold !== undefined)) throw new Error('Invalid storage response');
      const slots = new Set<number>();
      for (const item of storage.items) {
        if (!Number.isInteger(item.slot) || item.slot < 0 || item.slot >= capacity || slots.has(item.slot) ||
          !Number.isInteger(item.itemId) || item.itemId < 1 || item.itemId > 65535 || typeof item.name !== 'string' ||
          !Number.isInteger(item.quantity) || item.quantity < 1 || item.quantity > 255 ||
          !Array.isArray(item.effects) || item.effects.length !== 6 || item.effects.some(value => !Number.isInteger(value) || value < 0 || value > 255)) throw new Error('Invalid stored item');
        slots.add(item.slot);
      }
      const fragment = document.createDocumentFragment();
      [...storage.items].sort((a, b) => a.slot - b.slot).forEach(item => {
        const tile = document.createElement('li'); tile.className = 'equipment-slot';
        const position = document.createElement('span'); position.className = 'muted'; position.textContent = `Slot ${item.slot + 1}`;
        const name = document.createElement(item.name ? 'a' : 'span'); name.textContent = item.name || `Item ${item.itemId}`;
        if (item.name) (name as HTMLAnchorElement).href = `/database/items/${item.itemId}/`;
        const quantity = document.createElement('span'); quantity.textContent = `×${item.quantity}`;
        tile.append(position, name, quantity); fragment.append(tile);
      });
      this.items.replaceChildren(fragment);
      this.summary.textContent = `${storage.items.length} / ${capacity} slots${slot === null ? ` · Gold ${storage.gold!.toLocaleString('en-US')}` : ''}`;
      this.timestamp.textContent = `Saved: ${displayTime(storage.asOf)}`;
      this.status.textContent = storage.items.length === 0 ? 'No items.' : '';
    } catch {
      if (generation === this.generation && owner === this.identity()) {
        this.items.replaceChildren(); this.summary.replaceChildren(); this.timestamp.replaceChildren();
        this.status.textContent = 'Unable to load saved items. Try again.'; this.status.dataset.error = 'true';
      }
    } finally {
      if (generation === this.generation && owner === this.identity()) this.refresh.disabled = false;
    }
  }
}
