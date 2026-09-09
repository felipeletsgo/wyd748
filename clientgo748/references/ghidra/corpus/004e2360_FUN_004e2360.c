// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e2360 | Name: FUN_004e2360


undefined4 * __thiscall FUN_004e2360(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 in_stack_00000010;
  undefined4 in_stack_00000014;
  undefined4 in_stack_00000018;
  undefined4 in_stack_0000001c;
  undefined4 uVar5;
  int *local_2c;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0eb4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4dfc;
  param_1[0x25] = in_stack_00000010;
  FUN_004310a0(&stack0x00000004);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x22] = 0;
  param_1[0x26] = in_stack_00000014;
  param_1[0x21] = 800;
  param_1[0x23] = in_stack_00000018;
  param_1[0x24] = in_stack_0000001c;
  pvVar2 = operator_new(0xc0);
  local_8._0_1_ = 1;
  if (pvVar2 == (void *)0x0) {
    local_2c = (int *)0x0;
  }
  else {
    local_2c = (int *)FUN_004df250(4,7,0x3f800000);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  if (local_2c != (int *)0x0) {
    if (param_1[0x25] == 0) {
      (**(code **)(*local_2c + 0x44))(0x77775511);
    }
    else {
      (**(code **)(*local_2c + 0x44))(0x1122);
    }
    local_2c[0x1b] = 1;
    puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
    (**(code **)(*local_2c + 0x3c))(*puVar3,puVar3[1]);
    local_2c[0x28] = param_1[0x21] + 1000;
    FUN_0054ac09(local_2c);
  }
  if (DAT_005ccf98 != 0) {
    iVar4 = FUN_00429a6d(0x9b);
    if (iVar4 != 0) {
      uVar5 = 0;
      uVar1 = 0;
      FUN_00429a6d(0x9b);
      FUN_0042ad2b(uVar1,uVar5);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

