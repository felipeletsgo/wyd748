# Score canônico — WYD 7.48+

## Contrato

`model.Score` é a única representação autoritativa de atributos de personagem e mob.
O mesmo layout lógico é declarado como `STRUCT_SCORE` no client TMProject. São 35 campos
`uint32`, em ordem fixa, totalizando **140 bytes** no wire do client TMProject.

Ordem: `Version`, `Level`, `Attack`, `MagicAttack`, `Defense`, `MaxHP`, `MaxMP`,
`CurHP`, `CurMP`, `Str`, `Int`, `Dex`, `Con`, `Accuracy`, `Evasion`, `Parry`,
`Critical`, `Range`, quatro resistências, `SaveMana`, `MagicAmp`, `RegenHP`,
`RegenMP`, `StatusPts`, `MasterPts`, `SkillPts`, quatro `Mastery`, `AttackRun`
e `Merchant`.

Todos os valores persistidos continuam limitados a 2.000.000.000 para manter os
cálculos intermediários e caminhos antigos do renderer dentro do domínio signed.

## Autoridade

- `World` muta `Score`; o client nunca envia score autoritativo.
- `RuntimeScore` é apenas uma cópia calculada do mesmo tipo `Score` com buffs/debuffs;
  não é um segundo formato nem uma fonte persistente.
- PostgreSQL persiste `Score` diretamente no estado da conta. O JSON antigo com
  `score` não possui migração: as contas anteriores a este contrato serão
  recriadas.

## Client TMProject 7.48

O client TMProject recebe os 140 bytes diretamente nos pacotes que embutem score.
Não há truncamento de level, atributos, masteries, HP/MP, ataque ou defesa, e os
campos adicionais fazem parte de `STRUCT_SCORE`. O contrato ativo é uma
extensão coordenada client/server; não usa sidecar nem oferece um caminho
alternativo para o executável histórico 7.48.

A ABI C++ é protegida por `WYD748Compat.cpp`; o encoder Go possui teste byte-a-byte.
Qualquer alteração de ordem/tamanho exige mudança atômica nas duas pontas.
