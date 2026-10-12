import assert from 'node:assert/strict';
import { readFileSync, existsSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import test from 'node:test';

const dist = new URL('../dist/', import.meta.url);
const read = path => readFileSync(new URL(path, dist), 'utf8');
const admin = read('admin/index.html');
const attributes = id => {
  const tag = admin.match(new RegExp(`<input\\b[^>]*\\bid="${id}"[^>]*>`));
  assert.ok(tag, `Input ${id} exists`);
  return Object.fromEntries([...tag[0].matchAll(/([\w-]+)="([^"]*)"/g)].map(match => [match[1], match[2]]));
};

test('both staff PIN forms enforce six masked ASCII digits', () => {
  for (const id of ['adminPin', 'new-account-pin']) {
    const attrs = attributes(id);
    assert.equal(attrs.type, 'password');
    assert.equal(attrs.inputmode, 'numeric');
    assert.equal(attrs.minlength, '6');
    assert.equal(attrs.maxlength, '6');
    assert.equal(attrs.pattern, '[0-9]{6}');
    assert.equal(attrs.autocomplete, 'off');
  }
});

test('staff navigation and administrative creation start inside authenticated UI', () => {
  assert.match(admin, /<nav\b[^>]*id="staff-navigation"[^>]*hidden/);
  const dashboard = admin.indexOf('<div id="dashboard" hidden>');
  const create = admin.indexOf('id="accounts"');
  assert.ok(dashboard >= 0 && create > dashboard);
  assert.equal(admin.slice(0, dashboard).includes('id="account-create-form"'), false);
  assert.ok(admin.indexOf('id="events"') > admin.indexOf('id="overview"'));
});

test('all routes have the shared theme, English document and content anchor', () => {
  for (const path of ['index.html', 'admin/index.html', 'register/index.html', 'player/index.html', 'ranking/index.html', 'download/index.html']) {
    const html = read(path);
    assert.match(html, /<html lang="en"/);
    assert.match(html, /<meta name="theme-color" content="#101211"/);
    assert.match(html, /href="#main"/);
    assert.match(html, /<main id="main"/);
    const styles = [...html.matchAll(/<link[^>]*href="(\/_astro\/[^" ]+\.css)"/g)];
    assert.ok(styles.length > 0);
    const css = styles.map(match => read(match[1].slice(1))).join('\n');
    assert.ok(css.includes('--accent:#d4bd83') || css.includes('--accent: #d4bd83'));
  }
});

test('native identity resources are packaged and game credential limits stay unchanged', () => {
  for (const path of ['ui/logo.png', 'ui/button.png', 'ui/panel.png']) {
    assert.ok(existsSync(fileURLToPath(new URL(path, dist))), path);
  }
  const registration = read('register/index.html');
  assert.match(registration, /id="reg-username"[^>]*maxlength="12"/);
  assert.match(registration, /id="reg-password"[^>]*maxlength="10"/);
});
