import type { components } from './api.generated';

type Advice = components['schemas']['PlayerCelestialAscension'];
const gates = ['arch', 'level', 'mortalLevel', 'idealStone'] as const;
const displayedLevel = (value: number) => Number.isInteger(value) && value >= 1 && value <= 4294967296;

// Render only server-derived saved requirements, never live authorization.
export class CelestialAdvisor {
  constructor(private disclosure: HTMLDetailsElement, private checks: HTMLElement, private status: HTMLElement) {}
  clear() {
    this.disclosure.open = false; this.disclosure.hidden = true;
    this.checks.replaceChildren(); this.status.replaceChildren();
  }
  render(advice: Advice | undefined) {
    this.clear();
    if (!advice) return; // Older API: keep the character view usable.
    if (typeof advice.applicable !== 'boolean' || typeof advice.requirementsMet !== 'boolean' ||
      !displayedLevel(advice.requiredLevel) || !displayedLevel(advice.requiredMortalLevel) ||
      !Array.isArray(advice.checks) || advice.checks.length !== gates.length ||
      advice.checks.some(check => !check || typeof check !== 'object' || !gates.includes(check.id) || typeof check.met !== 'boolean') ||
      new Set(advice.checks.map(check => check.id)).size !== gates.length ||
      advice.applicable !== advice.checks.find(check => check.id === 'arch')?.met ||
      advice.requirementsMet !== advice.checks.every(check => check.met)) {
      this.disclosure.hidden = false; this.status.textContent = 'Progression unavailable. Refresh to try again.'; return;
    }
    if (!advice.applicable) return;
    const labels = new Map<string, string>([
      ['level', `Arch level ${advice.requiredLevel}`], ['mortalLevel', `Source Mortal level ${advice.requiredMortalLevel}`],
      ['idealStone', 'Ideal Stone · Inventory'],
    ]);
    const fragment = document.createDocumentFragment();
    for (const id of gates.slice(1)) {
      const term = document.createElement('dt'); term.textContent = labels.get(id)!;
      const value = document.createElement('dd'); value.textContent = advice.checks.find(check => check.id === id)!.met ? 'Met' : 'Missing';
      fragment.append(term, value);
    }
    this.checks.replaceChildren(fragment);
    this.status.textContent = advice.requirementsMet ? 'Saved requirements met. Use the Ideal Stone in-game.' : 'Missing requirements.';
    this.disclosure.hidden = false;
  }
}
