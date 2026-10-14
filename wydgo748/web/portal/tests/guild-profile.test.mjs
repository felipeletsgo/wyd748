import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';
import ts from 'typescript';
const source = readFileSync(new URL('../src/lib/guild-profile.ts', import.meta.url), 'utf8');
const compiled = ts.transpileModule(source, { compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 } }).outputText;
const flush = () => new Promise(resolve => setImmediate(resolve));
class Element {
  children = []; handlers = {}; dataset = {}; attributes = {}; value = ''; textContent = ''; disabled = false; hidden = false;
  constructor(tag = 'div') { this.tag = tag; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.children = []; this.append(...children); }
  addEventListener(event, handler) { this.handlers[event] = handler; }
  setAttribute(key, value) { this.attributes[key] = value; }
}
function setup(responses = [], path = '/guilds/Alpha%20Team/') {
  const elements = new Map(), requests = [], exports = {}, root = new Element();
  root.querySelector = selector => { if (!elements.has(selector)) elements.set(selector, new Element()); return elements.get(selector); };
  vm.runInNewContext(compiled, { exports, AbortSignal, TextEncoder, window: { location: { pathname: path } },
    document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') },
    fetch: async (path, options) => { requests.push({ path, options }); const reply = responses.shift(); if (reply instanceof Error) throw reply; return reply; },
  });
  return { exports, requests, root, element: id => root.querySelector(`#guild-profile-${id}`) };
}
const fixture = () => ({ version: 1, asOf: '2026-10-13T12:00:00Z', source: 'persisted snapshot', guild: {
  name: 'Alpha Team', kingdom: 7, fame: 4294967295, createdAt: '0001-01-01T00:00:00Z', ally: null, warTarget: 'Beta',
  members: [{ name: 'Leader', rank: 9 }, ...Array.from({ length: 24 }, (_, i) => ({ name: `Member${i}`, rank: 0 }))],
} });
const response = value => ({ ok: true, status: 200, headers: { get: () => null }, text: async () => JSON.stringify(value) });
test('public guild profile validates roles, canonical identity and uint32 Fame', () => {
  const h = setup(); assert.equal(h.exports.parseGuildProfile(fixture(), 'alpha team').fame, 4294967295);
  const mutations = [v => v.version++, v => v.guild.name = 'Other', v => v.guild.kingdom = 1, v => v.guild.kingdom = '7', v => v.guild.fame = -1, v => v.guild.fame = 1.5, v => v.guild.fame = 4294967296,
    v => v.guild.members[0].rank = 0, v => v.guild.members[1].rank = 9, v => v.guild.members[1].rank = '0', v => v.guild.members[1].rank = 5,
    v => v.guild.members[1].name = 'Leader', v => v.guild.ally = '<img>', v => v.guild.warTarget = 'Alpha Team', v => delete v.guild.ally, v => v.asOf = 'bad'];
  for (const mutate of mutations) { const v = fixture(); mutate(v); assert.throws(() => h.exports.parseGuildProfile(v, 'Alpha Team'), /Invalid guild profile/); }
});
test('profile loads anonymously, uses text nodes and paginates the roster', async () => {
  const v = fixture(); v.guild.members[1].name = '<img src=x>';
  const h = setup([response(v)]); h.exports.mountGuildProfile(h.root); await flush();
  assert.equal(h.requests[0].path, '/api/player/v1/guilds/Alpha%20Team'); assert.equal(h.requests[0].options.credentials, 'omit'); assert.equal(h.requests[0].options.cache, 'no-store');
  assert.equal(h.element('details').hidden, false); assert.equal(h.element('name').textContent, 'Alpha Team'); assert.equal(h.element('kingdom').textContent, 'Hekalotia'); assert.equal(h.element('fame').textContent, '4,294,967,295');
  assert.equal(h.element('members').children.length, 20); assert.equal(h.element('members').children[1].children[0].textContent, '<img src=x>');
  h.element('next').handlers.click(); assert.equal(h.element('members').children.length, 5);
  h.element('search').value = 'leader'; h.element('search').handlers.input(); assert.equal(h.element('members').children.length, 1); assert.equal(h.element('previous').disabled, true);
  assert.equal(h.element('relations').textContent, 'War target: Beta');
});
test('failed refresh clears old roster and permits recovery; 404 differs from unavailability', async () => {
  const h = setup([response(fixture()), new Error('private backend'), { status: 404 }, response(fixture())]); h.exports.mountGuildProfile(h.root); await flush();
  h.element('refresh').handlers.click(); await flush(); assert.equal(h.element('details').hidden, true); assert.equal(h.element('members').children.length, 0); assert.equal(h.element('fame').textContent, ''); assert.doesNotMatch(h.element('status').textContent, /private/);
  h.element('refresh').handlers.click(); await flush(); assert.equal(h.element('status').textContent, 'Guild not found.');
  h.element('refresh').handlers.click(); await flush(); assert.equal(h.element('details').hidden, false);
});
test('invalid paths never query the server and concurrent refresh coalesces', async () => {
  for (const path of ['/guilds/', '/guilds/%ZZ/', '/guilds/%3Cimg%3E/', '/guilds/A/B/', '/guild-profile/']) { const h = setup([], path); h.exports.mountGuildProfile(h.root); await flush(); assert.equal(h.requests.length, 0); assert.equal(h.element('refresh').disabled, true); }
  let complete; const pending = new Promise(resolve => { complete = resolve; }); const h = setup([pending]); h.exports.mountGuildProfile(h.root); h.element('refresh').handlers.click(); assert.equal(h.requests.length, 1); complete(response(fixture())); await flush(); assert.equal(h.element('details').hidden, false);
});
test('malformed and oversized responses fail closed', async () => {
  for (const reply of [response({}), { ok: false, status: 503 }, { ok: true, status: 200, headers: { get: () => '8388609' } }]) { const h = setup([reply]); h.exports.mountGuildProfile(h.root); await flush(); assert.equal(h.element('details').hidden, true); assert.equal(h.element('refresh').disabled, false); }
});
