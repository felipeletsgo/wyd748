import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const load = name => {
  const context = { exports: {} };
  vm.runInNewContext(ts.transpileModule(readFileSync(new URL(`../src/scripts/${name}.ts`, import.meta.url), 'utf8'), {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
  }).outputText, context);
  return context.exports;
};

test('Player filters mirror the server bounds and combinations', () => {
  const { playerFilters } = load('player-filters');
  assert.deepEqual({ ...playerFilters({ minLevel: ' 10 ', class: '0', x: '', guild: '' }).filters }, { minLevel: '10', class: '0' });
  assert.deepEqual({ ...playerFilters({ x: '2100', y: '2100', radius: '16' }).filters }, { x: '2100', y: '2100', radius: '16' });
  for (const bad of [{ minLevel: '0' }, { class: '256' }, { guild: '1.5' }, { x: '1', y: '1' }, { radius: '3' }, { minLevel: '30', maxLevel: '20' }]) {
    assert.ok(playerFilters(bad).error, JSON.stringify(bad));
  }
});

test('Sparkline scales into the box and keeps flat zero series on the baseline', () => {
  const { sparkline, charts } = load('runtime-charts');
  assert.equal(sparkline([], 240, 46), '');
  assert.equal(sparkline([0, 10, 5], 240, 46), '0.0,46.0 120.0,0.0 240.0,23.0');
  assert.equal(sparkline([0, 0], 240, 46), '0.0,46.0 240.0,46.0');
  const lag = charts.find(chart => chart.title === 'Loop lag p99');
  assert.equal(lag.format(1000001), '>1000 ms');
  assert.equal(lag.format(5000), '5 ms');
});

test('Diagnostic charts read transport, pool, persistence and channel groups', () => {
  const { charts } = load('runtime-charts');
  const value = title => charts.find(chart => chart.title === title).value;
  const point = {
    transport: { closes: { output_overflow: 2, peer_closed: 5 }, waitP99Micros: 25000 },
    pool: { acquireMeanMicros: 900 }, critical: { pending: 3 },
    channels: [{ heartbeatAgeMicros: 400 }, { heartbeatAgeMicros: 1200 }],
  };
  assert.equal(value('Disconnects')(point), 7);
  assert.equal(value('Output queue overflows')(point), 2);
  assert.equal(value('Output queue wait p99')(point), 25000);
  assert.equal(value('Database pool wait (mean)')(point), 900);
  assert.equal(value('Critical writes pending')(point), 3);
  assert.equal(value('Game loop heartbeat age')(point), 1200);
  // Points recorded before an upgrade have no diagnostic groups.
  for (const chart of charts.slice(6)) assert.equal(chart.value({}), 0, chart.title);
});
