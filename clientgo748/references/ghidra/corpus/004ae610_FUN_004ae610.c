// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ae610 | Name: FUN_004ae610


undefined4 * __fastcall FUN_004ae610(undefined4 *param_1)

{
  int local_8;
  
  param_1[0x296] = 0;
  *param_1 = &PTR_FUN_005a45d0;
  for (local_8 = 0; local_8 < 0x100; local_8 = local_8 + 1) {
    param_1[local_8 + 0xd] = 0;
  }
  param_1[0x112] = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[0x113] = 0;
  param_1[0x10f] = 0;
  _memset(param_1 + 0x115,0,0x200);
  _memset(param_1 + 0x195,0,0x200);
  _memset(param_1 + 0x215,0,0x200);
  param_1[0x295] = 0;
  DAT_005d0504 = param_1;
  param_1[0x10d] = 0;
  param_1[0x110] = 0;
  return param_1;
}

