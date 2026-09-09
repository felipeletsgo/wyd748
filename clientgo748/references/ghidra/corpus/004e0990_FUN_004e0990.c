// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e0990 | Name: FUN_004e0990


undefined4 * __thiscall FUN_004e0990(undefined4 *param_1)

{
  undefined4 uVar1;
  undefined4 in_stack_00000010;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0d89;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4d44;
  FUN_004310a0(&stack0x00000004);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x20] = uVar1;
  param_1[0x21] = 700;
  param_1[0x23] = 0;
  param_1[0x22] = in_stack_00000010;
  ExceptionList = local_10;
  return param_1;
}

