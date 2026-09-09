// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e3ba0 | Name: FUN_004e3ba0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004e3ba0(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  void *pvVar4;
  undefined4 in_stack_00000010;
  undefined4 in_stack_00000014;
  undefined4 in_stack_00000018;
  undefined4 uVar5;
  int *local_2c;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0fe4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a4eb0;
  param_1[0x27] = in_stack_00000010;
  param_1[0x28] = in_stack_00000014;
  param_1[0x29] = in_stack_00000018;
  FUN_004310a0(&stack0x00000004);
  uVar1 = FUN_00430f20(0,0);
  FUN_0040bd30(uVar1);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 2000;
  param_1[0x22] = 0;
  param_1[0x26] = 0;
  param_1[0x25] = 0;
  param_1[0x2b] = 0;
  param_1[0x2a] = 0x2c2;
  if (param_1[0x27] == 1) {
    param_1[0x2a] = 0x2c3;
  }
  if (param_1[0x28] == 1) {
    param_1[0x2a] = 0xb02;
    param_1[0x21] = 2000;
    iVar2 = (**(code **)(*DAT_0092e654 + 8))();
    iVar3 = _rand();
    param_1[0x20] = iVar2 + iVar3 % 500;
    param_1[0x2b] = 1;
  }
  else if (((int)param_1[0x27] < 2) || (8 < (int)param_1[0x27])) {
    pvVar4 = operator_new(0x124);
    local_8._0_1_ = 1;
    if (pvVar4 == (void *)0x0) {
      local_2c = (int *)0x0;
    }
    else {
      local_2c = (int *)FUN_004cb900(2,2000,0x40000000,0x40000000,0x40000000,0x3b03126f,0);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (local_2c != (int *)0x0) {
      local_2c[0x1b] = 1;
      FUN_004310a0(param_1 + 0x1d);
      local_2c[0x42] = 1;
      local_2c[0x1e] = (int)((float)local_2c[0x1e] + _DAT_005a40ec);
      (**(code **)(*local_2c + 0x3c))(0xff2255aa);
      FUN_0054ac09(local_2c);
    }
  }
  else {
    param_1[0x2a] = param_1[0x27] + 0x2c6;
    param_1[0x21] = 0x5dc;
  }
  if ((param_1[0x28] != 1) && (DAT_005ccf98 != 0)) {
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

