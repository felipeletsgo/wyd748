import assert from 'node:assert/strict';
import { readFileSync, readdirSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const context = { exports: {} };
vm.runInNewContext(ts.transpileModule(readFileSync(new URL('../src/data/news-format.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText, context);
const { parseNewsPost, orderNews } = context.exports;
const dir = new URL('../src/content/news/', import.meta.url);

test('Every news post parses and the order is newest first', () => {
  const posts = readdirSync(dir).filter(name => name.endsWith('.md'))
    .map(name => parseNewsPost(`../content/news/${name}`, readFileSync(new URL(name, dir), 'utf8')));
  const ordered = orderNews(posts);
  assert.ok(ordered.length >= 2);
  assert.equal(ordered[0].slug, 'client-7-48-017');
  for (let i = 1; i < ordered.length; i++) assert.ok(ordered[i - 1].date >= ordered[i].date, `${ordered[i].slug} is out of date order`);
});

test('The post format rejects malformed or unsafe posts', () => {
  const post = body => `---\nnumber: 9\nslug: sample-post\ndate: 2026-10-22\ncategory: News\ntitle: Patch 1: title\nsummary: Short.\n---\n\n${body}\n`;
  const ok = parseNewsPost('../content/news/sample-post.md', post('First line\ncontinues here.\n\nSecond paragraph.'));
  assert.deepEqual([...ok.paragraphs], ['First line continues here.', 'Second paragraph.']);
  assert.equal(ok.title, 'Patch 1: title');
  for (const [name, raw] of [
    ['../content/news/other-name.md', post('Body')],
    ['../content/news/sample-post.md', post('<script>alert(1)</script>')],
    ['../content/news/sample-post.md', post('')],
    ['../content/news/sample-post.md', post('Body').replace('number: 9', 'number: x')],
    ['../content/news/sample-post.md', post('Body').replace('date: 2026-10-22', 'date: 22/10/2026')],
    ['../content/news/sample-post.md', post('Body').replace('category: News\n', 'category: News\nauthor: someone\n')],
  ]) assert.throws(() => parseNewsPost(name, raw), undefined, name);
  assert.throws(() => orderNews([ok, { ...ok }]));
});
