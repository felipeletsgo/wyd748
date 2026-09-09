// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d2230 | Name: FUN_004d2230


undefined4 * __thiscall
FUN_004d2230(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  undefined4 uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a09a9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a49a8;
  param_1[0x20] = 0;
  param_1[0x21] = param_2;
  param_1[0x23] = param_4;
  param_1[0x24] = 0;
  param_1[0x25] = 0x3fc90fdb;
  param_1[0x28] = param_5;
  param_1[0x26] = 0x3f800000;
  param_1[0x27] = 0x3f800000;
  param_1[0x2a] = 0;
  param_1[0x29] = 0xffffffff;
  *(undefined1 *)(param_1 + 0x2b) = 0;
  *(undefined1 *)((int)param_1 + 0xad) = 0;
  param_1[0x1b] = 1;
  param_1[0x2c] = 0;
  param_1[0x2e] = 1000;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x2d] = uVar1;
  FUN_004d2da9(param_3);
  ExceptionList = local_10;
  return param_1;
}

