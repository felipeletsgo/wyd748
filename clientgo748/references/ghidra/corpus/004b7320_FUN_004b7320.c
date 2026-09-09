// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7320 | Name: FUN_004b7320


int FUN_004b7320(int param_1,int param_2,int param_3)

{
  for (; param_1 != param_2; param_1 = param_1 + 0x28) {
    FUN_004b7410(param_3,param_1);
    param_3 = param_3 + 0x28;
  }
  return param_3;
}

