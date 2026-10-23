import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const load = (name, globals = {}) => {
  const context = { exports: {}, Date, Math, Set, require: () => ({ auditSummary: () => '' }), ...globals };
  vm.runInNewContext(ts.transpileModule(readFileSync(new URL(`../src/scripts/${name}.ts`, import.meta.url), 'utf8'), {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
  }).outputText, context);
  return context.exports;
};

const deferred = () => {
  let resolve, reject;
  const promise = new Promise((yes, no) => { resolve = yes; reject = no; });
  return { promise, resolve, reject };
};
const flush = () => new Promise(resolve => setImmediate(resolve));
function panelFixture(name) {
  const elements = new Map(), requests = [], images = [];
  const drawing = { clearRect() {}, drawImage() {}, strokeRect() {}, fillRect() {} };
  const makeElement = () => ({ value: '16', textContent: '', disabled: false, children: [], handlers: {}, width: 1024, height: 1024,
    append(...children) { this.children.push(...children); },
    replaceChildren(...children) { this.children = children; },
    addEventListener(type, handler) { this.handlers[type] = handler; },
    getContext: () => drawing, scrollIntoView() {},
  });
  const element = id => {
    if (!elements.has(id)) elements.set(id, makeElement());
    return elements.get(id);
  };
  const document = { getElementById: element, createElement: makeElement };
  const module = load(name, { document, URLSearchParams, Image: class { constructor() { images.push(this); } } });
  const controller = module[name === 'world-map' ? 'setupWorldMap' : 'setupInstances'](path => {
    const request = { path, ...deferred() }; requests.push(request); return request.promise;
  }, () => 'Request failed.');
  return { controller, element, requests, images };
}
const zone = { x: 1342, y: 1466, terrainLoaded: false, teleports: [], questZones: [], clearAreas: [], instances: [] };
const snapshot = name => ({ x: 1342, y: 1466, radius: 16, asOf: '2026-10-22T12:00:00Z', mobs: [],
  players: [{ uid: '1', name, account: `${name}-account`, level: 100, x: 1342, y: 1466 }] });
const instanceView = name => ({ asOf: '2026-10-22T12:00:00Z', instances: [{ name, runtimeId: 'private-instance',
  members: [{ name: `${name}-member`, leader: true }], stage: 1, mobs: 2, x: 1342, y: 1466 }] });
const content = element => [element.textContent, ...element.children.map(content)].join(' ');

for (const outcome of ['success', 'failure']) {
  test(`Map ignores old-session ${outcome} without releasing a new request`, async () => {
    const f = panelFixture('world-map');
    f.controller.focus(1342, 1466);
    const old = f.requests.slice();
    f.controller.reset();
    f.controller.focus(1400, 1500);
    if (outcome === 'success') { old[0].resolve(zone); old[1].resolve(snapshot('OldPlayer')); }
    else old.forEach(request => request.reject(new Error('Expired session')));
    await flush();
    assert.equal(content(f.element('world-zone')).trim(), '');
    assert.equal(content(f.element('world-live')).trim(), '');
    assert.equal(f.element('world-status').textContent, 'Reading…');
    f.controller.focus(1500, 1600);
    assert.equal(f.requests.length, 4, 'old completion released the new request guard');
    f.requests[2].resolve(zone); f.requests[3].resolve(snapshot('NewPlayer')); await flush();
    assert.match(content(f.element('world-live')), /NewPlayer-account/);
    assert.doesNotMatch(content(f.element('world-live')), /OldPlayer/);
  });

  test(`Instances ignore old-session ${outcome} and keep the new refresh disabled`, async () => {
    const f = panelFixture('instances');
    const old = f.controller.refresh();
    f.controller.reset();
    assert.equal(f.element('instances-refresh').disabled, false);
    const current = f.controller.refresh();
    if (outcome === 'success') f.requests[0].resolve(instanceView('OldInstance'));
    else f.requests[0].reject(new Error('Expired session'));
    await old;
    assert.match(content(f.element('instances-rows')), /Not queried/);
    assert.equal(f.element('instances-status').textContent, '');
    assert.equal(f.element('instances-refresh').disabled, true);
    await f.controller.refresh();
    assert.equal(f.requests.length, 2);
    f.requests[1].resolve(instanceView('NewInstance')); await current;
    assert.match(content(f.element('instances-rows')), /NewInstance-member/);
    assert.doesNotMatch(content(f.element('instances-rows')), /OldInstance/);
    assert.equal(f.element('instances-refresh').disabled, false);
  });
}

test('Map ignores terrain image callbacks from an expired session', () => {
  const f = panelFixture('world-map');
  f.element('world-load').handlers.click();
  f.controller.reset();
  f.images[0].onload(); f.images[0].onerror();
  assert.equal(f.element('world-status').textContent, 'Terrain not loaded.');
  f.element('world-load').handlers.click(); f.images[1].onload();
  assert.match(f.element('world-status').textContent, /Terrain loaded/);
});

test('Map pixels and cells convert with the 4x4 attribute scale', () => {
  const { toPixel, toCell, zoneRows } = load('world-map');
  assert.equal(toPixel(2103), 525);
  assert.equal(toCell(525, 1), 2100);
  assert.equal(toCell(5000, 1), 4095);
  const rows = zoneRows({ version: 1, x: 1342, y: 1466, terrainLoaded: true, height: -3, attribute: 0x12, walkable: false, blocked: true,
    teleportActivation: true, newbieZone: false, teleports: [{ name: 'Return', destX: 2100, destY: 2100, price: 0 }], questZones: [], clearAreas: [], instances: [] });
  assert.deepEqual([...rows.map(r => r[0])], ['Cell', 'Terrain', 'Movement', 'Flags', 'Portal']);
  assert.match(rows[1][1], /attribute 0x12/);
});

test('Instance deadlines show the earliest upcoming one', () => {
  const { nextDeadline } = load('instances');
  const now = Date.parse('2026-10-22T12:00:00Z');
  const instance = { combatEnds: '2026-10-22T12:05:30Z', transitionAt: null, quizEnds: '2026-10-22T11:59:00Z', exitAt: '2026-10-22T12:01:00Z', hardDeadline: null, scheduleEnd: null };
  assert.equal(nextDeadline(instance, now), 'exit in 1 min 0 s');
  assert.equal(nextDeadline({ ...instance, combatEnds: null, exitAt: null }, now), '');
});

test('Content lines describe drop chances and spawns', () => {
  const { chance, itemLines, mobLines } = load('content-index');
  assert.equal(chance(0), 'every kill'); assert.equal(chance(1500), '1 in 1,500'); assert.equal(chance(80, true), 'every kill');
  assert.deepEqual([...itemLines({ id: 400, name: 'Potion', soldBy: [{ npc: 'Merchant', slot: 2 }], quests: [],
    droppedBy: [{ mob: 'Wolf', slot: 0, oneIn: 80, always: false, baseOneIn: 1000 }, { mob: 'Pup', slot: 11, oneIn: 0, always: true, baseOneIn: 0 }] })],
    ['Sold by Merchant (slot 2)', 'Dropped by Wolf · carry slot 0 · 1 in 80 per kill (base 1 in 1,000)', 'Dropped by Pup · carry slot 11 · every kill']);
  const lines = mobLines({ name: 'Wolf', kind: 'monster', level: 5, gold: 0, merchant: false, sells: [],
    drops: [{ itemId: 400, name: 'Potion', slot: 0, oneIn: 8, baseOneIn: 8 }],
    spawns: [{ generator: 3, enabled: false, role: 'leader', x: 1, y: 2, maxMobs: 4 },
      { generator: 5, enabled: true, reserved: 'item_instance', role: 'both', x: 3, y: 4, maxMobs: 1 }] });
  assert.equal(lines[1], 'Spawns via NPCGener[3] as leader at 1, 2 · up to 4 · disabled');
  assert.equal(lines[2], 'Reserved for item instance (no open-world spawn): NPCGener[5] as leader and follower at 3, 4 · up to 1');
  assert.equal(lines[3], 'Drops Potion (400) · slot 0 · 1 in 8');
});

test('Bug references must be distinct register IDs', () => {
  const page = load('account-page');
  // account-page imports audit-log; only the pure helper is exercised here.
  assert.deepEqual([...page.bugRefs('bug-004, BUG-011')], ['BUG-004', 'BUG-011']);
  assert.equal(page.bugRefs('BUG-4'), undefined);
  assert.equal(page.bugRefs('BUG-004 BUG-004'), undefined);
  assert.deepEqual([...page.bugRefs('')], []);
});
