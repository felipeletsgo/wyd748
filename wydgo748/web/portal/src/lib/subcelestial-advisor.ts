import type { components } from './api.generated';

type Advice = components['schemas']['PlayerSubCelestialAscension'];
const gates = ['celestial', 'level', 'fame', 'sefirot', 'idealStone', 'noAlternate', 'face', 'rewardSpace'] as const;
const integer = (value: number, minimum: number, maximum: number) => Number.isInteger(value) && value >= minimum && value <= maximum;

// Saved information, not a website transformation or live authorization.
export class SubCelestialAdvisor {
  constructor(private disclosure: HTMLDetailsElement, private checks: HTMLElement, private status: HTMLElement) {}
  clear() {
    this.disclosure.open = false; this.disclosure.hidden = true;
    this.checks.replaceChildren(); this.status.replaceChildren();
  }
  render(advice: Advice | undefined) {
    this.clear();
    if (!advice) return;
    if (typeof advice.applicable !== 'boolean' || typeof advice.requirementsMet !== 'boolean' ||
      !integer(advice.requiredLevel, 1, 4294967296) || !integer(advice.requiredFame, 0, 4294967295) ||
      !Array.isArray(advice.checks) || advice.checks.length !== gates.length ||
      advice.checks.some(check => !check || typeof check !== 'object' || !gates.includes(check.id) || typeof check.met !== 'boolean') ||
      new Set(advice.checks.map(check => check.id)).size !== gates.length ||
      advice.applicable !== advice.checks.find(check => check.id === 'celestial')?.met ||
      advice.requirementsMet !== advice.checks.every(check => check.met)) {
      this.disclosure.hidden = false; this.status.textContent = 'Progression unavailable. Refresh to try again.'; return;
    }
    if (!advice.applicable) return;
    const labels = new Map<string, string>([
      ['level', `Celestial level ${advice.requiredLevel}`], ['fame', `${advice.requiredFame} Fame`],
      ['sefirot', 'Class Sefirot · Equipped'], ['idealStone', 'Ideal Stone · Inventory'],
      ['noAlternate', 'SubCelestial not created'], ['face', 'Character appearance'], ['rewardSpace', 'Reward space'],
    ]);
    const fragment = document.createDocumentFragment();
    for (const id of gates.slice(1)) {
      const met = advice.checks.find(check => check.id === id)!.met;
      if (id === 'face' && met) continue; // Surface corrupt appearance only when it blocks creation.
      const term = document.createElement('dt'); term.textContent = labels.get(id)!;
      const value = document.createElement('dd'); value.textContent = met ? 'Met' : 'Missing';
      fragment.append(term, value);
    }
    this.checks.replaceChildren(fragment);
    this.status.textContent = advice.requirementsMet ? 'Saved requirements met. Use the Ideal Stone in-game.' : 'Missing requirements.';
    this.disclosure.hidden = false;
  }
}
