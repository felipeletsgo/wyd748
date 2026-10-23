import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const sources = Object.fromEntries(['colosseum', 'ox-event'].map(name => [name,
  ts.transpileModule(readFileSync(new URL(`../src/scripts/${name}.ts`, import.meta.url), 'utf8'), {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
  }).outputText,
]));
const asOf = '2026-10-21T15:00:00Z';
const colosseum = () => ({ version: 1, asOf, epoch: 'a'.repeat(32), revision: 7, enabled: true,
  active: false, tier: '', stage: 0, endsAt: null, pendingExits: 0, timezone: 'America/Sao_Paulo',
  schedule: ['N', 'M', 'A'].map((tier, i) => ({ tier, startsAt: `2026-10-21T${21 + i}:05:00Z`, claimed: false })),
});
const ox = () => ({ version: 1, asOf, quizMode: false, doors: 6, doorsLocked: true, playersX: 1, playersO: 2,
  phase: 'inactive', round: 0, questions: 24, participants: 0, endsAt: null });
function fixture(name, snapshot) {
  const elements = new Map(), calls = [], confirmations = [];
  let respond = async () => snapshot;
  const element = id => {
    if (!elements.has(id)) elements.set(id, { value: id.endsWith('-tier') ? 'N' : 'Start before the war',
      disabled: false, textContent: '', handlers: {}, reportValidity: () => true,
      addEventListener(event, fn) { this.handlers[event] = fn; } });
    return elements.get(id);
  };
  const context = { exports: {}, document: { getElementById: element },
    crypto: { randomUUID: () => 'operation-0000001' },
    window: { confirm: message => { confirmations.push(message); return true; } },
  };
  vm.runInNewContext(sources[name], context);
  const controller = context.exports[name === 'colosseum' ? 'setupColosseum' : 'setupOXEvent'](
    async (path, method, body) => { calls.push({ path, method, body }); return respond(path, method, body); },
    () => 'Request failed.');
  return { controller, element, calls, confirmations, respond: handler => { respond = handler; } };
}
const flush = () => new Promise(resolve => setImmediate(resolve));

test('Colosseum start submits the selected tier and authoritative concurrency envelope', async () => {
  const f = fixture('colosseum', colosseum()); await f.controller.refresh();
  f.element('colosseum-tier').value = 'M'; f.element('colosseum-tier').handlers.change();
  f.respond(async (_path, _method, command) => ({ operationId: command.operationId, code: 'ok', replayed: false,
    status: { ...colosseum(), active: true, tier: 'M', stage: 1, endsAt: '2026-10-21T15:12:00Z' } }));
  f.element('colosseum-start').handlers.click(); await flush();
  assert.equal(f.calls[1].path, '/staff/events/colosseum'); assert.equal(f.calls[1].method, 'POST');
  const command = f.calls[1].body;
  assert.equal(command.tier, 'M'); assert.equal(command.expectedRevision, 7);
  assert.equal(command.epoch, 'a'.repeat(32)); assert.equal(command.reason, 'Start before the war');
  assert.match(f.confirmations[0], /replacing today's scheduled session until restart/);
  assert.equal(f.element('colosseum-start').disabled, true);
  assert.match(f.element('colosseum-message').textContent, /server started/);
});

test('Colosseum controls reject duplicate, claimed, late, busy and disabled starts', async () => {
  const f = fixture('colosseum', colosseum()); await f.controller.refresh();
  let complete;
  f.respond(() => new Promise(resolve => { complete = resolve; }));
  f.element('colosseum-start').handlers.click(); f.element('colosseum-start').handlers.click();
  assert.equal(f.calls.length, 2); assert.equal(f.element('colosseum-tier').disabled, true);
  complete({ operationId: 'operation-0000001', code: 'ok', replayed: true, status: colosseum() }); await flush();
  assert.match(f.element('colosseum-message').textContent, /no new event/);
  for (const blocked of [{ ...colosseum(), enabled: false }, { ...colosseum(), pendingExits: 1 },
    { ...colosseum(), asOf: '2026-10-21T21:05:00Z' },
    { ...colosseum(), schedule: colosseum().schedule.map(entry => ({ ...entry, claimed: true })) }]) {
    f.respond(async () => blocked); await f.controller.refresh();
    assert.equal(f.element('colosseum-start').disabled, true);
    const count = f.calls.length; f.element('colosseum-start').handlers.click(); await flush();
    assert.equal(f.calls.length, count);
  }
});

for (const [name, snapshot] of [['colosseum', colosseum], ['ox-event', ox]]) {
  test(`${name} discards snapshots after logout and requires refresh after an uncertain command`, async () => {
    const f = fixture(name, snapshot()); let complete;
    f.respond(() => new Promise(resolve => { complete = resolve; }));
    const pending = f.controller.refresh(); f.controller.reset(); complete(snapshot()); await pending;
    assert.equal(f.element(`${name}-start`).disabled, true);
    f.respond(async () => snapshot()); await f.controller.refresh();
    f.element(`${name}-reason`).value = 'Start before the war';
    f.respond(async () => { throw new Error('network failure'); });
    f.element(`${name}-start`).handlers.click(); await flush();
    assert.equal(f.element(`${name}-start`).disabled, true);
    assert.match(f.element(`${name}-message`).textContent, /Outcome not confirmed/);
    assert.equal(f.element(`${name}-refresh`).disabled, false);
  });
}

test('OX one-click start opens entry once and cannot reopen a running question', async () => {
  const f = fixture('ox-event', ox()); await f.controller.refresh();
  f.respond(async (_path, _method, command) => ({ operationId: command.operationId, code: 'ok', moved: 0,
    status: { ...ox(), quizMode: true, doorsLocked: false, phase: 'enrollment', round: 1 } }));
  f.element('ox-event-start').handlers.click(); await flush();
  assert.equal(f.calls[1].body.action, 'start'); assert.equal(f.calls[1].path, '/staff/events/ox');
  f.respond(async () => ({ ...ox(), quizMode: true })); await f.controller.refresh();
  assert.equal(f.element('ox-event-start').disabled, true);
  const count = f.calls.length; f.element('ox-event-start').handlers.click(); await flush();
  assert.equal(f.calls.length, count); assert.equal(f.element('ox-event-doors').textContent, 'Locked');
});

test('OX side overrides require a question and preserve the full response window', async () => {
  const f = fixture('ox-event', ox()); await f.controller.refresh();
  assert.equal(f.element('ox-event-result-o').disabled, true);
  assert.match(f.element('ox-event-mode').textContent, /ticket is preserved/);
  f.respond(async () => ({ ...ox(), quizMode: true, phase: 'question', round: 2, participants: 3,
    endsAt: '2026-10-21T15:00:10Z' })); await f.controller.refresh();
  assert.match(f.element('ox-event-cycle').textContent, /question 2\/24; 3 admitted/);
  for (const side of ['o', 'x']) {
    f.respond(async (_path, _method, command) => ({ operationId: command.operationId, code: 'ok', moved: 0,
      status: { ...ox(), quizMode: true, phase: 'question' } }));
    f.element(`ox-event-result-${side}`).handlers.click(); await flush();
    assert.equal(f.calls.at(-1).body.action, `result_${side}`);
    assert.match(f.confirmations.at(-1), /10-second answer window ends/);
    assert.match(f.element('ox-event-message').textContent, /evaluation waits/);
  }
});
