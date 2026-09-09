// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004afac0 | Name: FUN_004afac0


undefined4 * __fastcall FUN_004afac0(undefined4 *param_1)

{
  int local_10;
  int local_c;
  int local_8;
  
  *param_1 = &PTR_FUN_005a45f0;
  _memset(param_1 + 2,0,0x417c0);
  _memset(param_1 + 0x105f2,0,0x16000);
  _memset(param_1 + 0x15df2,0,0x122a0);
  for (local_8 = 0; local_8 < 6; local_8 = local_8 + 1) {
    for (local_c = 0; local_c < 0x3c; local_c = local_c + 1) {
      for (local_10 = 0; local_10 < 0x38; local_10 = local_10 + 1) {
        *(undefined2 *)(&DAT_005d0508 + local_10 * 2 + local_c * 0x70 + local_8 * 0x1a40) = 0;
      }
    }
  }
  DAT_006703a8 = param_1;
  param_1[1] = 0;
  return param_1;
}

