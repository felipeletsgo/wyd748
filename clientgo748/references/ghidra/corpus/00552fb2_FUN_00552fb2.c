// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552fb2 | Name: FUN_00552fb2


void FUN_00552fb2(int param_1,int *param_2)

{
  *param_2 = (param_1 >> 10 & 0x1fU) - 1;
  return;
}

