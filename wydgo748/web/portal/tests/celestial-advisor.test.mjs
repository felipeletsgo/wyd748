import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import test from 'node:test';
import ts from 'typescript';

const compiled = ts.transpileModule(readFileSync(new URL('../src/lib/celestial-advisor.ts', import.meta.url), 'utf8'), {
  compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 },
}).outputText;
class Element {
  children = []; hidden = false; open = false; textContent = '';
  constructor(tag = 'div') { this.tag = tag; }
  append(...children) { for (const child of children) this.children.push(...(child.tag === 'fragment' ? child.children : [child])); }
  replaceChildren(...children) { this.textContent = ''; this.children = []; this.append(...children); }
}
const advice = { applicable: true, requirementsMet: false, requiredLevel: 356, requiredMortalLevel: 400,
  checks: ['arch', 'level', 'mortalLevel', 'idealStone'].map(id => ({ id, met: id !== 'idealStone' })) };
function fixture() {
  const context = { exports: {}, document: { createElement: tag => new Element(tag), createDocumentFragment: () => new Element('fragment') } };
  vm.runInNewContext(compiled, context);
  const disclosure = new Element('details'), checks = new Element('dl'), status = new Element();
  return { controller: new context.exports.CelestialAdvisor(disclosure, checks, status), disclosure, checks, status };
}
test('Celestial advice starts collapsed and shows only saved gates, not a Fame or crystal prerequisite', () => {
  const f = fixture(); f.controller.render(advice);
  assert.equal(f.disclosure.hidden, false); assert.equal(f.disclosure.open, false);
  assert.deepEqual(f.checks.children.map(child => child.textContent), [
    'Arch level 356', 'Met', 'Source Mortal level 400', 'Met', 'Ideal Stone · Inventory', 'Missing',
  ]);
  assert.equal(f.status.textContent, 'Missing requirements.');
  f.controller.render({ ...advice, requirementsMet: true, checks: advice.checks.map(check => ({ ...check, met: true })) });
  assert.equal(f.status.textContent, 'Saved requirements met. Use the Ideal Stone in-game.');
  assert.equal(f.checks.children.some(child => /Fame|CP|crystal/i.test(child.textContent)), false);
});
test('clear, non-Arch and older replies remove private advice and collapse the section', () => {
  const f = fixture(); f.controller.render(advice); f.disclosure.open = true; f.controller.clear();
  assert.equal(f.disclosure.hidden, true); assert.equal(f.disclosure.open, false);
  assert.equal(f.checks.children.length, 0); assert.equal(f.status.textContent, '');
  f.controller.render({ ...advice, applicable: false, checks: advice.checks.map(check => ({ ...check, met: check.id !== 'arch' })) });
  assert.equal(f.disclosure.hidden, true);
  f.controller.render(undefined); assert.equal(f.disclosure.hidden, true);
});
test('malformed Celestial advice fails closed without text injection or stale readiness', () => {
  const f = fixture();
  for (const value of [null, {}, false, 'bad', { ...advice, applicable: false }, { ...advice, requirementsMet: true },
    { ...advice, requiredLevel: 0 }, { ...advice, requiredMortalLevel: 4294967297 },
    { ...advice, requiredMortalLevel: '<img onerror=attack()>' }, { ...advice, checks: [] },
    { ...advice, checks: [null, ...advice.checks.slice(1)] }, { ...advice, checks: [false, ...advice.checks.slice(1)] },
    { ...advice, checks: advice.checks.map(check => ({ ...check, id: 'arch' })) },
    { ...advice, checks: advice.checks.map(check => ({ ...check, met: 'yes' })) }]) {
    f.controller.render(advice); f.controller.render(value);
    assert.equal(f.checks.children.length, 0);
    assert.equal(f.status.textContent.includes('requirements met'), false);
    assert.equal(f.status.textContent.includes('<img'), false);
  }
});
