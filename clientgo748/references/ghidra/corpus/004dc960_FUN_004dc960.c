// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004dc960 | Name: FUN_004dc960


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004dc960(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  int in_stack_00000010;
  undefined4 in_stack_00000014;
  undefined4 uVar4;
  undefined1 local_34 [24];
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0cc9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4be8;
  if (in_stack_00000010 != 0) {
    param_1[7] = 0;
    param_1[0x14] = 0;
    param_1[0x16] = 0x3f800000;
    FUN_004310a0(&stack0x00000004);
    uVar1 = FUN_00430f50(*(undefined4 *)(in_stack_00000010 + 0x28),
                         *(float *)(in_stack_00000010 + 0x30) + _DAT_005a3660,
                         *(undefined4 *)(in_stack_00000010 + 0x2c));
    FUN_004310a0(uVar1);
    uVar1 = FUN_00493db0(local_34,param_1 + 9);
    FUN_004310a0(uVar1);
    fVar3 = (float10)FUN_004b3aa0();
    param_1[0x16] = (float)fVar3;
    param_1[0x15] = in_stack_00000014;
    FUN_00430f50(param_1[0xf],param_1[0x10],param_1[0x11]);
    thunk_FUN_005611dd(&local_1c,&local_1c);
    param_1[0xf] = local_1c;
    param_1[0x10] = local_18;
    param_1[0x11] = local_14;
    uVar1 = __ftol();
    param_1[0x13] = uVar1;
    uVar1 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x12] = uVar1;
    if ((uint)param_1[0x13] < 0x9c5) {
      if (param_1[0x13] == 0) {
        param_1[0x13] = 1;
      }
    }
    else {
      param_1[0x13] = 0x9c4;
    }
    if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x4d), iVar2 != 0)) {
      FUN_00429a6d(0x4d);
      iVar2 = FUN_0042afd0();
      if (iVar2 == 0) {
        uVar4 = 0;
        uVar1 = 0;
        FUN_00429a6d(0x4d);
        FUN_0042ad2b(uVar1,uVar4);
      }
    }
  }
  ExceptionList = local_10;
  return param_1;
}

