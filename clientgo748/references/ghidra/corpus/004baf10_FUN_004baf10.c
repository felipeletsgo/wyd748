// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004baf10 | Name: FUN_004baf10


undefined4 * __fastcall FUN_004baf10(undefined4 *param_1)

{
  undefined4 uVar1;
  
  CSimpleArray<>();
  *param_1 = &PTR_FUN_005a46a8;
  param_1[0xc6] = 0;
  param_1[199] = 0;
  param_1[2] = 0;
  param_1[0xd7] = 1;
  param_1[3] = 0;
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[0xca] = 0;
  param_1[200] = 0;
  param_1[0xcc] = 0x3f800000;
  param_1[0xcb] = 0x3f800000;
  param_1[0xd0] = 0x3f800000;
  param_1[0xd1] = 0x800000;
  param_1[0xd2] = 0x7f7fffff;
  param_1[0xd3] = 0x800000;
  param_1[0xd4] = 0x7f7fffff;
  param_1[0xd5] = 0x800000;
  param_1[0xd6] = 0x7f7fffff;
  param_1[1] = 0;
  uVar1 = FUN_00430f50(0,0,0);
  FUN_004310a0(uVar1);
  _memset(param_1 + 6,0,0x280);
  _memset(param_1 + 0xa6,0,0x80);
  return param_1;
}

