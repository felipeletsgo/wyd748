import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const source = ts.transpileModule(readFileSync(new URL('../src/scripts/npc-editor.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
const flush = () => new Promise(resolve => setImmediate(resolve));
function fixture(lang = 'en', code = 'ok', options = {}) {
  const elements = new Map(), writes = [], confirmations = [], reads = [];
  const node = id => {
    if (!elements.has(id)) elements.set(id, { value: '', textContent: '', disabled: false, handlers: {}, children: [],
      addEventListener(type, fn) { this.handlers[type] = fn; }, replaceChildren(...children) { this.children = children; },
      add(child) { this.children.push(child); } });
    return elements.get(id);
  };
  const npc = { name: 'Aki', displayName: 'Aki', channel: 1, version: 1, appliedVersion: 1, active: true,
    overridden: false, definition: { name: 'Aki' }, source: { name: 'Aki' }, updatedBy: '' };
  let failLoad = false;
  const context = { exports: {}, document: { getElementById: node, documentElement: { lang } }, navigator: { language: lang },
    window: { confirm: s => { confirmations.push(s); return options.confirm ? options.confirm(s) : true; } }, Option: class { constructor(label, value) { this.text = label; this.value = value; } } };
  vm.runInNewContext(source, context);
  const controller = context.exports.setupNPCEditor(async (url, method, body) => {
    if (method === 'PUT') {
      writes.push(body);
      if (options.put) return options.put(body);
	  const views = (body.targets ?? [{ channel: body.channel, expectedVersion: body.expectedVersion }]).map(target => ({ ...npc, channel: target.channel, version: target.expectedVersion + 1, appliedVersion: code === 'ok' ? target.expectedVersion + 1 : target.expectedVersion, overridden: true }));
      return { code, npc: views.find(entry => entry.channel === body.channel), npcs: views };
    }
	reads.push(url);
    if (failLoad) throw new Error('Unavailable');
    if (options.get) return options.get(npc, url);
	const channel = Number(new URL(url, 'https://fixture.test').searchParams.get('channel'));
    return { version: 1, available: true, channel, npcs: [{ ...npc, channel, version: channel === 2 ? 4 : npc.version }] };
  }, () => 'Request failed');
  node('npc-channel').value = '1';
  async function load() {
    await controller.refresh();
    node('npc-select').value = 'Aki';
    node('npc-select').handlers.change({ currentTarget: node('npc-select') });
    node('npc-reason').value = 'Update stock';
  }
  async function save() { node('npc-definition-form').handlers.submit({ preventDefault() {} }); await flush(); }
  return { node, controller, writes, confirmations, reads, load, save, npc, fail() { failLoad = true; } };
}

for (const lang of ['en', 'pt-BR']) {
	for (const code of ['ok', 'saved_pending']) {
		test(`${lang}: both-channel ${code} keeps versions and acknowledgement separate`, async () => {
			const f = fixture(lang, code); f.node('npc-scope').value = 'both'; await f.load(); await f.save();
			assert.equal(f.reads.length, 2); assert.equal(f.writes.length, 1);
			assert.deepEqual(JSON.parse(JSON.stringify(f.writes[0].targets)), [{ channel: 1, expectedVersion: 1 }, { channel: 2, expectedVersion: 4 }]);
			assert.match(f.confirmations.at(-1), /1.*2/);
			assert.match(f.node('npc-target-state').textContent, /1.*v2.*2.*v5/);
			assert.match(f.node('npc-editor-message').textContent, code === 'ok' ? (lang === 'en' ? /applied to both/ : /aplicada nos dois/) : (lang === 'en' ? /Waiting for application/ : /Aguardando aplicação/));
		});
	}
}

test('a missing or unavailable peer prevents a partial edit', async () => {
	for (const missing of ['unavailable', 'absent']) {
		const f = fixture('en', 'ok', { get: async (npc, url) => {
			const channel = Number(new URL(url, 'https://fixture.test').searchParams.get('channel'));
			return { version: 1, channel, available: channel === 1 || missing !== 'unavailable', npcs: channel === 2 ? [] : [npc] };
		} });
		f.node('npc-scope').value = 'both'; await f.load(); await f.save();
		assert.equal(f.node('npc-save').disabled, true); assert.equal(f.writes.length, 0);
		assert.match(f.node('npc-editor-message').textContent, /Load channels 1 and 2/);
	}
});

test('a batch acknowledgement cannot hide pending application in its peer', async () => {
	let npc;
	const f = fixture('en', 'ok', { put: async () => ({ code: 'ok', npc: { ...npc, version: 2, appliedVersion: 2 }, npcs: [
		{ ...npc, version: 2, appliedVersion: 2 }, { ...npc, channel: 2, version: 5, appliedVersion: 4 },
	] }) });
	npc = f.npc; f.node('npc-scope').value = 'both'; await f.load(); await f.save();
	assert.match(f.node('npc-editor-message').textContent, /Waiting for application/);
	assert.match(f.node('npc-target-state').textContent, /2.*v5.*v4.*Application pending/);
});

test('a batch stale response updates both expected versions without retry', async () => {
	const f = fixture('en', 'stale_version'); f.node('npc-scope').value = 'both'; await f.load(); await f.save();
	assert.equal(f.writes.length, 1); assert.match(f.node('npc-editor-message').textContent, /Another staff member/);
	await f.save(); assert.equal(f.writes.length, 2);
	assert.deepEqual(JSON.parse(JSON.stringify(f.writes[1].targets)), [{ channel: 1, expectedVersion: 2 }, { channel: 2, expectedVersion: 5 }]);
});

test('changing scope without loading cannot submit old target versions', async () => {
	const f = fixture(); await f.load(); f.node('npc-scope').value = 'both'; await f.save();
	assert.equal(f.writes.length, 0); assert.match(f.node('npc-editor-message').textContent, /Load the selected channel/);
});

test('declining a scope change preserves unsaved content and loaded scope', async () => {
	const f = fixture('en', 'ok', { confirm: () => false }); await f.load();
	f.node('npc-definition').value = '{"name":"Aki","gold":9}'; f.node('npc-scope').value = 'both';
	f.node('npc-scope').handlers.change(); await flush();
	assert.equal(f.node('npc-scope').value, 'channel'); assert.match(f.node('npc-definition').value, /gold/);
	assert.equal(f.reads.length, 1); assert.equal(f.node('npc-save').disabled, false);
});

test('batch restore carries both versions and excludes local JSON', async () => {
	const f = fixture(); f.node('npc-scope').value = 'both'; await f.load();
	f.node('npc-definition').value = '{broken'; f.node('npc-restore').handlers.click(); await flush();
	assert.equal(f.writes.length, 1); assert.equal(f.writes[0].restoreSource, true);
	assert.equal('definition' in f.writes[0], false); assert.equal(f.writes[0].targets.length, 2);
});

test('reset clears the previous session channel statuses and scope', async () => {
	const f = fixture(); f.node('npc-scope').value = 'both'; await f.load();
	assert.match(f.node('npc-target-state').textContent, /Channel 2/);
	f.controller.reset();
	assert.equal(f.node('npc-target-state').textContent, ''); assert.equal(f.node('npc-scope').value, 'channel');
	assert.match(f.node('npc-editor-help').textContent, /only to this channel/);
});

test('invalid JSON never submits a definition', async () => {
  const f = fixture(); await f.load();
  for (const value of ['{', 'null', '[]', '12']) {
    f.node('npc-definition').value = value; await f.save();
    assert.equal(f.writes.length, 0);
    assert.match(f.node('npc-editor-message').textContent, /syntax error/);
  }
});

test('declining confirmation leaves edits local', async () => {
  const f = fixture('en', 'ok', { confirm: () => false }); await f.load();
  f.node('npc-definition').value = '{"name":"Aki","gold":5}'; await f.save();
  assert.equal(f.writes.length, 0);
  assert.match(f.node('npc-definition').value, /gold/);
});

test('reset ignores a delayed successful save from the previous session', async () => {
  let resolve;
  const f = fixture('en', 'ok', { put: () => new Promise(done => { resolve = done; }) });
  await f.load(); await f.save();
  assert.equal(f.node('npc-save').disabled, true);
  f.controller.reset();
  resolve({ code: 'ok', npc: { ...f.npc, version: 2, appliedVersion: 2 } }); await flush();
  assert.equal(f.node('npc-definition').value, '');
  assert.equal(f.node('npc-save').disabled, true);
  assert.equal(f.node('npc-editor-message').textContent, '');
});

test('reset ignores a delayed catalog from the previous session', async () => {
  let resolve;
  const f = fixture('en', 'ok', { get: () => new Promise(done => { resolve = done; }) });
  const loading = f.controller.refresh(); f.controller.reset();
  resolve({ version: 1, available: true, channel: 1, npcs: [f.npc] }); await loading;
  assert.equal(f.node('npc-select').children.length, 1);
  assert.equal(f.node('npc-save').disabled, true);
  assert.equal(f.node('npc-definition').value, '');
});

test('stale version loads the current definition without automatically saving again', async () => {
  const f = fixture('en', 'stale_version'); await f.load(); await f.save();
  assert.equal(f.writes.length, 1);
  assert.equal(f.writes[0].expectedVersion, 1);
  assert.match(f.node('npc-editor-message').textContent, /Another staff member/);
  await f.save(); assert.equal(f.writes[1].expectedVersion, 2);
});

test('source restore sends the version and excludes edited JSON', async () => {
  const f = fixture(); await f.load();
  f.node('npc-definition').value = '{broken';
  f.node('npc-restore').handlers.click(); await flush();
  assert.equal(f.writes.length, 1);
  assert.equal(f.writes[0].restoreSource, true);
  assert.equal(f.writes[0].expectedVersion, 1);
  assert.equal('definition' in f.writes[0], false);
});

test('an ok response with an unconfirmed applied version stays pending', async () => {
  let npc;
  const f = fixture('en', 'ok', { put: async () => ({ code: 'ok', npc: { ...npc, version: 2, appliedVersion: 1 } }) });
  npc = f.npc; await f.load(); await f.save();
  assert.match(f.node('npc-editor-message').textContent, /Waiting for channel application/);
});

test('changing channel blocks stale edits, including programmatic submits', async () => {
  const f = fixture(); await f.load();
  f.node('npc-channel').value = '2';
  f.node('npc-channel').handlers.input();
  assert.equal(f.node('npc-save').disabled, true);
  assert.equal(f.node('npc-definition').disabled, true);
  await f.save(); assert.equal(f.writes.length, 0);
  f.node('npc-channel').value = '1'; f.node('npc-channel').handlers.input();
  await f.save(); assert.equal(f.writes.length, 1);
  assert.equal(f.writes[0].channel, 1);
  assert.match(f.confirmations.at(-1), /channel 1\?/);
});

test('a failed channel load cannot leave the old catalog editable', async () => {
  const f = fixture(); await f.load(); f.fail();
  await f.controller.refresh(); await f.save();
  assert.equal(f.writes.length, 0); assert.equal(f.node('npc-save').disabled, true);
});

for (const lang of ['en', 'pt-BR']) {
  test(`${lang}: pending persistence does not claim actor application`, async () => {
    const f = fixture(lang, 'saved_pending'); await f.load(); await f.save();
    assert.match(f.node('npc-editor-message').textContent, lang === 'en' ? /Waiting for channel application/ : /Aguardando aplicação/);
    assert.match(f.node('npc-editor-state').textContent, lang === 'en' ? /Application pending/ : /Aplicação pendente/);
  });
  test(`${lang}: acknowledgement confirms the applied version`, async () => {
    const f = fixture(lang); await f.load(); await f.save();
    assert.match(f.node('npc-editor-message').textContent, lang === 'en' ? /saved and applied/ : /salva e aplicada/);
  });
}
