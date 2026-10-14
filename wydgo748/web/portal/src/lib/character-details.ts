import type { components } from './api.generated';
import { displayTime } from './account-sessions';
import type { ArchAdvisor } from './arch-advisor';
import type { CelestialAdvisor } from './celestial-advisor';
import type { SubCelestialAdvisor } from './subcelestial-advisor';

type Response = components['schemas']['PlayerCharacterResponse'];
const equipmentSlots = ['Helmet', 'Armor', 'Pants', 'Gloves', 'Boots', 'Weapon', 'Shield',
  'Accessory 1', 'Accessory 2', 'Accessory 3', 'Accessory 4', 'Accessory 5', 'Accessory 6', 'Mount', 'Cape'];
const classes = ['TransKnight', 'Foema', 'BeastMaster', 'Hunter'];
const evolutions: Record<string, string> = { mortal: 'Mortal', arch: 'Arch', celestial: 'Celestial', subcelestial: 'Subcelestial' };

// Every selection invalidates older replies. Private DOM is cleared before a
// request, on failures and when the account/session changes.
export class CharacterDetails {
  private generation = 0;
  private selected: number | undefined;
  constructor(private panel: HTMLElement, private title: HTMLElement, private stats: HTMLElement,
    private equipment: HTMLElement, private timestamp: HTMLElement, private status: HTMLElement,
    private refresh: HTMLButtonElement, private api: <T>(path: string) => Promise<T>,
    private identity: () => string | undefined, private selectInventory: (slot: number) => void = () => {}, private advisor?: ArchAdvisor,
    private celestialAdvisor?: CelestialAdvisor, private subCelestialAdvisor?: SubCelestialAdvisor) {
    refresh.addEventListener('click', () => { if (this.selected !== undefined) void this.load(this.selected); });
  }
  clear() {
    this.advisor?.clear();
    this.celestialAdvisor?.clear();
    this.subCelestialAdvisor?.clear();
    this.generation++; this.selected = undefined;
    this.panel.hidden = true; this.refresh.disabled = true;
    for (const node of [this.title, this.stats, this.equipment, this.timestamp, this.status]) node.replaceChildren();
  }
  async load(slot: number) {
    const owner = this.identity();
    if (!owner || !Number.isInteger(slot) || slot < 0 || slot > 3) { this.clear(); return; }
    this.selectInventory(slot);
    this.clear();
    const generation = this.generation;
    this.selected = slot; this.panel.hidden = false;
    this.title.textContent = 'Character'; this.status.textContent = 'Loading…'; this.status.dataset.error = 'false';
    try {
      const response = await this.api<Response>(`/characters/${slot}`);
      if (generation !== this.generation || owner !== this.identity()) return;
      const character = response.character;
      if (response.version !== 1 || character.slot !== slot || !Array.isArray(character.equipment) || character.equipment.length > 15) throw new Error('Invalid character response');
      if (character.fame !== undefined && (!Number.isInteger(character.fame) || character.fame < 0 || character.fame > 4294967295)) throw new Error('Invalid Fame response');
      this.title.textContent = character.name;
      const labels = [
        ['Class', classes[character.class] ?? 'Unknown'], ['Evolution', evolutions[character.evolution] ?? 'Unknown'],
        ['Level', String(character.level + 1)], ['EXP', character.exp.toLocaleString('en-US')],
        ['Gold', character.gold.toLocaleString('en-US')], ['CP', String(character.cp)],
      ];
      // Omit unknown data during a rolling release; never derive Fame from CP.
      if (character.fame !== undefined) labels.push(['Fame', character.fame.toLocaleString('en-US')]);
      const stats = document.createDocumentFragment();
      for (const [label, value] of labels) {
        const term = document.createElement('dt'); term.textContent = label;
        const definition = document.createElement('dd'); definition.textContent = value; stats.append(term, definition);
      }
      const items = new Map(character.equipment.map(item => [item.slot, item]));
      if (items.size !== character.equipment.length || character.equipment.some(item => !Number.isInteger(item.slot) || item.slot < 1 || item.slot > 15 || !Number.isInteger(item.itemId) || item.itemId < 1 || item.itemId > 65535)) throw new Error('Invalid equipment response');
      const equipment = document.createDocumentFragment();
      equipmentSlots.forEach((label, index) => {
        const tile = document.createElement('li'); tile.className = 'equipment-slot';
        const heading = document.createElement('span'); heading.className = 'muted'; heading.textContent = label;
        const item = items.get(index + 1);
        const name = document.createElement(item?.name ? 'a' : 'span');
        name.textContent = item ? item.name || `Item ${item.itemId}` : 'Empty';
        if (item?.name) (name as HTMLAnchorElement).href = `/database/items/${item.itemId}/`;
        tile.append(heading, name); equipment.append(tile);
      });
      this.stats.replaceChildren(stats); this.equipment.replaceChildren(equipment);
      this.timestamp.textContent = `Saved: ${displayTime(character.asOf)}`;
      this.advisor?.render(character.ascension);
      this.celestialAdvisor?.render(character.celestialAscension);
      this.subCelestialAdvisor?.render(character.subCelestialAscension);
      this.status.textContent = '';
    } catch {
      if (generation === this.generation && owner === this.identity()) {
        this.advisor?.clear();
        this.celestialAdvisor?.clear();
        this.subCelestialAdvisor?.clear();
        this.title.textContent = 'Character'; this.stats.replaceChildren(); this.equipment.replaceChildren(); this.timestamp.replaceChildren();
        this.status.textContent = 'Unable to load this character. Try again.'; this.status.dataset.error = 'true';
      }
    } finally {
      if (generation === this.generation && owner === this.identity()) this.refresh.disabled = false;
    }
  }
}
