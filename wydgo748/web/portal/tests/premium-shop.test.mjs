import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const source = ts.transpileModule(readFileSync(new URL('../src/scripts/premium-shop.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
const snapshot = { version: 1, available: true, catalogVersion: 7, updatedBy: '', updatedAt: '',
  items: [{ slot: 26, itemId: 3394, effects: [0, 0, 0, 0, 0, 0], price: 1000, category: 2 }] };
const flush = () => new Promise(resolve => setImmediate(resolve));
function fixture(lang = 'en', initial = snapshot) {
  const nodes = new Map(), calls = [];
  const element = () => ({ children: [], value: '', textContent: '', handlers: {},
    append(...children) { this.children.push(...children); },
    replaceChildren(...children) { this.children = children; },
    setAttribute(name, value) { this[name] = value; },
    addEventListener(name, handler) { this.handlers[name] = handler; }, reportValidity: () => true,
  });
  const node = id => { if (!nodes.has(id)) nodes.set(id, element()); return nodes.get(id); };
  const context = { exports: {}, document: { documentElement: { lang }, getElementById: node, createElement: element },
    window: { confirm: () => true }, crypto: { randomUUID: () => 'credit-id' } };
  vm.runInNewContext(source, context);
  const controller = context.exports.setupPremiumShop(async (path, method, body) => {
    calls.push({ path, method, body });
    return method === 'PUT' ? { code: 'ok', catalog: { ...initial, items: body.items, catalogVersion: 8 } } : initial;
  }, () => 'Request failed');
  return { controller, node, calls };
}

test('staff edits the category without changing item, cell, effects or price', async () => {
  const f = fixture(); await f.controller.refresh();
  const row = f.node('premium-rows').children[0];
  const select = row.children[3].children[0];
  assert.deepEqual(select.children.map(option => option.textContent), ['Featured', 'Armia', 'Erion', 'Nifleheim']);
  assert.equal(select.value, '2');
  select.value = '3'; select.handlers.change();
  f.node('premium-reason').value = 'Move to Nifleheim';
  f.node('premium-form').handlers.submit({ preventDefault() {} }); await flush();
  assert.equal(f.calls[1].method, 'PUT');
  const command = f.calls[1].body;
  assert.equal(command.expectedCatalogVersion, 7);
  assert.equal(JSON.stringify(command.items), JSON.stringify([{ slot: 26, itemId: 3394, effects: [0, 0, 0, 0, 0, 0], price: 1000, category: 3 }]));
  assert.equal(f.node('premium-rows').children[0].children[3].children[0].value, '3');
});

test('existing items without category and new items default to Featured in both locales', async () => {
  for (const lang of ['en', 'pt-BR']) {
    const f = fixture(lang, { ...snapshot, items: [{ ...snapshot.items[0], category: undefined }] });
    await f.controller.refresh();
    assert.equal(f.node('premium-category-heading').textContent, lang === 'en' ? 'Category' : 'Categoria');
    assert.equal(f.node('premium-rows').children[0].children[3].children[0].value, '0');
    f.node('premium-add').handlers.click();
    const select = f.node('premium-rows').children[1].children[3].children[0];
    assert.equal(select.value, '0');
    assert.equal(select.children[0].textContent, lang === 'en' ? 'Featured' : 'Destaque');
  }
});
