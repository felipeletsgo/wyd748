// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004219a0 | Name: FUN_004219a0


undefined4 * __fastcall FUN_004219a0(undefined4 *param_1)

{
  param_1[1] = 0xff000000;
  param_1[2] = 0xffffffff;
  param_1[0x67] = 0;
  *param_1 = &PTR_FUN_005a4100;
  param_1[4] = 0x3f800000;
  param_1[3] = 0;
  _memset(param_1 + 6,0,0x100);
  _memset(param_1 + 0x46,0,0x81);
  _memset(param_1 + 0x6a,0,6);
  param_1[0x68] = 0xffffffff;
  param_1[0x69] = 0xffffffff;
  return param_1;
}

