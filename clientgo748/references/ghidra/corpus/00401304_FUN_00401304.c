// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401304 | Name: FUN_00401304


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00401304(int param_1)

{
  *(float *)(param_1 + 0x4c) =
       (*(float *)(param_1 + 0x4c) / _DAT_005a34a8) * (float)*(int *)(DAT_005ccf9c + 0x2a504);
  *(float *)(param_1 + 0x50) =
       (*(float *)(param_1 + 0x50) / _DAT_005a34a4) * (float)*(int *)(DAT_005ccf9c + 0x2a508);
  return;
}

