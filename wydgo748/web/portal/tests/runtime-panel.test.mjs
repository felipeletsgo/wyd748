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
