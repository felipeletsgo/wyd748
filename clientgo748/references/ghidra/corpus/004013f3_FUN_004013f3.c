// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004013f3 | Name: FUN_004013f3


void __fastcall FUN_004013f3(int param_1)

{
  *(float *)(param_1 + 0x4c) =
       (float)(*(int *)(DAT_005ccf9c + 0x2a504) - 800U >> 1) + *(float *)(param_1 + 0x4c);
  *(float *)(param_1 + 0x50) =
       (float)(*(int *)(DAT_005ccf9c + 0x2a508) - 600U >> 1) + *(float *)(param_1 + 0x50);
  return;
}

