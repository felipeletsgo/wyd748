// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004013c2 | Name: FUN_004013c2


void __fastcall FUN_004013c2(int param_1)

{
  *(float *)(param_1 + 0x50) = (float)*(uint *)(DAT_005ccf9c + 0x2a508) - *(float *)(param_1 + 0x58)
  ;
  return;
}

