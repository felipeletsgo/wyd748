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
