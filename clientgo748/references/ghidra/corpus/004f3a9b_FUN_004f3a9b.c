// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f3a9b | Name: FUN_004f3a9b


undefined4 * __fastcall FUN_004f3a9b(undefined4 *param_1)

{
  FUN_0053e1d2();
  *param_1 = &PTR_FUN_005a54f8;
  param_1[0x70] = 0;
  param_1[0x6b] = 0;
  _memset(param_1 + 0x58,0,8);
  _memset(param_1 + 0x73,0,0x70);
  param_1[0x8f] = 0;
  param_1[0x90] = 0;
  param_1[0x91] = 0;
  param_1[0x92] = 0xffffffff;
  _memset(param_1 + 0x93,0,0x80);
  param_1[0xb3] = 0;
  param_1[0xb4] = 0;
  param_1[0xb7] = 0;
  DAT_0092e7c8 = DAT_0092e7c8 + 1;
  return param_1;
}

