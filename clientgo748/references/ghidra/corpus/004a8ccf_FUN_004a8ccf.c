// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a8ccf | Name: FUN_004a8ccf


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_004a8ccf(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a02e9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00493e70();
  local_8 = 0;
  FUN_00421910(param_1 + 0x9ee3,8,0x32,FUN_00430f10);
  *param_1 = &PTR_FUN_005a4544;
  param_1[9] = 0x7534;
  param_1[0x9b87] = 0xffffffff;
  param_1[0x9b9b] = 0;
  param_1[0x8027] = 0;
  param_1[0x9f4a] = 0;
  param_1[0x9f4b] = 0;
  *(undefined1 *)(param_1 + 0x9bc2) = 0;
  param_1[0x9f4c] = 0;
  param_1[0x9b9d] = 0;
  param_1[0x9bbe] = 0;
  param_1[0x9b9d] = 1;
  if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
    FUN_00429480();
  }
  _memset(param_1 + 0x25,0,0x10000);
  if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
    FUN_00428ba0(0);
  }
  DAT_005b2980 = 0;
  _DAT_005d04dc = 0;
  ExceptionList = local_10;
  return param_1;
}

