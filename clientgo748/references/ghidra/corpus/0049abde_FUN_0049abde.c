// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049abde | Name: FUN_0049abde


undefined4 __thiscall FUN_0049abde(int param_1,undefined4 param_2)

{
  int iVar1;
  
  if (DAT_0067cf38 == param_1) {
    if ((*(int *)(param_1 + 0x28) != 0) &&
       (iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0xc))(param_2), iVar1 == 1)) {
      return 1;
    }
    FUN_0054ab48(param_2);
  }
  return 0;
}

