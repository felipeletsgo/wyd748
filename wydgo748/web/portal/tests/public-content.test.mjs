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
  for (const path of ['guilds', 'play', 'play/getting-started', 'play/classes', 'play/server', 'play/rules', 'play/faq', 'news', 'news/account-protection', 'news/weather-update', 'news/fps-command-update', 'news/transformation-citizenship-fix', 'news/transformations-and-armia-update', 'news/costumes-and-refinement-update', 'news/kr-mounts-update', 'news/critical-and-chat-update', 'news/capes-and-arch-update', 'news/world-and-interface-fixes', 'changelog', 'roadmap']) {
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
test('download receipt distinguishes verified incremental patch from unknown full-package metadata', () => {
  const receipt = JSON.parse(html('client-manifest.json'));
  assert.equal(receipt.version, 1); assert.equal(receipt.patchVersion, 10031);
  assert.equal(receipt.publishedAt, '2026-10-16');
  assert.equal(receipt.patch.sizeBytes, 832800);
  assert.equal(receipt.patch.sha256, '38D1B4C3B069A3F19F61E6280EF697FA53BE0E6631FA1617DAD59F1CDD1B03B2');
  assert.equal(receipt.patch.executableSha256, '2F75CA77281CCF21F899A6879DBB1A353C6FBE69A98A2F24C52D6E9309A51F1A');
  assert.equal(receipt.patch.url, '/patch/patch-10031.zip'); assert.equal(receipt.updateFeed, '/patch/update.htm');
  assert.match(html('index.html'), /Latest published patch 10031/);
  assert.match(html('news/chat-labels-and-buff-icons-10031/index.html'), /Critical Armor/);
  assert.match(html('changelog/index.html'), /Patch 10031 — chat labels and buff icons/);
  assert.match(html('news/mounts-interface-and-equipment-update/index.html'), /item-drop crash remains unresolved/);
  assert.match(html('changelog/index.html'), /Patch 10025 — shining item icons/);
  assert.match(html('news/shining-item-icons/index.html'), /Glittering Stone/);
  assert.match(html('changelog/index.html'), /Patch 10026 — guild marks, C.C. rebuff and server fixes/);
  assert.match(html('news/guild-marks-and-cc-update/index.html'), /16x12, 24-bit BMP/);
  assert.match(html('changelog/index.html'), /Patch 10029 — player-reported fixes/);
  assert.match(html('news/player-report-fixes-10029/index.html'), /second Cythera/);
  // Every published patch must be announced in News, not only in the changelog.
  assert.ok(html('news/index.html').includes(String(receipt.patchVersion)), 'News does not announce the latest published patch');
  assert.match(html('changelog/index.html'), /Patch 10022 — Aylin previews, \+10 glow and Lac \+11/);
  assert.match(html('changelog/index.html'), /Patch 10023/);
  assert.match(html('news/aylin-native-glow-follow-up/index.html'), /not fixed by this patch/);
  assert.match(html('changelog/index.html'), /Patch 10021 — dye, trade and chat fixes/);
  assert.match(html('news/dye-trade-chat-fixes/index.html'), /armor set pieces/);
  assert.match(html('news/kefra-uxmal-passage-fix/index.html'), /700 gold/);
  assert.match(html('news/kefra-uxmal-passage-fix/index.html'), /\+100% combat EXP/);
  assert.match(html('news/combat-capes-kefra-update/index.html'), /49/);
  assert.match(html('changelog/index.html'), /In-game confirmation remains pending/);
  assert.equal(receipt.fullClient.sha256, null); assert.equal(receipt.fullClient.sizeBytes, null);
  assert.equal(receipt.minimumLauncherVersion, null);
  assert.ok(html('download/index.html').includes(receipt.patch.sha256));
  assert.match(html('roadmap/index.html'), /Planned/);
  assert.match(html('player/index.html'), /id="profile-panel" hidden/);
  assert.match(html('player/index.html'), /id="sessions-revoke-all"/);
  assert.match(html('player/index.html'), /not live game state/);
});
