import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const load = name => {
  const context = { exports: {}, Date, Math, Set, require: () => ({ auditSummary: () => '' }) };
  vm.runInNewContext(ts.transpileModule(readFileSync(new URL(`../src/scripts/${name}.ts`, import.meta.url), 'utf8'), {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
  }).outputText, context);
  return context.exports;
};

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
