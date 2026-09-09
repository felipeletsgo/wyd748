// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e5d10 | Name: FUN_004e5d10


undefined4 * __thiscall FUN_004e5d10(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  float10 fVar4;
  undefined4 in_stack_0000001c;
  undefined4 in_stack_00000020;
  undefined4 uVar5;
  undefined4 local_30;
  undefined1 local_20 [12];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1104;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a4ff8;
  param_1[0x28] = 0;
  param_1[0x29] = 0;
  uVar1 = FUN_004310a0(&stack0x00000004);
  FUN_004310a0(uVar1);
  FUN_004310a0(&stack0x00000010);
  param_1[0x27] = in_stack_0000001c;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  if (DAT_005ccf98 != 0) {
    iVar2 = FUN_00429a6d(0x9a);
    if (iVar2 != 0) {
      uVar5 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x9a);
      FUN_0042ad2b(uVar1,uVar5);
    }
  }
  FUN_00493db0(local_20,&stack0x00000004);
  fVar4 = (float10)FUN_004b3aa0();
  local_14 = (float)fVar4;
  iVar2 = __ftol();
  param_1[0x2a] = iVar2 * 100;
  if (param_1[0x2a] == 0) {
    param_1[0x2a] = 1;
  }
  if (5000 < (uint)param_1[0x2a]) {
    param_1[0x2a] = 5000;
  }
  pvVar3 = operator_new(0xc0);
  local_8._0_1_ = 1;
  if (pvVar3 == (void *)0x0) {
    local_30 = 0;
  }
  else {
    local_30 = FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[0x2b] = local_30;
  if (param_1[0x2b] != 0) {
    (**(code **)(*(int *)param_1[0x2b] + 0x44))(0x33333344);
    *(undefined4 *)(param_1[0x2b] + 0x6c) = 1;
  }
  param_1[7] = in_stack_00000020;
  if (DAT_005ccf98 != 0) {
    iVar2 = FUN_00429a6d(0xa0);
    if (iVar2 != 0) {
      uVar5 = 0;
      uVar1 = 0;
      FUN_00429a6d(0xa0);
      FUN_0042ad2b(uVar1,uVar5);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

