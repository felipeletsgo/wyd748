import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const sources = Object.fromEntries(['sanctions', 'staff-notices', 'maintenance', 'audit-log'].map(name => [name,
  ts.transpileModule(readFileSync(new URL(`../src/scripts/${name}.ts`, import.meta.url), 'utf8'), {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
  }).outputText,
]));
const setups = { sanctions: 'setupSanctions', 'staff-notices': 'setupStaffNotices', maintenance: 'setupMaintenance', 'audit-log': 'setupAuditLog' };
const flush = () => new Promise(resolve => setImmediate(resolve));

function node(id = '') {
  return { id, value: '', disabled: false, hidden: false, textContent: '', className: '', type: '', colSpan: 0, children: [],
    handlers: {}, attributes: {}, selectedOptions: [{ textContent: '1 hour' }],
    reportValidity: () => true, scrollIntoView() {}, focus() {}, setAttribute(name, value) { this.attributes[name] = value; },
    append(...items) { this.children.push(...items); }, replaceChildren(...items) { this.children = items; },
    addEventListener(event, fn) { this.handlers[event] = fn; } };
}

function fixture(name, defaults = {}) {
  const elements = new Map(), calls = [], confirmations = [], prompts = [];
  let respond = async () => ({});
  const element = id => {
    if (!elements.has(id)) { const created = node(id); created.value = defaults[id] ?? ''; elements.set(id, created); }
    return elements.get(id);
  };
  let uuid = 0;
  const context = { exports: {}, Object, Number, Math, JSON, URLSearchParams, Date, Error,
    document: { getElementById: element, createElement: () => node(), querySelectorAll: () => [] },
    crypto: { randomUUID: () => `operation-${String(++uuid).padStart(10, '0')}` },
    window: { confirm: message => { confirmations.push(message); return true; }, prompt: message => { prompts.push(message); return 'appeal accepted'; } },
  };
  vm.runInNewContext(sources[name], context);
  const controller = context.exports[setups[name]](
    async (path, method, body) => { calls.push({ path, method, body }); return respond(path, method, body); },
    () => 'Request failed.');
  return { controller, element, calls, confirmations, prompts, context, respond: handler => { respond = handler; } };
}

const sanctionView = (sanctions = []) => ({ version: 1, asOf: '2026-10-21T15:00:00Z', account: 'player', sanctions });

test('Sanctions submit the selected kind and duration and retry with the same request ID', async () => {
  const f = fixture('sanctions', { 'sanction-kind': 'ban', 'sanction-duration': '1440', 'sanction-reason': 'Botting in Dungeon' });
  f.respond(async () => sanctionView());
  f.controller.open('Player'); await flush();
  assert.equal(f.calls[0].path, '/staff/accounts/player/sanctions');
  assert.equal(f.element('sanction-submit').disabled, false);
  let fail = true;
  f.respond(async (_path, method, command) => {
    if (method === 'GET') return sanctionView([{ id: 3, account: 'player', kind: 'ban', reason: command?.reason ?? '', actor: 'operator', durationMinutes: 1440,
      createdAt: '2026-10-21T15:00:00Z', expiresAt: '2026-10-22T15:00:00Z', revokedAt: null, revokedBy: '', revokeReason: '', active: true }]);
    if (fail) { fail = false; throw new Error('network'); }
    return { code: 'ok', replayed: true, live: 'applied', sanction: { id: 3 } };
  });
  f.element('sanction-form').handlers.submit({ preventDefault() {} }); await flush();
  const first = f.calls[1].body;
  assert.deepEqual({ ...first }, { version: 1, requestId: first.requestId, account: 'player', kind: 'ban', durationMinutes: 1440, reason: 'Botting in Dungeon' });
  assert.match(f.confirmations[0], /disconnects the account now/);
  assert.match(f.element('sanction-message').textContent, /Outcome not confirmed/);
  assert.equal(f.element('sanction-retry').hidden, false);
  f.element('sanction-retry').handlers.click(); await flush(); await flush();
  assert.equal(f.calls[2].body.requestId, first.requestId);
  assert.match(f.element('sanction-message').textContent, /nothing was applied twice.*Online sessions/);
  assert.equal(f.element('sanction-retry').hidden, true);
});

test('Sanctions refuse an invalid account and an empty reason', async () => {
  const f = fixture('sanctions', { 'sanction-account': 'bad name!', 'sanction-reason': '  ' });
  f.element('sanction-lookup').handlers.submit({ preventDefault() {} });
  assert.equal(f.calls.length, 0);
  f.respond(async () => sanctionView());
  f.controller.open('player'); await flush();
  f.element('sanction-form').handlers.submit({ preventDefault() {} });
  assert.equal(f.calls.length, 1);
  assert.match(f.element('sanction-message').textContent, /valid reason/);
});

test('Notices reject non-ASCII text and convert minutes to seconds', async () => {
  const f = fixture('staff-notices', { 'notice-message': 'Evento às 20h', 'notice-delay': '5', 'notice-count': '3', 'notice-every': '10' });
  f.element('notice-form').handlers.submit({ preventDefault() {} });
  assert.equal(f.calls.length, 0);
  assert.match(f.element('notice-message-status').textContent, /printable ASCII/);
  f.element('notice-message').value = 'Double EXP at 20:00!';
  f.respond(async (_path, _method, command) => ({ operationId: command.operationId, code: 'ok', replayed: false, delivered: 0,
    view: { version: 1, asOf: '2026-10-21T15:00:00Z', notices: [] } }));
  f.element('notice-form').handlers.submit({ preventDefault() {} }); await flush();
  assert.deepEqual({ ...f.calls[0].body }, { version: 1, operationId: f.calls[0].body.operationId, message: 'Double EXP at 20:00!', delaySeconds: 300, repeatCount: 3, repeatEverySeconds: 600 });
  assert.match(f.confirmations[0], /in 5 minutes, 3 times every 10 minutes/);
  assert.equal(f.element('notice-message').value, '');
  f.element('notice-message').value = 'Server event now!'; f.element('notice-delay').value = '0'; f.element('notice-count').value = '1';
  f.element('notice-form').handlers.submit({ preventDefault() {} }); await flush();
  assert.equal(f.calls[1].body.repeatEverySeconds, 0);
});

test('Player messages bind the observed session and epoch', async () => {
  const f = fixture('staff-notices', { 'player-message-text': 'Please contact staff.' });
  const player = { uid: 'char-1', name: 'Hero', account: 'Player', sessionId: 9 };
  f.controller.selectPlayer(player, 'z'.repeat(32));
  await f.element('player-message-form').handlers.submit({ preventDefault() {} });
  assert.equal(f.calls.length, 0);
  assert.match(f.element('player-message-status').textContent, /Refresh data/);
  f.controller.selectPlayer(player, 'a'.repeat(32));
  f.respond(async (_path, _method, command) => ({ operationId: command.operationId, code: 'stale_target', replayed: false }));
  await f.element('player-message-form').handlers.submit({ preventDefault() {} });
  assert.equal(f.calls[0].path, '/staff/players/char-1/message');
  assert.equal(f.calls[0].body.account, 'player'); assert.equal(f.calls[0].body.expectedSessionId, 9);
  assert.match(f.element('player-message-status').textContent, /no longer online/);
});

const shutdownStatus = (overrides = {}) => ({ version: 1, asOf: '2026-10-21T15:00:00Z', scheduled: false, mode: '', at: null,
  secondsRemaining: 0, actor: '', reason: '', loginsBlocked: false, available: true, ...overrides });

test('Maintenance schedules and cancels with the authoritative envelope', async () => {
  const f = fixture('maintenance', { 'maintenance-kind': 'shutdown', 'maintenance-minutes': '15', 'maintenance-reason': 'Database upgrade' });
  f.respond(async () => shutdownStatus());
  await f.controller.refresh();
  assert.equal(f.element('maintenance-schedule').disabled, false);
  assert.equal(f.element('maintenance-cancel').disabled, true);
  const scheduled = shutdownStatus({ scheduled: true, mode: 'shutdown', at: '2026-10-21T15:15:00Z', secondsRemaining: 900, actor: 'operator', reason: 'Database upgrade' });
  f.respond(async (_path, _method, command) => ({ operationId: command.operationId, code: 'ok', replayed: false, status: scheduled }));
  f.element('maintenance-form').handlers.submit({ preventDefault() {} }); await flush();
  assert.deepEqual({ ...f.calls[1].body }, { version: 1, operationId: f.calls[1].body.operationId, action: 'schedule', mode: 'shutdown', delaySeconds: 900, reason: 'Database upgrade' });
  assert.match(f.confirmations[0], /stays OFFLINE/);
  assert.equal(f.element('maintenance-schedule').disabled, true);
  assert.equal(f.element('maintenance-cancel').disabled, false);
  f.element('maintenance-reason').value = 'Upgrade postponed';
  f.respond(async (_path, _method, command) => ({ operationId: command.operationId, code: 'ok', replayed: false, status: shutdownStatus() }));
  f.element('maintenance-cancel').handlers.click(); await flush();
  assert.deepEqual({ ...f.calls[2].body }, { version: 1, operationId: f.calls[2].body.operationId, action: 'cancel', mode: '', delaySeconds: 0, reason: 'Upgrade postponed' });
});

test('Maintenance stays disabled without a process hook', async () => {
  const f = fixture('maintenance', { 'maintenance-reason': 'Patch' });
  f.respond(async () => shutdownStatus({ available: false }));
  await f.controller.refresh();
  assert.equal(f.element('maintenance-schedule').disabled, true);
  assert.match(f.element('maintenance-state').textContent, /Unavailable/);
});

test('Audit history filters, pages by keyset and summarizes details as text', async () => {
  const f = fixture('audit-log', { 'audit-actor': 'Operator', 'audit-action': 'account.sanction' });
  const entry = { id: 90, at: '2026-10-21T15:00:00Z', actor: 'operator', action: 'account.sanction', result: 'ok', requestId: 'r', clientIp: '127.0.0.1',
    method: 'POST', path: '/x', details: { kind: 'ban', reason: '<b>x</b>'.repeat(60) } };
  f.respond(async () => ({ version: 1, asOf: '2026-10-21T15:00:00Z', entries: [entry], nextBefore: 90 }));
  f.element('audit-form').handlers.submit({ preventDefault() {} }); await flush();
  assert.equal(f.calls[0].path, '/staff/audit?limit=50&actor=operator&action=account.sanction');
  assert.equal(f.element('audit-older').disabled, false);
  f.element('audit-older').handlers.click(); await flush();
  assert.equal(f.calls[1].path, '/staff/audit?limit=50&actor=operator&action=account.sanction&before=90');
  const summary = f.context.exports.auditSummary(entry);
  assert.ok(summary.startsWith('kind=ban · reason=<b>x</b>') && summary.length === 240 && summary.endsWith('…'));
});
