// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573e81 | Name: FUN_00573e81


void FUN_00573e81(int param_1)

{
  *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) | 1;
  return;
}

