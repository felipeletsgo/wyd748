// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0045061f | Name: FUN_0045061f


void __thiscall FUN_0045061f(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_50;
  short local_48;
  undefined1 local_3c [4];
  undefined2 local_38;
  undefined2 local_36;
  undefined4 local_30;
  int local_2c;
  undefined2 local_20;
  undefined2 local_1e;
  undefined2 local_1c;
  int local_18;
  undefined4 local_14;
  int local_10;
  int *local_c;
  int local_8;
  
  local_c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x151);
  local_8 = (**(code **)(*local_c + 0xb4))(param_2,param_3);
  if (((local_8 != 0) &&
      (local_10 = FUN_0054cd07(*(undefined4 *)(local_8 + 0x670),0x26), local_10 == 0xe)) &&
     ((local_18 = (**(code **)(*DAT_0092e654 + 8))(), *(int *)(param_1 + 0x27478) == 0 ||
      (199 < (uint)(local_18 - *(int *)(param_1 + 0x27478)))))) {
    (**(code **)(*local_c + 0xc0))
              (*(undefined4 *)(*(int *)(local_8 + 0x1e4) + 0x1e4),
               *(undefined4 *)(*(int *)(local_8 + 0x1e4) + 0x400));
    local_48 = (**(code **)(*local_c + 0xc4))(*(undefined4 *)(*(int *)(local_8 + 0x1e4) + 0x1e4));
    if (local_48 == -1) {
      local_48 = (short)*(undefined4 *)(local_8 + 0x1d0) +
                 (short)*(undefined4 *)(local_8 + 0x1d4) * 9;
    }
    _memset(local_3c,0,0x24);
    local_36 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_38 = 0x373;
    local_30 = 1;
    local_2c = (int)local_48;
    local_1c = 0;
    local_20 = __ftol();
    local_1e = __ftol();
    FUN_0055f2dd(local_3c,0x24);
    *(int *)(param_1 + 0x27478) = local_18;
    *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
    local_50 = FUN_0054e93b(*(undefined4 *)(local_8 + 0x670));
    if ((0x919 < **(short **)(local_8 + 0x670)) && (**(short **)(local_8 + 0x670) < 0x956)) {
      local_50 = 0;
    }
    if (local_50 < 2) {
      puVar1 = (undefined4 *)(**(code **)(*local_c + 0xa4))(param_2,param_3);
      if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
         (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar1)) {
        *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
      }
      if ((puVar1 != (undefined4 *)0x0) && (puVar1 != (undefined4 *)0x0)) {
        (**(code **)*puVar1)(1);
      }
    }
    else {
      FUN_0054e9ab(*(undefined4 *)(local_8 + 0x670),local_50 + -1);
      iVar2 = local_8;
      FUN_0058f078(local_8 + 0x39a,&PTR_DAT_005b7d74,local_50 + -1);
      FUN_00421b8f(iVar2 + 0x39a,*(undefined4 *)(iVar2 + 900),0);
    }
    local_14 = 0x2f;
    if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x2f), iVar2 != 0)) {
      uVar4 = 0;
      uVar3 = 0;
      FUN_00429a6d(local_14);
      FUN_0042ad2b(uVar3,uVar4);
    }
    FUN_004431e4(0);
    if (local_50 < 2) {
      _memset((void *)(DAT_013b71e8 + 0x7c8 + local_48 * 8),0,8);
    }
  }
  return;
}

