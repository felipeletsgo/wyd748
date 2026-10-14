import type { components } from './api.generated';

type Advice = components['schemas']['PlayerArchAscension'];
const gates = ['mortal', 'level', 'eternalStone', 'sefirot', 'freeSlot'] as const;

// The API evaluates the saved snapshot; the browser renders, never authorizes
// an ascension or recalculates gameplay eligibility from catalogue defaults.
export class ArchAdvisor {
  constructor(private disclosure: HTMLDetailsElement, private checks: HTMLElement, private status: HTMLElement) {}
  clear() {
    this.disclosure.open = false; this.disclosure.hidden = true;
    this.checks.replaceChildren(); this.status.replaceChildren();
  }
  render(advice: Advice | undefined) {
    this.clear();
    // An older API has no advisor. Keep its existing character view usable.
    if (!advice) return;
    if (typeof advice.applicable !== 'boolean' || typeof advice.requirementsMet !== 'boolean' ||
      !Number.isInteger(advice.requiredLevel) || advice.requiredLevel < 1 || advice.requiredLevel > 4294967296 ||
      !Array.isArray(advice.checks) || advice.checks.length !== gates.length ||
      advice.checks.some(check => !check || typeof check !== 'object' || !gates.includes(check.id) || typeof check.met !== 'boolean') ||
      new Set(advice.checks.map(check => check.id)).size !== gates.length ||
      advice.applicable !== advice.checks.find(check => check.id === 'mortal')?.met ||
      advice.requirementsMet !== advice.checks.every(check => check.met)) {
      this.disclosure.hidden = false; this.status.textContent = 'Progression unavailable. Refresh to try again.'; return;
    }
    if (!advice.applicable) return;
    const labels = new Map<string, string>([
      ['level', `Level ${advice.requiredLevel}`], ['eternalStone', 'Eternal Stone · Accessory 3'],
      ['sefirot', 'Sefirot · Accessory 4'], ['freeSlot', 'Free character slot'],
    ]);
    const fragment = document.createDocumentFragment();
    for (const id of gates.slice(1)) {
      const term = document.createElement('dt'); term.textContent = labels.get(id)!;
      const value = document.createElement('dd'); value.textContent = advice.checks.find(check => check.id === id)!.met ? 'Met' : 'Missing';
      fragment.append(term, value);
    }
    this.checks.replaceChildren(fragment);
    this.status.textContent = advice.requirementsMet ? 'Saved requirements met. Ascend at the king in-game.' : 'Missing requirements.';
    this.disclosure.hidden = false;
  }
}
