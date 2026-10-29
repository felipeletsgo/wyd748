# Contrato nativo 7.48: seleção dos trajes KR

Procedência: síntese da análise já registrada na antiga documentação da cadeia
de patches, retirada da documentação operacional em 23/09/2026. Não representa
nova execução do client nem nova análise de Ghidra nesta revisão. O manifesto
`references/client748/Costumes-KR.json`, o binário de referência e o histórico
Git preservam os dados originais para conferência posterior.

## Fronteira observada

- `Equip[13]` (`Pos=8192`) alimenta a seleção do traje no 7.48.
- O seletor nativo admite originalmente apenas IDs `4151..4200`; a coleção KR
  precisa de admissão pela lista exata de itens disponíveis, preservando o
  comportamento original para IDs externos ao manifesto.
- A representação visual enviada ao mundo conserva os 12 bits baixos do item;
  o namespace `4xxx` precisa ser recomposto antes da consulta exata.
- A primeira parte pode conter geometria real. Quando `part0` está vazia, a
  face nativa continua; partes vazias posteriores não deslocam as demais.
- A marca `0x4000` em `m_nCosType` isola os renderers importados e é removida
  antes da consulta à tabela, sem interceptar personagens ou NPCs nativos.
- A classificação corporal no manifesto usa `EF_CLASS=5` para TK/BM,
  `EF_CLASS=10` para FM/HT e `EF_CLASS=15` somente para renderers dinâmicos.

Este registro preserva o contrato necessário à implementação em source, não o
procedimento antigo de modificação binária. O funcionamento visual do executável
recompilado ainda exige `CLIENT-TESTED` em ambiente que permita observá-lo.

## Costume faces: KR SetCostume part 0 (2026-10-29)

Provenance: read-only Ghidra 12.1.2 inspection of the WYD KR client
(`references/WYD_KR 7.70+/WYD.exe`, SHA-256
`71d2e7746831fae932313602cfe827575fa3c2a0c61d3566142563b2814c872a`, project
`tmproject/build/native-research/kr/WYDKR.gpr`). Static evidence only.

- `TMSkinMesh::RestoreDeviceObjects` calls `SetCostume` (`0x005298F0`, call at
  `0x00537E9E`) for every part when `m_nCosType` (`+0x368`) is neither 0 nor
  100. `SetCostume` selects a case by type and a part by the cursor `+0x378`.
- Case `0x21` (type 33, Student_Set 4183) at `0x0052BB4C`: texture
  `mesh\ch0101114.wys`; jump table `0x00536314` maps cursor 1..6 to
  `ch0101114.msh` .. `ch0106114.msh`. Cursor 1 is part 0, the costume face.
- Case `0x19` (type 25, Dancing_Girl 4175) at `0x0052AA9C`: texture
  `mesh\ch0101105.wys`, meshes `ch0101105.msh` .. `ch0106105.msh`.
- Cases 131..138 and 271..274 have five entries (`02`..`06`) and no `01`
  face asset; the KR cursor puts mesh `02` into part 0.
- Asset bones: `ch0101114.msh` and `ch0101105.msh` bind only bone 8 (head).
  `ch0102114.msh` and `ch0102105.msh` bind the torso and bone 8; they form the
  neck and head shell around which the costume face sits.

Consequence: the manifest left part 0 empty for types 25 and 33, so the client
kept the native face inside the costume head shell, and the head showed no
face. The manifest now carries both KR face parts. An empty part 0 remains only
for the twelve five-entry types, where no costume face asset exists.
