// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ecd90 | Name: FUN_004ecd90


undefined4 * __thiscall FUN_004ecd90(undefined4 *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int in_stack_0000001c;
  undefined4 in_stack_00000020;
  undefined4 uVar3;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a1469;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  CSimpleArray<>();
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a5174;
  param_1[0x29] = in_stack_0000001c;
  uVar1 = FUN_004310a0(&stack0x00000004);
  FUN_004310a0(uVar1);
  FUN_004310a0(&stack0x00000010);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x26] = uVar1;
  param_1[0x27] = 2000;
  param_1[0x28] = 0;
  param_1[7] = in_stack_00000020;
  if ((in_stack_0000001c == 0) && (DAT_005ccf98 != 0)) {
    iVar2 = FUN_00429a6d(0xa7);
    if (iVar2 != 0) {
      uVar3 = 0;
      uVar1 = 0;
      FUN_00429a6d(0xa7);
      FUN_0042ad2b(uVar1,uVar3);
    }
  }
  ExceptionList = local_10;
  return param_1;
}

