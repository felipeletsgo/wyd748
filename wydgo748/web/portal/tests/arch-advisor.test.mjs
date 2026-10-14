import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const compiled = ts.transpileModule(readFileSync(new URL('../src/lib/arch-advisor.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
class Element {
  children = []; hidden = false; open = false; textContent = '';
  constructor(tag = 'div') { this.tag = tag; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.textContent = ''; this.children = []; this.append(...children); }
}
const advice = { applicable: true, requirementsMet: false, requiredLevel: 371,
  checks: ['mortal', 'level', 'eternalStone', 'sefirot', 'freeSlot'].map(id => ({ id, met: id !== 'sefirot' })) };
function fixture() {
  const context = { exports: {}, document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') } };
  vm.runInNewContext(compiled, context);
  const disclosure = new Element('details'), checks = new Element('dl'), status = new Element();
  return { controller: new context.exports.ArchAdvisor(disclosure, checks, status), disclosure, checks, status };
}
test('Arch advice renders a compact collapsed checklist, saved status and no invented Fame requirement', () => {
  const f = fixture(); f.controller.render(advice);
  assert.equal(f.disclosure.hidden, false); assert.equal(f.disclosure.open, false);
  assert.deepEqual(f.checks.children.map(child => child.textContent), [
    'Level 371', 'Met', 'Eternal Stone · Accessory 3', 'Met', 'Sefirot · Accessory 4', 'Missing', 'Free character slot', 'Met',
  ]);
  assert.equal(f.status.textContent, 'Missing requirements.');
  f.controller.render({ ...advice, requirementsMet: true, checks: advice.checks.map(check => ({ ...check, met: true })) });
  assert.equal(f.status.textContent, 'Saved requirements met. Ascend at the king in-game.');
  assert.equal(f.checks.children.some(child => /Fame|CP/.test(child.textContent)), false);
});
test('clear and non-Mortal replies remove all private advice and collapse the section', () => {
  const f = fixture(); f.controller.render(advice); f.disclosure.open = true; f.controller.clear();
  assert.equal(f.disclosure.hidden, true); assert.equal(f.disclosure.open, false);
  assert.equal(f.checks.children.length, 0); assert.equal(f.status.textContent, '');
  f.controller.render({ ...advice, applicable: false, checks: advice.checks.map(check => ({ ...check, met: check.id !== 'mortal' })) });
  assert.equal(f.disclosure.hidden, true); assert.equal(f.checks.children.length, 0);
  f.controller.render(undefined); assert.equal(f.disclosure.hidden, true);
});
test('malformed advice never claims requirements met, injects content or retains an old checklist', () => {
  const f = fixture();
  const invalid = [null, {}, { ...advice, applicable: 'true' }, { ...advice, requirementsMet: true },
    { ...advice, requiredLevel: '<img onerror=attack()>' }, { ...advice, requiredLevel: 0 },
    { ...advice, requiredLevel: 4294967297 }, { ...advice, checks: [] },
    { ...advice, checks: [null, ...advice.checks.slice(1)] },
    { ...advice, checks: [false, ...advice.checks.slice(1)] },
    { ...advice, checks: advice.checks.map(check => ({ ...check, id: 'mortal' })) },
    { ...advice, checks: advice.checks.map(check => ({ ...check, met: 'yes' })) },
    { ...advice, checks: [...advice.checks.slice(0, 4), { id: '<img onerror=attack()>', met: false }] },
    { ...advice, applicable: false },
  ];
  for (const value of invalid) {
    f.controller.render(advice); f.controller.render(value);
    assert.equal(f.checks.children.length, 0);
    assert.equal(f.status.textContent.includes('requirements met'), false);
    assert.equal(f.status.textContent.includes('<img'), false);
  }
});
