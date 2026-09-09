// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e7610 | Name: FUN_004e7610


undefined4 * __thiscall FUN_004e7610(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  float10 fVar4;
  undefined4 in_stack_0000001c;
  undefined4 in_stack_00000020;
  undefined4 uVar5;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_2c;
  undefined1 local_28 [12];
  float local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a11ef;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a507c;
  param_1[0x2a] = 0;
  param_1[0x2c] = 0;
  param_1[0x27] = 0;
  param_1[0x28] = 0;
  uVar1 = FUN_004310a0(&stack0x00000004);
  FUN_004310a0(uVar1);
  FUN_004310a0(&stack0x00000010);
  param_1[0x29] = in_stack_0000001c;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x2e] = 0xffaa88ff;
  param_1[0x2f] = 0xff4422aa;
  local_2c = 0x3f333333;
  local_18 = 0x2bd;
  local_14 = 0xa1;
  FUN_00493db0(local_28,&stack0x00000004);
  fVar4 = (float10)FUN_004b3aa0();
  local_1c = (float)fVar4;
  iVar2 = __ftol();
  param_1[0x2d] = iVar2 * 0x32;
  if (param_1[0x29] == 1) {
    local_2c = 0x3f800000;
    local_18 = 8;
    local_14 = 0x9b;
    param_1[0x2e] = 0x44444444;
    param_1[0x2f] = 0xffaa8800;
    uVar1 = __ftol();
    param_1[0x2d] = uVar1;
  }
  else if (param_1[0x29] == 2) {
    local_2c = 0x3f800000;
    local_18 = 8;
    local_14 = 0x9b;
    param_1[0x2e] = 0xff004488;
    param_1[0x2f] = 0xff001122;
    uVar1 = __ftol();
    param_1[0x2d] = uVar1;
  }
  else if (param_1[0x29] == 5) {
    local_2c = 0x3f19999a;
    local_18 = 8;
    local_14 = 0x9b;
    param_1[0x2e] = 0xff226644;
    param_1[0x2f] = 0xff112211;
    uVar1 = __ftol();
    param_1[0x2d] = uVar1;
  }
  else if (param_1[0x29] == 3) {
    local_18 = 0x1f9;
    param_1[0x2d] = 4000;
    param_1[0x2e] = 0xffffffff;
  }
  else if (param_1[0x29] == 4) {
    local_18 = 0x1f8;
    param_1[0x2d] = 4000;
    param_1[0x2e] = 0xffffffff;
  }
  if (((param_1[0x29] != 3) && (param_1[0x29] != 4)) && (param_1[0x29] != 5)) {
    pvVar3 = operator_new(0xc0);
    local_8._0_1_ = 1;
    if (pvVar3 == (void *)0x0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_004df250(4,7,0x3f800000);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    param_1[0x27] = local_44;
    if (param_1[0x27] != 0) {
      (**(code **)(*(int *)param_1[0x27] + 0x44))(param_1[0x2f]);
      *(undefined4 *)(param_1[0x27] + 0x6c) = 1;
    }
    if (DAT_005ccf98 != 0) {
      iVar2 = FUN_00429a6d(local_14);
      if (iVar2 != 0) {
        uVar5 = 0;
        uVar1 = 0;
        FUN_00429a6d(local_14);
        FUN_0042ad2b(uVar1,uVar5);
      }
    }
  }
  if (param_1[0x2d] == 0) {
    param_1[0x2d] = 1;
  }
  if (5000 < (uint)param_1[0x2d]) {
    param_1[0x2d] = 5000;
  }
  pvVar3 = operator_new(0xcc);
  local_8 = CONCAT31(local_8._1_3_,2);
  if (pvVar3 == (void *)0x0) {
    local_48 = 0;
  }
  else {
    local_48 = FUN_004d2230(local_18,param_1[0x2e],param_1[0x2a],1);
  }
  param_1[0x28] = local_48;
  if (param_1[0x28] != 0) {
    *(undefined4 *)(param_1[0x28] + 0x98) = local_2c;
    *(undefined4 *)(param_1[0x28] + 0x9c) = local_2c;
    if ((param_1[0x29] == 3) || (param_1[0x29] == 4)) {
      *(undefined4 *)(param_1[0x28] + 0x6c) = 0;
      *(undefined1 *)(param_1[0x28] + 0xad) = 0;
      *(undefined4 *)(param_1[0x28] + 0x98) = 0x3f800000;
      *(undefined4 *)(param_1[0x28] + 0x9c) = 0x3f800000;
    }
  }
  param_1[7] = in_stack_00000020;
  ExceptionList = local_10;
  return param_1;
}

