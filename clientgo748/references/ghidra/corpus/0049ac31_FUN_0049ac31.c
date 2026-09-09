// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ac31 | Name: FUN_0049ac31


undefined4 __thiscall FUN_0049ac31(int param_1,undefined4 param_2)

{
  int iVar1;
  
  if (DAT_0067cf38 == param_1) {
    if ((*(int *)(param_1 + 0x28) != 0) &&
       (iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0x10))(param_2), iVar1 == 1)) {
      return 1;
    }
    FUN_0054ab57(param_2);
  }
  return 0;
}

