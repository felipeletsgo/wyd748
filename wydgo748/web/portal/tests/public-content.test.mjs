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
  for (const path of ['guilds', 'play', 'play/getting-started', 'play/classes', 'play/server', 'play/rules', 'play/faq', 'news', 'news/account-protection', 'news/client-7-48-001', 'news/client-7-48-002', 'changelog', 'roadmap']) {
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
test('download receipt pairs update 7.48.011 with the existing full-client baseline', () => {
  const receipt = JSON.parse(html('client-manifest.json'));
  assert.equal(receipt.version, 2); assert.equal(receipt.clientVersion, '7.48.011');
  assert.equal(receipt.feedVersion, 48011);
  assert.equal(receipt.patch.url, '/patch/patch-48011.zip');
  assert.equal(receipt.patch.sizeBytes, 929053);
  assert.equal(receipt.patch.sha256, '362682032FCCEA7C3A6F96CE4F80DAF146D047CE4407D5335AE10EE8DAB71806');
  assert.equal(receipt.patch.executableSha256, '3C68615BCA0DCEDD73ACF7309C04690B5B6303A1630533FB6978EE50F7ADE8A8');
  assert.equal(receipt.fullClient.clientVersion, '7.48.001');
  assert.equal(receipt.updateFeed, '/patch/update.htm');
  assert.equal(receipt.fullClient.sha256, null); assert.equal(receipt.fullClient.sizeBytes, null);
  assert.equal(receipt.minimumLauncherVersion, null);
  assert.equal(receipt.fullClient.url, 'https://drive.google.com/file/d/1ouCjq-A8JOnHRXpO0m8fEhRm2NuGgmFk/view?usp=drive_link');
  assert.ok(html('download/index.html').includes('1ouCjq-A8JOnHRXpO0m8fEhRm2NuGgmFk'), 'download page does not link the full client');
  assert.match(html('index.html'), /Client 7\.48\.011/);
  assert.match(html('download/index.html'), /Full installation: 7\.48\.001/);
  assert.match(html('download/index.html'), /href="\/patch\/patch-48011\.zip"/);
  assert.match(html('download/index.html'), /Earlier patches are no longer offered/);
  assert.equal(/patch-1\d{4}\.zip/.test(html('download/index.html')), false, 'retired patch ZIP still linked');
  assert.match(html('changelog/index.html'), /Client 7\.48\.001 — new full client/);
  assert.match(html('changelog/index.html'), /Client 7\.48\.011/);
  assert.match(html('news/client-7-48-002/index.html'), /Every client update is mandatory/);
  assert.equal(/Patch 100\d\d/.test(html('changelog/index.html')), false, 'retired patch history still listed');
  assert.match(html('news/client-7-48-001/index.html'), /choose the game language/);
  // Every published client must be announced in News, not only in the changelog.
  assert.ok(html('news/index.html').includes(receipt.clientVersion), 'News does not announce the current client');
  assert.match(html('roadmap/index.html'), /Planned/);
  assert.match(html('player/index.html'), /id="profile-panel" hidden/);
  assert.match(html('player/index.html'), /id="sessions-revoke-all"/);
  assert.match(html('player/index.html'), /not live game state/);
});
