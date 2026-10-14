import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const compiled = ts.transpileModule(readFileSync(new URL('../src/lib/character-details.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
class Element {
  children = []; dataset = {}; handlers = {}; disabled = false; hidden = false; textContent = '';
  constructor(tag = 'div') { this.tag = tag; }
  addEventListener(name, fn) { this.handlers[name] = fn; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.textContent = ''; this.children = []; this.append(...children); }
}
const timestamp = '2026-10-13T00:00:00Z';
const detail = { slot: 0, name: 'Knight', class: 0, evolution: 'mortal', level: 399, exp: 12345, gold: 4000000000, cp: -15, asOf: timestamp,
  equipment: [{ slot: 1, itemId: 100, name: '<img onerror=attack()>', effects: [43, 9, 0, 0, 0, 0] }, { slot: 15, itemId: 101, name: '', effects: [0, 0, 0, 0, 0, 0] }] };
function fixture() {
  let identity = 'current-session'; let responder = async () => ({ version: 1, character: detail });
  const calls = [];
  const context = { exports: {}, require: () => ({ displayTime: value => value }), document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') } };
  vm.runInNewContext(compiled, context);
  const panel = new Element(), title = new Element(), stats = new Element('dl'), equipment = new Element('ul'), saved = new Element(), status = new Element(), refresh = new Element('button');
  const advisor = { clears: 0, rendered: [], clear() { this.clears++; this.rendered = []; }, render(value) { this.rendered.push(value); } };
  const celestialAdvisor = { ...advisor, rendered: [] };
  const subCelestialAdvisor = { ...advisor, rendered: [] };
  const controller = new context.exports.CharacterDetails(panel, title, stats, equipment, saved, status, refresh,
    async path => { calls.push(path); return responder(path); }, () => identity, () => {}, advisor, celestialAdvisor, subCelestialAdvisor);
  return { controller, panel, title, stats, equipment, saved, status, refresh, calls, advisor, celestialAdvisor, subCelestialAdvisor,
    respond: value => { responder = value; }, identity: value => { identity = value; } };
}
test('saved equipment has fifteen slots, text-only names, safe item links and no combat recomputation', async () => {
  const f = fixture(); await f.controller.load(0);
  assert.equal(f.panel.hidden, false); assert.equal(f.title.textContent, 'Knight');
  assert.equal(f.stats.children[5].textContent, '400'); assert.equal(f.stats.children[9].textContent, '4,000,000,000');
  assert.equal(f.equipment.children.length, 15);
  const helmet = f.equipment.children[0].children[1];
  assert.equal(helmet.textContent, '<img onerror=attack()>'); assert.equal(helmet.href, '/database/items/100/');
  assert.equal(f.equipment.children[1].children[1].textContent, 'Empty');
  assert.equal(f.equipment.children[14].children[1].textContent, 'Item 101'); assert.equal(f.equipment.children[14].children[1].tag, 'span');
  assert.match(f.saved.textContent, /Saved:/); assert.equal(f.refresh.disabled, false);
  assert.equal(f.status.textContent, ''); assert.equal(f.calls[0], '/characters/0');
});
test('character advice shares selection, failed-refresh and logout invalidation', async () => {
  const f = fixture(); const ascension = { applicable: true };
  const celestialAscension = { applicable: false };
  const subCelestialAscension = { applicable: true };
  f.respond(async () => ({ version: 1, character: { ...detail, ascension, celestialAscension, subCelestialAscension } }));
  await f.controller.load(0);
  assert.equal(f.advisor.rendered[0], ascension);
  assert.equal(f.celestialAdvisor.rendered[0], celestialAscension);
  assert.equal(f.subCelestialAdvisor.rendered[0], subCelestialAscension);
  let complete;
  f.respond(() => new Promise(resolve => { complete = resolve; }));
  const pending = f.controller.load(1);
  assert.equal(f.advisor.rendered.length, 0);
  assert.equal(f.celestialAdvisor.rendered.length, 0);
  assert.equal(f.subCelestialAdvisor.rendered.length, 0);
  f.controller.clear();
  complete({ version: 1, character: { ...detail, slot: 1, ascension, celestialAscension, subCelestialAscension } }); await pending;
  assert.equal(f.advisor.rendered.length, 0);
  f.respond(async () => { throw new Error('private error'); }); await f.controller.load(0);
  assert.equal(f.advisor.rendered.length, 0);
  assert.equal(f.advisor.clears >= 4, true);
  assert.equal(f.celestialAdvisor.rendered.length, 0);
  assert.equal(f.celestialAdvisor.clears >= 4, true);
  assert.equal(f.subCelestialAdvisor.rendered.length, 0);
  assert.equal(f.subCelestialAdvisor.clears >= 4, true);
});
test('saved Fame is a separate wide counter; older replies omit it and invalid values clear private data', async () => {
  const f = fixture();
  const stats = () => Object.fromEntries(f.stats.children.filter((_, i) => i % 2 === 0).map((term, i) => [term.textContent, f.stats.children[i * 2 + 1].textContent]));
  for (const fame of [0, 123456, 4294967295]) {
    f.respond(async () => ({ version: 1, character: { ...detail, fame } })); await f.controller.load(0);
    assert.equal(stats().Fame, fame.toLocaleString('en-US')); assert.equal(stats().CP, '-15');
  }
  f.respond(async () => ({ version: 1, character: detail })); await f.controller.load(0);
  assert.equal(Object.hasOwn(stats(), 'Fame'), false);
  for (const fame of [null, '100', -1, 1.5, 4294967296, NaN, Infinity]) {
    f.respond(async () => ({ version: 1, character: { ...detail, fame } })); await f.controller.load(0);
    assert.equal(f.stats.children.length, 0); assert.equal(f.equipment.children.length, 0);
    assert.equal(f.saved.textContent, ''); assert.match(f.status.textContent, /Unable to load/);
  }
  f.respond(async () => ({ version: 1, character: { ...detail, fame: 100 } })); await f.controller.load(0);
  f.controller.clear(); assert.equal(f.stats.children.length, 0);
});
test('last selection wins when requests finish out of order', async () => {
  const f = fixture(); let old, latest;
  f.respond(path => new Promise(resolve => { if (path.endsWith('/0')) old = resolve; else latest = resolve; }));
  const one = f.controller.load(0); const two = f.controller.load(1);
  latest({ version: 1, character: { ...detail, slot: 1, name: 'Hunter' } }); await two;
  old({ version: 1, character: detail }); await one;
  assert.equal(f.title.textContent, 'Hunter'); assert.equal(f.refresh.disabled, false);
});
test('sign-out and account switching cannot repopulate private character data', async () => {
  for (const clear of [true, false]) {
    const f = fixture(); let complete;
    f.respond(() => new Promise(resolve => { complete = resolve; })); const pending = f.controller.load(0);
    f.identity('other-session'); if (clear) f.controller.clear();
    complete({ version: 1, character: detail }); await pending;
    assert.equal(f.equipment.children.length, 0); assert.equal(f.stats.children.length, 0);
    assert.notEqual(f.title.textContent, 'Knight');
    if (clear) assert.equal(f.panel.hidden, true);
  }
});
test('failed refresh clears stale data; missing slots and invalid equipment fail closed', async () => {
  const f = fixture(); await f.controller.load(0);
  for (const response of [null, { version: 2, character: detail }, { version: 1, character: { ...detail, slot: 1 } },
    { version: 1, character: { ...detail, equipment: [...detail.equipment, detail.equipment[0]] } },
    { version: 1, character: { ...detail, equipment: [{ slot: 16, itemId: 100 }] } },
    { version: 1, character: { ...detail, equipment: [{ slot: 1, itemId: '/attack' }] } }]) {
    f.respond(async () => { if (!response) throw new Error('private failure'); return response; });
    await f.controller.load(0);
    assert.equal(f.equipment.children.length, 0); assert.equal(f.stats.children.length, 0); assert.equal(f.saved.textContent, '');
    assert.match(f.status.textContent, /Unable to load/); assert.equal(f.status.textContent.includes('private'), false); assert.equal(f.refresh.disabled, false);
  }
  const before = f.calls.length;
  await f.controller.load(4); await f.controller.load(-1); await f.controller.load(NaN);
  f.identity(undefined); await f.controller.load(0);
  assert.equal(f.calls.length, before); assert.equal(f.panel.hidden, true);
});
