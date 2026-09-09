// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cdad4 | Name: FUN_004cdad4


void __thiscall FUN_004cdad4(int param_1,float param_2,float param_3)

{
  *(float *)(param_1 + 0x120) = param_2 * DAT_005b490c;
  *(float *)(param_1 + 0x124) = param_3 * DAT_005b4910;
  return;
}

