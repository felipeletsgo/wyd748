// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052b72a | Name: FUN_0052b72a


undefined4 __thiscall FUN_0052b72a(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_118 [4];
  undefined2 local_114;
  undefined2 local_112;
  undefined1 local_10c [116];
  undefined1 local_98 [4];
  undefined2 local_94;
  undefined2 local_92;
  undefined1 local_8c [116];
  int local_18;
  int local_14;
  uint local_10;
  int local_c;
  int local_8;
  
  local_8 = param_2;
  local_c = 0;
  iVar1 = FUN_00494dcf();
  if (iVar1 == 30000) {
    local_c = DAT_0067cf38;
  }
  if ((local_c != 0) && (param_1 == *(int *)(DAT_0067cf38 + 0x4c))) {
    local_10 = (**(code **)(*DAT_0092e654 + 8))();
    for (local_14 = 0; local_14 < 0x10; local_14 = local_14 + 1) {
      local_18 = (*(int *)(local_8 + 0x10 + local_14 * 8) * 8 + -4) -
                 (local_10 - *(int *)(local_c + 0x27480 + local_14 * 4)) / 1000;
      if (((int)(local_18 + (local_18 >> 0x1f & 7U)) >> 3 != *(int *)(local_8 + 0x10 + local_14 * 8)
          ) || (*(char *)(param_1 + 0x6a4 + local_14 * 8) != *(char *)(local_8 + 0xc + local_14 * 8)
               )) {
        FUN_0058f220(param_1 + 0x6a4 + local_14 * 8,local_8 + 0xc + local_14 * 8,8);
        uVar2 = (**(code **)(*DAT_0092e654 + 8))();
        *(undefined4 *)(local_c + 0x27480 + local_14 * 4) = uVar2;
      }
    }
    if ((*(int *)(local_c + 0x276a4) == 0) && (*(int *)(local_c + 0x276a8) == 0)) {
      *(uint *)(local_c + 0x276ac) = local_10;
      _memset(local_98,0,0x80);
      local_92 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
      local_94 = 0x334;
      FUN_0058f078(local_8c,&PTR_DAT_005bb804);
      FUN_0055f2dd(local_98,0x80);
    }
    else if ((*(int *)(local_c + 0x276ac) != 0) &&
            (*(int *)(local_c + 0x276ac) + 3600000U < local_10)) {
      *(uint *)(local_c + 0x276ac) = local_10;
      _memset(local_118,0,0x80);
      local_112 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
      local_114 = 0x334;
      FUN_0058f078(local_10c,&PTR_DAT_005bb808);
      FUN_0055f2dd(local_118,0x80);
    }
  }
  return 1;
}

