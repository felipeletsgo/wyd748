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
  assert.equal(receipt.version, 1); assert.equal(receipt.patchVersion, 10043);
  assert.equal(receipt.publishedAt, '2026-10-23');
  assert.deepEqual(receipt.serverUpdate, { id: 'mandatory-update-sizes-10043', publishedAt: '2026-10-23' });
  assert.match(html('news/mandatory-update-sizes-10043/index.html'), /Client update required/);
  assert.match(html('changelog/index.html'), /Patch 10043 — required update, creature sizes and fairy fixes/);
  assert.match(html('news/big-cube-staff-notices-10042/index.html'), /ten-second timer/);
  assert.match(html('changelog/index.html'), /Patch 10042 — Big Cube, staff notices and planned maintenance/);
  assert.match(html('news/combat-events-persistence-10041/index.html'), /payment confirmation/);
  assert.match(html('changelog/index.html'), /Patch 10041 — combat resources and event safety/);
  assert.match(html('news/player-scale-10039/index.html'), /active game-server release/);
  assert.match(html('news/camera-con-10040/index.html'), /No game-server restart is required/);
  assert.match(html('changelog/index.html'), /Patch 10040 — camera height correction/);
  assert.match(html('changelog/index.html'), /Patch 10039 — player size correction/);
  assert.match(html('news/world-bosses-limits-10038/index.html'), /gate keepers/);
  assert.match(html('changelog/index.html'), /Patch 10038 — world bosses and 32-bit limits/);
  assert.match(html('news/skills-items-10037/index.html'), /Exchange Stones/);
  assert.match(html('changelog/index.html'), /Patch 10037 — skills, items and exchange stones/);
  assert.match(html('news/instance-expiry-persistence/index.html'), /pending kill settlement/);
  assert.match(html('news/instance-expiry-persistence/index.html'), /Launcher patch 10036/);
  assert.match(html('news/client-controls-10036/index.html'), /Ctrl\+Right-click/);
  assert.match(html('changelog/index.html'), /Patch 10036 — keyboard and inventory shortcuts/);
  assert.match(html('changelog/index.html'), /Instance expiry persistence/);
  assert.match(html('news/hell-gate-rune-track-759/index.html'), /regardless of party order/);
  assert.match(html('news/hell-gate-rune-track-759/index.html'), /latest launcher patch remains 10035/);
  assert.match(html('changelog/index.html'), /Hell Gate and Rune Track/);
  assert.equal(receipt.patch.sizeBytes, 913376);
  assert.equal(receipt.patch.sha256, '98F54183D7361484026CE8F4213568886FC76D0549E24A7A96980E1AE16C2FFD');
  assert.equal(receipt.patch.executableSha256, '7FE98A22B51E2EA8864FF31483186866075B648F81C554592218AD523E55DC64');
  assert.equal(receipt.patch.url, '/patch/patch-10043.zip'); assert.equal(receipt.updateFeed, '/patch/update.htm');
  assert.match(html('index.html'), /Latest published patch 10043/);
  assert.match(html('news/client-safety-10035/index.html'), /client stability safeguards/);
  assert.match(html('changelog/index.html'), /Patch 10035 — client stability safeguards/);
  assert.match(html('news/hell-gate-routes-10034/index.html'), /400,000 HP/);
  assert.match(html('news/combat-controls-and-balances-10033/index.html'), /201% attack speed/);
  assert.match(html('news/combat-controls-and-balances-10033/index.html'), /one private top notification/);
  assert.match(html('changelog/index.html'), /Patch 10033 — combat, controls and balances/);
  assert.match(html('news/nightmare-lifecycle-10032/index.html'), /15 minutes/);
  assert.match(html('news/nightmare-lifecycle-10032/index.html'), /five minutes remaining/);
  assert.match(html('changelog/index.html'), /Patch 10032 — Nightmare lifecycle/);
  assert.match(html('news/instance-leveling-balance/index.html'), /200,000, 300,000 and 500,000/);
  assert.match(html('news/instance-leveling-balance/index.html'), /neither gold nor items/);
  assert.match(html('changelog/index.html'), /Dedicated instance leveling monsters/);
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
