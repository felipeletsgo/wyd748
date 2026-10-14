import generated from '../data/catalog.generated.json';

export type CatalogItem = { id: number; name: string; baseValue: number; effects: { code: string; value: number }[]; bookSkillId?: number };
export type CatalogSkill = { id: number; name: string; bookItemId: number; class: number | null; skillPoints: number; baseMana: number; baseCooldown: number };
export type CatalogIndexRow = { id: number; name: string; class?: number | null };
type QuestItem = { id: number; quantity: number };
type Counter = { name: string; quantity: number };
export type CatalogQuest = { id: number; name: string; objective: string; repeatable: boolean; mortalOnly: boolean; minLevel: number; maxLevel?: number; afterQuestId?: number; goldCost: number; requiredItems: QuestItem[]; consumedItems: QuestItem[]; rewardItems: QuestItem[]; requiredCounters: Counter[]; consumedCounters: Counter[]; rewardCounters: Counter[]; rewardGold: number; baseRewardExp: number; citizenship: boolean; destination?: { x: number; y: number; globalReset: boolean }; refinement?: { slot: number; level: number }; recharge?: { itemId: number; counter: string; quantity: number } };
export const catalog: { schemaVersion: number; contentHash: string; sources: { file: string; sha256: string }[]; items: CatalogItem[]; skills: CatalogSkill[]; quests: CatalogQuest[] } = generated;
if (catalog.schemaVersion !== 2 || !/^[a-f0-9]{64}$/.test(catalog.contentHash)) throw new Error('Unsupported public catalog');
export const classNames = ['TransKnight', 'Foema', 'BeastMaster', 'Hunter'];
export const classLabel = (value: number | null) => value === null ? 'Shared' : classNames[value] ?? 'Unknown';
export const itemURL = (id: number) => `/database/items/${id}/`;
export const skillURL = (id: number) => `/database/skills/${id}/`;
export const questURL = (id: number) => `/database/quests/${id}/`;
export const catalogIndex = {
  schemaVersion: catalog.schemaVersion,
  contentHash: catalog.contentHash,
  items: catalog.items.map(({ id, name }) => ({ id, name })),
  skills: catalog.skills.map(({ id, name, class: skillClass }) => ({ id, name, class: skillClass })),
  quests: catalog.quests.map(({ id, name }) => ({ id, name })),
};
