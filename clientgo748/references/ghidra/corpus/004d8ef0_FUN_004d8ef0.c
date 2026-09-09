// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d8ef0 | Name: FUN_004d8ef0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004d8ef0(undefined4 *param_1)

{
  undefined4 uVar1;
  void *pvVar2;
  undefined4 in_stack_00000010;
  undefined4 in_stack_00000014;
  int local_24;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0bd4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4b5c;
  param_1[0x21] = 3000;
  param_1[0x22] = 0;
  param_1[0x23] = 0;
  FUN_004310a0(&stack0x00000004);
  param_1[0x24] = in_stack_00000010;
  if (param_1[0x24] == 4) {
    param_1[0x21] = 0;
  }
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[7] = in_stack_00000014;
  if (param_1[0x24] == 0) {
    pvVar2 = operator_new(0x124);
    local_8._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      local_24 = 0;
    }
    else {
      local_24 = FUN_004cb900(1,2000,0x3f000000,0x3f000000,0x3f000000,0x3b03126f,0);
    }
    local_8 = (uint)local_8._1_3_ << 8;
    if (local_24 != 0) {
      *(undefined4 *)(local_24 + 0x6c) = 1;
      FUN_004310a0(&stack0x00000004);
      *(float *)(local_24 + 0x78) = *(float *)(local_24 + 0x78) + _DAT_005a439c;
      FUN_0054ac09(local_24);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

