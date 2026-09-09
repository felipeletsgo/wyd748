// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00531970 | Name: FUN_00531970


undefined4 * __thiscall FUN_00531970(undefined4 *param_1,undefined4 param_2)

{
  FUN_0053e1d2();
  *param_1 = &PTR_FUN_005a57f4;
  param_1[9] = param_2;
  param_1[0x18] = 0;
  param_1[0x17] = 0x3e;
  param_1[100] = 0;
  param_1[0x66] = 0;
  param_1[0x65] = 0;
  _memset(param_1 + 0x58,0,0x20);
  _memset(param_1 + 0x60,0,0x10);
  return param_1;
}

