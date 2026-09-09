// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059a886 | Name: FUN_0059a886


undefined4 FUN_0059a886(int *param_1)

{
  int iVar1;
  
  iVar1 = 0;
  do {
    if (*param_1 != 0) {
      return 0;
    }
    iVar1 = iVar1 + 1;
    param_1 = param_1 + 1;
  } while (iVar1 < 3);
  return 1;
}

