import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const compile = name => ts.transpileModule(readFileSync(new URL(`../src/scripts/${name}.ts`, import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;

function node() {
  return { value: '', disabled: false, textContent: '', className: '', colSpan: 0, children: [], handlers: {},
    append(...items) { this.children.push(...items); }, replaceChildren(...items) { this.children = items; },
    scrollIntoView() {}, addEventListener(event, fn) { this.handlers[event] = fn; } };
}

function load(name, extra = {}) {
  const elements = new Map();
  const context = { exports: {}, URLSearchParams, Date, Error, Promise, Number, String, Object,
    document: { getElementById: id => { if (!elements.has(id)) elements.set(id, node()); return elements.get(id); }, createElement: () => node() },
    require: module => extra[module], ...extra.globals };
  vm.runInNewContext(compile(name), context);
  return { exports: context.exports, element: id => context.document.getElementById(id) };
}

test('Saved item lines show slot names, catalog names and only non-zero effects', () => {
  const { exports } = load('player-storage');
  assert.equal(exports.itemLine({ slot: 6, itemId: 801, name: 'Sword of Test', effects: [43, 5, 0, 0, 0, 0] }, 'Weapon'), 'Weapon · Sword of Test (801) · effects 43:5');
  assert.equal(exports.itemLine({ slot: 0, itemId: 400, name: '', effects: [0, 0, 0, 0, 0, 0] }), '#1 · item 400 (400)');
});

test('Log queries keep a fixed window and add the cursor only for older pages', () => {
  const { exports } = load('server-logs');
  const since = new Date('2026-10-22T03:00:00Z'), until = new Date('2026-10-22T04:00:00Z');
  const first = exports.logQuery('game', since, until, '6', '');
  assert.equal(first.toString(), 'source=game&since=2026-10-22T03%3A00%3A00.000Z&until=2026-10-22T04%3A00%3A00.000Z&priority=6&limit=200');
  const older = exports.logQuery('game', since, until, '3', 'LOGIN', 'c=1');
  assert.equal(older.get('query'), 'LOGIN'); assert.equal(older.get('cursor'), 'c=1'); assert.equal(older.get('since'), first.get('since'));
  assert.equal(exports.levels[3], 'error');
});

test('Account page shows login history even when staff actions are forbidden', async () => {
  const audit = load('audit-log').exports;
  const page = load('account-page', { './audit-log': audit });
  const calls = [];
  const controller = page.exports.setupAccountPage(async path => {
    calls.push(path);
    if (path.endsWith('/actions')) throw new Error('forbidden');
    if (path.endsWith('/notes')) return { version: 1, asOf: '2026-10-22T04:00:00Z', account: 'fixture', notes: [{ id: 1, at: '2026-10-22T03:30:00Z', actor: 'operator', body: 'Checked report', bugRefs: ['BUG-004'] }] };
    return { version: 1, asOf: '2026-10-22T04:00:00Z', account: 'fixture', ipVisible: false, logins: [{ at: '2026-10-22T03:00:00Z', clientIp: '', channel: 1 }] };
  }, () => 'Not allowed.');
  controller.open('Fixture', { characters: [{ slot: 0, name: 'Hero', class: 1, level: 120, evolution: '' }] });
  await new Promise(resolve => setImmediate(resolve));
  assert.deepEqual(calls, ['/staff/accounts/fixture/logins', '/staff/accounts/fixture/actions', '/staff/accounts/fixture/notes']);
  assert.match(page.element('account-notes').children[0].textContent, /operator · BUG-004 — Checked report/);
  const logins = page.element('account-logins').children;
  assert.equal(logins.length, 1); assert.equal(logins[0].children[2].textContent, 'hidden');
  assert.equal(page.element('account-actions').children[0].children[0].textContent, 'Not allowed.');
  assert.equal(page.element('account-characters').children[0].children[1].textContent, 'Hero');
  assert.match(page.element('account-logins-note').textContent, /moderation\.account\.ip/);
});
