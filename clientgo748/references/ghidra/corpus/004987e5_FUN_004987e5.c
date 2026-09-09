// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004987e5 | Name: FUN_004987e5


undefined4 FUN_004987e5(char *param_1)

{
  bool bVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  int local_10;
  undefined4 local_8;
  
  local_8 = 0;
  bVar1 = false;
  iVar2 = _strcmp(param_1,&DAT_005b8fd8);
  if (iVar2 == 0) {
    local_8 = 0;
  }
  else {
    uVar3 = extraout_var;
    for (local_10 = 0; local_10 < 0x800; local_10 = local_10 + 1) {
      iVar2 = _strcmp(param_1,(char *)(DAT_013b71e8 + 0xf48 + local_10 * 0x34));
      if (iVar2 == 0) {
        local_8 = *(undefined4 *)(DAT_013b71e8 + 0xf78 + local_10 * 0x34);
        bVar1 = true;
        break;
      }
    }
    if (!bVar1) {
      FUN_00431159(s_Cannot_Match_Resource_ID___s__in_005b8ff0,param_1,s_UI_TMResource_h_005b8fe0,
                   uVar3);
    }
  }
  return local_8;
}

