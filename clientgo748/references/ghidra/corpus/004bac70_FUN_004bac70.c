// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bac70 | Name: FUN_004bac70


undefined4 * __fastcall FUN_004bac70(undefined4 *param_1)

{
  BOOL BVar1;
  LARGE_INTEGER local_c;
  
  *param_1 = &PTR_FUN_005a4688;
  BVar1 = QueryPerformanceFrequency(&local_c);
  param_1[10] = BVar1;
  if (param_1[10] != 0) {
    param_1[0x12] = local_c.s.LowPart;
    param_1[0x13] = local_c.s.HighPart;
  }
  param_1[10] = 0;
  param_1[3] = 0;
  param_1[0x14] = 0;
  param_1[0x18] = 0;
  param_1[0x1a] = 0;
  DAT_0092e654 = param_1;
  return param_1;
}

