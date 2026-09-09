// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7530 | Name: FUN_004b7530


int FUN_004b7530(int param_1,int param_2,int param_3)

{
  while (param_1 != param_2) {
    param_2 = param_2 + -0x28;
    param_3 = param_3 + -0x28;
    FUN_004b7640(param_2);
  }
  return param_3;
}

