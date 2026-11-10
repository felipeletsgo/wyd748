import assert from 'node:assert/strict';
import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { resolve, relative, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import test from 'node:test';
const dist = fileURLToPath(new URL('../dist/', import.meta.url));
const html = path => readFileSync(resolve(dist, path), 'utf8');
function pages(folder) {
  return readdirSync(folder, { withFileTypes: true }).flatMap(entry => entry.isDirectory() ? pages(resolve(folder, entry.name)) : entry.name.endsWith('.html') ? [resolve(folder, entry.name)] : []);
}
test('public content ships real static routes with valid internal links and no inline execution', () => {
  for (const path of ['guilds', 'play', 'play/getting-started', 'play/classes', 'play/server', 'play/rules', 'play/faq', 'news', 'news/account-protection', 'news/client-7-48-001', 'news/client-7-48-002', 'news/client-7-48-013', 'news/client-7-48-014', 'changelog', 'roadmap']) {
    assert.match(html(`${path}/index.html`), /<html lang="en"/);
  }
  for (const path of pages(dist).filter(path => !relative(dist, path).startsWith(`admin${sep}`))) {
    const source = readFileSync(path, 'utf8');
    const navigation = source.match(/<nav aria-label="Main navigation">([\s\S]*?)<\/nav>/)?.[1];
    assert.ok(navigation, `${path}: missing main navigation`);
    assert.equal(navigation.includes('/database/'), false, `${path}: database navigation must remain hidden`);
    assert.equal(navigation.includes('/play/'), false, `${path}: play navigation must remain hidden`);
    assert.match(navigation, /href="\/guilds\/"[^>]*>Guilds<\/a>/, `${path}: missing guild directory navigation`);
    assert.equal(/\son(?:click|load|error)=/i.test(source), false, path);
    assert.equal(/<script(?![^>]*\bsrc=)[^>]*>\s*\S/i.test(source), false, path);
    for (const match of source.matchAll(/(?:href|src)="(\/[^"?#]*)(?:[?#][^"]*)?"/g)) {
      const link = match[1]; if (link.startsWith('/patch/')) continue;
      assert.ok(existsSync(resolve(dist, `.${link}`, ...(link.endsWith('/') ? ['index.html'] : []))), `${path}: broken ${link}`);
    }
  }
});
test('download receipt preserves the native launcher and unpublished news', () => {
  const receipt = JSON.parse(html('client-manifest.json'));
  assert.equal(receipt.version, 2); assert.equal(receipt.clientVersion, '7.48.108');
  assert.equal(receipt.feedVersion, 48108);
  assert.equal(receipt.patch.url, '/patch/patch-48108.zip');
  assert.equal(receipt.fullClient.clientVersion, '7.48.100');
  assert.equal(receipt.updateFeed, '/patch/update.htm');
  assert.equal(receipt.fullClient.sha256, '7f5f720a51c6ff592b0c805ad50a5bfa0878393613d6a7b1e6fff18cac509c66');
  assert.equal(receipt.fullClient.sizeBytes, 260877805);
  assert.equal(receipt.minimumLauncherVersion, null);
  assert.equal(receipt.fullClient.url, 'https://drive.google.com/file/d/1Q0tx4tz73Yqp9xbNntZR6jGXoPXiIgvh/view?usp=drive_link');
  assert.equal(new URL(receipt.fullClient.url).hostname, 'drive.google.com');
  assert.match(html('download/index.html'), /adapted native launcher/);
  assert.match(html('download/index.html'), /launcher nativo adaptado/);
  assert.equal(html('download/index.html').includes('HTTPS launcher'), false);
  assert.ok(html('download/index.html').includes(receipt.fullClient.url), 'download page does not link the full client');
  assert.ok(html('index.html').includes(`Client ${receipt.clientVersion}`));
  assert.match(html('download/index.html'), /Full installation: 7\.48\.100/);
  assert.equal(html('download/index.html').includes('href="/patch/patch-48018.zip"'), false);
  assert.match(html('download/index.html'), /Earlier patches are no longer offered/);
  assert.equal(/patch-1\d{4}\.zip/.test(html('download/index.html')), false, 'retired patch ZIP still linked');
  assert.match(html('changelog/index.html'), /Client 7\.48\.001 — new full client/);
  assert.match(html('changelog/index.html'), /Client 7\.48\.017/);
  assert.match(html('news/client-7-48-002/index.html'), /Every client update is mandatory/);
  assert.equal(/Patch 100\d\d/.test(html('changelog/index.html')), false, 'retired patch history still listed');
  assert.match(html('news/client-7-48-001/index.html'), /choose the game language/);
  assert.match(html('news/client-7-48-013/index.html'), /Master.*Elite/s);
  assert.match(html('news/client-7-48-017/index.html'), /Cube.*Mystic Land/s);
  assert.match(html('news/server-stability-2026-11-01/index.html'), /database delay.*No client update is needed/s);
  assert.match(html('changelog/index.html'), /Server update — fewer disconnects/);
  assert.ok(html('changelog/index.html').includes(receipt.clientVersion));
  assert.match(html('news/index.html'), /client-7-48-108/);
  assert.equal(existsSync(resolve(dist, 'news/client-7-48-018/index.html')), false, 'review draft must not be published');
  assert.equal(html('news/index.html').includes('client-7-48-018'), false);
  assert.match(html('roadmap/index.html'), /Planned/);
  assert.match(html('player/index.html'), /id="profile-panel" hidden/);
  assert.match(html('player/index.html'), /id="sessions-revoke-all"/);
  assert.match(html('player/index.html'), /not live game state/);
});
