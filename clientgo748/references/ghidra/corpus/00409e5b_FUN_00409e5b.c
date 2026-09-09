// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409e5b | Name: FUN_00409e5b


void __thiscall FUN_00409e5b(int param_1,int param_2,int param_3)

{
  *(float *)(param_1 + 0x4c) = (float)param_2;
  *(float *)(param_1 + 0x50) = (float)param_3;
  return;
}

