import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const compiled = ts.transpileModule(readFileSync(new URL('../src/lib/storage-view.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
class Element {
  children = []; dataset = {}; handlers = {}; disabled = false; open = false; textContent = '';
  constructor(tag = 'div') { this.tag = tag; }
  addEventListener(name, fn) { this.handlers[name] = fn; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.textContent = ''; this.children = []; this.append(...children); }
}
const item = { slot: 62, itemId: 412, name: '<img onerror=attack()>', effects: [61, 255, 0, 0, 0, 0], quantity: 255 };
const inventory = { capacity: 63, characterSlot: 0, asOf: '2026-10-13T00:00:00Z', items: [item] };
const result = storage => ({ version: 1, source: 'persisted snapshot', storage });
function fixture(slot = 0) {
  let identity = 'session'; let responder = async () => result(inventory); const calls = [];
  const context = { exports: {}, require: () => ({ displayTime: value => value }), document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') } };
  vm.runInNewContext(compiled, context);
  const disclosure = new Element('details'), items = new Element('ul'), summary = new Element(), saved = new Element(), status = new Element(), refresh = new Element('button');
  const controller = new context.exports.StorageView(disclosure, items, summary, saved, status, refresh,
    async path => { calls.push(path); return responder(path); }, () => identity);
  controller.select(slot);
  return { controller, disclosure, items, summary, saved, status, refresh, calls,
    respond: value => { responder = value; }, identity: value => { identity = value; },
    open: async () => { disclosure.open = true; await controller.load(); } };
}

test('storage reads are lazy and render safe names, server quantities and occupied slots only', async () => {
  const f = fixture(); await f.controller.load(); assert.equal(f.calls.length, 0);
  await f.open(); assert.equal(f.calls[0], '/characters/0/inventory'); assert.equal(f.items.children.length, 1);
  const tile = f.items.children[0]; assert.equal(tile.children[0].textContent, 'Slot 63');
  assert.equal(tile.children[1].textContent, item.name); assert.equal(tile.children[1].href, '/database/items/412/');
  assert.equal(tile.children[2].textContent, '×255'); assert.equal(f.summary.textContent, '1 / 63 slots');
  assert.match(f.saved.textContent, /Saved:/); assert.equal(f.refresh.disabled, false);
  const mount = { ...item, slot: 0, itemId: 2330, name: '', quantity: 1 };
  f.respond(async () => result({ ...inventory, items: [mount] })); await f.controller.load();
  assert.equal(f.items.children[0].children[2].textContent, '×1');
  assert.equal(f.items.children[0].children[1].tag, 'span');
});

test('Cargo supports all visible slots, unsigned gold and empty snapshots', async () => {
  const f = fixture(null);
  const cargo = { capacity: 120, gold: 4000000000, asOf: inventory.asOf, items: [{ ...item, slot: 119 }] };
  f.respond(async () => result(cargo)); await f.open();
  assert.equal(f.calls[0], '/cargo'); assert.equal(f.items.children[0].children[0].textContent, 'Slot 120');
  assert.match(f.summary.textContent, /4,000,000,000/);
  f.respond(async () => result({ ...cargo, gold: 0, items: [] })); await f.controller.load();
  assert.equal(f.items.children.length, 0); assert.equal(f.status.textContent, 'No items.');
  assert.equal(f.summary.textContent, '0 / 120 slots · Gold 0');
});

test('selection, collapse and sign-out discard old replies and clear private DOM', async () => {
  for (const action of ['select', 'collapse', 'logout', 'identity']) {
    const f = fixture(); let complete;
    f.respond(() => new Promise(resolve => { complete = resolve; }));
    const pending = f.open();
    if (action === 'select') f.controller.select(1);
    if (action === 'collapse') { f.disclosure.open = false; f.disclosure.handlers.toggle(); }
    if (action === 'logout') f.controller.clear();
    if (action === 'identity') f.identity('other-account');
    complete(result(inventory)); await pending;
    assert.equal(f.items.children.length, 0); assert.equal(f.summary.textContent, ''); assert.equal(f.saved.textContent, '');
  }
  const f = fixture(); let old, latest;
  f.respond(path => new Promise(resolve => { if (path.includes('/0/')) old = resolve; else latest = resolve; }));
  const previous = f.open(); f.controller.select(1); const current = f.open();
  latest(result({ ...inventory, characterSlot: 1, items: [] })); await current;
  old(result(inventory)); await previous;
  assert.equal(f.items.children.length, 0); assert.equal(f.status.textContent, 'No items.');
});

test('storage failures and malformed responses clear old data without exposing raw errors', async () => {
  const f = fixture(); await f.open();
  for (const storage of [null, { ...inventory, characterSlot: 1 }, { ...inventory, gold: 1 }, { ...inventory, capacity: 64 },
    { ...inventory, asOf: 'invalid' }, { ...inventory, items: [item, item] },
    ...[{ ...item, slot: 63 }, { ...item, quantity: 0 }, { ...item, quantity: 256 }, { ...item, itemId: '/attack' },
      { ...item, effects: [61, 255] }, { ...item, name: null }].map(invalid => ({ ...inventory, items: [invalid] }))]) {
    f.respond(async () => { if (!storage) throw new Error('private failure'); return result(storage); });
    await f.controller.load(); assert.equal(f.items.children.length, 0); assert.equal(f.summary.textContent, '');
    assert.equal(f.saved.textContent, ''); assert.match(f.status.textContent, /Unable to load saved items/);
    assert.equal(f.status.textContent.includes('private'), false); assert.equal(f.refresh.disabled, false);
  }
  const before = f.calls.length; f.controller.select(4); await f.open(); f.controller.select(NaN); await f.open();
  f.controller.select(0); f.identity(undefined); await f.open(); assert.equal(f.calls.length, before);
});
