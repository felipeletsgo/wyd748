import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const source = readFileSync(new URL('../src/lib/registration-challenge.ts', import.meta.url), 'utf8');
const compiled = ts.transpileModule(source, { compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 } }).outputText;
function fixture(sitekey = 'public-test-key', action = 'register') {
  let script, options, timeout, resets = 0, removed = false;
  const changes = [];
  const context = {
    exports: {},
    document: { createElement: () => ({ remove: () => { removed = true; } }), head: { append: value => { script = value; } } },
    window: { setTimeout: callback => { timeout = callback; return 1; }, clearTimeout: () => {}, turnstile: {
      // The real provider rejects ready() when its script uses async/defer.
      ready: () => { throw new Error('Remove async/defer before using ready().'); },
      render: (_container, value) => { options = value; return 'widget-1'; }, reset: id => { assert.equal(id, 'widget-1'); resets++; },
    } },
  };
  vm.runInNewContext(compiled, context);
  const challenge = new context.exports.RegistrationChallenge({ dataset: { sitekey, action } }, error => changes.push(error));
  return { challenge, context, changes, get script() { return script; }, get options() { return options; }, get resets() { return resets; }, get removed() { return removed; }, timeout: () => timeout() };
}

test('widget binds the public site key and registration action without an inline script', async () => {
  const f = fixture(); const started = f.challenge.start();
  assert.equal(f.challenge.token, '');
  assert.equal(f.script.src, 'https://challenges.cloudflare.com/turnstile/v0/api.js?render=explicit');
  assert.equal(f.script.async, true);
  assert.equal(f.options, undefined);
  f.script.onload(); await started;
  assert.equal(f.options.sitekey, 'public-test-key'); assert.equal(f.options.action, 'register');
  assert.equal(f.options.theme, 'dark'); assert.equal(f.options.language, 'en'); assert.equal(f.options['response-field'], false);
  f.options.callback('single-use-token'); assert.equal(f.challenge.token, 'single-use-token');
  f.challenge.reset(); assert.equal(f.challenge.token, ''); assert.equal(f.resets, 1);
});

test('expiry, timeout and provider errors invalidate an already solved token', async () => {
  const f = fixture(); const started = f.challenge.start(); f.script.onload(); await started;
  for (const callback of ['expired-callback', 'timeout-callback', 'error-callback']) {
    f.options.callback('token'); f.options[callback]();
    assert.equal(f.challenge.token, ''); assert.equal(typeof f.changes.at(-1), 'string');
  }
});

test('missing configuration, blocked script and loading timeout fail closed', async () => {
  await assert.rejects(fixture('').challenge.start(), /not configured/);
  for (const kind of ['error', 'timeout']) {
    const f = fixture(); const started = f.challenge.start();
    if (kind === 'error') f.script.onerror(); else f.timeout();
    await assert.rejects(started, /unavailable/);
    assert.equal(f.challenge.token, ''); assert.equal(f.removed, true);
  }
  const f = fixture(); const started = f.challenge.start(); delete f.context.window.turnstile; f.script.onload();
  await assert.rejects(started, /unavailable/);
});

test('authentication pages package action-bound widgets and a strict CSP', () => {
  const dist = new URL('../dist/', import.meta.url);
  const registration = readFileSync(new URL('register/index.html', dist), 'utf8');
  assert.match(registration, /id="registration-challenge" data-sitekey="0x[A-Za-z0-9_-]+"/);
  assert.match(registration, /type="submit" disabled/);
  assert.match(registration, /<noscript>/);
  assert.match(registration, /minlength="8" maxlength="10"/);
  for (const [path, action] of [['player/index.html', 'login'], ['admin/index.html', 'staff_login']]) {
    assert.match(readFileSync(new URL(path, dist), 'utf8'), new RegExp(`data-action="${action}"`));
  }
  for (const path of ['index.html', 'player/index.html', 'ranking/index.html', 'download/index.html']) {
    assert.equal(readFileSync(new URL(path, dist), 'utf8').includes('id="registration-challenge"'), false);
  }
  const caddy = readFileSync(new URL('../../../../tools/deployment/vps/wyd-site.caddy', import.meta.url), 'utf8');
  assert.match(caddy, /@registration path \/register \/register\//);
  assert.match(caddy, /script-src 'self' https:\/\/challenges.cloudflare.com; frame-src https:\/\/challenges.cloudflare.com/);
  assert.equal(caddy.includes('unsafe-inline'), false); assert.equal(caddy.includes('unsafe-eval'), false);
});

test('login challenges use distinct actions and reject unrecognized actions', async () => {
  for (const action of ['login', 'staff_login']) {
    const f = fixture('public-test-key', action); const started = f.challenge.start(); f.script.onload(); await started;
    assert.equal(f.options.action, action);
    f.options.callback('token'); f.challenge.reset(); assert.equal(f.challenge.token, '');
  }
  await assert.rejects(fixture('public-test-key', 'unknown').challenge.start(), /Invalid verification action/);
});

test('late script callbacks cannot activate a timed-out widget and render errors fail closed', async () => {
  const f = fixture(); const started = f.challenge.start(); f.timeout();
  await assert.rejects(started, /unavailable/); f.script.onload();
  assert.equal(f.options, undefined);
  const broken = fixture(); broken.context.window.turnstile.render = () => { throw new Error('Provider failure'); };
  const rejected = broken.challenge.start(); broken.script.onload(); await assert.rejects(rejected, /unavailable/);
});
