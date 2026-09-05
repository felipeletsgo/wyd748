# Documentação do Basedef

## Escopo e procedência

Lote local de documentação, dentro de `MODERNIZACAO_COMPATIVEL`.
Os comentários descrevem a implementação atual de `internal/core/Basedef.cpp`;
não promovem equivalência com o binário nativo nem maturidade de pesquisa.
Não houve mudança de lógica, assinatura, layout ou ownership neste lote.

## Cobertura confirmada

23 funções receberam comentários individuais em português com finalidade,
entradas, retornos, efeitos e pré-condições relevantes:

- início do arquivo: `BASE_ScreenResize` até `BASE_InitializeBaseDef`, incluindo
  `strfmt`, loaders e somas legadas;
- final do arquivo: `BASE_CanRefine` até `BASE_RemoveEffect`.

O restante ainda não está documentado função a função. Títulos genéricos de
seção não contam como cobertura de API. A divisão integral do header também
continua pendente; `UiLayout.h` é apenas a extração inicial.

## Lacunas observadas na source

- `BASE_ReadInitItem` retorna sucesso sem carregar nada: stub pendente.
- `BASE_InitializeAttribute` não verifica leitura curta.
- `BASE_InitEffectString` não verifica os resultados de `fscanf` e suas larguras
  não garantem espaço para o terminador em todas as leituras.
- `strfmt` devolve armazenamento estático compartilhado.

Essas limitações foram documentadas, não corrigidas neste lote de comentários.

## Validação e retomada

`git diff --check` passou. Nenhum build novo é exigido para comentários pela
skill `wyd-go-feature`. O resultado final do último build iniciado na conversa
anterior não foi recuperado; não apresentar ausência de processo como sucesso.

## Inventário completo da API legada

As funções restantes foram revisadas e permanecem agrupadas por contrato:

- carregamento/configuração: `BASE_ReadItemPrice`, `BASE_ReadTOTOList`, `BASE_InitializeServerList`, `BASE_GetLanguage`, `ReadItemicon`, `ReadItemName`, `ReadUIString`, `ReadNameFiltraDataBase`, `ReadChatFiltraDataBase`, `EnableSysKey`, `DisableSysKey`, `CheckOS`;
- texto/validação: `BASE_UnderBarToSpace`, `BASE_GetHttpRequest`, `BASE_CheckValidString`, `BASE_CheckChatValid`, `CheckGuildName`, `IsClearString`, `IsClearString2`;
- tabelas/localização: `BASE_GetWeekNumber`, `BASE_GetVillage`, `BASE_GetSubGuild`, `BASE_GetAttribute`, `BASE_GetAttr`, `BASE_IsInLowZone`, `BASE_GetColorCount`;
- itens/equipamento: `BASE_GetItemSanc`, `BASE_GetItemAbility`, `BASE_GetStaticItemAbility`, `BASE_GetBonusItemAbilityNosanc`, `BASE_GetBonusItemAbility`, `BASE_GetItemAbilityNosanc`, `BASE_GetItemAmount`, `BASE_SetItemAmount`, `BASE_CanCarry`, `BASE_CanTrade`, `BASE_ClearItem`, `BASE_SortTradeItem`, `BASE_CanCargo`, `BASE_CanEquip`, `BASE_CanEquip_RecvRes`, `BASE_GetItemColorEffect`, `BASE_GetMeshIndex`;
- combate: `BASE_GetSpeed`, `BASE_GetManaSpent`, `BASE_GetSkillDamage`, `BASE_GetMobAbility`, `BASE_GetMaxAbility`, `BASE_DefineSkinMeshType`, `BASE_GetMountScale`, `IsSkill`, `GetSkillIndex`, `IsValidSkill`, `IsValidClassSkill`;
- navegação/geometria: `BASE_GetRoute`, `BASE_GetDistance`, `BASE_GetHitPosition`, `BASE_GetHitPosition2`, `BASE_Get3DTo2DPos`, `BASE_SetBit`, `BASE_UpdateItem2`;
- efeitos: `BASE_CanRefine`, `IsPassiveSkill`, `BASE_HasSancAdd`, `BASE_GetSancEffValue`, `BASE_GetItemSancSuccess`, `BASE_GetEffectValue`, `BASE_ChangeOrAddEffectValue`, `BASE_RemoveEffect`.

As assinaturas, buffers emprestados, tabelas globais e retornos históricos foram
preservados. Funções sem comentário individual ainda exigem detalhamento
posterior; não foram reinterpretadas como contratos nativos 7.48.
