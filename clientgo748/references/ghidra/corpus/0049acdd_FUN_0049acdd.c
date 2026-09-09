// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049acdd | Name: FUN_0049acdd


undefined4 __fastcall FUN_0049acdd(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x28) != 0) &&
     (iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0x38))(), iVar1 == 1)) {
    return 1;
  }
  return 0;
}

