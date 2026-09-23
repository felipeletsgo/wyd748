# Basedef documentation

## Scope and provenance

This document records the current implementation of `internal/core/Basedef.cpp`
under `MODERNIZACAO_COMPATIVEL`. Its comments do not establish native 7.48
equivalence or research maturity. The initial documentation batch did not
change logic, signatures, layouts, or ownership.

## Confirmed coverage

Twenty-three functions received individual comments describing their purpose,
inputs, outputs, side effects, and relevant preconditions:

- Near the start of the file: `BASE_ScreenResize` through
  `BASE_InitializeBaseDef`, including `strfmt`, loaders, and legacy sums.
- Near the end of the file: `BASE_CanRefine` through `BASE_RemoveEffect`.

The remaining functions have not been documented individually. Generic section
headings do not count as API coverage. The header has not been fully split;
`UiLayout.h` is only the initial extraction.

## Observed source gaps

- `BASE_ReadInitItem` returns success without loading anything; it remains a stub.
- `BASE_InitializeAttribute` does not check for a short read.
- `BASE_InitEffectString` now reads the effect and optional subeffect assets
  through a bounded, atomic loader. It preserves native 7.48 effect indexes
  and leaves a table untouched when its asset is missing or malformed. The
  shipped 7.48 runtime has `EffectString.txt` but no `EffectSubString.txt`.
- The source-only `ReadItemicon` startup read and its unused `g_itemicon` table
  were removed as `MODERNIZACAO_COMPATIVEL`: the 7.48 runtime has no
  `itemicon.bin`, and no active source consumer reads the table. `ReadItemName`
  and `ReadUIString` still run at startup. This is not a native parity claim.
- `strfmt` returns shared static storage.

The other limitations are documented, not fixed by the effect-name loader.

## Validation

The effect-name loader has automated tests for the shipped asset, indexes,
missing rows, overlong tokens, table overflow, and unchanged output after
rejection. A successful build does not establish in-client visual validation.

## Legacy API inventory

The remaining functions were reviewed and grouped by contract:

- Loading/configuration: `BASE_ReadItemPrice`, `BASE_ReadTOTOList`, `BASE_InitializeServerList`, `BASE_GetLanguage`, `ReadItemName`, `ReadUIString`, `ReadNameFiltraDataBase`, `ReadChatFiltraDataBase`, `EnableSysKey`, `DisableSysKey`, `CheckOS`.
- Text/validation: `BASE_UnderBarToSpace`, `BASE_GetHttpRequest`, `BASE_CheckValidString`, `BASE_CheckChatValid`, `CheckGuildName`, `IsClearString`, `IsClearString2`.
- Tables/localization: `BASE_GetWeekNumber`, `BASE_GetVillage`, `BASE_GetSubGuild`, `BASE_GetAttribute`, `BASE_GetAttr`, `BASE_IsInLowZone`, `BASE_GetColorCount`.
- Items/equipment: `BASE_GetItemSanc`, `BASE_GetItemAbility`, `BASE_GetStaticItemAbility`, `BASE_GetBonusItemAbilityNosanc`, `BASE_GetBonusItemAbility`, `BASE_GetItemAbilityNosanc`, `BASE_GetItemAmount`, `BASE_SetItemAmount`, `BASE_CanCarry`, `BASE_CanTrade`, `BASE_ClearItem`, `BASE_SortTradeItem`, `BASE_CanCargo`, `BASE_CanEquip`, `BASE_CanEquip_RecvRes`, `BASE_GetItemColorEffect`, `BASE_GetMeshIndex`.
- Combat: `BASE_GetSpeed`, `BASE_GetManaSpent`, `BASE_GetSkillDamage`, `BASE_GetMobAbility`, `BASE_GetMaxAbility`, `BASE_DefineSkinMeshType`, `BASE_GetMountScale`, `IsSkill`, `GetSkillIndex`, `IsValidSkill`, `IsValidClassSkill`.
- Navigation/geometry: `BASE_GetRoute`, `BASE_GetDistance`, `BASE_GetHitPosition`, `BASE_GetHitPosition2`, `BASE_Get3DTo2DPos`, `BASE_SetBit`, `BASE_UpdateItem2`.
- Effects: `BASE_CanRefine`, `IsPassiveSkill`, `BASE_HasSancAdd`, `BASE_GetSancEffValue`, `BASE_GetItemSancSuccess`, `BASE_GetEffectValue`, `BASE_ChangeOrAddEffectValue`, `BASE_RemoveEffect`.

Historical signatures, borrowed buffers, global tables, and return values were
preserved. Functions without individual comments still require documentation;
they are not reinterpreted here as native 7.48 contracts.
