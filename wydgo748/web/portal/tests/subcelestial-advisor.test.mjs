import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const compiled = ts.transpileModule(readFileSync(new URL('../src/lib/subcelestial-advisor.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
class Element {
  children = []; hidden = false; open = false; textContent = '';
  constructor(tag = 'div') { this.tag = tag; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.textContent = ''; this.children = []; this.append(...children); }
}
const advice = { applicable: true, requirementsMet: false, requiredLevel: 121, requiredFame: 100,
  checks: ['celestial', 'level', 'fame', 'sefirot', 'idealStone', 'noAlternate', 'face', 'rewardSpace'].map(id => ({ id, met: id !== 'fame' })) };
function fixture() {
  const context = { exports: {}, document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') } };
  vm.runInNewContext(compiled, context);
  const disclosure = new Element('details'), checks = new Element('dl'), status = new Element();
  return { controller: new context.exports.SubCelestialAdvisor(disclosure, checks, status), disclosure, checks, status };
}

test('SubCelestial advice is collapsed with six compact saved gates, not CP or an account slot', () => {
  const f = fixture(); f.controller.render(advice);
  assert.equal(f.disclosure.hidden, false); assert.equal(f.disclosure.open, false);
  assert.deepEqual(f.checks.children.map(child => child.textContent), [
    'Celestial level 121', 'Met', '100 Fame', 'Missing', 'Class Sefirot · Equipped', 'Met',
    'Ideal Stone · Inventory', 'Met', 'SubCelestial not created', 'Met', 'Reward space', 'Met',
  ]);
  assert.equal(f.status.textContent, 'Missing requirements.');
  f.controller.render({ ...advice, requirementsMet: true, checks: advice.checks.map(check => ({ ...check, met: true })) });
  assert.equal(f.status.textContent, 'Saved requirements met. Use the Ideal Stone in-game.');
  assert.equal(f.checks.children.some(child => /CP|account slot|crystal/i.test(child.textContent)), false);
  f.controller.render({ ...advice, checks: advice.checks.map(check => ({ ...check, met: check.id !== 'face' })) });
  assert.equal(f.checks.children[10].textContent, 'Character appearance');
  assert.equal(f.checks.children[11].textContent, 'Missing');
});

test('clear, non-Celestial and older replies remove saved readiness and collapse the section', () => {
  const f = fixture(); f.controller.render(advice); f.disclosure.open = true; f.controller.clear();
  assert.equal(f.disclosure.hidden, true); assert.equal(f.disclosure.open, false);
  assert.equal(f.checks.children.length, 0); assert.equal(f.status.textContent, '');
  f.controller.render({ ...advice, applicable: false, checks: advice.checks.map(check => ({ ...check, met: check.id !== 'celestial' })) });
  assert.equal(f.disclosure.hidden, true);
  f.controller.render(undefined); assert.equal(f.disclosure.hidden, true);
});

test('malformed SubCelestial advice fails closed without injection, duplicate gates or stale readiness', () => {
  const f = fixture();
  for (const value of [null, {}, false, 'bad', { ...advice, applicable: false }, { ...advice, requirementsMet: true },
    { ...advice, requiredLevel: 0 }, { ...advice, requiredFame: -1 }, { ...advice, requiredFame: 4294967296 },
    { ...advice, requiredFame: '<img onerror=attack()>' }, { ...advice, checks: [] },
    { ...advice, checks: [null, ...advice.checks.slice(1)] }, { ...advice, checks: [false, ...advice.checks.slice(1)] },
    { ...advice, checks: advice.checks.map(check => ({ ...check, id: 'celestial' })) },
    { ...advice, checks: advice.checks.map(check => ({ ...check, met: 'yes' })) }]) {
    f.controller.render(advice); f.controller.render(value);
    assert.equal(f.checks.children.length, 0);
    assert.equal(f.status.textContent.includes('requirements met'), false);
    assert.equal(f.status.textContent.includes('<img'), false);
  }
});
