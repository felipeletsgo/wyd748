// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004eef50 | Name: FUN_004eef50


undefined4 * __fastcall FUN_004eef50(undefined4 *param_1)

{
  undefined4 *puVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a15c9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_00430f10();
  FUN_00421910(param_1 + 0x3c,0x1c,4,FUN_004cb8e0);
  *param_1 = &PTR_FUN_005a52a4;
  param_1[0x59] = 1;
  param_1[0x5a] = 0x3f800000;
  puVar1 = (undefined4 *)FUN_00430f50(0xbf800000,0x3f333333,0x3e99999a);
  param_1[0x39] = *puVar1;
  param_1[0x3a] = puVar1[1];
  param_1[0x3b] = puVar1[2];
  _memset(param_1 + 9,0,0xc0);
  _memset(param_1 + 0x3c,0,0x70);
  ExceptionList = local_10;
  return param_1;
}

