import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const compiled = ts.transpileModule(readFileSync(new URL('../src/lib/account-sessions.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
class Element {
  children = []; dataset = {}; handlers = {}; disabled = false; textContent = '';
  constructor(tag = 'div') { this.tag = tag; }
  addEventListener(name, fn) { this.handlers[name] = fn; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.children = []; this.append(...children); }
  querySelectorAll(tag) { return this.children.flatMap(child => [...(child.tag === tag ? [child] : []), ...child.querySelectorAll(tag)]); }
}
const timestamp = '2026-10-12T18:30:00Z';
const session = { id: 'a'.repeat(64), current: true, createdAt: timestamp, lastSeenAt: timestamp, expiresAt: timestamp };
function fixture() {
  const calls = []; let identity = 'csrf-current', signedOut = 0;
  let responder = async () => ({ version: 1, asOf: timestamp, sessions: [session, { ...session, current: false, id: 'b'.repeat(64) }] });
  const context = { exports: {}, document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') } };
  vm.runInNewContext(compiled, context);
  const list = new Element('ul'), status = new Element(), refresh = new Element('button'), all = new Element('button');
  const controller = new context.exports.AccountSessions(list, status, refresh, all,
    async (path, body) => { calls.push([path, body]); return responder(path, body); }, () => identity, () => { signedOut++; identity = undefined; });
  return { controller, list, status, refresh, all, calls, context,
    respond: value => { responder = value; }, identity: value => { identity = value; }, get signedOut() { return signedOut; } };
}
const flush = () => new Promise(resolve => setImmediate(resolve));
test('session list uses text-only labels and revokes the selected display ID', async () => {
  const f = fixture(); await f.controller.load();
  assert.equal(f.list.children.length, 2);
  assert.equal(f.list.children[0].children[0].textContent, 'This website session');
  assert.match(f.status.textContent, /Session snapshot/);
  const button = f.list.children[1].querySelectorAll('button')[0];
  assert.equal(button.textContent, 'Revoke session');
  f.respond(async path => path.endsWith('/revoke') ? { signedOut: false } : { sessions: [session], asOf: timestamp });
  button.handlers.click(); await flush();
  assert.equal(f.calls[1][0], '/sessions/revoke'); assert.equal(f.calls[1][1].id, 'b'.repeat(64));
  assert.equal(f.list.children.length, 1); assert.equal(f.refresh.disabled, false);
});
test('logout-all clears the private list and notifies the owner', async () => {
  const f = fixture(); await f.controller.load();
  f.respond(async () => ({ signedOut: true }));
  f.all.handlers.click(); await flush();
  assert.equal(f.calls[1][0], '/sessions/revoke-all'); assert.deepEqual(Object.keys(f.calls[1][1]), []);
  assert.equal(f.signedOut, 1); assert.equal(f.list.children.length, 0);
});
test('concurrent controls are disabled and stale responses cannot repopulate a signed-out account', async () => {
  const f = fixture(); let complete;
  f.respond(() => new Promise(resolve => { complete = resolve; }));
  const pending = f.controller.load();
  assert.equal(f.refresh.disabled, true); assert.equal(f.all.disabled, true);
  await f.controller.revoke(); await f.controller.load(); assert.equal(f.calls.length, 1);
  f.identity(undefined); f.controller.clear();
  complete({ sessions: [session], asOf: timestamp }); await pending;
  assert.equal(f.list.children.length, 0); assert.equal(f.refresh.disabled, true);
});
test('an account switch discards old responses even without a clear call', async () => {
  const f = fixture(); let complete;
  f.respond(() => new Promise(resolve => { complete = resolve; })); const pending = f.controller.load();
  f.identity('new-session'); complete({ sessions: [session], asOf: timestamp }); await pending;
  assert.equal(f.list.children.length, 0);
});
test('failed reads clear stale data and uncertain revocations do not claim success', async () => {
  const f = fixture(); await f.controller.load();
  f.respond(async () => { throw new Error('<img src=x onerror=attack()>'); });
  await f.controller.revoke(session.id);
  assert.match(f.status.textContent, /not confirmed/); assert.equal(f.signedOut, 0);
  assert.equal(f.status.textContent.includes('<img'), false); assert.equal(f.refresh.disabled, false);
  await f.controller.load(); assert.equal(f.list.children.length, 0); assert.match(f.status.textContent, /Unable to load/);
  assert.equal(f.context.exports.displayTime('invalid'), 'Unavailable');
});
