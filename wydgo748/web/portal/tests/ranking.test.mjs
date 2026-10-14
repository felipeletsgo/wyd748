import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';
import ts from 'typescript';

const source = readFileSync(new URL('../src/lib/ranking.ts', import.meta.url), 'utf8');
const compiled = ts.transpileModule(source, { compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 } }).outputText;
const flush = () => new Promise(resolve => setImmediate(resolve));
class Element {
  children = []; handlers = {}; dataset = {}; attributes = {}; value = ''; textContent = ''; disabled = false;
  constructor(tag = 'div') { this.tag = tag; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.children = []; this.append(...children); }
  addEventListener(event, handler) { this.handlers[event] = handler; }
  setAttribute(key, value) { this.attributes[key] = value; }
}
const rows = Array.from({ length: 100 }, (_, index) => ({ name: `Player${String(index).padStart(3, '0')}`, class: index % 4, level: 100 - index, evolution: index % 2 ? 'arch' : 'mortal' }));
function setup(url = 'https://example.test/ranking/', responses = []) {
  const elements = new Map();
  const root = new Element(); root.querySelector = selector => {
    if (!elements.has(selector)) elements.set(selector, new Element());
    return elements.get(selector);
  };
  const state = { url: new URL(url), requests: [], events: {} };
  const exports = {};
  vm.runInNewContext(compiled, { exports, URL, URLSearchParams, AbortSignal,
    location: { get search() { return state.url.search; }, get href() { return state.url.href; } },
    history: { replaceState(_state, _unused, next) { state.url = new URL(next); } },
    window: { addEventListener(event, handler) { state.events[event] = handler; } },
    document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') },
    fetch: async (path, options) => { state.requests.push({ path, options }); const response = responses.shift(); if (response instanceof Error) throw response; return response; },
  });
  return { exports, state, root, element: selector => root.querySelector(selector) };
}
const response = (characters = rows, extra = {}) => ({ ok: true, json: async () => ({ version: 1, asOf: '2026-10-12T12:00:00Z', characters, ...extra }) });
const visible = harness => harness.element('#ranking-characters').children.map(row => row.children.map(cell => cell.textContent));

test('ranking filters reject unsupported values and cap query length', () => {
  const { exports } = setup();
  const filters = exports.readFilters(new URLSearchParams({ q: 'a'.repeat(40), class: '4', evolution: '__proto__', page: 'Infinity' }));
  assert.equal(filters.search.length, 24); assert.equal(filters.class, ''); assert.equal(filters.evolution, ''); assert.equal(filters.page, 1);
  assert.equal(exports.readFilters(new URLSearchParams('class=0&page=5')).class, '0');
  for (const page of ['0', '-1', '1.5', '6', 'NaN']) assert.equal(exports.readFilters(new URLSearchParams({ page })).page, 1);
});

test('ranking filters the published top 100 without renumbering global ranks', () => {
  const { exports } = setup();
  const filters = exports.readFilters(new URLSearchParams('class=0&page=2'));
  const page = exports.rankingPage([...rows, { ...rows[0], name: 'Outside' }], filters);
  assert.equal(page.total, 25); assert.equal(page.pages, 2); assert.equal(page.rows[0].rank, 81);
  const search = exports.rankingPage(rows, { ...filters, search: ' PLAYER008 ', page: 5 });
  assert.equal(search.page, 1); assert.equal(search.rows[0].rank, 9);
  assert.equal(exports.rankingPage(rows, { ...filters, evolution: 'arch' }).total, 0);
});

test('ranking navigation and filters are local, shareable, and preserve unrelated URL fields', async () => {
  const harness = setup('https://example.test/ranking/?page=2&ref=invite#list', [response()]);
  harness.exports.mountRanking(harness.root); await flush();
  assert.equal(visible(harness)[0][0], '21'); assert.equal(visible(harness).length, 20);
  assert.equal(harness.state.requests[0].options.credentials, 'omit');
  harness.element('#ranking-next').handlers.click(); assert.equal(visible(harness)[0][0], '41');
  harness.element('#ranking-class').value = '0'; harness.element('#ranking-class').handlers.change();
  assert.equal(harness.state.url.searchParams.get('class'), '0'); assert.equal(harness.state.url.searchParams.has('page'), false);
  harness.element('#ranking-search').value = 'Player008'; harness.element('#ranking-search').handlers.input();
  assert.equal(visible(harness)[0][0], '9'); assert.equal(visible(harness)[0][3], '93');
  assert.equal(harness.state.url.searchParams.get('ref'), 'invite'); assert.equal(harness.state.url.hash, '#list');
  harness.element('#ranking-reset').handlers.click(); assert.equal(visible(harness).length, 20);
  harness.state.url = new URL('https://example.test/ranking/?page=5'); harness.state.events.popstate();
  assert.equal(visible(harness)[0][0], '81'); assert.equal(harness.element('#ranking-next').disabled, true);
  assert.equal(harness.state.requests.length, 1);
});

test('ranking treats character names as text and distinguishes no matches from empty data', async () => {
  const harness = setup(undefined, [response([{ name: '<img onerror=alert(1)>', class: 200, level: 0, evolution: 'new' }]), response([])]);
  harness.exports.mountRanking(harness.root); await flush();
  assert.equal(visible(harness)[0][1], '<img onerror=alert(1)>'); assert.equal(visible(harness)[0][2], 'Unknown');
  harness.element('#ranking-search').value = 'missing'; harness.element('#ranking-search').handlers.input();
  assert.equal(visible(harness)[0][0], 'No matches in the top 100.');
  harness.element('#ranking-refresh').handlers.click(); await flush();
  assert.equal(visible(harness)[0][0], 'No ranked characters yet.');
});

test('failed refresh clears stale rows, offers retry and never exposes server errors', async () => {
  const harness = setup(undefined, [response(), new Error('private server detail'), response()]);
  harness.exports.mountRanking(harness.root); await flush();
  harness.element('#ranking-refresh').handlers.click(); await flush();
  assert.equal(visible(harness)[0][0], 'Ranking unavailable.'); assert.equal(harness.element('fieldset').disabled, true);
  assert.equal(harness.element('#ranking-refresh').disabled, false); assert.equal(harness.element('#ranking-status').dataset.error, 'true');
  assert.doesNotMatch(harness.element('#ranking-status').textContent, /private/);
  harness.element('#ranking-refresh').handlers.click(); await flush(); assert.equal(visible(harness).length, 20);
});

test('ranking rejects malformed or incompatible snapshots', async () => {
  for (const invalid of [response(rows, { version: 2 }), response(rows, { asOf: null }), response([...rows, rows[0]]), response([{ ...rows[0], level: -1 }]), response([{ ...rows[0], class: 1.5 }]), { ok: false }]) {
    const harness = setup(undefined, [invalid]); harness.exports.mountRanking(harness.root); await flush();
    assert.equal(visible(harness)[0][0], 'Ranking unavailable.'); assert.equal(harness.root.attributes['aria-busy'], 'false');
  }
});

test('concurrent refresh clicks share one in-flight request', async () => {
  let complete;
  const pending = new Promise(resolve => { complete = resolve; });
  const harness = setup(undefined, [pending]); harness.exports.mountRanking(harness.root);
  harness.element('#ranking-refresh').handlers.click(); assert.equal(harness.state.requests.length, 1);
  assert.equal(harness.element('fieldset').disabled, true);
  complete(response()); await flush(); assert.equal(harness.element('fieldset').disabled, false);
});
