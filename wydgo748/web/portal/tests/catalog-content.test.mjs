import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { readFileSync } from 'node:fs';
import test from 'node:test';
const file = path => readFileSync(new URL(`../dist/${path}`, import.meta.url), 'utf8');
test('public catalog and search index agree, use stable routes and preserve verified source hashes', () => {
  const catalog = JSON.parse(file('database/catalog.json'));
  const index = JSON.parse(file('database/index.json'));
  assert.equal(catalog.schemaVersion, 2); assert.equal(index.contentHash, catalog.contentHash);
  const claimed = catalog.contentHash;
  catalog.contentHash = '';
  // Go escapes HTML characters in JSON; hash the Go-produced artifact instead
  // of pretending JavaScript serialization is its canonical representation.
  const generated = JSON.parse(readFileSync(new URL('../src/data/catalog.generated.json', import.meta.url), 'utf8'));
  assert.equal(generated.contentHash, claimed);
  for (const source of catalog.sources) {
    assert.ok(['catalog/itemlist.csv', 'catalog/Itemname.csv', 'catalog/ItemEffect.h', 'catalog/SkillData.csv', 'content/quests.json', 'content/quest_zones.json'].includes(source.file));
    const bytes = readFileSync(new URL(`../../../data/${source.file}`, import.meta.url));
    assert.equal(createHash('sha256').update(bytes).digest('hex'), source.sha256);
  }
  assert.equal(index.items.length, catalog.items.length); assert.equal(index.skills.length, 103);
  assert.ok(index.items.every(item => Object.keys(item).sort().join(',') === 'id,name'));
  for (const item of catalog.items) {
    const page = file(`database/items/${item.id}/index.html`);
    assert.ok(page.includes(`ID ${item.id}`));
    assert.equal(/<details[^>]*\bopen\b/.test(page), false);
  }
  for (const skill of catalog.skills) {
    assert.match(file(`database/skills/${skill.id}/index.html`), /Base cooldown/);
    assert.ok(file(`database/items/${skill.bookItemId}/index.html`).includes(`/database/skills/${skill.id}/`));
  }
});
test('quest pages preserve display levels, rewards, prerequisites and reverse item links', () => {
  const catalog = JSON.parse(file('database/catalog.json'));
  const index = JSON.parse(file('database/index.json'));
  assert.equal(catalog.quests.length, 22); assert.equal(index.quests.length, 22);
  assert.match(file('database/quests/1/index.html'), /40–116/);
  assert.match(file('database/quests/1/index.html'), /not a personal duration/);
  assert.match(file('database/quests/18/index.html'), /zero balance/);
  assert.match(file('database/quests/2/index.html'), /400,000/);
  for (const q of catalog.quests) {
    const page = file(`database/quests/${q.id}/index.html`);
    assert.ok(page.includes(`ID ${q.id}`));
    assert.equal(/<details[^>]*\bopen\b/.test(page), false);
    if (q.afterQuestId) assert.ok(page.includes(`/database/quests/${q.afterQuestId}/`));
    const ids = [...q.requiredItems, ...q.consumedItems, ...q.rewardItems].map(ref => ref.id);
    if (q.recharge) ids.push(q.recharge.itemId);
    for (const id of ids) {
      assert.ok(page.includes(`/database/items/${id}/`));
      assert.ok(file(`database/items/${id}/index.html`).includes(`/database/quests/${q.id}/`));
    }
  }
});
