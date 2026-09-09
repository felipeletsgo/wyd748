// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059482d | Name: FUN_0059482d


void FUN_0059482d(char *param_1,int param_2,undefined4 param_3,int *param_4)

{
  char cVar1;
  
  do {
    if (param_2 < 1) {
      return;
    }
    param_2 = param_2 + -1;
    cVar1 = *param_1;
    param_1 = param_1 + 1;
    FUN_005947c7((int)cVar1,param_3,param_4);
  } while (*param_4 != -1);
  return;
}

