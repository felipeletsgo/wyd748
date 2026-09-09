// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429530 | Name: FUN_00429530


undefined4 * __fastcall FUN_00429530(undefined4 *param_1)

{
  int local_8;
  
  *param_1 = 0;
  param_1[1] = 0;
  DAT_005ccf98 = param_1;
  param_1[0x8402] = 0;
  param_1[0x8403] = 0;
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    _memset(param_1 + local_8 * 0x42 + 3,0,0x80);
    param_1[local_8 * 0x42 + 2] = 0;
    param_1[local_8 * 0x42 + 0x43] = 1;
  }
  return param_1;
}

