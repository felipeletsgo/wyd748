---
id: skill-visual-dispatch
title: Sequencia de animacao e despacho visual das skills
subsystem: combat
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-15
---

# Sequência de animação e despacho visual das skills

## Pergunta

Por que o personagem pode ficar parado ao lançar uma skill e por que algumas
skills criam o efeito de outra? O caso inicial é a redução de resistência de
TransKnight. A triagem cobre todos os 104 registros, sem equiparar essa cobertura
à comparação visual completa de todas as partículas e transformações.

## Fronteira de evidência

- Nativo 7.48: corpus `C:/Users/felipe/Tools/GhidraAnalysis/20260821/decompiled`,
  com o SHA histórico acima. Binário de referência não modificado.
- Runtime: `tmproject/client748/SkillData.bin`, `SkillData.csv`, `AniSound4.txt`,
  `mesh/BoneAni4.txt`, `mesh/ValidIndex.bin`, animações e tabela de texturas.
- Source: `TMHuman.cpp`, `TMSkillBash.cpp`, `TMSkillMagicShield.cpp`,
  `WYD748Assets.cpp`, `MeshManager.cpp`, `TMFieldScene.cpp`.
- Servidor: leitura do affect 3 em `wydgo748/internal/game/affects.go`, que reduz
  resistências. Nenhuma mudança Go ou no wire neste lote.

## Fluxo nativo 7.48

### Callers

O processamento de ataque `FUN_00489a3e` seleciona o ator, destino e skill e
alimenta a animação/efeito. A recepção atual é `TMFieldScene::OnPacketAttack`;
`SkillUse` também usa esse caminho para previsão local. O tick do ator consome
o evento diferido em `FUN_00506f9d` / `TMHuman::FrameMoveEffect`.

### Função principal

`FUN_00522126` (Attack por coordenadas) indexa cada passo por `i + 3` somente
quando montado, e por `i` a pé. Na source, a precedência de `i + mount ? 3 : 0`
repetia os slots 0/3 em vez de percorrer a sequência. `FUN_00521d53` cobre a
sobrecarga com alvo. O efeito em `FUN_00506f9d` usa o ID mesmo com alvo nulo:
as coordenadas do evento são suficientes para casts no chão.

### Callees

- `FUN_0054f946`: carga dos registros compactos de skills, XOR 0x5A.
- `FUN_0051dba4`: seleção de animação; consumidores atuais SetAnimation,
  AniSound4 e slots de MeshManager::InitBoneAnimation.
- `FUN_004ecd90`: SlowSlash, recebe owner opcional; ID 2.
- `FUN_004e0990` / `FUN_004e0af7`: construção/tick de Bash, vetores X,Y,Z.
- `FUN_004ee730`: TownPortal, vetor X,Y,Z sem troca interna.
- `FUN_004e4a10`: Haste, variantes 0/1/2/3 para IDs 41/43/90/54.
- `FUN_004e88a0` / `FUN_004e8a95`: MagicShield e StartVisible. Em
  `FUN_004faf92`, a variante 1 ocupa actor+0x51c (m_pRescue atual).
  ID 3 desperta esse objeto no alvo e toca som 158, não cria Haste.

## Estado e lifecycle

Attack monta a fila Act1/Act2, seleciona a primeira animação e conserva o término
ECMOTION_NONE. O evento diferido tem prazo, ID, destino e alvo opcional. Os efeitos
temporários continuam pertencendo ao container e usando seus tempos de vida.
Rescue reutiliza o objeto do alvo, não cria outro owner. Seus checks de alvo e
objeto nulos são preservados. O construtor do shield passa a obter Z do owner.
ID 89 passa a exigir skin mesh antes de acessar seus rastros de arma.

O cleanup atual de efeitos não projéteis foi preservado, retirando apenas ID 90
da exceção de multiataque: Haste não incrementa contador de projétil. Não se afirma
paridade literal de todo o cleanup moderno com o nativo. Nenhuma mudança de
destrutores, logout, shutdown, ownership ou protocolo neste lote.

## Wire, ABI e recursos

104 registros de 96 bytes, Act1 em 48, Act2 em 56; Passive em 88. O loader atual
copia corretamente os campos: não foi encontrada defasagem da tabela binária.
CSV e binário concordam nos campos comparáveis (ForceDamage não está no CSV).
Os nomes do CSV não provam a semântica: ID 2 aparece como Golpe_Duplo, mas o
affect de resistência e o despacho nativo indicam SlowSlash. Não renomear assets
com base apenas nesse rótulo. 14 registros são passivos e 103 é reservado.

## Mapeamento atual

Correções funcionais apenas nos três C++ acima. A auditoria reproduz os slots de
animações humanas 0/1, fallback de arma e tabelas de quatro classes; projeta as
89 skills ativas a pé/montadas, 60 armas, até três passos. Ela registra os IDs
com despacho nativo explícito e nomes das classes instanciadas, mas isso não
prova identidade de todos os parâmetros dos efeitos.

## Matriz de delta

| Claim | Nativo 7.48 | Source antes | Decisão |
| --- | --- | --- | --- |
| Fila no chão | i + offset da montaria | ternário com precedência errada | PARIDADE_NATIVA: corrigir Act1/Act2 |
| Evento sem alvo | conserva ID e destino | zera ID; também elimina por distância local | PARIDADE_NATIVA: preservar despacho |
| Resistência ID 2 | SlowSlash sem exigir sobreposição | DoubleSwing; SlowSlash no ID 16 com sobreposição | PARIDADE_NATIVA: restabelecer IDs 2/16 |
| Rescue ID 3 | objeto do alvo, som 158 | Haste novo, som 159 | PARIDADE_NATIVA: StartVisible do m_pRescue |
| Hunter | azul 74, branco 79, LevelUp 86, Haste 90, som 91, flechas 95 | 75,81,85,90 como som,79 como flechas | PARIDADE_NATIVA: restabelecer despacho |
| Posição Bash/Portal/Shield | X,Y,Z | eixos trocados ou Z sem owner | PARIDADE_NATIVA: corrigir coordenadas |
| Ausência de mesh ID 89 | acesso dependente do ator | dereferência sem guarda | MODERNIZACAO_COMPATIVEL: guarda local |
| Término ID 90 | Haste, não projétil | exceção moderna de projétil | MODERNIZACAO_COMPATIVEL: retirar exceção obsoleta |

## Decisões

Corrigir somente divergências rastreadas; preservar as mudanças anteriores de
C.C., cooldown, cor de dano e segurança dos buffers. Não remover ID 13 apenas
por não haver ramo equivalente no dispatcher nativo. Não alterar servidor,
assets ou executável instalado. Build candidato com `-NoDeploy`.

## Lacunas

- A cobertura 104/104 é triagem estática, não prova visual integral. Skills sem
  ramo diferido precisam também de packet-time effects, affects e trails.
- AniSound4 humano 1 montado mapeia HOLY para slot 24, podendo resultar em pose
  parada (inclui ID 2). Falta baseline nativo desse asset e execução dessa
  combinação; não substituir a animação por uma escolhida arbitrariamente.
- Morphs, armas/montarias especiais e movimento real das matrizes .ani não
  estão completamente validados. Existência/tamanho do arquivo não prova gesto.
- Texturas-base verificadas: IDs 2/51/52/56/60/122 e respectivos arquivos.
  Frames dinâmicos, demais meshes, cores e todos os parâmetros das partículas
  não foram integralmente comparados/renderizados.
- Não há captura in-game antes/depois; nenhuma skill promovida a CLIENT_TESTED.

## Validação

- STATICALLY VERIFIED: comparação nativa das alterações descritas, nenhum delta
  wire/ABI, 51 IDs explícitos do dispatcher nativo representados na source.
- AUTOMATED TESTED: `pwsh -NoProfile -File
  .agents/research/client748/skill-visual-audit.ps1 -OutputPath
  tmproject/build/skill-visual-audit.json`: 104 registros, 89 ativos,
  86.400 projeções, nenhuma animação listada ausente e zero falhas.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1
  -Configuration Release -NoDeploy`: build Release PASS; ArchitectureTests
  40.721 checks PASS e static assertions PASS. Avisos C4018 preexistentes fora
  dos trechos alterados. Candidato em tmproject/build/TMProject748/Release/WYD.exe.
- SHA256 do candidato: `7BB835A9303E762D17B9C84477B2C7983AF9913A18AA00EF2CFAD1893E06C7F3`.
- Regressão negativa: auditor executado com a expressão antiga Act1 injetada
  apenas em memória rejeitou o caso por precedência e ausência do índice nativo.
  Nenhuma source/asset foi modificada por esse ensaio.
- Client real: não executado. O executável instalado não foi substituído.
- Próximo gate: comparar/renderizar as lacunas acima, começando por HOLY montado
  do humano 1 e depois os efeitos persistentes fora do dispatcher diferido.
