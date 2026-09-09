// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7360 | Name: FUN_004b7360


void FUN_004b7360(int param_1,int param_2,undefined4 param_3)

{
  for (; param_2 != 0; param_2 = param_2 + -1) {
    FUN_004b7410(param_1,param_3);
    param_1 = param_1 + 0x28;
  }
  return;
}

