// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ad10 | Name: FUN_0049ad10


undefined4 __thiscall FUN_0049ad10(int param_1,undefined4 param_2)

{
  int iVar1;
  
  if (((DAT_0067cf38 == param_1) && (*(int *)(param_1 + 0x28) != 0)) &&
     (iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0x3c))(param_2), iVar1 == 1)) {
    return 1;
  }
  return 0;
}

