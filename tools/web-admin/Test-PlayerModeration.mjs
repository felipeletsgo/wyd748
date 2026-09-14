// Focused DOM-state tests; does not claim browser rendering or in-game validation.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createRequire } from 'node:module';
import { runInNewContext } from 'node:vm';
const require = createRequire(new URL('../../wydgo748/web/portal/package.json', import.meta.url));
const ts = require('typescript');
const source = readFileSync(new URL('../../wydgo748/web/portal/src/scripts/player-moderation.ts', import.meta.url), 'utf8');
const code = ts.transpileModule(source, { compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 } }).outputText;
const flush = () => new Promise(resolve => setImmediate(resolve));
function fixture(api, confirm = true) {
  const nodes = new Map();
  const node = id => {
    if (!nodes.has(id)) nodes.set(id, { value: '', textContent: '', disabled: false, hidden: false,
      handlers: {}, addEventListener(event, fn) { this.handlers[event] = fn; },
      reportValidity() { return true; }, scrollIntoView() {}, focus() {},
    });
    return nodes.get(id);
  };
  const exports = {};
  runInNewContext(code, { exports, document: { getElementById: node }, window: { confirm: () => confirm }, crypto: { randomUUID: () => 'operation-0000001' } });
  const panel = exports.setupPlayerModeration(api, () => 'Falha HTTP.');
  const player = { name: 'Knight', uid: 'char-1', account: 'Player', sessionId: 77 };
  const epoch = 'a'.repeat(32);
  const select = () => panel.select(player, epoch);
  const submit = () => { node('kick-reason').value = 'moderation test'; node('kick-form').handlers.submit({ preventDefault() {} }); };
  return { panel, node, player, epoch, select, submit };
}
test('confirmation sends the selected identity and consumes matching receipt', async () => {
  let call;
  const f = fixture(async (path, method, body) => { call = { path, method, body }; return { code: 'ok', operationId: body.operationId, replayed: false }; });
  f.select(); f.submit(); await flush();
  assert.equal(call.path, '/staff/players/char-1/kick');
  assert.equal(call.method, 'POST');
  assert.equal(call.body.expectedSessionId, 77);
  assert.equal(call.body.account, 'player');
  assert.equal(call.body.epoch, f.epoch);
  assert.equal(f.node('kick-submit').disabled, true);
  assert.equal(f.node('kick-retry').hidden, true);
  assert.match(f.node('kick-message').textContent, /Jogador desconectado/);
});
test('cancelled confirmation sends nothing', async () => {
  let calls = 0;
  const f = fixture(async () => { calls++; }, false);
  f.select(); f.submit(); await flush(); assert.equal(calls, 0);
});
test('uncertain retry retains exact target/body and blocks target replacement', async () => {
  const bodies = [];
  const f = fixture(async (_path, _method, body) => {
    bodies.push(JSON.stringify(body));
    if (bodies.length === 1) throw new Error('timeout after commit');
    return { code: 'ok', operationId: body.operationId, replayed: true };
  });
  f.select(); f.submit(); await flush();
  assert.equal(f.node('kick-retry').hidden, false);
  assert.equal(f.node('kick-reason').disabled, true);
  f.panel.select({ ...f.player, uid: 'another', sessionId: 99 }, f.epoch);
  f.node('kick-retry').handlers.click(); await flush();
  assert.equal(bodies.length, 2); assert.equal(bodies[0], bodies[1]);
  assert.match(f.node('kick-message').textContent, /Recibo recuperado/);
});
test('logout discards late response without altering new session controls', async () => {
  let resolve;
  const f = fixture(() => new Promise(r => { resolve = r; }));
  f.select(); f.submit(); f.panel.reset(); f.select();
  resolve({ code: 'ok', operationId: 'operation-0000001', replayed: false }); await flush();
  assert.equal(f.node('kick-message').textContent, '');
  assert.equal(f.node('kick-submit').disabled, false);
});
test('mismatched receipt remains uncertain and recoverable', async () => {
  const f = fixture(async () => ({ code: 'ok', operationId: 'different-operation', replayed: false }));
  f.select(); f.submit(); await flush();
  assert.equal(f.node('kick-retry').hidden, false);
  assert.match(f.node('kick-message').textContent, /Resultado não confirmado/);
});
test('persistence failure is not reported as successful saving', async () => {
  const f = fixture(async (_path, _method, body) => ({ code: 'disconnected_persistence_failed', operationId: body.operationId, replayed: false }));
  f.select(); f.submit(); await flush();
  assert.match(f.node('kick-message').textContent, /falha ao salvar/);
  assert.equal(f.node('kick-retry').hidden, true);
});
test('older overview cannot enable moderation', () => {
  const f = fixture(async () => { throw new Error('unexpected command'); });
  f.panel.reset(); f.panel.select(f.player);
  assert.equal(f.node('kick-submit').disabled, true);
});
