// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057f592 | Name: FUN_0057f592


void FUN_0057f592(ulonglong *param_1)

{
  ulonglong uVar1;
  
  uVar1 = (ulonglong)DAT_005a8690;
  *param_1 = uVar1;
  param_1[1] = 0;
  param_1[3] = 0;
  param_1[4] = 0;
  param_1[5] = uVar1;
  param_1[6] = 0;
  param_1[2] = uVar1 << 0x20;
  param_1[7] = uVar1 << 0x20;
  FastExitMediaState();
  return;
}

