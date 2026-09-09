// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005947fc | Name: FUN_005947fc


void FUN_005947fc(undefined4 param_1,int param_2,undefined4 param_3,int *param_4)

{
  do {
    if (param_2 < 1) {
      return;
    }
    param_2 = param_2 + -1;
    FUN_005947c7(param_1,param_3,param_4);
  } while (*param_4 != -1);
  return;
}

