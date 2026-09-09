// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ee30 | Name: FUN_0049ee30


undefined4 * __fastcall FUN_0049ee30(undefined4 *param_1)

{
  undefined4 uVar1;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0219;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00493e70();
  local_8 = 0;
  FUN_00430f10();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a44b4;
  DAT_005b48e8 = 0x428c0000;
  param_1[0x9b9d] = 0;
  param_1[0x9b9e] = 0;
  param_1[0x9b9f] = 0;
  param_1[0x9ba0] = 0;
  param_1[0x9b9c] = 0;
  param_1[0x9b87] = 0;
  param_1[0x9b94] = 0;
  param_1[0x9b95] = 0;
  param_1[0x9b96] = 0;
  param_1[0x9b97] = 0;
  param_1[0x9b9a] = 0;
  param_1[0x9b9b] = 0;
  param_1[0x9ba4] = 0;
  param_1[0x9ba3] = 0;
  param_1[0x14] = 0;
  param_1[0x9ba1] = 0;
  param_1[0x9ba2] = 0;
  param_1[0x9bbe] = 0x3f800000;
  param_1[0x9b88] = 0;
  uVar1 = FUN_00430f20(0x45004000,0x45004000);
  FUN_0040bd30(uVar1);
  uVar1 = FUN_00430f20(0x45020000,0x45020000);
  FUN_0040bd30(uVar1);
  *(undefined1 *)(DAT_013b71e8 + 0xef4) = 0xff;
  *(undefined2 *)(DAT_013b71e8 + 0xf10) = 0xffff;
  *(undefined2 *)(DAT_013b71e8 + 0xf12) = 0xffff;
  param_1[0x9b89] = 0;
  for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
    param_1[local_14 + 0x9b8c] = 0;
  }
  param_1[9] = 0x7531;
  param_1[0x9bbd] = 1;
  ExceptionList = local_10;
  return param_1;
}

