import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';
import ts from 'typescript';

const source = readFileSync(new URL('../src/lib/guilds.ts', import.meta.url), 'utf8');
const compiled = ts.transpileModule(source, { compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 } }).outputText;
const flush = () => new Promise(resolve => setImmediate(resolve));
class Element {
  children = []; handlers = {}; dataset = {}; attributes = {}; value = ''; textContent = ''; disabled = false;
  constructor(tag = 'div') { this.tag = tag; }
  get childNodes() { return this.children; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.children = []; this.append(...children); }
  addEventListener(event, handler) { this.handlers[event] = handler; }
  setAttribute(key, value) { this.attributes[key] = value; }
}
function setup(responses = []) {
  const elements = new Map(); const requests = []; const exports = {};
  const root = new Element(); root.querySelector = selector => {
    if (!elements.has(selector)) elements.set(selector, new Element()); return elements.get(selector);
  };
  vm.runInNewContext(compiled, { exports, AbortSignal,
    document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') },
    fetch: async (path, options) => { requests.push({ path, options }); const reply = responses.shift(); if (reply instanceof Error) throw reply; return reply; },
  });
  return { exports, requests, root, element: selector => root.querySelector(selector) };
}
const response = (text, size = null) => ({ ok: true, headers: { get: () => size }, text: async () => text });
const visible = harness => harness.element('#guilds-list').children.map(row => row.children[0].children[0]?.textContent ?? row.children[0].textContent);

test('guild directory parses the current public export and preserves names with spaces', () => {
  const { exports } = setup();
  const guilds = exports.parseGuildDirectory('0 0 4095 Zebra\r\n0 0 1 Alpha Team\r\n');
  assert.deepEqual(JSON.parse(JSON.stringify(guilds)), [{ id: 1, name: 'Alpha Team' }, { id: 4095, name: 'Zebra' }]);
  assert.equal(exports.parseGuildDirectory('').length, 0);
});
test('guild directory rejects malformed, oversized, duplicate and foreign-channel exports', () => {
  const { exports } = setup();
  for (const value of ['0 0 0 Invalid', '0 0 4096 Invalid', '0 0 1 ThirteenChars', '0 0 1 <script>', '0 1 1 Other', '1 0 1 Other', '0 0 1 Name ', '0 0 1 Alpha\n0 0 1 Beta', '0 0 1 Alpha\n0 0 2 ALPHA', ' '.repeat(131073)]) {
    assert.throws(() => exports.parseGuildDirectory(value), /Invalid guild directory/);
  }
});
test('guild directory loads without credentials, searches and paginates twenty names', async () => {
  const text = Array.from({ length: 25 }, (_, index) => `0 0 ${index + 1} Guild${String(index).padStart(2, '0')}`).join('\n');
  const h = setup([response(text)]); h.exports.mountGuilds(h.root); await flush();
  assert.equal(h.requests[0].path, '/patch/guilds.txt'); assert.equal(h.requests[0].options.credentials, 'omit');
  assert.equal(visible(h).length, 20); h.element('#guilds-next').handlers.click();
  assert.equal(h.element('#guilds-list').children[0].children[0].children[0].href, '/guilds/Guild20/');
  assert.equal(visible(h)[0], 'Guild20'); assert.equal(visible(h).length, 5);
  h.element('#guilds-search').value = 'guild03'; h.element('#guilds-search').handlers.input();
  assert.deepEqual(visible(h), ['Guild03']); assert.equal(h.element('#guilds-previous').disabled, true);
  h.element('#guilds-search').value = 'absent'; h.element('#guilds-search').handlers.input();
  assert.deepEqual(visible(h), ['No matching guilds.']); assert.equal(h.requests.length, 1);
});
test('empty directory is distinct from failure; failed refresh clears stale names and permits retry', async () => {
  const h = setup([response('0 0 1 Alpha'), new Error('private error'), response('')]);
  h.exports.mountGuilds(h.root); await flush(); assert.deepEqual(visible(h), ['Alpha']);
  h.element('#guilds-refresh').handlers.click(); await flush();
  assert.deepEqual(visible(h), ['Guild directory unavailable.']); assert.equal(h.element('#guilds-search').disabled, true);
  assert.equal(h.element('#guilds-refresh').disabled, false); assert.doesNotMatch(h.element('#guilds-status').textContent, /private/);
  h.element('#guilds-refresh').handlers.click(); await flush(); assert.deepEqual(visible(h), ['No guilds yet.']);
  assert.equal(h.root.attributes['aria-busy'], 'false'); assert.equal(h.element('#guilds-search').disabled, false);
});
test('invalid and oversized replies fail closed; concurrent refresh uses one request', async () => {
  for (const reply of [{ ok: false }, response('0 0 1 <img>'), response('', '131073')]) {
    const h = setup([reply]); h.exports.mountGuilds(h.root); await flush();
    assert.deepEqual(visible(h), ['Guild directory unavailable.']); assert.equal(h.element('#guilds-search').disabled, true);
  }
  let complete; const pending = new Promise(resolve => { complete = resolve; }); const h = setup([pending]);
  h.exports.mountGuilds(h.root); h.element('#guilds-refresh').handlers.click(); assert.equal(h.requests.length, 1);
  complete(response('0 0 1 Alpha')); await flush(); assert.deepEqual(visible(h), ['Alpha']);
});
