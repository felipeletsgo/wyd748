// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d3d20 | Name: FUN_004d3d20


undefined4 * __thiscall FUN_004d3d20(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  void *pvVar3;
  undefined4 in_stack_00000010;
  undefined4 in_stack_00000014;
  int in_stack_00000018;
  undefined4 in_stack_0000001c;
  undefined4 local_2c;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a09df;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a49e4;
  param_1[0x2c] = in_stack_00000010;
  param_1[0x2b] = 0x3f800000;
  param_1[0x2e] = 0x3f800000;
  if (param_1[0x2c] == 1) {
    param_1[0x25] = 2000;
  }
  else {
    param_1[0x25] = 1000;
  }
  param_1[0x28] = 0;
  uVar1 = FUN_004310a0(&stack0x00000004);
  FUN_004310a0(uVar1);
  iVar2 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x24] = iVar2 + ((uint)param_1 % 10) * 100;
  if (param_1[0x2c] == 2) {
    uVar1 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x24] = uVar1;
  }
  param_1[7] = in_stack_00000014;
  param_1[0x27] = in_stack_0000001c;
  param_1[0x2d] = 0xffffffff;
  param_1[0x20] = 0;
  param_1[0x26] = 0;
  if (in_stack_00000018 == 1) {
    if (param_1[0x2c] == 0) {
      pvVar3 = operator_new(0x180);
      local_8._0_1_ = 1;
      if (pvVar3 == (void *)0x0) {
        local_28 = 0;
      }
      else {
        local_28 = FUN_004c8f70(0xb,0,0x3f4ccccd,0x3f800000,0x3f4ccccd,0,8,0x50);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x20] = local_28;
      if (param_1[0x20] != 0) {
        (**(code **)(*(int *)param_1[0x20] + 0x3c))(0xff440000);
        *(undefined4 *)(param_1[0x20] + 0x6c) = 1;
        *(undefined4 *)(param_1[0x20] + 0x104) = 0;
        FUN_004310a0(param_1 + 0x21);
        FUN_0054ac09(param_1[0x20]);
      }
    }
    else if (param_1[0x2c] == 1) {
      pvVar3 = operator_new(0x180);
      local_8._0_1_ = 2;
      if (pvVar3 == (void *)0x0) {
        local_2c = 0;
      }
      else {
        local_2c = FUN_004c8f70(0x38,0,0x3fc00000,0x3fc00000,0x3fc00000,0,1,0x50);
      }
      local_8 = (uint)local_8._1_3_ << 8;
      param_1[0x20] = local_2c;
      if (param_1[0x20] != 0) {
        (**(code **)(*(int *)param_1[0x20] + 0x3c))(0xff55aaff);
        *(undefined4 *)(param_1[0x20] + 0x6c) = 1;
        *(undefined4 *)(param_1[0x20] + 0x104) = 2;
        FUN_004310a0(param_1 + 0x21);
        FUN_0054ac09(param_1[0x20]);
      }
    }
  }
  param_1[0x2a] = 0;
  FUN_004d40f3(0xffffffff);
  ExceptionList = local_10;
  return param_1;
}

