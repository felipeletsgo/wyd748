// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044f46b | Name: FUN_0044f46b


void __fastcall FUN_0044f46b(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  short local_50;
  undefined1 local_44 [4];
  undefined2 local_40;
  undefined2 local_3e;
  undefined4 local_38;
  int local_34;
  undefined2 local_28;
  undefined2 local_26;
  undefined2 local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int *local_8;
  
  local_8 = *(int **)(param_1 + 0x2873c);
  local_18 = 0;
  local_14 = 0;
  local_c = 0;
  for (local_10 = 8; -1 < local_10; local_10 = local_10 + -1) {
    for (local_14 = 6; -1 < local_14; local_14 = local_14 + -1) {
      local_18 = (**(code **)(*local_8 + 0xb4))(local_10,local_14);
      if ((local_18 != 0) &&
         ((((**(short **)(local_18 + 0x670) == 0xcfb ||
            ((0x194 < **(short **)(local_18 + 0x670) && (**(short **)(local_18 + 0x670) < 0x19a))))
           || ((0x1af < **(short **)(local_18 + 0x670) && (**(short **)(local_18 + 0x670) < 0x1b4)))
           ) || ((0x2ad < **(short **)(local_18 + 0x670) && (**(short **)(local_18 + 0x670) < 0x2b4)
                 ))))) {
        local_c = 1;
        break;
      }
    }
    if (local_c == 1) break;
  }
  if (local_c != 1) {
    return;
  }
  if (local_18 == 0) {
    return;
  }
  local_1c = FUN_0054cd07(*(undefined4 *)(local_18 + 0x670),0x26);
  if (local_1c == 1) {
    if (*(char *)(*(int *)(param_1 + 0x4c) + 0x788) == '\x01') {
      return;
    }
    (**(code **)(*local_8 + 0xc0))
              (*(undefined4 *)(*(int *)(local_18 + 0x1e4) + 0x1e4),
               *(undefined4 *)(*(int *)(local_18 + 0x1e4) + 0x400));
    local_50 = (**(code **)(*local_8 + 0xc4))(*(undefined4 *)(*(int *)(local_18 + 0x1e4) + 0x1e4));
    if (local_50 == -1) {
      local_50 = (short)*(undefined4 *)(local_18 + 0x1d0) +
                 (short)*(undefined4 *)(local_18 + 0x1d4) * 9;
    }
    local_20 = (**(code **)(*DAT_0092e654 + 8))();
    if ((*(int *)(param_1 + 0x27478) != 0) && ((uint)(local_20 - *(int *)(param_1 + 0x27478)) < 200)
       ) {
      return;
    }
    _memset(local_44,0,0x24);
    local_3e = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_40 = 0x373;
    local_38 = 1;
    local_34 = (int)local_50;
    local_24 = 0;
    local_28 = __ftol();
    local_26 = __ftol();
    FUN_0055f2dd(local_44,0x24);
    *(int *)(param_1 + 0x27478) = local_20;
    iVar2 = FUN_0054e93b(*(undefined4 *)(local_18 + 0x670));
    if (iVar2 < 2) {
      puVar3 = (undefined4 *)(**(code **)(*local_8 + 0xa4))(local_10,local_14);
      if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == puVar3)) {
        DAT_005ccec0[0x7a] = 0;
      }
      if ((puVar3 != (undefined4 *)0x0) && (puVar3 != (undefined4 *)0x0)) {
        (**(code **)*puVar3)(1);
      }
      (**(code **)(*DAT_005ccec0 + 0x98))();
    }
    else {
      FUN_0054e9ab(*(undefined4 *)(local_18 + 0x670),iVar2 + -1);
      iVar1 = local_18;
      FUN_0058f078(local_18 + 0x39a,&PTR_DAT_005b7d68,iVar2 + -1);
      FUN_00421b8f(iVar1 + 0x39a,*(undefined4 *)(iVar1 + 900),0);
    }
    if (iVar2 < 2) {
      _memset((void *)(DAT_013b71e8 + 0x7c8 + local_50 * 8),0,8);
    }
    if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x29), iVar2 != 0)) {
      uVar5 = 0;
      uVar4 = 0;
      FUN_00429a6d(0x29);
      FUN_0042ad2b(uVar4,uVar5);
    }
  }
  FUN_004431e4(0x10);
  return;
}

