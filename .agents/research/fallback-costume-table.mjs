// Generate or verify FallbackCostumeTable.h from the original
// TMSkinMesh::SetCostume branch chain at a fixed Git revision.
//
// The original body only assigns szTexture/szName from string literals, cycles
// m_Cos, and delegates unknown types to SetOldCostume. It is valid JavaScript
// after those three statement forms are rewritten, so it is evaluated directly
// over a domain much larger than its literals. Every type must either delegate
// for every m_Cos, or follow the table schema exactly:
//   texture for every m_Cos (part 1 may use firstPartTexture),
//   meshes[m_Cos - 1] and m_Cos -> m_Cos % 6 + 1 for parts 1..6,
//   no mesh and no m_Cos change otherwise.
//
// Usage (repository root):
//   node .agents/research/fallback-costume-table.mjs --write
//   node .agents/research/fallback-costume-table.mjs --check
import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync } from 'node:fs';

const REVISION = '71c4b313';
const BASELINE = 'tmproject/TMProject748/internal/render/mesh/TMSkinMesh.cpp';
const HEADER = 'tmproject/TMProject748/internal/render/mesh/FallbackCostumeTable.h';
const SIGNATURE = 'void TMSkinMesh::SetCostume(int Costype, char* szTexture, char* szName)';

const source = execFileSync('git', ['show', `${REVISION}:${BASELINE}`]).toString().replace(/\r\n/g, '\n');
const start = source.indexOf(SIGNATURE);
const end = source.indexOf('\n}\n', start);
if (start < 0 || end < 0) throw new Error('SetCostume not found at baseline');
let body = source.slice(source.indexOf('{', start) + 1, end);
body = body.replace(/\/\/[^\n]*/g, '')
  .replace(/strcpy\(szTexture, ("[^"]*")\);/g, 'out.texture = $1;')
  .replace(/strcpy\(szName, ("[^"]*")\);/g, 'out.name = $1;')
  .replace(/SetOldCostume\(Costype, szTexture, szName\);/g, 'out.delegated = true;');
if (/strcpy|sprintf|SetOldCostume|\bm_(?!Cos\b)\w+|\bthis\b/.test(body))
  throw new Error('SetCostume contains a statement outside the evaluated subset');
const evaluate = new Function('Costype', 'cos', `const out = {}; let m_Cos = cos;\n${body}\nout.next = m_Cos; return out;`);

const COS = [];
for (let c = -16; c <= 16; ++c) COS.push(c);
const rows = [];
for (let type = -64; type <= 1024; ++type) {
  const results = COS.map((c) => [c, evaluate(type, c)]);
  if (results.every(([c, o]) => o.delegated && o.texture === undefined && o.name === undefined && o.next === c))
    continue;
  if (results.some(([, o]) => o.delegated)) throw new Error(`type ${type} delegates only for some parts`);
  const texture = evaluate(type, 2).texture;
  const first = evaluate(type, 1).texture;
  const meshes = [];
  for (const [c, o] of results) {
    const expectedTexture = c === 1 ? first : texture;
    if (o.texture === undefined || o.texture !== expectedTexture) throw new Error(`type ${type} part ${c}: texture`);
    if (c >= 1 && c <= 6) {
      if (o.name === undefined || o.next !== c % 6 + 1) throw new Error(`type ${type} part ${c}: mesh cycle`);
      meshes.push(o.name);
    } else if (o.name !== undefined || o.next !== c) {
      throw new Error(`type ${type} part ${c}: unexpected mesh or part change`);
    }
  }
  rows.push({ type, texture, first: first === texture ? null : first, meshes });
}

const literal = (s) => (s === null ? 'nullptr' : JSON.stringify(s));
const header = [
  '#pragma once',
  '',
  '#include <cstring>',
  '',
  '// Mesh and texture selection for costume types without a native 7.48 renderer',
  '// (see CostumeSelection.h). Generated from the original TMSkinMesh::SetCostume',
  `// branch chain at ${REVISION} by .agents/research/fallback-costume-table.mjs;`,
  '// regenerate or verify with that tool instead of editing rows by hand.',
  'namespace fallback_costume',
  '{',
  'struct Costume',
  '{',
  '    int type;',
  '    const char* texture;',
  '    const char* firstPartTexture; // part 1 override, or nullptr',
  '    const char* meshes[6];        // parts 1..6',
  '};',
  '',
  `constexpr Costume kCostumes[${rows.length}] = {`,
  ...rows.map((r) => `    {${r.type}, ${literal(r.texture)}, ${literal(r.first)},\n        {${r.meshes.map(literal).join(', ')}}},`),
  '};',
  '',
  'constexpr const Costume* Find(int type)',
  '{',
  '    for (const Costume& costume : kCostumes)',
  '        if (costume.type == type)',
  '            return &costume;',
  '    return nullptr;',
  '}',
  '',
  '// Applies one part selection. The texture is always written; parts 1..6 also',
  '// write their mesh and advance the part cycle, other parts leave both alone.',
  'inline void Apply(const Costume& costume, int& part, char* texture, char* mesh)',
  '{',
  '    const char* selected = part == 1 && costume.firstPartTexture ? costume.firstPartTexture : costume.texture;',
  '    std::memcpy(texture, selected, std::strlen(selected) + 1);',
  '    if (part >= 1 && part <= 6) {',
  '        const char* partMesh = costume.meshes[part - 1];',
  '        std::memcpy(mesh, partMesh, std::strlen(partMesh) + 1);',
  '        part = part % 6 + 1;',
  '    }',
  '}',
  '}',
  '',
].join('\n');

if (process.argv.includes('--write')) {
  writeFileSync(HEADER, header);
  console.log(`wrote ${rows.length} costume types to ${HEADER}`);
} else if (process.argv.includes('--check')) {
  const current = readFileSync(HEADER, 'utf8').replace(/\r\n/g, '\n');
  if (current !== header) {
    console.error(`FAIL: ${HEADER} differs from the table generated from ${REVISION}`);
    process.exit(1);
  }
  console.log(`PASS: ${rows.length} costume types match ${SIGNATURE} at ${REVISION}`);
} else {
  console.log(`${rows.length} costume types (dry run)`);
}
