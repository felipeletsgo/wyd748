import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';
import ts from 'typescript';
const source = readFileSync(new URL('../src/lib/catalog-search.ts', import.meta.url), 'utf8');
const compiled = ts.transpileModule(source, { compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 } }).outputText;
const flush = () => new Promise(resolve => setImmediate(resolve));
const hash = 'a'.repeat(64);
const rows = Array.from({ length: 45 }, (_, id) => ({ id, name: `Skill ${id}`, class: id === 44 ? null : id % 4 }));
class Element {
  children = []; handlers = {}; dataset = {}; attributes = {}; value = ''; textContent = ''; disabled = false;
  constructor(tag = 'div') { this.tag = tag; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.children = []; this.append(...children); }
  addEventListener(event, handler) { this.handlers[event] = handler; }
}
function setup(kind = 'skills', response = { ok: true, json: async () => ({ schemaVersion: 2, contentHash: hash, items: rows, skills: rows, quests: rows }) }, url = 'https://example.test/database/skills/') {
  const root = new Element(); root.dataset = { kind, hash };
  const elements = new Map();
  root.querySelector = selector => {
    if (kind !== 'skills' && selector === '[data-class]') return null;
    if (!elements.has(selector)) elements.set(selector, new Element());
    return elements.get(selector);
  };
  root.querySelector('fieldset').disabled = true;
  root.querySelector('tbody').children = ['static rows'];
  const state = { url: new URL(url), requests: [], events: {} };
  const exports = {};
  vm.runInNewContext(compiled, { exports, URL, URLSearchParams, AbortSignal,
    location: { get search() { return state.url.search; }, get href() { return state.url.href; } },
    history: { replaceState(_state, _unused, url) { state.url = new URL(url); } },
    window: { addEventListener(event, handler) { state.events[event] = handler; } },
    document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') },
    fetch: async (path, options) => { state.requests.push({ path, options }); if (response instanceof Error) throw response; return response; },
  });
  return { exports, root, state, element: selector => root.querySelector(selector) };
}
test('catalog filters handle exact IDs, class zero, shared skills and oversized pages', () => {
  const { exports } = setup();
  const filters = exports.readFilters(new URLSearchParams('q=44&class=shared&page=999'));
  const result = exports.searchPage(rows, filters);
  assert.equal(result.entries[0].id, 44); assert.equal(result.page, 1);
  const zero = exports.searchPage(rows, exports.readFilters(new URLSearchParams('class=0')));
  assert.equal(zero.total, 11); assert.ok(zero.entries.every(row => row.class === 0));
  for (const invalid of ['NaN', '-1', '0', '1.5', '9007199254740992']) assert.equal(exports.readFilters(new URLSearchParams(`page=${invalid}`)).page, 1);
  assert.equal(exports.readFilters(new URLSearchParams('class=__proto__')).selectedClass, '');
  assert.equal(exports.readFilters(new URLSearchParams(`q=${'a'.repeat(100)}`)).search.length, 64);
});
test('catalog search, pagination, clear and back navigation use one anonymous request', async () => {
  const h = setup(); h.exports.mountCatalog(h.root); await flush();
  assert.equal(h.element('tbody').children.length, 20);
  h.element('[data-next]').handlers.click(); assert.match(h.element('[data-page]').textContent, /^2 \/ 3/);
  assert.equal(h.state.url.searchParams.get('page'), '2');
  h.element('[data-search]').value = 'SKILL 44'; h.element('[data-search]').handlers.input();
  assert.equal(h.element('tbody').children.length, 1); assert.equal(h.element('tbody').children[0].children[2].textContent, 'Shared');
  assert.equal(h.element('[data-page]').textContent, '1 / 1 · 1 skill');
  assert.equal(h.element('[data-next]').disabled, true);
  h.element('[data-reset]').handlers.click(); assert.equal(h.element('[data-search]').value, '');
  h.state.url.search = '?class=0&page=99'; h.state.events.popstate();
  assert.equal(h.element('tbody').children.length, 11); assert.equal(h.element('[data-class]').value, '0');
  assert.equal(h.state.requests.length, 1); assert.equal(h.state.requests[0].options.credentials, 'omit');
});
test('item search ignores injected class filters and renders names as text', async () => {
  const malicious = [{ id: 5000, name: '<img src=x onerror=alert(1)>' }];
  const h = setup('items', { ok: true, json: async () => ({ schemaVersion: 2, contentHash: hash, items: malicious }) }, 'https://example.test/database/items/?class=0&source=link#top');
  h.exports.mountCatalog(h.root); await flush();
  const link = h.element('tbody').children[0].children[1].children[0];
  assert.equal(link.textContent, malicious[0].name); assert.equal(link.href, '/database/items/5000/');
  assert.equal(h.state.url.searchParams.has('class'), false); assert.equal(h.state.url.searchParams.get('source'), 'link'); assert.equal(h.state.url.hash, '#top');
  h.element('[data-search]').value = 'absent'; h.element('[data-search]').handlers.input();
  assert.equal(h.element('tbody').children[0].children[0].textContent, 'No matches.');
});
test('catalog failure preserves static links and rejects stale or malformed public data', async () => {
  for (const data of [new Error('private database diagnostic'), { schemaVersion: 1, contentHash: hash, skills: rows }, { schemaVersion: 2, contentHash: 'b'.repeat(64), skills: rows }, { schemaVersion: 2, contentHash: hash, skills: [{ id: -1, name: 'Bad', class: 0 }] }, { schemaVersion: 2, contentHash: hash, skills: [{ id: 0, name: 'Bad', class: 9 }] }, { schemaVersion: 2, contentHash: hash, skills: null }]) {
    const h = setup('skills', data instanceof Error ? data : { ok: true, json: async () => data });
    h.exports.mountCatalog(h.root); await flush();
    assert.equal(h.element('fieldset').disabled, true); assert.deepEqual(h.element('tbody').children, ['static rows']);
    assert.equal(h.element('[data-status]').textContent, 'Search unavailable. Reload to try again.');
  }
});
test('quest search uses quest routes, safe IDs and no class filter', async () => {
  const h = setup('quests', undefined, 'https://example.test/database/quests/?q=44&class=0');
  h.exports.mountCatalog(h.root); await flush();
  assert.equal(h.element('tbody').children.length, 1);
  assert.equal(h.element('tbody').children[0].children[1].children[0].href, '/database/quests/44/');
  assert.equal(h.element('[data-page]').textContent, '1 / 1 · 1 quest');
  assert.equal(h.state.url.searchParams.has('class'), false);
});
