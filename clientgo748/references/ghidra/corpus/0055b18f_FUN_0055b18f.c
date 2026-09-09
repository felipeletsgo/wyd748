// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b18f | Name: FUN_0055b18f


undefined4 * __fastcall FUN_0055b18f(undefined4 *param_1)

{
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[4] = 800;
  param_1[5] = 600;
  param_1[0x38] = 0;
  param_1[0x39] = 0;
  param_1[0x3a] = 0;
  param_1[0x3b] = 0;
  param_1[0x3c] = 0;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[0x3f] = 0;
  *param_1 = &PTR_FUN_005a6104;
  DAT_013b71e0 = param_1;
  FUN_0054f25a();
  FUN_005551af();
  FUN_0054f5b5();
  FUN_004310e0(s_WYD_log_005c4a8c);
  FUN_0058f078(param_1 + 8,s_With_Your_Destiny_005c4a94);
  return param_1;
}

