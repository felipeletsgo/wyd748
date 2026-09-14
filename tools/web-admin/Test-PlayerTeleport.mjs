// Focused DOM-state tests; does not claim browser rendering or in-game validation.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createRequire } from 'node:module';
import { runInNewContext } from 'node:vm';
const require = createRequire(new URL('../../wydgo748/web/portal/package.json', import.meta.url));
const ts = require('typescript');
const source = readFileSync(new URL('../../wydgo748/web/portal/src/scripts/player-teleport.ts', import.meta.url), 'utf8');
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
  const panel = exports.setupPlayerTeleport(api, () => 'Falha HTTP.');
  const player = { name: 'Knight', uid: 'char-1', account: 'Player', sessionId: 77 };
  const epoch = 'a'.repeat(32);
  const select = () => panel.select(player, epoch);
  const submit = () => { node('teleport-reason').value = 'moderation test'; node('teleport-form').handlers.submit({ preventDefault() {} }); };
  return { panel, node, player, epoch, select, submit };
}
test('confirmation sends the selected identity and consumes matching receipt', async () => {
  let call;
  const f = fixture(async (path, method, body) => { call = { path, method, body }; return { code: 'ok', operationId: body.operationId, replayed: false, x: 2086, y: 2093 }; });
  f.select(); f.submit(); await flush();
  assert.equal(call.path, '/staff/players/char-1/teleport');
  assert.equal(call.method, 'POST');
  assert.equal(call.body.expectedSessionId, 77);
  assert.equal(call.body.account, 'player');
  assert.equal(call.body.destination, 'armia');
  assert.match(f.node('teleport-message').textContent, /X: 2086 · Y: 2093/);
  assert.equal(call.body.epoch, f.epoch);
  assert.equal(f.node('teleport-submit').disabled, true);
  assert.equal(f.node('teleport-retry').hidden, true);
  assert.match(f.node('teleport-message').textContent, /Jogador teleportado/);
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
    return { code: 'ok', operationId: body.operationId, replayed: true, x: 2086, y: 2093 };
  });
  f.select(); f.submit(); await flush();
  assert.equal(f.node('teleport-retry').hidden, false);
  assert.equal(f.node('teleport-reason').disabled, true);
  f.panel.select({ ...f.player, uid: 'another', sessionId: 99 }, f.epoch);
  assert.equal(f.node('teleport-destination').disabled, true);
  f.node('teleport-destination').value = 'azran';
  f.node('teleport-retry').handlers.click(); await flush();
  assert.equal(bodies.length, 2); assert.equal(bodies[0], bodies[1]);
  assert.match(f.node('teleport-message').textContent, /Recibo recuperado/);
});
test('logout discards late response without altering new session controls', async () => {
  let resolve;
  const f = fixture(() => new Promise(r => { resolve = r; }));
  f.select(); f.submit(); f.panel.reset(); f.select();
  resolve({ code: 'ok', operationId: 'operation-0000001', replayed: false, x: 2086, y: 2093 }); await flush();
  assert.equal(f.node('teleport-message').textContent, '');
  assert.equal(f.node('teleport-submit').disabled, false);
});
test('mismatched receipt remains uncertain and recoverable', async () => {
  const f = fixture(async () => ({ code: 'ok', operationId: 'different-operation', replayed: false, x: 2086, y: 2093 }));
  f.select(); f.submit(); await flush();
  assert.equal(f.node('teleport-retry').hidden, false);
  assert.match(f.node('teleport-message').textContent, /Resultado não confirmado/);
});
test('persistence failure is not reported as successful saving', async () => {
  const f = fixture(async (_path, _method, body) => ({ code: 'persistence_failed', operationId: body.operationId, replayed: false, x: 0, y: 0 }));
  f.select(); f.submit(); await flush();
  assert.match(f.node('teleport-message').textContent, /posição anterior foi restaurada/);
  assert.equal(f.node('teleport-retry').hidden, true);
});
test('older overview cannot enable moderation', () => {
  const f = fixture(async () => { throw new Error('unexpected command'); });
  f.panel.reset(); f.panel.select(f.player);
  assert.equal(f.node('teleport-submit').disabled, true);
});


test('selected city is sent and unsupported destinations are rejected', async () => {
  let calls = [];
  const f = fixture(async (_path, _method, body) => { calls.push(body); return { code: 'ok', operationId: body.operationId, replayed: false, x: 2494, y: 1707 }; });
  f.select(); f.node('teleport-destination').value = 'arena'; f.submit(); await flush();
  assert.equal(calls.length, 0);
  f.node('teleport-destination').value = 'azran'; f.submit(); await flush();
  assert.equal(calls[0].destination, 'azran');
});

test('invalid success coordinates cannot confirm an operation', async () => {
  const f = fixture(async (_path, _method, body) => ({ code: 'ok', operationId: body.operationId, replayed: false, x: 0, y: 0 }));
  f.select(); f.submit(); await flush();
  assert.equal(f.node('teleport-retry').hidden, false);
  assert.match(f.node('teleport-message').textContent, /Resultado não confirmado/);
});
